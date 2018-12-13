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
