#ifndef ESMC_H
#define ESMC_H

#include <stddef.h>

typedef struct { void* ptr; } ESMC_Clock;
typedef struct { void* ptr; } ESMC_Field;
typedef struct { void* ptr; } ESMC_GridComp;
typedef struct { void* ptr; } ESMC_State;
typedef struct { void* ptr; } ESMC_TimeInterval;
typedef struct { void* ptr; } ESMC_Grid;
typedef struct { void* ptr; } ESMC_Mesh;
typedef struct { void* ptr; } ESMC_InterArrayInt;

typedef long long ESMC_I8;

#define ESMF_SUCCESS 0
#define ESMF_FAILURE -1

#define ESMF_METHOD_INITIALIZE 1
#define ESMF_METHOD_RUN 2
#define ESMF_METHOD_FINALIZE 3

#define ESMC_FILESTATUS_REPLACE 1
#define ESMF_IOFMT_NETCDF 1
#define ESMC_FILEFORMAT_SCRIP 1

#ifdef __cplusplus
extern "C" {
#endif

int ESMC_StateGetField(ESMC_State state, const char* name, ESMC_Field* field);
int ESMC_FieldGetBounds(ESMC_Field field, int* localDe, int* lbound, int* ubound, int rank);
void* ESMC_FieldGetPtr(ESMC_Field field, int localDe, int* rc);
int ESMC_FieldWrite(ESMC_Field field, const char* fileName, const char* fieldName, int timeslice, int status, int overwrite, int format);

int ESMC_GridCompSetInternalState(ESMC_GridComp comp, void* data);
void* ESMC_GridCompGetInternalState(ESMC_GridComp comp, int* rc);
int ESMC_GridCompSetEntryPoint(ESMC_GridComp comp, int method, void (*func)(ESMC_GridComp, ESMC_State, ESMC_State, ESMC_Clock*, int*), int phase);

int ESMC_ClockGet(ESMC_Clock clock, ESMC_TimeInterval* currSimTime, ESMC_I8* advanceCount);
int ESMC_TimeIntervalGet(ESMC_TimeInterval timeInterval, ESMC_I8* s, int* h);

void ESMC_InterArrayIntSet(ESMC_InterArrayInt* interArrayInt, int* array, int len);
ESMC_Grid ESMC_GridCreateNoPeriDim(ESMC_InterArrayInt* iCounts, void* coordSys, void* coordTypeKind, void* distgrid, int* rc);
ESMC_Mesh ESMC_MeshCreateFromFile(const char* fileName, int fileFormat, void* convertTo3D, void* rc1, void* rc2, void* rc3, void* rc4, int* rc5);

#ifdef __cplusplus
}
#endif

#endif
