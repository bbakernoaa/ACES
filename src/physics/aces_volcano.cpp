#include "aces/physics/aces_volcano.hpp"

#include <Kokkos_Core.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace aces {

void VolcanoScheme::Initialize(const YAML::Node& /*config*/,
                               AcesDiagnosticManager* /*diag_manager*/) {
    std::cout << "VolcanoScheme: Initialized." << std::endl;
}

void VolcanoScheme::Run(AcesImportState& import_state, AcesExportState& export_state) {
    auto it_so2 = export_state.fields.find("total_so2_emissions");
    auto it_zsfc = import_state.fields.find("zsfc");
    auto it_bxheight = import_state.fields.find("bxheight_m");

    if (it_so2 == export_state.fields.end() || it_zsfc == import_state.fields.end() ||
        it_bxheight == import_state.fields.end())
        return;

    auto so2 = it_so2->second.view_device();
    auto zsfc = it_zsfc->second.view_device();
    auto bxheight = it_bxheight->second.view_device();

    int nx = so2.extent(0);
    int ny = so2.extent(1);
    int nz = so2.extent(2);

    // Mock volcano location for this port (should come from config table in real
    // port) Lat: 50.17, Lon: 6.85, Sulf: 1.0kg/s, Elv: 600m, Cld: 2000m
    const int target_i = 1, target_j = 1;
    const double volcano_sulf = 1.0;
    const double volcano_elv = 600.0;
    const double volcano_cld = 2000.0;

    Kokkos::parallel_for(
        "VolcanoKernel_Faithful",
        Kokkos::MDRangePolicy<Kokkos::DefaultExecutionSpace, Kokkos::Rank<3>>({0, 0, 0},
                                                                              {nx, ny, nz}),
        KOKKOS_LAMBDA(int i, int j, int k) {
            if (i != target_i || j != target_j) return;

            double z_bot_box = zsfc(i, j, 0);
            for (int l = 0; l < k; ++l) z_bot_box += bxheight(i, j, l);
            double z_top_box = z_bot_box + bxheight(i, j, k);

            double z_bot_volc = std::max(volcano_elv, zsfc(i, j, 0));
            double z_top_volc = std::max(volcano_cld, zsfc(i, j, 0));

            // Eruptive: top 1/3
            if (z_bot_volc != z_top_volc) {
                z_bot_volc = z_top_volc - (z_top_volc - z_bot_volc) / 3.0;
            }

            double plume_hgt = z_top_volc - z_bot_volc;
            if (plume_hgt <= 0.0) {
                if (k == 0) so2(i, j, k) += volcano_sulf;
                return;
            }

            if (z_bot_volc >= z_top_box || z_top_volc <= z_bot_box) return;

            double overlap = std::min(z_top_volc, z_top_box) - std::max(z_bot_volc, z_bot_box);
            double frac = overlap / plume_hgt;
            so2(i, j, k) += frac * volcano_sulf;
        });

    Kokkos::fence();
    it_so2->second.modify_device();
}

}  // namespace aces
