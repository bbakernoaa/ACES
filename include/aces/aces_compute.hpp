#ifndef ACES_COMPUTE_HPP
#define ACES_COMPUTE_HPP

#include <functional>
#include <string>

#include "aces/aces_config.hpp"
#include "aces/aces_state.hpp"

namespace aces {

/**
 * @brief Interface for resolving fields by name into Kokkos Views.
 * This allows the compute engine to be decoupled from ESMF for testing.
 */
class FieldResolver {
   public:
    virtual ~FieldResolver() = default;
    virtual UnmanagedHostView3D ResolveImport(const std::string& name, int nx, int ny, int nz) = 0;
    virtual UnmanagedHostView3D ResolveExport(const std::string& name, int nx, int ny, int nz) = 0;

    /**
     * @brief Resolves an import field and returns its device-side View.
     */
    virtual Kokkos::View<const double***, Kokkos::LayoutLeft, Kokkos::DefaultExecutionSpace>
    ResolveImportDevice(const std::string& name, int nx, int ny, int nz) = 0;

    /**
     * @brief Resolves an export field and returns its device-side View.
     */
    virtual Kokkos::View<double***, Kokkos::LayoutLeft, Kokkos::DefaultExecutionSpace>
    ResolveExportDevice(const std::string& name, int nx, int ny, int nz) = 0;
};

/**
 * @brief Performs the emission computation for all species defined in the config.
 * @param config The ACES configuration.
 * @param resolver A FieldResolver to retrieve Kokkos Views for import/export fields.
 * @param nx Grid X dimension.
 * @param ny Grid Y dimension.
 * @param nz Grid Z dimension.
 * @param default_mask Persistent 1.0 mask.
 * @param category_scratch Persistent scratch view for category accumulation.
 * @param hour Current hour of the day (0-23) for diurnal cycles.
 * @param day_of_week Current day of the week (0-6) for weekly cycles.
 */
void ComputeEmissions(
    const AcesConfig& config, FieldResolver& resolver, int nx, int ny, int nz,
    Kokkos::View<double***, Kokkos::LayoutLeft, Kokkos::DefaultExecutionSpace> default_mask = {},
    Kokkos::View<double***, Kokkos::LayoutLeft, Kokkos::DefaultExecutionSpace> category_scratch =
        {},
    int hour = 0, int day_of_week = 0);

}  // namespace aces

#endif  // ACES_COMPUTE_HPP
