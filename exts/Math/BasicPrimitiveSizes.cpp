// Copyright 2016-2017 Nicolaus Anderson
#include "BasicPrimitiveSizes.h"
#include "../FFIBase.h"
#include "intmath.h"
#include "ulongmath.h"
#include "floatmath.h"
#include "doublemath.h"
#include <climits>
#include <float.h> // For FLT_MAX and DBL_MAX

namespace Cu {
namespace Numeric {
namespace Sizes {

using FFI::simpleFunctionAdd;

void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames) {
	simpleFunctionAdd(engine, String("sizeof_int"), getSizeofInt);
	simpleFunctionAdd(engine, String("sizeof_ulong"), getSizeofULong);
	simpleFunctionAdd(engine, String("sizeof_float"), getSizeofFloat);
	simpleFunctionAdd(engine, String("sizeof_double"), getSizeofDouble);

	simpleFunctionAdd(engine, String("max_int"), getMaxofInt);
	simpleFunctionAdd(engine, String("max_ulong"), getMaxofULong);
	simpleFunctionAdd(engine, String("max_float"), getMaxofFloat);
	simpleFunctionAdd(engine, String("max_double"), getMaxofDouble);
}

bool getSizeofInt( const util::List<Object*>& params, RefPtr<Object>& result ) {
	result.setWithoutRef(new ULong::ULong(sizeof(int)));
	return true;
}

bool getSizeofULong( const util::List<Object*>& params, RefPtr<Object>& result ) {
	result.setWithoutRef(new ULong::ULong(sizeof(unsigned long)));
	return true;
}

bool getSizeofFloat( const util::List<Object*>& params, RefPtr<Object>& result ) {
	result.setWithoutRef(new ULong::ULong(sizeof(float)));
	return true;
}

bool getSizeofDouble( const util::List<Object*>& params, RefPtr<Object>& result ) {
	result.setWithoutRef(new ULong::ULong(sizeof(double)));
	return true;
}

bool getMaxofInt( const util::List<Object*>& params, RefPtr<Object>& result ) {
	result.setWithoutRef(new Int::Int(INT_MAX));
	return true;
}

bool getMaxofULong( const util::List<Object*>& params, RefPtr<Object>& result ) {
	result.setWithoutRef(new ULong::ULong(ULONG_MAX));
	return true;
}

bool getMaxofFloat( const util::List<Object*>& params, RefPtr<Object>& result ) {
	result.setWithoutRef(new Float::Float(FLT_MAX));
	return true;
}

bool getMaxofDouble( const util::List<Object*>& params, RefPtr<Object>& result ) {
	result.setWithoutRef(new Double::Double(DBL_MAX));
	return true;
}

}}}
