#include <gtest/gtest.h>
#include "aces/aces_compute.hpp"
#include "aces/aces_config.hpp"
#include "aces/aces_utils.hpp"
#include "ESMC.h"
#include <Kokkos_Core.hpp>
#include <fstream>

namespace aces {

class AcesComputeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Kokkos initialization is handled in test_aces_cap or here if needed.
        // But since we are using GTest, we can use a singleton approach or just init once.
        if (!Kokkos::is_initialized()) {
            Kokkos::initialize();
        }
    }

    // We don't finalize Kokkos here to avoid issues with subsequent tests.
};

TEST_F(AcesComputeTest, BranchlessReplaceLogic) {
    int nx = 10, ny = 10, nz = 1;

    // 1. Create data views
    Kokkos::View<double***, Kokkos::LayoutLeft, Kokkos::HostSpace> background_data("background", nx, ny, nz);
    Kokkos::View<double***, Kokkos::LayoutLeft, Kokkos::HostSpace> regional_data("regional", nx, ny, nz);
    Kokkos::View<double***, Kokkos::LayoutLeft, Kokkos::HostSpace> mask_data("mask", nx, ny, nz);
    Kokkos::View<double***, Kokkos::LayoutLeft, Kokkos::HostSpace> export_data("export", nx, ny, nz);

    // 2. Initialize data
    Kokkos::deep_copy(background_data, 5.0);
    Kokkos::deep_copy(regional_data, 10.0);
    Kokkos::deep_copy(export_data, 0.0);

    // Left half (i < 5) has mask 1.0, right half (i >= 5) has mask 0.0
    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < ny; ++j) {
            mask_data(i, j, 0) = (i < nx / 2) ? 1.0 : 0.0;
        }
    }

    // 3. Set up mock ESMF states
    ESMC_State importState = Mock_StateCreate();
    ESMC_State exportState = Mock_StateCreate();

    Mock_StateAddField(importState, "background_field", background_data.data());
    Mock_StateAddField(importState, "regional_field", regional_data.data());
    Mock_StateAddField(importState, "half_mask", mask_data.data());
    Mock_StateAddField(exportState, "total_nox_emissions", export_data.data());

    // 4. Create configuration
    AcesConfig config;

    // Layer 1: Global background (Add)
    EmissionLayer layer1;
    layer1.operation = "add";
    layer1.field_name = "background_field";
    layer1.scale = 1.0;
    layer1.mask_name = ""; // Global

    // Layer 2: Regional replacement (Replace)
    EmissionLayer layer2;
    layer2.operation = "replace";
    layer2.field_name = "regional_field";
    layer2.mask_name = "half_mask";
    layer2.scale = 1.0;

    config.species_layers["nox"] = {layer1, layer2};

    // 5. Execute computation
    ComputeEmissions(config, importState, exportState, nx, ny, nz);

    // 6. Verify results
    // Left half: 5.0 + (10.0 replaced) -> 10.0
    // Right half: 5.0 + (nothing replaced) -> 5.0
    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < ny; ++j) {
            if (i < nx / 2) {
                EXPECT_DOUBLE_EQ(export_data(i, j, 0), 10.0) << "Failed at i=" << i << ", j=" << j;
            } else {
                EXPECT_DOUBLE_EQ(export_data(i, j, 0), 5.0) << "Failed at i=" << i << ", j=" << j;
            }
        }
    }

    // Cleanup mock
    Mock_StateDestroy(importState);
    Mock_StateDestroy(exportState);
}

TEST_F(AcesComputeTest, YamlParsing) {
    // Create a temporary YAML file
    std::ofstream out("test_config.yaml");
    out << "species:\n"
        << "  nox:\n"
        << "    - operation: add\n"
        << "      field: background_nox\n"
        << "      scale: 1.0\n"
        << "    - operation: replace\n"
        << "      field: regional_nox\n"
        << "      mask: europe_mask\n"
        << "      scale: 1.5\n";
    out.close();

    AcesConfig config = ParseConfig("test_config.yaml");

    ASSERT_EQ(config.species_layers.count("nox"), 1);
    auto layers = config.species_layers["nox"];
    ASSERT_EQ(layers.size(), 2);

    EXPECT_EQ(layers[0].operation, "add");
    EXPECT_EQ(layers[0].field_name, "background_nox");
    EXPECT_EQ(layers[0].scale, 1.0);
    EXPECT_EQ(layers[0].mask_name, "");

    EXPECT_EQ(layers[1].operation, "replace");
    EXPECT_EQ(layers[1].field_name, "regional_nox");
    EXPECT_EQ(layers[1].mask_name, "europe_mask");
    EXPECT_EQ(layers[1].scale, 1.5);

    std::remove("test_config.yaml");
}

} // namespace aces
