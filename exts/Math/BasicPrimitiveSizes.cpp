// Copyright 2016-2017 Nicolaus Anderson
#include "BasicPrimitiveSizes.h"
#include "intmath.h"
#include "ulongmath.h"
//#include "floatmath.h"
//#include "doublemath.h"
#include <climits>
#include <float.h> // For FLT_MAX and DBL_MAX

namespace Cu {
namespace Numeric {
namespace Sizes {

void addFunctionsToEngine( Engine& engine ) {

	addForeignFuncInstance<GetSizeofInt>	(engine, "sizeof_int");
	addForeignFuncInstance<GetSizeofULong>	(engine, "sizeof_ulong");
	addForeignFuncInstance<GetSizeofFloat>	(engine, "sizeof_float");
	addForeignFuncInstance<GetSizeofDouble>	(engine, "sizeof_double");

	addForeignFuncInstance<GetMaxofInt>		(engine, "max_int");
	addForeignFuncInstance<GetMaxofULong>	(engine, "max_ulong");
	//addForeignFuncInstance<GetMaxofFloat>	(engine, "max_float");
	//addForeignFuncInstance<GetMaxofDouble>	(engine, "max_double");
}

bool
GetSizeofInt::call( FFIServices& ffi ) {
	Object* out = new ULong::ULong(sizeof(int));
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
GetSizeofULong::call( FFIServices& ffi ) {
	Object* out = new ULong::ULong(sizeof(unsigned long));
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
GetSizeofFloat::call( FFIServices& ffi ) {
	Object* out = new ULong::ULong(sizeof(float));
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
GetSizeofDouble::call( FFIServices& ffi ) {
	Object* out = new ULong::ULong(sizeof(double));
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
GetMaxofInt::call( FFIServices& ffi ) {
	Object* out = new Int::Int(INT_MAX);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
GetMaxofULong::call( FFIServices& ffi ) {
	Object* out = new ULong::ULong(ULONG_MAX);
	ffi.setResult(out);
	out->deref();
	return true;
}
/*
bool GetMaxofFloat::call( FFIServices& ffi ) {
	Object* out = new Float::Float(FLT_MAX);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool GetMaxofDouble::call( FFIServices& ffi ) {
	Object* out = new Double::Double(DBL_MAX);
	ffi.setResult(out);
	out->deref();
	return true;
}
*/
}}}
