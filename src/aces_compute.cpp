#include "aces/aces_compute.hpp"
#include "aces/aces_utils.hpp"
#include <Kokkos_Core.hpp>
#include <iostream>

namespace aces {

/**
 * @brief Implementation of the emission computation using Kokkos MDRangePolicy.
 *
 * This function iterates through each species and its defined layers in the configuration,
 * applying the "add" or "replace" operations in a branchless manner.
 */
void ComputeEmissions(
    const AcesConfig& config,
    ESMC_State importState,
    ESMC_State exportState,
    int nx, int ny, int nz
) {
    // Create a 1.0 mask view for layers without an explicit mask.
    // We use a managed view in HostSpace to match our current unmanaged views.
    Kokkos::View<double***, Kokkos::LayoutLeft, Kokkos::HostSpace> default_mask("default_mask", nx, ny, nz);
    Kokkos::deep_copy(default_mask, 1.0);

    for (auto const& [species, layers] : config.species_layers) {
        // Retrieve the export field for this species.
        // Convention: "total_<species>_emissions"
        ESMC_Field exportField;
        std::string export_name = "total_" + species + "_emissions";
        int rc = ESMC_StateGetField(exportState, export_name.c_str(), &exportField);
        if (rc != ESMF_SUCCESS) {
            std::cerr << "ACES_Compute: Warning - Could not find export field " << export_name << std::endl;
            continue;
        }
        auto total_view = WrapESMCField(exportField, nx, ny, nz);

        // Initialize export field to 0 before accumulating layers.
        Kokkos::deep_copy(total_view, 0.0);

        for (auto const& layer : layers) {
            ESMC_Field field;
            rc = ESMC_StateGetField(importState, layer.field_name.c_str(), &field);
            if (rc != ESMF_SUCCESS) {
                std::cerr << "ACES_Compute: Warning - Could not find input field " << layer.field_name << " for species " << species << std::endl;
                continue;
            }
            auto field_view = WrapESMCField(field, nx, ny, nz);

            UnmanagedHostView3D mask_view;
            if (!layer.mask_name.empty()) {
                ESMC_Field maskField;
                rc = ESMC_StateGetField(importState, layer.mask_name.c_str(), &maskField);
                if (rc == ESMF_SUCCESS) {
                    mask_view = WrapESMCField(maskField, nx, ny, nz);
                } else {
                    std::cerr << "ACES_Compute: Warning - Could not find mask " << layer.mask_name << " for layer " << layer.field_name << ", using default 1.0" << std::endl;
                    mask_view = default_mask;
                }
            } else {
                mask_view = default_mask;
            }

            // replace_flag: 1.0 for "replace", 0.0 for "add"
            double replace_flag = (layer.operation == "replace") ? 1.0 : 0.0;
            double scale = layer.scale;

            // Launch branchless kernel
            Kokkos::parallel_for("EmissionLayerKernel",
                Kokkos::MDRangePolicy<Kokkos::Rank<3>>({0, 0, 0}, {nx, ny, nz}),
                KOKKOS_LAMBDA(int i, int j, int k) {
                    /**
                     * Branchless Math Logic:
                     * If operation is 'add' (replace_flag = 0.0):
                     *   Total = Total * (1.0 - 0.0 * Mask) + (Field * Scale * Mask)
                     *   Total = Total + (Field * Scale * Mask)
                     *
                     * If operation is 'replace' (replace_flag = 1.0):
                     *   Total = Total * (1.0 - 1.0 * Mask) + (Field * Scale * Mask)
                     *   Total = Total * (1.0 - Mask) + (Field * Scale * Mask)
                     *
                     * Note: Mask is expected to be 1.0 where the layer applies and 0.0 otherwise.
                     */
                    total_view(i, j, k) = total_view(i, j, k) * (1.0 - replace_flag * mask_view(i, j, k)) +
                                          field_view(i, j, k) * scale * mask_view(i, j, k);
                }
            );
        }
    }
    // Ensure all kernels finish before returning to the framework.
    Kokkos::fence();
}

} // namespace aces
