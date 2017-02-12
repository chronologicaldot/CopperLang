// Copyright 2016 Nicolaus Anderson
#ifndef COPPER_DOUBLE_MATH_H
#define COPPER_DOUBLE_MATH_H
#include "../../Copper/src/Strings.h"
#include "../../Copper/src/Copper.h"
#include "BasicPrimitive.h"
#include "../FFIBase.h"

namespace Cu {
namespace Numeric {
namespace Double {

using util::List;
using util::String;
using FFI::Base;

	// You should really just use me. :)
void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames);

bool isObjectDouble(const Object* pObject);
bool getDouble(const Object* pObject, double& pValue); // Returns "true" if a double could be obtained.

bool iszero(double);
bool isEqual(double, double);
bool isGreaterThan(double, double);
bool isLessThan(double, double);
bool isGreaterThanOrEqual(double, double);
bool isLessThanOrEqual(double, double);

double add(double, double, Logger*);
double subtract(double, double, Logger*);
double multiply(double, double, Logger*);
double divide(double, double, Logger*);
double power(double, double, Logger*);
double mod(double, double, Logger*);

double sine(double, Logger*);
double cosine(double, Logger*);
double tangent(double, Logger*);


class Double : public BasicPrimitive {
	double value;

public:
	Double()
		: BasicPrimitive( BasicPrimitive::Type::_double )
		, value(0)
	{}

	Double(double pInitValue)
		: BasicPrimitive( BasicPrimitive::Type::_double )
		, value(pInitValue)
	{}

	Double(const String& pInitValue);

	virtual Object* copy() {
		return new Double(value);
	}

	bool equal(const Double& pOther);

	virtual void writeToString(String& out) const;

	virtual int getAsInt() const;

	virtual unsigned int getAsUnsignedInt() const;

	virtual unsigned long getAsUnsignedLong() const;

	virtual float getAsFloat() const;

	virtual double writeToDouble() const;
};

// Double object identification
struct AreDouble : public ForeignFunc {
	AreDouble()
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

// Double Creation
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
	bool (*comparison)(double, double);

	Compare(Logger* pLogger, bool (*pComparison)(double, double) )
		: Base(pLogger)
		, comparison(pComparison)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

// For performing general math
struct SingleOperation : public Base {
	double (*operation)(double, double, Logger*);

	SingleOperation(Logger* pLogger, double (*pOperation)(double, double, Logger*) )
		: Base(pLogger)
		, operation(pOperation)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

struct SingleParamOperation : public Base {
	double (*operation)(double, Logger*);

	SingleParamOperation(Logger* pLogger, double (*pOperation)(double, Logger*) )
		: Base(pLogger)
		, operation(pOperation)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

// Double Average
struct Avg : public Base {
	Avg(Logger* pLogger)
		: Base(pLogger)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

}}}
#endif
