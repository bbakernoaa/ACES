#ifndef ACES_COMPUTE_HPP
#define ACES_COMPUTE_HPP

#include "aces/aces_config.hpp"
#include "aces/aces_state.hpp"
#include "ESMC.h"

namespace aces {

/**
 * @brief Performs the emission computation for all species defined in the config.
 * @param config The ACES configuration.
 * @param importState The ESMF import state.
 * @param exportState The ESMF export state.
 * @param nx Grid X dimension.
 * @param ny Grid Y dimension.
 * @param nz Grid Z dimension.
 */
void ComputeEmissions(
    const AcesConfig& config,
    ESMC_State importState,
    ESMC_State exportState,
    int nx, int ny, int nz
);

} // namespace aces

#endif // ACES_COMPUTE_HPP
