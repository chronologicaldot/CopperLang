// Copyright 2018 Nicolaus Anderson
#include "cu_info.h"

namespace Cu {
namespace System {

void
addFunctionsToEngine( Engine&  engine ) {
	addForeignFuncInstance(engine, "sys_lang_version", &GetLangVersion);
	addForeignFuncInstance(engine, "sys_vm_version", &GetVMVersion);
	addForeignFuncInstance(engine, "sys_vm_branch", &GetVMBranch);
}

ForeignFunc::Result
GetLangVersion( FFIServices&  ffi ) {
#ifdef COPPER_LANG_VERSION
	ffi.setNewResult( new DecimalNumObject( COPPER_LANG_VERSION ) );
#else
	ffi.setNewResult( new DecimalNumObject( 0 ) );
#endif
	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
GetVMVersion( FFIServices&  ffi ) {
#ifdef COPPER_INTERPRETER_VERSION
	ffi.setNewResult( new DecimalNumObject( COPPER_INTERPRETER_VERSION ) );
#else
	ffi.setNewResult( new DecimalNumObject( 0 ) );
#endif
	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
GetVMBranch( FFIServices&  ffi ) {
#ifdef COPPER_INTERPRETER_BRANCH
	ffi.setNewResult( new IntegerObject( COPPER_INTERPRETER_BRANCH ) );
#else
	ffi.setNewResult( new IntegerObject( 0 ) );
#endif
	return ForeignFunc::FINISHED;
}

}}
