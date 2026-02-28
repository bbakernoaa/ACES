#include "aces/physics/aces_dms.hpp"

#include <Kokkos_Core.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace aces {

/**
 * @brief DMS Sea-Air Flux (Ported from hcox_seaflux_mod.F90)
 */

KOKKOS_INLINE_FUNCTION
double get_sc_w_dms(double tc) {
  return 2674.0 - 147.12 * tc + 3.726 * tc * tc - 0.038 * tc * tc * tc;
}

KOKKOS_INLINE_FUNCTION
double get_kw_nightingale(double u10, double sc_w) {
  return (0.222 * u10 * u10 + 0.333 * u10) * std::pow(sc_w / 600.0, -0.5);
}

void DMSScheme::Initialize(const YAML::Node& /*config*/,
                           AcesDiagnosticManager* /*diag_manager*/) {
  std::cout << "DMSScheme: Initialized." << std::endl;
}

void DMSScheme::Run(AcesImportState& import_state,
                    AcesExportState& export_state) {
  auto it_u10 = import_state.fields.find("wind_speed_10m");
  auto it_tskin = import_state.fields.find("tskin");
  auto it_seaconc = import_state.fields.find("DMS_seawater");
  auto it_dms_emis = export_state.fields.find("total_dms_emissions");

  if (it_u10 == import_state.fields.end() ||
      it_tskin == import_state.fields.end() ||
      it_seaconc == import_state.fields.end() ||
      it_dms_emis == export_state.fields.end())
    return;

  auto u10m = it_u10->second.view_device();
  auto tskin = it_tskin->second.view_device();
  auto seaconc = it_seaconc->second.view_device();
  auto dms_emis = it_dms_emis->second.view_device();

  int nx = dms_emis.extent(0);
  int ny = dms_emis.extent(1);
  int nz = dms_emis.extent(2);

  Kokkos::parallel_for(
      "DMSKernel_Faithful",
      Kokkos::MDRangePolicy<Kokkos::DefaultExecutionSpace, Kokkos::Rank<3>>(
          {0, 0, 0}, {nx, ny, nz}),
      KOKKOS_LAMBDA(int i, int j, int k) {
        if (k != 0) return;  // Surface restricted

        double tk = tskin(i, j, k);
        double tc = tk - 273.15;
        double w = u10m(i, j, k);
        double conc = seaconc(i, j, k);

        if (tc < -10.0) return;

        double sc_w = get_sc_w_dms(tc);
        double k_w = get_kw_nightingale(w, sc_w);  // cm/hr
        k_w /= 360000.0;                           // cm/hr -> m/s

        dms_emis(i, j, k) += k_w * conc;
      });

  Kokkos::fence();
  it_dms_emis->second.modify_device();
}

}  // namespace aces
