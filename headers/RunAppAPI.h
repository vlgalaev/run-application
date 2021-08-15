#pragma once

#include "MAIN/Common_p.h"
#include "RdxModuleDefault.h"

unsigned long RDX_MODULE_CALL             runRunApp (COMMON_PARAM *p_cp, const char *p_packed_parameters);
void          RDX_MODULE_CALL              ExtraParameters (EXTRA_PARAM *p_extra_parameters);
int           RDX_MODULE_CALL             editParameters (void *p_parent_window, int *p_packed_parameters_size_in_bytes,
															void *p_packed_parameters);
void          RDX_MODULE_CALL             GetParamPathList (std::vector<std::string> *p_encoded_paths,
															char *p_packed_parameters);
void          RDX_MODULE_CALL             SetParamPathList (DM_DATAHANDLE dh, std::vector<std::string> *p_encoded_paths,
															char *p_packed_parameters);
char*         RDX_MODULE_CALL             getSuffix (int packed_parameters_size_in_bytes, void *p_packed_parameters);
