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
	virtual bool call( FFIServices& ffi );
};

struct DecimalCast : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};


bool ToString( FFIServices& ffi );
bool DecimalInfinity( FFIServices& ffi );

struct AreZero : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

struct Power : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

struct PI : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

struct SmallPI : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

class IntDeciSharedFuncs : public ForeignFunc {
protected:
	virtual void IntegerSubFunction( Integer current, FFIServices& ffi ) =0;
	virtual void DecimalSubFunction( Decimal current, FFIServices& ffi ) =0;
public:
	virtual ~IntDeciSharedFuncs();
	virtual bool call( FFIServices& ffi );
};

class Pick_min : public IntDeciSharedFuncs {
protected:
	virtual void IntegerSubFunction( Integer current, FFIServices& ffi );
	virtual void DecimalSubFunction( Decimal current, FFIServices& ffi );
};

class Pick_max : public IntDeciSharedFuncs {
protected:
	virtual void IntegerSubFunction( Integer current, FFIServices& ffi );
	virtual void DecimalSubFunction( Decimal current, FFIServices& ffi );
};

class Avg : public IntDeciSharedFuncs {
protected:
	virtual void IntegerSubFunction( Integer current, FFIServices& ffi );
	virtual void DecimalSubFunction( Decimal current, FFIServices& ffi );
};

// one decimal arg only
struct Sine : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

// one decimal arg only
struct Cosine : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

// one decimal arg only
struct Tangent : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

// one decimal arg only
struct Ceiling : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

// one decimal arg only
struct Floor : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

}}
#endif
