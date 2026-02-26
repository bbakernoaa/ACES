#ifndef ACES_BIOGENICS_HPP
#define ACES_BIOGENICS_HPP

/**
 * @file aces_biogenics.hpp
 * @brief Biogenic emissions module.
 */

#include "aces/aces_state.hpp"
#include <Kokkos_Core.hpp>

namespace aces {
namespace physics {

/**
 * @brief Biogenic emission kernel functor.
 *
 * This functor calculates Biogenic emissions (e.g., Isoprene) using a simplified
 * temperature-dependent formulation.
 */
struct BiogenicKernel {
    UnmanagedHostView3D temperature;      ///< [in] Surface air temperature [K]
    UnmanagedHostView3D biogenic_flux;    ///< [in/out] Biogenic flux [kg/m2/s]

    /**
     * @brief Kokkos operator for parallel execution.
     * @param i Lon index.
     * @param j Lat index.
     */
    KOKKOS_INLINE_FUNCTION
    void operator()(const int i, const int j) const {
        // Simple temperature dependent version for testing:
        // F = C * exp(beta * (T - T_ref))
        double T = temperature(i, j, 0); // Use surface temperature
        double beta = 0.09;
        double T_ref = 303.15;
        double flux = 1.0e-11 * Kokkos::exp(beta * (T - T_ref));

        // Atomically add to the surface layer (k=0) of the emission array.
        Kokkos::atomic_add(&biogenic_flux(i, j, 0), flux);
    }
};

/**
 * @brief Runs the Biogenic physics module.
 *
 * Dispatches the BiogenicKernel using Kokkos::parallel_for over the 2D grid.
 *
 * @param importState Input meteorological state.
 * @param exportState Output emissions state.
 */
void RunBiogenics(const AcesImportState& importState, AcesExportState& exportState);

} // namespace physics
} // namespace aces

#endif // ACES_BIOGENICS_HPP
