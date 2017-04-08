// Copyright 2016-2017 Nicolaus Anderson
#include "intmath.h"
#include <math.h>
#include <climits>
#include <sstream>

namespace Cu {
namespace Numeric {
namespace Int {

bool isObjectInt(const Object* pObject) {
	return ( util::equals( pObject->typeName(), BASIC_PRIMITIVE_TYPENAME )
		&& ((BasicPrimitive*)pObject)->isPrimitiveType( BasicPrimitive::Type::_int ) );
}

bool getInt(const Object* pObject, int& pValue) {
	if ( util::equals( pObject->typeName(), BASIC_PRIMITIVE_TYPENAME ) ) {
		pValue = ((BasicPrimitive*)pObject)->getAsInt();
		return true;
	}
	else // If not a number, return false
	if ( ! util::equals( pObject->typeName(), NUMBER_TYPENAME ) ) {
		pValue = 0;
		return false;
	}
	unsigned long ui = ((Number*)(pObject))->getAsUnsignedLong();
	if ( ui > INT_MAX ) {
		pValue = INT_MAX;
	} else {
		pValue = static_cast<int>(ui);
	}
	return true;
}

bool iszero(int p) {
	return p == 0;
}

bool isEqual(int p, int q) {
	return p == q;
}

bool isGreaterThan(int p, int q) {
	return p > q;
}

bool isLessThan(int p, int q) {
	return p < q;
}

bool isGreaterThanOrEqual(int p, int q) {
	return p >= q;
}

bool isLessThanOrEqual(int p, int q) {
	return p <= q;
}

// Thanks to pmg for the bounds tests.
// https://stackoverflow.com/questions/199333/how-to-detect-integer-overflow-in-c-c
// and thanks to Franz D. for observing more detection is needed for multiplication.

int add(int p, int q, Logger* logger) {
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
	if ((q > 0) && (p > INT_MAX - q)) // detect overflow
	{
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "Int-sum overflows. Returning max value.");
		}
		return INT_MAX;
	}
	else if ((q < 0) && (p < INT_MIN - q)) // detect underflow
	{
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "Int-sum underflows. Returning min value.");
		}
		return INT_MIN;
	}
#endif
	return p + q;
}

int subtract(int p, int q, Logger* logger) {
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
	if ((q < 0) && (p > INT_MAX + q)) // detect overflow
	{
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "Int-reduce overflows. Returning max value.");
		}
		return INT_MAX;
	}
	else if ((q > 0) && (p < INT_MIN + q)) // detect underflow
	{
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "Int-reduce underflows. Returning min value.");
		}
		return INT_MIN;
	}
#endif
	return p - q;
}

int multiply(int p, int q, Logger* logger) {
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
	int q0 = q;
	if ( q < 0 )
		q0 = -q;
	if ( !iszero(q0) ) {
		if ( p > INT_MAX / q0 ) {
			logger->print(LogLevel::warning, "Int-multiplication overflows. Returning max value.");
			return INT_MAX;
		}
		if ( p < INT_MIN / q0 ) {
			logger->print(LogLevel::warning, "Int-multiplication underflows. Returning min value.");
			return INT_MIN;
		}
	}
#endif
	return p * q;
}

int divide(int p, int q, Logger* logger) {
	if ( iszero(q) ) {
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "Int-division divisor is zero. Ignoring param value.");
		}
		return p;
	}
	else if ( p == INT_MIN && q == -1 ) {
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "Int-division is integer min over -1. Returning max value.");
		}
		return INT_MAX;
	}
	return p / q;
}

int power(int p, int q, Logger* logger) {
	double r = pow((double)p, (double)q);
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
	if ( r > INT_MAX ) {
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "Int-power result is greater than integer max. Returning max value.");
		}
		return INT_MAX;
	}
	else if ( r < INT_MIN ) {
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "Int-power result is less than integer min. Returning min value.");
		}
		return INT_MIN;
	}
#endif
	return (int)r;
}

int mod(int p, int q, Logger* logger) {
	if ( q == 0 ) {
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "Int-modulus divisor is zero. Ignoring param value.");
		}
		return p;
	}
	return p % q;
}

int pick_max(int p, int q, Logger*) {
	return (p > q ? p : q);
}

int pick_min(int p, int q, Logger*) {
	return (p < q ? p : q);
}

