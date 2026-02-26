#include "aces/physics/aces_biogenics.hpp"

namespace aces {
namespace physics {

/**
 * @brief Dispatch the Biogenic kernel.
 */
void RunBiogenics(const AcesImportState& importState, AcesExportState& exportState) {
    // Determine bounds for parallel execution (2D).
    int nx = exportState.biogenic_emissions.extent(0);
    int ny = exportState.biogenic_emissions.extent(1);

    // MDRangePolicy for 2D parallel loop.
    using Policy = Kokkos::MDRangePolicy<Kokkos::Rank<2>>;
    Policy policy({0, 0}, {nx, ny});

    // Create the kernel functor and dispatch.
    BiogenicKernel kernel;
    kernel.temperature = importState.temperature;
    kernel.biogenic_flux = exportState.biogenic_emissions;

    Kokkos::parallel_for("RunBiogenics", policy, kernel);
}

} // namespace physics
} // namespace aces
