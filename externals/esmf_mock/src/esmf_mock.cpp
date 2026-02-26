#include "ESMC.h"
#include <iostream>
#include <map>
#include <string>

/**
 * Internal structure to represent an ESMF State in the mock.
 */
struct MockState {
    std::map<std::string, void*> fields;
};

extern "C" {

// Note: Function pointer signature matches ESMC.h
int ESMC_GridCompSetEntryPoint(ESMC_GridComp comp, ESMC_Method method, void (*function)(ESMC_GridComp, ESMC_State, ESMC_State, ESMC_Clock*, int*), int phase) {
    return ESMF_SUCCESS;
}

int ESMC_StateGetField(ESMC_State state, const char* name, ESMC_Field* field) {
    if (field && state.ptr) {
        MockState* mockState = static_cast<MockState*>(state.ptr);
        auto it = mockState->fields.find(name);
        if (it != mockState->fields.end()) {
            field->ptr = it->second;
            return ESMF_SUCCESS;
        }
    }
    return 1; // Failure
}

void* ESMC_FieldGetPtr(ESMC_Field field, int localDe, int* rc) {
    if (rc) *rc = ESMF_SUCCESS;
    return field.ptr;
}

// Additional mock helper for testing
void Mock_StateAddField(ESMC_State state, const char* name, void* ptr) {
    if (state.ptr) {
        MockState* mockState = static_cast<MockState*>(state.ptr);
        mockState->fields[name] = ptr;
    }
}

ESMC_State Mock_StateCreate() {
    ESMC_State state;
    state.ptr = new MockState();
    return state;
}

void Mock_StateDestroy(ESMC_State state) {
    if (state.ptr) {
        delete static_cast<MockState*>(state.ptr);
    }
}

}
