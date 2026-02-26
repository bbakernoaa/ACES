#ifndef ACES_SEASALT_HPP
#define ACES_SEASALT_HPP

/**
 * @file aces_seasalt.hpp
 * @brief Sea Salt emissions module.
 */

#include "aces/aces_state.hpp"
#include <Kokkos_Core.hpp>

namespace aces {
namespace physics {

/**
 * @brief Sea Salt emission kernel functor.
 *
 * This functor calculates Sea Salt emissions using a simplified wind-speed
 * dependent formulation (Gong, 2003 type) and applies it to the surface layer.
 */
struct SeaSaltKernel {
    UnmanagedHostView3D wind_speed_10m;  ///< [in] 10m wind speed [m/s]
    UnmanagedHostView3D sea_salt_flux;   ///< [in/out] Sea Salt flux [kg/m2/s]

    /**
     * @brief Kokkos operator for parallel execution.
     * @param i Lon index.
     * @param j Lat index.
     */
    KOKKOS_INLINE_FUNCTION
    void operator()(const int i, const int j) const {
        // Simple Gong (2003) version for testing
        // F = C * U10^3.41
        // We use wind_speed_10m(i, j, 0) since 10m wind is 2D (k=0).
        double u10 = wind_speed_10m(i, j, 0);
        double flux = 1.0e-10 * Kokkos::pow(u10, 3.41);

        // Atomically add to the surface layer (k=0) of the emission array.
        Kokkos::atomic_add(&sea_salt_flux(i, j, 0), flux);
    }
};

/**
 * @brief Runs the Sea Salt physics module.
 *
 * Dispatches the SeaSaltKernel using Kokkos::parallel_for over the 2D grid.
 *
 * @param importState Input meteorological state.
 * @param exportState Output emissions state.
 */
void RunSeaSalt(const AcesImportState& importState, AcesExportState& exportState);

} // namespace physics
} // namespace aces

#endif // ACES_SEASALT_HPP
