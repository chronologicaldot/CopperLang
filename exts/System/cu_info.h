// Copyright 2018 Nicolaus Anderson
#ifndef CU_INFOLIB_INFO_H
#define CU_INFOLIB_INFO_H
#include "../../Copper/src/Copper.h"

namespace Cu {
namespace System {

/*
	Creates the following functions and adds them to the Copper engine:

sys_lang_version()
	Returns the Copper language version of the Copper engine.

sys_vm_version()
	Returns the version of the Copper engine.

sys_vm_branch()
	Returns the repository branch version of the Copper engine.
*/
void
addFunctionsToEngine( Engine& );

ForeignFunc::Result
GetLangVersion( FFIServices& );

ForeignFunc::Result
GetVMVersion( FFIServices& );

ForeignFunc::Result
GetVMBranch( FFIServices& );

}}
#endif // CU_INFOLIB_INFO_H
