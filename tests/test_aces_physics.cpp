#include <gtest/gtest.h>
#include <Kokkos_Core.hpp>
#include "aces/aces_state.hpp"
#include "aces/physics/aces_seasalt.hpp"
#include "aces/physics/aces_dust.hpp"
#include "aces/physics/aces_biogenics.hpp"

namespace aces {
namespace testing {

class PhysicsTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        if (!Kokkos::is_initialized()) {
            Kokkos::initialize();
        }
    }

    static void TearDownTestSuite() {
        // We will finalize in main to be sure it's done at the very end.
    }

    void SetUp() override {
        nx = 4;
        ny = 4;
        nz = 2;

        // Allocate managed views for test data, initialized to zero.
        temperature_m = Kokkos::View<double***, Kokkos::LayoutLeft>("temp", nx, ny, nz);
        wind_speed_10m_m = Kokkos::View<double***, Kokkos::LayoutLeft>("u10", nx, ny, 1);
        sea_salt_emissions_m = Kokkos::View<double***, Kokkos::LayoutLeft>("ss", nx, ny, nz);
        dust_emissions_m = Kokkos::View<double***, Kokkos::LayoutLeft>("dust", nx, ny, nz);
        biogenic_emissions_m = Kokkos::View<double***, Kokkos::LayoutLeft>("biog", nx, ny, nz);

        Kokkos::deep_copy(temperature_m, 0.0);
        Kokkos::deep_copy(wind_speed_10m_m, 0.0);
        Kokkos::deep_copy(sea_salt_emissions_m, 0.0);
        Kokkos::deep_copy(dust_emissions_m, 0.0);
        Kokkos::deep_copy(biogenic_emissions_m, 0.0);

        // Wrap them in UnmanagedHostView3D for the API
        importState.temperature = UnmanagedHostView3D(temperature_m.data(), nx, ny, nz);
        importState.wind_speed_10m = UnmanagedHostView3D(wind_speed_10m_m.data(), nx, ny, 1);

        exportState.sea_salt_emissions = UnmanagedHostView3D(sea_salt_emissions_m.data(), nx, ny, nz);
        exportState.dust_emissions = UnmanagedHostView3D(dust_emissions_m.data(), nx, ny, nz);
        exportState.biogenic_emissions = UnmanagedHostView3D(biogenic_emissions_m.data(), nx, ny, nz);
    }

    int nx, ny, nz;
    Kokkos::View<double***, Kokkos::LayoutLeft> temperature_m;
    Kokkos::View<double***, Kokkos::LayoutLeft> wind_speed_10m_m;
    Kokkos::View<double***, Kokkos::LayoutLeft> sea_salt_emissions_m;
    Kokkos::View<double***, Kokkos::LayoutLeft> dust_emissions_m;
    Kokkos::View<double***, Kokkos::LayoutLeft> biogenic_emissions_m;

    AcesImportState importState;
    AcesExportState exportState;
};

TEST_F(PhysicsTest, SeaSaltFluxCalculation) {
    // Set a constant wind speed
    double u10_val = 10.0;
    Kokkos::deep_copy(wind_speed_10m_m, u10_val);

    // Run Sea Salt physics once
    physics::RunSeaSalt(importState, exportState);

    // Verify the results (only k=0 should be updated)
    double expected_flux = 1.0e-10 * std::pow(u10_val, 3.41);

    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < ny; ++j) {
            EXPECT_NEAR(exportState.sea_salt_emissions(i, j, 0), expected_flux, 1e-15);
            for (int k = 1; k < nz; ++k) {
                EXPECT_EQ(exportState.sea_salt_emissions(i, j, k), 0.0);
            }
        }
    }

    // Run Sea Salt physics again to test atomic addition
    physics::RunSeaSalt(importState, exportState);
    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < ny; ++j) {
            EXPECT_NEAR(exportState.sea_salt_emissions(i, j, 0), 2.0 * expected_flux, 1e-15);
        }
    }
}

TEST_F(PhysicsTest, DustFluxCalculation) {
    // Set wind speeds: one below threshold, one above
    auto u10_host = Kokkos::create_mirror_view(wind_speed_10m_m);
    u10_host(0, 0, 0) = 4.0; // below 5.0
    u10_host(1, 1, 0) = 10.0; // above 5.0
    Kokkos::deep_copy(wind_speed_10m_m, u10_host);

    // Run Dust physics once
    physics::RunDust(importState, exportState);

    // Verify the results
    double ut = 5.0;

    // Case 1: Below threshold
    EXPECT_EQ(exportState.dust_emissions(0, 0, 0), 0.0);

    // Case 2: Above threshold
    double u10_val = 10.0;
    double expected_flux = 1.0e-9 * (u10_val * u10_val) * (u10_val - ut);
    EXPECT_NEAR(exportState.dust_emissions(1, 1, 0), expected_flux, 1e-15);

    // Verify only k=0 is updated
    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < ny; ++j) {
            for (int k = 1; k < nz; ++k) {
                EXPECT_EQ(exportState.dust_emissions(i, j, k), 0.0);
            }
        }
    }

    // Run Dust physics again to test atomic addition
    physics::RunDust(importState, exportState);
    EXPECT_NEAR(exportState.dust_emissions(1, 1, 0), 2.0 * expected_flux, 1e-15);
}

TEST_F(PhysicsTest, BiogenicFluxCalculation) {
    // Set a constant temperature
    double temp_val = 303.15 + 10.0; // 313.15 K
    Kokkos::deep_copy(temperature_m, temp_val);

    // Run Biogenic physics
    physics::RunBiogenics(importState, exportState);

    // Verify the results
    double beta = 0.09;
    double T_ref = 303.15;
    double expected_flux = 1.0e-11 * std::exp(beta * (temp_val - T_ref));

    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < ny; ++j) {
            EXPECT_NEAR(exportState.biogenic_emissions(i, j, 0), expected_flux, 1e-15);
            for (int k = 1; k < nz; ++k) {
                EXPECT_EQ(exportState.biogenic_emissions(i, j, k), 0.0);
            }
        }
    }
}

} // namespace testing
} // namespace aces

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    if (Kokkos::is_initialized()) {
        Kokkos::finalize();
    }
    return result;
}
