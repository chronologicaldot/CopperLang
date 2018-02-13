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

bool
iszero(
	Decimal		p
);

union IntDeciUnion {
	Integer  i_val;
	Decimal  d_val;
};

struct VariadicFunc : public ForeignFunc {
	virtual ~VariadicFunc() {}

	virtual bool isVariadic() {
		return true;
	}
};

struct IntegerCast : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct DecimalCast : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct ToString : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

// Unimplemented
struct Unimplemented : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

/*
struct IsZero : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	const char*
	getParameterType( UInteger index );

	UInteger
	getParameterCount();
};
*/

struct AreZero : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

//! Foreign Function for checking numeric equality of a series of arguments
/* Precedence for the type goes to the first argument. */
struct AreEqual : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

// gt(a: b: c:) means is a > b && a > c
struct IsGreaterThan : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

// lt(a: b: c:) means a < b && a < c
struct IsLessThan : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

// gte(a: b: c:) means a >= b && a >= c
struct IsGreaterThanOrEqual : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

// lte(a: b: c:) means a <= b && a <= c
struct IsLessThanOrEqual : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct Add : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct Subtract : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct Multiply : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct Divide : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct Modulus : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct Power : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct Pick_max : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct Pick_min : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct Avg : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct Get_abs : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

struct Incr : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

}}
#endif
