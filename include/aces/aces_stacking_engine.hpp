#ifndef ACES_STACKING_ENGINE_HPP
#define ACES_STACKING_ENGINE_HPP

#include <Kokkos_Core.hpp>
#include <memory>
#include <string>
#include <vector>

#include "aces/aces_compute.hpp"
#include "aces/aces_config.hpp"

namespace aces {

/**
 * @class StackingEngine
 * @brief High-performance engine for stacking emission layers using Kokkos.
 *
 * @details The StackingEngine pre-compiles emission layer configurations to minimize
 * host-side overhead during the simulation's main loop. It sorts layers by hierarchy
 * and prepares them for efficient execution using Kokkos parallel kernels.
 */
class StackingEngine {
   public:
    /**
     * @brief Constructs a StackingEngine with the given configuration.
     * @param config The ACES configuration containing species and layer definitions.
     */
    explicit StackingEngine(const AcesConfig& config);

    /**
     * @brief Executes the emission stacking for all species.
     *
     * @param resolver The field resolver to obtain device views for fields, masks, and scales.
     * @param nx Grid X dimension.
     * @param ny Grid Y dimension.
     * @param nz Grid Z dimension.
     * @param default_mask Fallback 1.0 mask if no masks are specified for a layer.
     * @param hour Current hour (0-23) for diurnal scaling.
     * @param day_of_week Current day (0-6) for weekly scaling.
     */
    void Execute(
        FieldResolver& resolver, int nx, int ny, int nz,
        Kokkos::View<double***, Kokkos::LayoutLeft, Kokkos::DefaultExecutionSpace> default_mask,
        int hour, int day_of_week);

   private:
    /**
     * @struct CompiledLayer
     * @brief Internal representation of an emission layer optimized for execution.
     */
    struct CompiledLayer {
        std::string field_name;                 ///< Name of the base field.
        std::string operation;                  ///< "add" or "replace".
        double base_scale;                      ///< Constant scaling factor.
        int hierarchy;                          ///< Global hierarchy level.
        std::vector<std::string> masks;         ///< List of mask field names.
        std::vector<std::string> scale_fields;  ///< List of scaling field names.
        std::string diurnal_cycle;              ///< Name of diurnal cycle.
        std::string weekly_cycle;               ///< Name of weekly cycle.
    };

    /**
     * @struct CompiledSpecies
     * @brief Group of compiled layers for a specific species.
     */
    struct CompiledSpecies {
        std::string name;                   ///< Name of the species (e.g., "NO2").
        std::vector<CompiledLayer> layers;  ///< Pre-sorted layers for this species.
    };

    AcesConfig m_config;                       ///< Stored configuration.
    std::vector<CompiledSpecies> m_compiled;  ///< Pre-compiled execution plan.

    /**
     * @brief Performs one-time compilation of the species and layers.
     */
    void PreCompile();
};

}  // namespace aces

#endif  // ACES_STACKING_ENGINE_HPP
