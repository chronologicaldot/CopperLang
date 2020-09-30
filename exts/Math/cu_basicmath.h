// Copyright 2018 Nicolaus Anderson
#ifndef COPPER_BASIC_MATH_H
#define COPPER_BASIC_MATH_H
#include "../../Copper/src/Strings.h"
#include "../../Copper/src/Copper.h"

// Comment out to disable integer bounds checks.
// Bounds checks for division and mod are always enabled.
#define ENABLE_COPPER_INTEGER_BOUNDS_CHECKS

namespace Cu {
namespace Numeric {

	// You should really just use me. :)
/*
	Creates the following functions in the Copper engine:

int( [numeric] )
	Converts a given number to an IntegerObject/int.

dcml( [numeric] )
	Converts a given number to an DecimalNumObject/dcml.

num_to_str( [numeric] )
	Returns the string conversion of a number.

infinity()
	Returns DecimalObject/dcml infinity if CU_MATH_USE_C_LIMITS is defined, else empty function is returned.

avg( [numeric], ... )
	Accepts any number of numeric objects and returns the average of them.

min( [numeric], ... )
	Accepts any number of numeric objects and returns the minimum of them.

max( [numeric], ... )
	Accepts any number of numeric objects and returns the maximum of them.

pow( [numeric], [numeric] )
	Returns the first given number raised to the power of the second number.

PI()
	Returns the longest DecimalNumObject/dcml version of Pi.

small_PI()
	Returns a short DecimalNumObject/dcml version of Pi.

sin( [numeric] )
	Returns the sine of the given numeric value assumed to be in radians.

cos( [numeric] )
	Returns the cosine of the given numeric value assumed to be in radians.

tan( [numeric] )
	Returns the tangent of the given numeric value assumed to be in radians.

floor( [numeric] )
	Returns the floor value of the given DecimalNumObject/dcml number.
	ex: 5.1 returns 6

ceiling( [numeric] )
	Returns the floor value of the given DecimalNumObject/dcml number.
	ex: 5.1 returns 5
*/
void
addFunctionsToEngine(
	Engine&		engine
);

Integer
getIntegerValue( Object& );

Decimal
getDecimalValue( Object& );

bool
iszero(
	Decimal		p
);

struct IntegerCast : public ForeignFunc {
	virtual Result call( FFIServices& ffi );
};

struct DecimalCast : public ForeignFunc {
	virtual Result call( FFIServices& ffi );
};


ForeignFunc::Result ToString( FFIServices& ffi );
ForeignFunc::Result DecimalInfinity( FFIServices& ffi );

struct AreZero : public ForeignFunc {
	virtual Result call( FFIServices& ffi );
};

struct Power : public ForeignFunc {
	virtual Result call( FFIServices& ffi );
};

struct PI : public ForeignFunc {
	virtual Result call( FFIServices& ffi );
};

struct SmallPI : public ForeignFunc {
	virtual Result call( FFIServices& ffi );
};

struct PickMin : public ForeignFunc {
	virtual Result call( FFIServices& ffi );
};

struct PickMax : public ForeignFunc {
	virtual Result call( FFIServices& ffi );
};

struct Avg : public ForeignFunc {
	virtual Result call( FFIServices& ffi );
};

// one decimal arg only
struct Sine : public ForeignFunc {
	virtual Result call( FFIServices& ffi );
};

// one decimal arg only
struct Cosine : public ForeignFunc {
	virtual Result call( FFIServices& ffi );
};

// one decimal arg only
struct Tangent : public ForeignFunc {
	virtual Result call( FFIServices& ffi );
};

// one decimal arg only
struct Ceiling : public ForeignFunc {
	virtual Result call( FFIServices& ffi );
};

// one decimal arg only
struct Floor : public ForeignFunc {
	virtual Result call( FFIServices& ffi );
};

}}
#endif