void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames) {

	AreInt*				intAreInt	= new AreInt();
	Create*				intCreate	= new Create(&logger);
	Compare*			intEqual	= new Compare(&logger, isEqual);
	Compare*			intGreaterThanOrEq	= new Compare(&logger, isGreaterThanOrEqual);
	Compare*			intLessThanOrEq		= new Compare(&logger, isLessThanOrEqual);
	Compare*			intGreaterThan		= new Compare(&logger, isGreaterThan);
	Compare*			intLessThan			= new Compare(&logger, isLessThan);
	SingleOperation*	intSum = new SingleOperation(&logger, add);
	SingleOperation*	intRdc = new SingleOperation(&logger, subtract);
	SingleOperation*	intMul = new SingleOperation(&logger, multiply);
	SingleOperation*	intDiv = new SingleOperation(&logger, divide);
	SingleOperation*	intMod = new SingleOperation(&logger, mod);
	SingleOperation*	intPow = new SingleOperation(&logger, power);
	Avg*				intAvg = new Avg(&logger);
	SingleOperation*	intMax = new SingleOperation(&logger, pick_max);
	SingleOperation*	intMin = new SingleOperation(&logger, pick_min);
	Incr*				incr	= new Incr(&logger);

	Unimplemented* intUnimplemented = new Unimplemented(&logger);

	engine.addForeignFunction(util::String("are_int"), intAreInt);
	engine.addForeignFunction(util::String("int"),	intCreate);

	if ( useShortNames ) {
		engine.addForeignFunction(util::String("equal"),intEqual);
		engine.addForeignFunction(util::String("gte"),	intGreaterThanOrEq);
		engine.addForeignFunction(util::String("lte"),	intLessThanOrEq);
		engine.addForeignFunction(util::String("gt"),	intGreaterThan);
		engine.addForeignFunction(util::String("lt"),	intLessThan);
		engine.addForeignFunction(util::String("+"),	intSum);
		engine.addForeignFunction(util::String("-"),	intRdc);
		engine.addForeignFunction(util::String("*"),	intMul);
		engine.addForeignFunction(util::String("/"),	intDiv);
		engine.addForeignFunction(util::String("%"),	intMod);
		engine.addForeignFunction(util::String("pow"),	intPow);
		engine.addForeignFunction(util::String("avg"),	intAvg);
		engine.addForeignFunction(util::String("max"),	intMax);
		engine.addForeignFunction(util::String("min"),	intMin);
		FFI::simpleFunctionAdd(engine, util::String("abs"),	get_abs, &logger);

		engine.addForeignFunction(util::String("sin"),	intUnimplemented);
		engine.addForeignFunction(util::String("cos"),	intUnimplemented);
		engine.addForeignFunction(util::String("tan"),	intUnimplemented);

		engine.addForeignFunction(util::String("++"),	incr);
	} else {
		engine.addForeignFunction(util::String("int_equal"),intEqual);
		engine.addForeignFunction(util::String("int_gte"),	intGreaterThanOrEq);
		engine.addForeignFunction(util::String("int_lte"),	intLessThanOrEq);
		engine.addForeignFunction(util::String("int_gt"),	intGreaterThan);
		engine.addForeignFunction(util::String("int_lt"),	intLessThan);
		engine.addForeignFunction(util::String("int+"),		intSum);
		engine.addForeignFunction(util::String("int-"),		intRdc);
		engine.addForeignFunction(util::String("int*"),		intMul);
		engine.addForeignFunction(util::String("int/"),		intDiv);
		engine.addForeignFunction(util::String("int%"),		intMod);
		engine.addForeignFunction(util::String("int_pow"),	intPow);
		engine.addForeignFunction(util::String("int_avg"),	intAvg);
		engine.addForeignFunction(util::String("int_max"),	intMax);
		engine.addForeignFunction(util::String("int_min"),	intMin);
		FFI::simpleFunctionAdd(engine, util::String("int_abs"),	get_abs, &logger);

		engine.addForeignFunction(util::String("int++"),	incr);
	}

	intAreInt->deref();
	intCreate->deref();
	intEqual->deref();
	intGreaterThanOrEq->deref();
	intLessThanOrEq->deref();
	intGreaterThan->deref();
	intLessThan->deref();
	intSum->deref();
	intRdc->deref();
	intMul->deref();
	intDiv->deref();
	intMod->deref();
	intPow->deref();
	intAvg->deref();
	intMax->deref();
	intMin->deref();
	incr->deref();
	intUnimplemented->deref();
}

Int::Int(const String& pInitValue)
		: BasicPrimitive( BasicPrimitive::Type::_int )
		, value(0)
{
	std::istringstream iss(pInitValue.c_str());
	if ( !( iss >> value ) )
		value = 0;
}

bool Int::equal(const Int& pOther) {
	return value == pOther.value;
}

void Int::writeToString(String& out) const {
	std::ostringstream os;
	if ( os << value )
		out = os.str().c_str();
}

int Int::getAsInt() const {
	return value;
}

unsigned int Int::getAsUnsignedInt() const {
	if ( value >= 0 )
		return (unsigned int)value;
	return 0;
}

unsigned long Int::getAsUnsignedLong() const {
	if ( value >= 0 )
		return (unsigned long)value;
	return 0;
}

