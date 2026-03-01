#ifndef NUOPC_H
#define NUOPC_H

#include "ESMC.h"

#ifdef __cplusplus
extern "C" {
#endif

#define label_Advertise "Advertise"

int NUOPC_CompDerive(ESMC_GridComp comp, void (*func)(ESMC_GridComp, int*));
int NUOPC_CompSpecialize(ESMC_GridComp comp, const char* specLabel, void (*func)(ESMC_GridComp, int*));

ESMC_State NUOPC_ModelGetImportState(ESMC_GridComp comp, int* rc);
ESMC_State NUOPC_ModelGetExportState(ESMC_GridComp comp, int* rc);

int NUOPC_Advertise(ESMC_State state, const char* internalName, const char* externalName);

void NUOPC_ModelSetServices(ESMC_GridComp comp, int* rc);

#ifdef __cplusplus
}
#endif

#endif
