// Copyright 2017 Nicolaus Anderson
#ifndef COPPER_ULONG_MATH_H
#define COPPER_ULONG_MATH_H
#include "../../Copper/src/Strings.h"
#include "../../Copper/src/Copper.h"
#include "BasicPrimitive.h"
#include "../FFIBase.h"

// Comment out to disable bounds checks.
// Bounds checks for division and mod are always enabled.
//#define ENABLE_COPPER_ULONG_BOUNDS_CHECKS

namespace Cu {
namespace Numeric {
namespace ULong {

using util::List;
using util::String;
using FFI::Base;

typedef unsigned long	ulong;

	// You should really just use me. :)
void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames);

bool isObjectULong(const Object* pObject);
bool getULong(const Object* pObject, ulong& pValue); // Returns "true" if an ulong could be obtained.

bool iszero(ulong);
bool isEqual(ulong, ulong);
bool isGreaterThan(ulong, ulong);
bool isLessThan(ulong, ulong);
bool isGreaterThanOrEqual(ulong, ulong);
bool isLessThanOrEqual(ulong, ulong);

ulong add(ulong, ulong, Logger*);
ulong subtract(ulong, ulong, Logger*);
ulong multiply(ulong, ulong, Logger*);
ulong divide(ulong, ulong, Logger*);
ulong power(ulong, ulong, Logger*);
ulong mod(ulong, ulong, Logger*);

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
};

// ULong object identification
struct AreULong : public ForeignFunc {
	AreULong()
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

// ULong Creation
struct Create : public Base {
	Create(Logger* pLogger)
		: Base(pLogger)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

// Unimplemented
struct Unimplemented : public Base {
	Unimplemented(Logger* pLogger)
		: Base(pLogger)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

// Base class for comparison
// Requires pointer to a static function that compares two numbers
struct Compare : public Base {
	bool (*comparison)(ulong, ulong);

	Compare(Logger* pLogger, bool (*pComparison)(ulong, ulong) )
		: Base(pLogger)
		, comparison(pComparison)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

// For performing general math
struct SingleOperation : public Base {
	ulong (*operation)(ulong, ulong, Logger*);

	SingleOperation(Logger* pLogger, ulong (*pOperation)(ulong, ulong, Logger*) )
		: Base(pLogger)
		, operation(pOperation)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

struct SingleParamOperation : public Base {
	ulong (*operation)(ulong, Logger*);

	SingleParamOperation(Logger* pLogger, ulong (*pOperation)(ulong, Logger*) )
		: Base(pLogger)
		, operation(pOperation)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

// ULong Average
struct Avg : public Base {
	Avg(Logger* pLogger)
		: Base(pLogger)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

}}}
#endif
