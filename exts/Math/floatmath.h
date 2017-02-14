// Copyright 2016 Nicolaus Anderson
#ifndef COPPER_FLOAT_MATH_H
#define COPPER_FLOAT_MATH_H
#include "../../Copper/src/Strings.h"
#include "../../Copper/src/Copper.h"
#include "BasicPrimitive.h"
#include "../FFIBase.h"

namespace Cu {
namespace Numeric {
namespace Float {

using util::List;
using util::String;
using FFI::Base;

	// You should really just use me. :)
void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames);

bool isObjectFloat(const Object* pObject);
bool getFloat(const Object* pObject, float& pValue); // Returns "true" if a float could be obtained.

bool iszero(float);
bool isEqual(float, float);
bool isGreaterThan(float, float);
bool isLessThan(float, float);
bool isGreaterThanOrEqual(float, float);
bool isLessThanOrEqual(float, float);

float add(float, float, Logger*);
float subtract(float, float, Logger*);
float multiply(float, float, Logger*);
float divide(float, float, Logger*);
float power(float, float, Logger*);
float mod(float, float, Logger*);
float pick_max(float, float, Logger*);
float pick_min(float, float, Logger*);
float abs_(float);


class Float : public BasicPrimitive {
	float value;

public:
	Float()
		: BasicPrimitive( BasicPrimitive::Type::_float )
		, value(0)
	{}

	Float(float pInitValue)
		: BasicPrimitive( BasicPrimitive::Type::_float )
		, value(pInitValue)
	{}

	Float(const String& pInitValue);

	virtual Object* copy() {
		return new Float(value);
	}

	bool equal(const Float& pOther);

	virtual void writeToString(String& out) const;

	virtual int getAsInt() const;

	virtual unsigned int getAsUnsignedInt() const;

	virtual unsigned long getAsUnsignedLong() const;

	virtual float getAsFloat() const;

	virtual double writeToDouble() const;
};

// Float object identification
struct AreFloat : public ForeignFunc {
	AreFloat()
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

// Float Creation
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
	bool (*comparison)(float, float);

	Compare(Logger* pLogger, bool (*pComparison)(float, float) )
		: Base(pLogger)
		, comparison(pComparison)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

// For performing general math
struct SingleOperation : public Base {
	float (*operation)(float, float, Logger*);

	SingleOperation(Logger* pLogger, float (*pOperation)(float, float, Logger*) )
		: Base(pLogger)
		, operation(pOperation)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

struct SingleParamOperation : public Base {
	float (*operation)(float);

	SingleParamOperation(Logger* pLogger, float (*pOperation)(float) )
		: Base(pLogger)
		, operation(pOperation)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

// Float Average
struct Avg : public Base {
	Avg(Logger* pLogger)
		: Base(pLogger)
	{}

	virtual bool call( const List<Object*>& params, RefPtr<Object>& result );
};

}}}
#endif
