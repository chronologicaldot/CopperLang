// Copyright 2016-2017 Nicolaus Anderson
#ifndef COPPER_INT_MATH_H
#define COPPER_INT_MATH_H
#include "../../Copper/src/Strings.h"
#include "../../Copper/src/Copper.h"
#include "BasicPrimitive.h"
#include "../FFIBase.h"

// Comment out to disable integer bounds checks.
// Bounds checks for division and mod are always enabled.
#define ENABLE_COPPER_INTEGER_BOUNDS_CHECKS

namespace Cu {
namespace Numeric {
namespace Int {

using util::List;
using util::String;
using FFI::Base;

	// You should really just use me. :)
void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames);

bool isObjectInt(const Object* pObject);
bool getInt(const Object* pObject, int& pValue); // Returns "true" if an int could be obtained.

bool iszero(int);
bool isEqual(int, int);
bool isGreaterThan(int, int);
bool isLessThan(int, int);
bool isGreaterThanOrEqual(int, int);
bool isLessThanOrEqual(int, int);

int add(int, int, Logger*);
int subtract(int, int, Logger*);
int multiply(int, int, Logger*);
int divide(int, int, Logger*);
int power(int, int, Logger*);
int mod(int, int, Logger*);
int pick_max(int, int, Logger*);
int pick_min(int, int, Logger*);

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
};

// Int object identification
struct AreInt : public ForeignFunc {
	AreInt()
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

// Int Creation
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
	bool (*comparison)(int, int);

	Compare(Logger* pLogger, bool (*pComparison)(int, int) )
		: Base(pLogger)
		, comparison(pComparison)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

// For performing general math
struct SingleOperation : public Base {
	int (*operation)(int, int, Logger*);

	SingleOperation(Logger* pLogger, int (*pOperation)(int, int, Logger*) )
		: Base(pLogger)
		, operation(pOperation)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};
/*
struct SingleParamOperation : public Base {
	int (*operation)(int, Logger*);

	SingleParamOperation(Logger* pLogger, int (*pOperation)(int, Logger*) )
		: Base(pLogger)
		, operation(pOperation)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};*/

// Int Average
struct Avg : public Base {
	Avg(Logger* pLogger)
		: Base(pLogger)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

bool get_abs( const List<Object*>& params, RefPtr<Object>& result, Logger* logger );

}}}
#endif
