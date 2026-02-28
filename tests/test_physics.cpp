#include <gtest/gtest.h>

#include <Kokkos_Core.hpp>

#include "aces/aces_physics_factory.hpp"
#include "aces/aces_state.hpp"

using namespace aces;

class PhysicsTest : public ::testing::Test {
   public:
    static void SetUpTestSuite() {
        if (!Kokkos::is_initialized()) Kokkos::initialize();
    }

    int nx = 4, ny = 4, nz = 2;
    AcesImportState import_state;
    AcesExportState export_state;

    void SetUp() override {
        // Common fields
        import_state.fields["temperature"] = create_dv("temp", 300.0);
        import_state.fields["wind_speed_10m"] = create_dv("wind", 5.0);
        import_state.fields["tskin"] = create_dv("tskin", 300.0);
        import_state.fields["lai"] = create_dv("lai", 3.0);
        import_state.fields["pardr"] = create_dv("pardr", 100.0);
        import_state.fields["pardf"] = create_dv("pardf", 50.0);
        import_state.fields["suncos"] = create_dv("suncos", 1.0);
        import_state.fields["DMS_seawater"] = create_dv("DMS_seawater", 1.0e-6);
        import_state.fields["convective_cloud_top_height"] = create_dv("conv_h", 5000.0);
        import_state.fields["gwettop"] = create_dv("gwettop", 0.5);

        // Export fields
        export_state.fields["total_SALA_emissions"] = create_dv("sala", 0.0);
        export_state.fields["total_SALC_emissions"] = create_dv("salc", 0.0);
        export_state.fields["total_isoprene_emissions"] = create_dv("isop", 0.0);
        export_state.fields["total_dms_emissions"] = create_dv("dms", 0.0);
        export_state.fields["total_lightning_nox_emissions"] = create_dv("light", 0.0);
        export_state.fields["total_soil_nox_emissions"] = create_dv("soil", 0.0);
        export_state.fields["total_nox_emissions"] = create_dv("total_nox", 0.0);
        import_state.fields["base_anthropogenic_nox"] = create_dv("base_nox", 1.0);
    }

    DualView3D create_dv(std::string name, double val) {
        DualView3D dv(name, nx, ny, nz);
        Kokkos::deep_copy(dv.view_host(), val);
        dv.modify<Kokkos::HostSpace>();
        dv.sync<Kokkos::DefaultExecutionSpace>();
        return dv;
    }
};

void TestParity(PhysicsTest* test, const std::string& cpp_name, const std::string& fortran_name, const std::string& field_name) {
    PhysicsSchemeConfig cfg_cpp, cfg_fort;
    cfg_cpp.name = cpp_name;
    cfg_fort.name = fortran_name;

    auto scheme_cpp = PhysicsFactory::CreateScheme(cfg_cpp);
    auto scheme_fort = PhysicsFactory::CreateScheme(cfg_fort);

    ASSERT_NE(scheme_cpp, nullptr);
    ASSERT_NE(scheme_fort, nullptr);

    // Run C++
    scheme_cpp->Run(test->import_state, test->export_state);
    auto& dv = test->export_state.fields[field_name];
    dv.sync<Kokkos::HostSpace>();
    double val_cpp = dv.view_host()(0,0,0);

    // Reset and Run Fortran
    Kokkos::deep_copy(dv.view_host(), 0.0);
    dv.modify<Kokkos::HostSpace>();
    dv.sync<Kokkos::DefaultExecutionSpace>();

    scheme_fort->Run(test->import_state, test->export_state);
    dv.sync<Kokkos::HostSpace>();
    double val_fort = dv.view_host()(0,0,0);

    EXPECT_NEAR(val_cpp, val_fort, std::abs(val_cpp) * 1e-6) << "Parity failed for " << cpp_name;
}

TEST_F(PhysicsTest, SeaSaltParity) {
    TestParity(this, "sea_salt", "sea_salt_fortran", "total_SALA_emissions");
}

TEST_F(PhysicsTest, MeganParity) {
    TestParity(this, "megan", "megan_fortran", "total_isoprene_emissions");
}

TEST_F(PhysicsTest, DMSParity) {
    TestParity(this, "dms", "dms_fortran", "total_dms_emissions");
}

TEST_F(PhysicsTest, LightningParity) {
    TestParity(this, "lightning", "lightning_fortran", "total_lightning_nox_emissions");
}

TEST_F(PhysicsTest, SoilNoxParity) {
    TestParity(this, "soil_nox", "soil_nox_fortran", "total_soil_nox_emissions");
}
