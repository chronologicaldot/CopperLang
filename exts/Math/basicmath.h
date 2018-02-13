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
/*
// DEPRECATED
//! Get an Integer only from a built-in number
bool
getInteger(
	const Object*	pObject,
	Integer&		pValue
);

// DEPRECATED
//! Get an Integer, converting from string if need-be
bool
getDataAsInteger(
	const Object*	pObject,
	Integer&		pValue
);

// DEPRECATED
//! Get an Decimal only from a built-in number
bool
getDecimal(
	const Object*	pObject,
	Decimal&		pValue
);

// DEPRECATED
//! Get an Decimal, converting from string if need-be
bool
getDataAsDecimal(
	const Object*	pObject,
	Decimal&		pValue
);
*/
bool
iszero(
	Decimal		p
);

union IntDeciUnion {
	Integer  i_val;
	Decimal  d_val;
};

struct IntegerCast : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

struct DecimalCast : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

struct ToString : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
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

struct AreZero : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

//! Foreign Function for checking numeric equality of a series of arguments
/* Precedence for the type goes to the first argument. */
struct AreEqual : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

// Stand-in
struct TwoArgBase : public ForeignFunc {

};

// Doesn't have to be a TwoArgBase
// gt(a: b: c:) means is a > b && a > c
struct IsGreaterThan : public TwoArgBase {
	virtual bool call( FFIServices& ffi );
};

// lt(a: b: c:) means a < b && a < c
struct IsLessThan : public TwoArgBase {
	virtual bool call( FFIServices& ffi );
};

// gte(a: b: c:) means a >= b && a >= c
struct IsGreaterThanOrEqual : public TwoArgBase {
	virtual bool call( FFIServices& ffi );
};

// lte(a: b: c:) means a <= b && a <= c
struct IsLessThanOrEqual : public TwoArgBase {
	virtual bool call( FFIServices& ffi );
};

struct Add : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

struct Subtract : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

struct Multiply : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

struct Divide : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

struct Power : public TwoArgBase {
	virtual bool call( FFIServices& ffi );
};

struct Modulus : public TwoArgBase {
	virtual bool call( FFIServices& ffi );
};

struct Pick_max : public TwoArgBase {
	virtual bool call( FFIServices& ffi );
};

struct Pick_min : public TwoArgBase {
	virtual bool call( FFIServices& ffi );
};

// Int Average
struct Avg : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};
/*
struct Get_abs : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual ObjectType::Value
	getParameterType( unsigned int index ) const;

	virtual unsigned int
	getParameterCount() const;
};
*/
struct Incr : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

}}
#endif
