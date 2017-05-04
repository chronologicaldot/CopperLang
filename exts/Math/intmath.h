// Copyright 2016-2017 Nicolaus Anderson
#ifndef COPPER_INT_MATH_H
#define COPPER_INT_MATH_H
#include "../../Copper/src/Strings.h"
#include "../../Copper/src/Copper.h"
#include "BasicPrimitive.h"

// Comment out to disable integer bounds checks.
// Bounds checks for division and mod are always enabled.
#define ENABLE_COPPER_INTEGER_BOUNDS_CHECKS

namespace Cu {
namespace Numeric {
namespace Int {

using util::List;
using util::String;

	// You should really just use me. :)
void addFunctionsToEngine(Engine& engine, bool useShortNames);

bool isObjectInt(const Object* pObject);
bool getInt(const Object* pObject, int& pValue); // Returns "true" if an int could be obtained.

bool iszero(int);

class Int : public BasicPrimitive {
	int value;

public:
	Int()
		: BasicPrimitive( BasicPrimitive::Type::_int )
		, value(0)
	{}

	Int(int pInitValue)
		: BasicPrimitive( BasicPrimitive::Type::_int )
		, value(pInitValue)
	{}

	Int(const String& pInitValue);

	virtual Object* copy() {
		return new Int(value);
	}

	bool equal(const Int& pOther);

	virtual void writeToString(String& out) const;

	virtual int getAsInt() const;

	virtual unsigned int getAsUnsignedInt() const;

	virtual unsigned long getAsUnsignedLong() const;

	virtual float getAsFloat() const;

	virtual double getAsDouble() const;

	void incr() {
		value++;
	}
};

// Int object identification
struct AreInt : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

// Int Creation
struct Create : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

// Unimplemented
struct Unimplemented : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

struct AreZero : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

struct AreEqual : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

struct IsGreaterThan : public TwoArgBase {
	virtual bool call( FFIServices& ffi );
};

struct IsLessThan : public TwoArgBase {
	virtual bool call( FFIServices& ffi );
};

struct IsGreaterThanOrEqual : public TwoArgBase {
	virtual bool call( FFIServices& ffi );
};

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

struct Get_abs : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual ObjectType::Value
	getParameterType( unsigned int index ) const;

	virtual unsigned int
	getParameterCount() const;
};

struct Incr : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

}}}
#endif
