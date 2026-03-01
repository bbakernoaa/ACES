#include "aces/aces_stacking_engine.hpp"

#include <Kokkos_Core.hpp>
#include <algorithm>
#include <iostream>

namespace aces {

StackingEngine::StackingEngine(const AcesConfig& config) : m_config(config) { PreCompile(); }

void StackingEngine::PreCompile() {
    m_compiled.clear();
    for (auto const& [species, layers] : m_config.species_layers) {
        CompiledSpecies spec;
        spec.name = species;
        for (auto const& layer : layers) {
            spec.layers.push_back({layer.field_name, layer.operation, layer.scale, layer.hierarchy,
                                   layer.masks, layer.scale_fields, layer.diurnal_cycle,
                                   layer.weekly_cycle});
        }

        // Sort layers by hierarchy once during construction.
        std::sort(spec.layers.begin(), spec.layers.end(),
                  [](const CompiledLayer& a, const CompiledLayer& b) {
                      return a.hierarchy < b.hierarchy;
                  });
        m_compiled.push_back(spec);
    }
}

void StackingEngine::Execute(
    FieldResolver& resolver, int nx, int ny, int nz,
    Kokkos::View<double***, Kokkos::LayoutLeft, Kokkos::DefaultExecutionSpace> default_mask,
    int hour, int day_of_week) {
    // Ensure we have a valid default mask
    if (!default_mask.data()) {
        default_mask = Kokkos::View<double***, Kokkos::LayoutLeft, Kokkos::DefaultExecutionSpace>(
            "default_mask_internal", nx, ny, nz);
        Kokkos::deep_copy(default_mask, 1.0);
    }

    for (auto const& spec : m_compiled) {
        std::string export_name = "total_" + spec.name + "_emissions";
        auto total_view = resolver.ResolveExportDevice(export_name, nx, ny, nz);

        if (total_view.data() == nullptr) {
            continue;
        }

        // Initialize export field to 0.0 before accumulation.
        Kokkos::deep_copy(total_view, 0.0);

        for (auto const& layer : spec.layers) {
            auto field_view = resolver.ResolveImportDevice(layer.field_name, nx, ny, nz);
            if (field_view.data() == nullptr) {
                continue;
            }

            // Resolve additional scale fields
            constexpr int MAX_SCALES = 16;
            Kokkos::Array<
                Kokkos::View<const double***, Kokkos::LayoutLeft, Kokkos::DefaultExecutionSpace>,
                MAX_SCALES>
                scales_arr;
            int num_scales = 0;
            for (const auto& sf_name : layer.scale_fields) {
                if (num_scales >= MAX_SCALES) break;
                auto sf_view = resolver.ResolveImportDevice(sf_name, nx, ny, nz);
                if (sf_view.data() != nullptr) {
                    scales_arr[num_scales++] = sf_view;
                }
            }

            // Resolve geographical masks
            constexpr int MAX_MASKS = 8;
            Kokkos::Array<
                Kokkos::View<const double***, Kokkos::LayoutLeft, Kokkos::DefaultExecutionSpace>,
                MAX_MASKS>
                masks_arr;
            int num_masks = 0;
            for (const auto& m_name : layer.masks) {
                if (num_masks >= MAX_MASKS) break;
                auto m_view = resolver.ResolveImportDevice(m_name, nx, ny, nz);
                if (m_view.data() != nullptr) {
                    masks_arr[num_masks++] = m_view;
                }
            }

            double replace_flag = (layer.operation == "replace") ? 1.0 : 0.0;
            double scale = layer.base_scale;

            // Apply temporal cycles
            if (!layer.diurnal_cycle.empty()) {
                auto it = m_config.temporal_cycles.find(layer.diurnal_cycle);
                if (it != m_config.temporal_cycles.end() && it->second.factors.size() == 24) {
                    scale *= it->second.factors[hour % 24];
                }
            }
            if (!layer.weekly_cycle.empty()) {
                auto it = m_config.temporal_cycles.find(layer.weekly_cycle);
                if (it != m_config.temporal_cycles.end() && it->second.factors.size() == 7) {
                    scale *= it->second.factors[day_of_week % 7];
                }
            }

            // Parallel stacking kernel
            Kokkos::parallel_for(
                "StackingEngine_LayerKernel",
                Kokkos::MDRangePolicy<Kokkos::Rank<3>>({0, 0, 0}, {nx, ny, nz}),
                KOKKOS_LAMBDA(int i, int j, int k) {
                    double combined_scale = scale;
                    for (int s = 0; s < num_scales; ++s) {
                        combined_scale *= scales_arr[s](i, j, k);
                    }

                    double combined_mask = 0.0;
                    if (num_masks > 0) {
                        combined_mask = 1.0;
                        for (int m = 0; m < num_masks; ++m) {
                            combined_mask *= masks_arr[m](i, j, k);
                        }
                    } else {
                        combined_mask = default_mask(i, j, k);
                    }

                    total_view(i, j, k) =
                        total_view(i, j, k) * (1.0 - replace_flag * combined_mask) +
                        field_view(i, j, k) * combined_scale * combined_mask;
                });
        }
    }
    Kokkos::fence();
}

}  // namespace aces
