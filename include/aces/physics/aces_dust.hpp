#ifndef ACES_DUST_HPP
#define ACES_DUST_HPP

/**
 * @file aces_dust.hpp
 * @brief Dust emissions module.
 */

#include "aces/aces_state.hpp"
#include <Kokkos_Core.hpp>

namespace aces {
namespace physics {

/**
 * @brief Dust emission kernel functor.
 *
 * This functor calculates Dust emissions using a simplified threshold-based
 * wind speed formulation and applies it to the surface layer.
 */
struct DustKernel {
    UnmanagedHostView3D wind_speed_10m;  ///< [in] 10m wind speed [m/s]
    UnmanagedHostView3D dust_flux;       ///< [in/out] Dust flux [kg/m2/s]

    /**
     * @brief Kokkos operator for parallel execution.
     * @param i Lon index.
     * @param j Lat index.
     */
    KOKKOS_INLINE_FUNCTION
    void operator()(const int i, const int j) const {
        // Simple Dust version for testing: F = C * U10^2 * (U10 - Ut)
        // We use wind_speed_10m(i, j, 0) since 10m wind is 2D (k=0).
        double u10 = wind_speed_10m(i, j, 0);
        double ut = 5.0; // threshold wind speed [m/s]
        double u_diff = Kokkos::fmax(0.0, u10 - ut);
        double flux = 1.0e-9 * (u10 * u10) * u_diff;

        // Atomically add to the surface layer (k=0) of the emission array.
        Kokkos::atomic_add(&dust_flux(i, j, 0), flux);
    }
};

/**
 * @brief Runs the Dust physics module.
 *
 * Dispatches the DustKernel using Kokkos::parallel_for over the 2D grid.
 *
 * @param importState Input meteorological state.
 * @param exportState Output emissions state.
 */
void RunDust(const AcesImportState& importState, AcesExportState& exportState);

} // namespace physics
} // namespace aces

#endif // ACES_DUST_HPP
