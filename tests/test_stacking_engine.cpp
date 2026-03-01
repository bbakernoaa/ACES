#include <gtest/gtest.h>

#include <Kokkos_Core.hpp>
#include <map>

#include "aces/aces_compute.hpp"
#include "aces/aces_config.hpp"
#include "aces/aces_stacking_engine.hpp"

namespace aces {

/**
 * @brief FieldResolver implementation that works with actual Kokkos DualViews.
 */
class ActualFieldResolver : public FieldResolver {
    std::map<std::string, DualView3D> fields;

   public:
    void AddField(const std::string& name, int nx, int ny, int nz) {
        fields[name] = DualView3D("test_" + name, nx, ny, nz);
    }

    void SetValue(const std::string& name, double val) {
        auto host = fields[name].view_host();
        Kokkos::deep_copy(host, val);
        fields[name].modify<Kokkos::HostSpace>();
        fields[name].sync<Kokkos::DefaultExecutionSpace::memory_space>();
    }

    double GetValue(const std::string& name) {
        fields[name].sync<Kokkos::HostSpace>();
        return fields[name].view_host()(0, 0, 0);
    }

    UnmanagedHostView3D ResolveImport(const std::string& name, int, int, int) override {
        return fields[name].view_host();
    }
    UnmanagedHostView3D ResolveExport(const std::string& name, int, int, int) override {
        return fields[name].view_host();
    }
    Kokkos::View<const double***, Kokkos::LayoutLeft, Kokkos::DefaultExecutionSpace>
    ResolveImportDevice(const std::string& name, int, int, int) override {
        return fields[name].view_device();
    }
    Kokkos::View<double***, Kokkos::LayoutLeft, Kokkos::DefaultExecutionSpace> ResolveExportDevice(
        const std::string& name, int, int, int) override {
        return fields[name].view_device();
    }
};

class StackingEngineTest : public ::testing::Test {
   protected:
    void SetUp() override {
        if (!Kokkos::is_initialized()) Kokkos::initialize();
    }
};

/**
 * @test Verify that hierarchy-based replacement works correctly in the StackingEngine.
 */
TEST_F(StackingEngineTest, HierarchyReplacement) {
    int nx = 1, ny = 1, nz = 1;
    AcesConfig config;

    // Layer 1: Add 10.0 (Hierarchy 1)
    EmissionLayer l1;
    l1.operation = "add";
    l1.field_name = "f1";
    l1.hierarchy = 1;
    l1.scale = 1.0;

    // Layer 2: Replace with 5.0 (Hierarchy 10)
    EmissionLayer l2;
    l2.operation = "replace";
    l2.field_name = "f2";
    l2.hierarchy = 10;
    l2.scale = 1.0;

    config.species_layers["test_species"] = {l2, l1};  // Intentionally out of order

    ActualFieldResolver resolver;
    resolver.AddField("f1", nx, ny, nz);
    resolver.SetValue("f1", 10.0);
    resolver.AddField("f2", nx, ny, nz);
    resolver.SetValue("f2", 5.0);
    resolver.AddField("total_test_species_emissions", nx, ny, nz);
    resolver.SetValue("total_test_species_emissions", 0.0);

    StackingEngine engine(config);
    engine.Execute(resolver, nx, ny, nz, {}, 0, 0);

    EXPECT_DOUBLE_EQ(resolver.GetValue("total_test_species_emissions"), 5.0);
}

/**
 * @test Verify that default_mask is correctly applied when no specific masks are provided.
 */
TEST_F(StackingEngineTest, DefaultMaskApplication) {
    int nx = 1, ny = 1, nz = 1;
    AcesConfig config;

    EmissionLayer l1;
    l1.operation = "add";
    l1.field_name = "f1";
    l1.scale = 1.0;

    config.species_layers["test_species"] = {l1};

    ActualFieldResolver resolver;
    resolver.AddField("f1", nx, ny, nz);
    resolver.SetValue("f1", 10.0);
    resolver.AddField("total_test_species_emissions", nx, ny, nz);
    resolver.SetValue("total_test_species_emissions", 0.0);

    // Provide a default mask of 0.5
    Kokkos::View<double***, Kokkos::LayoutLeft, Kokkos::DefaultExecutionSpace> dmask("dmask", nx,
                                                                                     ny, nz);
    Kokkos::deep_copy(dmask, 0.5);

    StackingEngine engine(config);
    engine.Execute(resolver, nx, ny, nz, dmask, 0, 0);

    // Should be 10.0 * 0.5 = 5.0
    EXPECT_DOUBLE_EQ(resolver.GetValue("total_test_species_emissions"), 5.0);
}

}  // namespace aces
