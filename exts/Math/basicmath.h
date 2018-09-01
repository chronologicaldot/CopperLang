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

	virtual bool
	isVariadic() const {
		return true;
	}
};

struct LoneDecimalFunc : public ForeignFunc {
	virtual ~LoneDecimalFunc() {}

	virtual UInteger
	getParameterCount() const {
		return 1;
	}

	virtual ObjectType::Value
	getParameterType( UInteger  CU_UNUSED_ARG(index) ) const {
		return ObjectType::Decimal;
	}
};

struct IntegerCast : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct DecimalCast : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};


bool ToString( FFIServices& ffi );
bool DecimalInfinity( FFIServices& ffi );

bool Unimplemented( FFIServices& ffi );

struct AreZero : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct Modulus : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct Power : public VariadicFunc {
	virtual bool call( FFIServices& ffi );
};

struct PI : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

struct SmallPI : public ForeignFunc {
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

struct Sine : public LoneDecimalFunc {
	virtual bool call( FFIServices& ffi );
};

struct Cosine : public LoneDecimalFunc {
	virtual bool call( FFIServices& ffi );
};

struct Tangent : public LoneDecimalFunc {
	virtual bool call( FFIServices& ffi );
};

struct Ceiling : public LoneDecimalFunc {
	virtual bool call( FFIServices& ffi );
};

struct Floor : public LoneDecimalFunc {
	virtual bool call( FFIServices& ffi );
};

}}
#endif
