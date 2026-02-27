#include "aces/physics/aces_fortran_bridge.hpp"

#include <Kokkos_Core.hpp>
#include <iostream>

/**
 * @file aces_fortran_bridge.cpp
 * @brief Implementation of the bridge between C++ ACES and legacy Fortran physics.
 */

// Declare the external Fortran function (implemented in legacy_fortran.F90)
extern "C" {
void run_legacy_fortran(double* temp, double* wind, double* nox, int nx, int ny, int nz);
}

namespace aces {

/**
 * @brief Initializes the Fortran bridge scheme.
 * @param config YAML node containing scheme-specific options.
 */
void FortranBridgeExample::Initialize(const YAML::Node& config) {
    std::cout << "FortranBridgeExample: Initialized." << std::endl;
}

/**
 * @brief Executes the legacy Fortran kernel.
 *
 * This function handles the host-device synchronization required before and after
 * calling a CPU-only Fortran routine.
 *
 * @param import_state Input data.
 * @param export_state Output data.
 */
void FortranBridgeExample::Run(AcesImportState& import_state, AcesExportState& export_state) {
    // 1. Ensure CPU has latest data by syncing to Host Execution Space
    // This is crucial if ACES is running on a GPU.
    import_state.temperature.sync<Kokkos::DefaultHostExecutionSpace::memory_space>();
    import_state.wind_speed_10m.sync<Kokkos::DefaultHostExecutionSpace::memory_space>();
    export_state.total_nox_emissions.sync<Kokkos::DefaultHostExecutionSpace::memory_space>();

    // 2. Extract dimensions
    int nx = export_state.total_nox_emissions.extent(0);
    int ny = export_state.total_nox_emissions.extent(1);
    int nz = export_state.total_nox_emissions.extent(2);

    if (nx == 0 || ny == 0 || nz == 0) return;

    // 3. Extract raw host pointers for C-Fortran interoperability
    double* temp_ptr = import_state.temperature.view_host().data();
    double* wind_ptr = import_state.wind_speed_10m.view_host().data();
    double* nox_ptr = export_state.total_nox_emissions.view_host().data();

    // 4. Call the Fortran routine
    run_legacy_fortran(temp_ptr, wind_ptr, nox_ptr, nx, ny, nz);

    // 5. Mark host modified and sync back to device
    export_state.total_nox_emissions.modify<Kokkos::DefaultHostExecutionSpace::memory_space>();
    export_state.total_nox_emissions.sync<Kokkos::DefaultExecutionSpace::memory_space>();

    std::cout << "FortranBridgeExample: Execution complete." << std::endl;
}

}  // namespace aces
