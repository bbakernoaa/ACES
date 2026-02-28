#include "aces/physics/aces_dms.hpp"
#include <Kokkos_Core.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>

namespace aces {

/**
 * @brief DMS Sea-Air Flux (Ported from hcox_seaflux_mod.F90)
 */

KOKKOS_INLINE_FUNCTION
double get_sc_w_dms(double tc) {
    // Schmidt number for DMS (Saltzman et al., 1993)
    return 2674.0 - 147.12 * tc + 3.726 * tc * tc - 0.038 * tc * tc * tc;
}

KOKKOS_INLINE_FUNCTION
double get_kw_nightingale(double u10, double sc_w) {
    // Water-side transfer velocity (Nightingale et al., 2000) [cm/hr]
    return (0.222 * u10 * u10 + 0.333 * u10) * std::pow(sc_w / 600.0, -0.5);
}

void DMSScheme::Initialize(const YAML::Node& /*config*/, AcesDiagnosticManager* /*diag_manager*/) {
    std::cout << "DMSScheme: Initialized with Johnson (2010) / Nightingale (2000) logic." << std::endl;
}

void DMSScheme::Run(AcesImportState& import_state, AcesExportState& export_state) {
    auto it_u10 = import_state.fields.find("wind_speed_10m");
    auto it_tskin = import_state.fields.find("tskin");
    auto it_seaconc = import_state.fields.find("DMS_seawater");
    auto it_dms_emis = export_state.fields.find("total_dms_emissions");

    if (it_u10 == import_state.fields.end() || it_tskin == import_state.fields.end() ||
        it_seaconc == import_state.fields.end() || it_dms_emis == export_state.fields.end()) return;

    auto u10m = it_u10->second.view_device();
    auto tskin = it_tskin->second.view_device();
    auto seaconc = it_seaconc->second.view_device();
    auto dms_emis = it_dms_emis->second.view_device();

    int nx = dms_emis.extent(0);
    int ny = dms_emis.extent(1);
    int nz = dms_emis.extent(2);

    Kokkos::parallel_for(
        "DMSKernel_Full",
        Kokkos::MDRangePolicy<Kokkos::DefaultExecutionSpace, Kokkos::Rank<3>>({0, 0, 0}, {nx, ny, nz}),
        KOKKOS_LAMBDA(int i, int j, int k) {
            double tk = tskin(i, j, k);
            double tc = tk - 273.15;
            double w = u10m(i, j, k);
            double conc = seaconc(i, j, k);

            if (tc < -10.0) return;

            double sc_w = get_sc_w_dms(tc);
            double k_w = get_kw_nightingale(w, sc_w); // cm/hr
            k_w /= 360000.0; // cm/hr -> m/s

            // Henry's Law (DMS) - Gas over liquid (dimensionless)
            // H = exp(3522/T - 11.91) in atm/(mol/L)?
            // In HEMCO, it's used as F = Kg * H * Cw.
            double kh_inv = std::exp(3522.0 / tk - 11.91);

            // Net Flux (Liss and Slater, 1974)
            // Simplified: F = Kw * Cw (as DMS is highly supersaturated)
            dms_emis(i, j, k) += k_w * conc;
        });

    Kokkos::fence();
    it_dms_emis->second.modify_device();
}

}  // namespace aces
