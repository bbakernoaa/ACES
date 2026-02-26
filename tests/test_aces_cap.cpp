#include <gtest/gtest.h>
#include "ESMC.h"
#include <iostream>
#include <cstring> // for memset

// Declare the functions we want to test
extern "C" {
void ACES_SetServices(ESMC_GridComp comp, int* rc);
void ACES_Initialize(ESMC_GridComp comp, ESMC_State importState, ESMC_State exportState, ESMC_Clock clock, ESMC_VM vm, int* rc);
void ACES_Finalize(ESMC_GridComp comp, ESMC_State importState, ESMC_State exportState, ESMC_Clock clock, ESMC_VM vm, int* rc);
}

TEST(ACES_Cap_Test, SetServices) {
    int rc = -1;
    // Create a dummy component handle.
    // In real ESMF, this might be a struct or pointer, so we zero-init it to be safe.
    ESMC_GridComp comp;
    std::memset(&comp, 0, sizeof(comp));

    ACES_SetServices(comp, &rc);
    EXPECT_EQ(rc, ESMF_SUCCESS);
}

TEST(ACES_Cap_Test, Lifecycle) {
    int rc = -1;
    ESMC_GridComp comp;
    std::memset(&comp, 0, sizeof(comp));

    ESMC_State importState;
    std::memset(&importState, 0, sizeof(importState));

    ESMC_State exportState;
    std::memset(&exportState, 0, sizeof(exportState));

    ESMC_Clock clock;
    std::memset(&clock, 0, sizeof(clock));

    ESMC_VM vm;
    std::memset(&vm, 0, sizeof(vm));

    // Initialize
    // Note: This initializes Kokkos
    ACES_Initialize(comp, importState, exportState, clock, vm, &rc);
    EXPECT_EQ(rc, ESMF_SUCCESS);

    // Finalize
    // Note: This finalizes Kokkos. After this, Kokkos cannot be re-initialized in this process.
    ACES_Finalize(comp, importState, exportState, clock, vm, &rc);
    EXPECT_EQ(rc, ESMF_SUCCESS);
}
