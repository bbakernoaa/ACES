#include "aces/physics/aces_lightning.hpp"
#include <Kokkos_Core.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>

namespace aces {

/**
 * @brief Lightning NOx Yield and Vertical Distribution (Ported from hcox_lightnox_mod.F90)
 */

KOKKOS_INLINE_FUNCTION
double get_lightning_yield(double rate, double mw_no) {
    const double RFLASH_TROPIC = 1.566e26; // molec/flash
    const double AVOGADRO = 6.022e23;
    // rate in flashes/km2/s -> yield in kg NO/m2/s
    return (rate * RFLASH_TROPIC) * (mw_no / 1000.0) / (AVOGADRO * 1.0e6);
}

void LightningScheme::Initialize(const YAML::Node& /*config*/, AcesDiagnosticManager* /*diag_manager*/) {
    std::cout << "LightningScheme: Initialized with Price and Rind (1992) logic." << std::endl;
}

void LightningScheme::Run(AcesImportState& import_state, AcesExportState& export_state) {
    auto it_conv_depth = import_state.fields.find("convective_cloud_top_height");
    auto it_light_emis = export_state.fields.find("total_lightning_nox_emissions");

    if (it_conv_depth == import_state.fields.end() || it_light_emis == export_state.fields.end()) return;

    auto conv_depth = it_conv_depth->second.view_device();
    auto light_nox = it_light_emis->second.view_device();

    int nx = light_nox.extent(0);
    int ny = light_nox.extent(1);
    int nz = light_nox.extent(2);

    const double MW_NO = 30.0;

    Kokkos::parallel_for(
        "LightningKernel_Full",
        Kokkos::MDRangePolicy<Kokkos::DefaultExecutionSpace, Kokkos::Rank<3>>({0, 0, 0}, {nx, ny, nz}),
        KOKKOS_LAMBDA(int i, int j, int k) {
            double h = conv_depth(i, j, k); // meters
            if (h <= 0.0) return;

            double h_km = h / 1000.0;
            // Price and Rind (1992) - land vs ocean omitted for simplicity in this port,
            // but the power law is the core algorithm.
            double flash_rate = 3.44e-5 * std::pow(h_km, 4.9); // flashes/km2/s

            double total_yield = get_lightning_yield(flash_rate, MW_NO);

            // Ott et al. (2010) / Pickering (1998) vertical distribution logic proxy:
            // For now, distribute across all levels equally as ACES doesn't yet have
            // a standardized way to handle the CDF tables in the device kernel.
            light_nox(i, j, k) += total_yield / nz;
        });

    Kokkos::fence();
    it_light_emis->second.modify_device();
}

}  // namespace aces
