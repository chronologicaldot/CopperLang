// Copyright 2017 Nicolaus Anderson
#ifndef COPPER_ULONG_MATH_H
#define COPPER_ULONG_MATH_H
#include "../../Copper/src/Strings.h"
#include "../../Copper/src/Copper.h"
#include "BasicPrimitive.h"

// Comment out to disable bounds checks.
// Bounds checks for division and mod are always enabled.
#define ENABLE_COPPER_ULONG_BOUNDS_CHECKS

namespace Cu {
namespace Numeric {
namespace ULong {

using util::List;
using util::String;

typedef unsigned long	ulong;

	// You should really just use me. :)
void addFunctionsToEngine(Engine& engine, bool useShortNames);

bool isObjectULong(const Object* pObject);
bool getULong(const Object* pObject, ulong& pValue); // Returns "true" if an ulong could be obtained.

bool iszero(ulong);

class ULong : public BasicPrimitive {
	ulong value;

public:
	ULong()
		: BasicPrimitive( BasicPrimitive::Type::_ulong )
		, value(0)
	{}

	ULong(ulong pInitValue)
		: BasicPrimitive( BasicPrimitive::Type::_ulong )
		, value(pInitValue)
	{}

	ULong(const String& pInitValue);

	virtual Object* copy() {
		return new ULong(value);
	}

	bool equal(const ULong& pOther);

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
struct AreULong : public ForeignFunc {
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

struct IsZero : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual const char* getParameterName( unsigned int index );

	virtual unsigned int getParameterCount();
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

	virtual const char* getParameterName( unsigned int index );

	virtual unsigned int getParameterCount();
};

struct Incr : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

}}}
#endif
