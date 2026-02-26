#include "aces/aces.hpp"
#include "aces/aces_state.hpp"
#include "aces/aces_utils.hpp"
#include "aces/physics/aces_seasalt.hpp"
#include "aces/physics/aces_dust.hpp"
#include "aces/physics/aces_biogenics.hpp"
#include "aces/aces_config.hpp"
#include "aces/aces_compute.hpp"
#include <iostream>
#include <Kokkos_Core.hpp>

namespace aces {

/**
 * @brief ESMF implementation of FieldResolver.
 */
class EsmfFieldResolver : public FieldResolver {
    ESMC_State importState;
    ESMC_State exportState;

public:
    EsmfFieldResolver(ESMC_State imp, ESMC_State exp)
        : importState(imp), exportState(exp) {}

    UnmanagedHostView3D ResolveImport(const std::string& name, int nx, int ny, int nz) override {
        ESMC_Field field;
        int rc = ESMC_StateGetField(importState, name.c_str(), &field);
        if (rc != ESMF_SUCCESS) return UnmanagedHostView3D();
        return WrapESMCField(field, nx, ny, nz);
    }

    UnmanagedHostView3D ResolveExport(const std::string& name, int nx, int ny, int nz) override {
        ESMC_Field field;
        int rc = ESMC_StateGetField(exportState, name.c_str(), &field);
        if (rc != ESMF_SUCCESS) return UnmanagedHostView3D();
        return WrapESMCField(field, nx, ny, nz);
    }
};

void Initialize(ESMC_GridComp comp, ESMC_State importState, ESMC_State exportState, ESMC_Clock* clock, int* rc) {
    if (!Kokkos::is_initialized()) {
        Kokkos::initialize();
        std::cout << "ACES_Initialize: Kokkos initialized." << std::endl;
    }
    if (rc) *rc = ESMF_SUCCESS;
}

void Run(ESMC_GridComp comp, ESMC_State importState, ESMC_State exportState, ESMC_Clock* clock, int* rc) {
    std::cout << "ACES_Run: Executing." << std::endl;

    // TODO: Retrieve actual dimensions from ESMF Grid/Field
    int nx = 360, ny = 180, nz = 72;

    AcesConfig config = ParseConfig("aces_config.yaml");

    ESMC_StateGetField(importState, "temperature", &field);
    aces_import.temperature = WrapESMCField(field, nx, ny, nz);

    ESMC_StateGetField(importState, "wind_speed_10m", &field);
    aces_import.wind_speed_10m = WrapESMCField(field, nx, ny, 1);

    ESMC_StateGetField(importState, "base_anthropogenic_nox", &field);
    aces_import.base_anthropogenic_nox = WrapESMCField(field, nx, ny, nz);

    // Populate Export State
    AcesExportState aces_export;
    ESMC_StateGetField(exportState, "total_nox_emissions", &field);
    aces_export.total_nox_emissions = WrapESMCField(field, nx, ny, nz);

    ESMC_StateGetField(exportState, "sea_salt_emissions", &field);
    aces_export.sea_salt_emissions = WrapESMCField(field, nx, ny, nz);

    ESMC_StateGetField(exportState, "dust_emissions", &field);
    aces_export.dust_emissions = WrapESMCField(field, nx, ny, nz);

    ESMC_StateGetField(exportState, "biogenic_emissions", &field);
    aces_export.biogenic_emissions = WrapESMCField(field, nx, ny, nz);

    // Main compute logic: Dispatch physics modules.
    physics::RunSeaSalt(aces_import, aces_export);
    physics::RunDust(aces_import, aces_export);
    physics::RunBiogenics(aces_import, aces_export);
    EsmfFieldResolver resolver(importState, exportState);
    ComputeEmissions(config, resolver, nx, ny, nz);

    if (rc) *rc = ESMF_SUCCESS;
}

void Finalize(ESMC_GridComp comp, ESMC_State importState, ESMC_State exportState, ESMC_Clock* clock, int* rc) {
    if (Kokkos::is_initialized()) {
        Kokkos::finalize();
        std::cout << "ACES_Finalize: Kokkos finalized." << std::endl;
    }
    if (rc) *rc = ESMF_SUCCESS;
}

} // namespace aces

extern "C" {

void ACES_Initialize(ESMC_GridComp comp, ESMC_State importState, ESMC_State exportState, ESMC_Clock* clock, int* rc) {
    aces::Initialize(comp, importState, exportState, clock, rc);
}

void ACES_Run(ESMC_GridComp comp, ESMC_State importState, ESMC_State exportState, ESMC_Clock* clock, int* rc) {
    aces::Run(comp, importState, exportState, clock, rc);
}

void ACES_Finalize(ESMC_GridComp comp, ESMC_State importState, ESMC_State exportState, ESMC_Clock* clock, int* rc) {
    aces::Finalize(comp, importState, exportState, clock, rc);
}

void ACES_SetServices(ESMC_GridComp comp, int* rc) {
    if (rc) *rc = ESMF_SUCCESS;
    ESMC_GridCompSetEntryPoint(comp, ESMF_METHOD_INITIALIZE, ACES_Initialize, 1);
    ESMC_GridCompSetEntryPoint(comp, ESMF_METHOD_RUN, ACES_Run, 1);
    ESMC_GridCompSetEntryPoint(comp, ESMF_METHOD_FINALIZE, ACES_Finalize, 1);
    std::cout << "ACES_SetServices: Services set." << std::endl;
}

} // extern "C"