float Int::getAsFloat() const {
	return (float)value;
}

double Int::getAsDouble() const {
	return (double)value;
}

bool AreInt::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	bool is = true;
	if ( paramsIter.has() ) {
		do {
			if ( ! isObjectInt(**paramsIter) ) {
				is = false;
				break;
			}
		} while( ++paramsIter );
	} else {
		is = false;
	}
	result.setWithoutRef(new ObjectBool(is));
	return true;
}

bool Create::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	int value = 0;
	// Use only the first param
	if ( params.size() != 1 ) {
		print(LogLevel::warning, "Int-creation requires one param.");
	} else {
		if ( ! getInt( *paramsIter, value ) ) {
			if ( util::equals( (*paramsIter)->typeName(), "number" ) ) {
				result.setWithoutRef( new Int( ((ObjectNumber*)*paramsIter)->getRawValue() ) );
				return true;
			} else {
				print(LogLevel::warning, "Int-creation parameter was not a compatible number type.");
			}
		}
	}
	result.setWithoutRef(new Int(value));
	return true;
}

bool Unimplemented::call( const List<Object*>& params, RefPtr<Object>& result ) {
	print(LogLevel::warning, "Int function not implemented.");
	result.setWithoutRef(new Int(0));
	return true;
}

bool Compare::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	int value = 0;
	int other_value = 0;
	bool match = true;
	if ( params.size() < 2 ) {
		print(LogLevel::warning, "Int-comparison function should have at least two parameters.");
	} else {
		if ( ! getInt(*paramsIter, value) ) {
			print(LogLevel::warning, "Int-comparison function should have only numeric parameters.");
		}
		while ( ++paramsIter ) {
			if ( ! getInt(*paramsIter, other_value) ) {
				print(LogLevel::warning, "Int-comparison function should have only numeric parameters.");
				other_value = 0;
			}
			if ( ! comparison(value, other_value) ) {
				match = false;
				break;
			}
		}
	}
	result.setWithoutRef(new ObjectBool(match));
	return true;
}

bool SingleOperation::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	int total = 0;
	int indie = 0;
	if ( paramsIter.has() ) {
		// First param is for the total
		if ( ! getInt(*paramsIter, total) ) {
			total = 0;
			print(LogLevel::warning, "Int math function should have only numeric parameters.");
		}
		while( ++paramsIter ) {
			if ( ! getInt(*paramsIter, indie) ) {
				indie = 0;
				print(LogLevel::warning, "Int math function should have only numeric parameters.");
			}
			total = operation(total, indie, logger);
		}
	}
	// Create a zeroed instance
	result.setWithoutRef(new Int(total));
	return true;
}
/*
bool SingleParamOperation::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	int total = 0;
	if ( paramsIter.has() ) {
		if ( ! getInt(*paramsIter, total) ) {
			total = 0;
			print(LogLevel::warning, "Int math function should have a single numeric parameter.");
		} else {
			total = operation(total, logger);
		}
	}
	// Create a zeroed instance
	result.setWithoutRef(new Int(total));
	return true;
}*/

bool Avg::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	int total = 0;
	int indie = 0;
	int used_count = 0;
	if ( paramsIter.has() ) {
		do {
			if ( getInt(*paramsIter, indie) ) {
				total += indie;
				used_count++;
			} else {
				print(LogLevel::warning, "Int-avg function should have only numeric parameters. Ignoring param value.");
			}
		} while( ++paramsIter );
	}
	// Create a zeroed instance
	if ( used_count > 0 )
		total /= used_count;
	result.setWithoutRef(new Int(total));
	return true;
}

bool get_abs( const List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	List<Object*>::ConstIter paramsIter = params.constStart();
	if ( ! paramsIter.has() ) {
		result.setWithoutRef(new Int(0));
		return true;
	}
	int value;
	if ( ! getInt(*paramsIter, value) ) {
		FFI::printWarning(logger, "Int-abs function requires numeric parameter.");
		result.setWithoutRef(new Int(0));
		return true;
	}
	result.setWithoutRef(new Int( value < 0 ? -value : value ));
	return true;
}

bool Incr::call( const List<Object*>& params, RefPtr<Object>& result ) {
	List<Object*>::ConstIter paramsIter = params.constStart();
	if ( ! paramsIter.has() ) {
		result.setWithoutRef(new Int(0));
		return true;
	}
	int value;
	if ( !isObjectInt(*paramsIter) ) {
		FFI::printWarning(logger, "Int-incr function requires numeric parameter.");
		result.setWithoutRef(new Int(0));
		return true;
	}
	Int* i = (Int*)(*paramsIter);
	i->incr();
	result.setWithoutRef(new Int(0)); // should be i->getAsInt() but that's expensize
	return true;
}

}}}
