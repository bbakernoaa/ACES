#include "aces/physics/aces_dust.hpp"
#include <iostream>

namespace aces {
namespace physics {

/**
 * @brief Dispatch the Dust kernel.
 */
void RunDust(const AcesImportState& importState, AcesExportState& exportState) {
    // Determine bounds for parallel execution (2D).
    int nx = exportState.dust_emissions.extent(0);
    int ny = exportState.dust_emissions.extent(1);

    // MDRangePolicy for 2D parallel loop.
    using Policy = Kokkos::MDRangePolicy<Kokkos::Rank<2>>;
    Policy policy({0, 0}, {nx, ny});

    // Create the kernel functor and dispatch.
    DustKernel kernel;
    kernel.wind_speed_10m = importState.wind_speed_10m;
    kernel.dust_flux = exportState.dust_emissions;

    Kokkos::parallel_for("RunDust", policy, kernel);
}

} // namespace physics
} // namespace aces
