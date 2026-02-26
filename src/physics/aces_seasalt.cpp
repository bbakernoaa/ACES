#include "aces/physics/aces_seasalt.hpp"
#include <iostream>

namespace aces {
namespace physics {

/**
 * @brief Dispatch the Sea Salt kernel.
 */
void RunSeaSalt(const AcesImportState& importState, AcesExportState& exportState) {
    // Determine bounds for parallel execution (2D).
    int nx = exportState.sea_salt_emissions.extent(0);
    int ny = exportState.sea_salt_emissions.extent(1);

    // MDRangePolicy for 2D parallel loop.
    using Policy = Kokkos::MDRangePolicy<Kokkos::Rank<2>>;
    Policy policy({0, 0}, {nx, ny});

    // Create the kernel functor and dispatch.
    SeaSaltKernel kernel;
    kernel.wind_speed_10m = importState.wind_speed_10m;
    kernel.sea_salt_flux = exportState.sea_salt_emissions;

    Kokkos::parallel_for("RunSeaSalt", policy, kernel);
}

} // namespace physics
} // namespace aces
