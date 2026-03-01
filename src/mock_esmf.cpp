#include <stdio.h>

#include "ESMC.h"
#include "NUOPC.h"

extern "C" {

int ESMC_StateGetField(ESMC_State state, const char* name, ESMC_Field* field) {
    return ESMF_SUCCESS;
}
int ESMC_FieldGetBounds(ESMC_Field field, int* localDe, int* lbound, int* ubound, int rank) {
    return ESMF_SUCCESS;
}
void* ESMC_FieldGetPtr(ESMC_Field field, int localDe, int* rc) {
    if (rc) *rc = 0;
    return NULL;
}
int ESMC_FieldWrite(ESMC_Field field, const char* fileName, const char* fieldName, int timeslice,
                    int status, int overwrite, int format) {
    return 0;
}

int ESMC_GridCompSetInternalState(ESMC_GridComp comp, void* data) {
    return 0;
}
void* ESMC_GridCompGetInternalState(ESMC_GridComp comp, int* rc) {
    if (rc) *rc = 0;
    return NULL;
}
int ESMC_GridCompSetEntryPoint(ESMC_GridComp comp, int method,
                               void (*func)(ESMC_GridComp, ESMC_State, ESMC_State, ESMC_Clock*,
                                            int*),
                               int phase) {
    return 0;
}

int ESMC_ClockGet(ESMC_Clock clock, ESMC_TimeInterval* currSimTime, ESMC_I8* advanceCount) {
    return 0;
}
int ESMC_TimeIntervalGet(ESMC_TimeInterval timeInterval, ESMC_I8* s, int* h) {
    if (s) *s = 0;
    return 0;
}

void ESMC_InterArrayIntSet(ESMC_InterArrayInt* interArrayInt, int* array, int len) {}
ESMC_Grid ESMC_GridCreateNoPeriDim(ESMC_InterArrayInt* iCounts, void* coordSys, void* coordTypeKind,
                                   void* distgrid, int* rc) {
    ESMC_Grid g = {NULL};
    return g;
}
ESMC_Mesh ESMC_MeshCreateFromFile(const char* fileName, int fileFormat, void* convertTo3D,
                                  void* rc1, void* rc2, void* rc3, void* rc4, int* rc5) {
    ESMC_Mesh m = {NULL};
    return m;
}

int NUOPC_CompDerive(ESMC_GridComp comp, void (*func)(ESMC_GridComp, int*)) {
    return 0;
}
int NUOPC_CompSpecialize(ESMC_GridComp comp, const char* specLabel,
                         void (*func)(ESMC_GridComp, int*)) {
    return 0;
}

ESMC_State NUOPC_ModelGetImportState(ESMC_GridComp comp, int* rc) {
    ESMC_State s = {NULL};
    return s;
}
ESMC_State NUOPC_ModelGetExportState(ESMC_GridComp comp, int* rc) {
    ESMC_State s = {NULL};
    return s;
}

int NUOPC_Advertise(ESMC_State state, const char* internalName, const char* externalName) {
    return 0;
}

void NUOPC_ModelSetServices(ESMC_GridComp comp, int* rc) {}
}
