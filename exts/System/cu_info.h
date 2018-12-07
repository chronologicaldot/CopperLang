// Copyright 2018 Nicolaus Anderson
#ifndef CU_INFOLIB_INFO_H
#define CU_INFOLIB_INFO_H
#include "../../Copper/src/Copper.h"

namespace Cu {
namespace System {

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
