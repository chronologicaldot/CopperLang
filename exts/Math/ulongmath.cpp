// Copyright 2017 Nicolaus Anderson
#include "ulongmath.h"
#include <math.h>
#include <climits>
#include <sstream>

namespace Cu {
namespace Numeric {
namespace ULong {

bool isObjectULong(const Object* pObject) {
	return ( util::equals( pObject->typeName(), BASIC_PRIMITIVE_TYPENAME )
		&& ((BasicPrimitive*)pObject)->isPrimitiveType( BasicPrimitive::Type::_ulong ) );
}

bool getULong(const Object* pObject, ulong& pValue) {
	if ( util::equals( pObject->typeName(), BASIC_PRIMITIVE_TYPENAME ) ) {
		pValue = ((BasicPrimitive*)pObject)->getAsUnsignedLong();
		return true;
	} else
	if ( ! util::equals( pObject->typeName(), NUMBER_TYPENAME ) ) {
		pValue = 0;
		return false;
	}
	pValue = ((Number*)(pObject))->getAsUnsignedLong();
	return true;
}

bool iszero(ulong p) {
	return p == 0;
}

bool isEqual(ulong p, ulong q) {
	return p == q;
}

bool isGreaterThan(ulong p, ulong q) {
	return p > q;
}

bool isLessThan(ulong p, ulong q) {
	return p < q;
}

bool isGreaterThanOrEqual(ulong p, ulong q) {
	return p >= q;
}

bool isLessThanOrEqual(ulong p, ulong q) {
	return p <= q;
}

// Thanks to pmg for the bounds tests for integers.
// https://stackoverflow.com/questions/199333/how-to-detect-integer-overflow-in-c-c
// and thanks to Franz D. for observing more detection is needed for multiplication.
// Tests have been modified for unsigned long.

ulong add(ulong p, ulong q, Logger* logger) {
#ifdef ENABLE_COPPER_ULONG_BOUNDS_CHECKS
	if ((q > 0) && (p > ULONG_MAX - q)) // detect overflow
	{
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "ULong-sum overflows. Returning max value.");
		}
		return ULONG_MAX;
	}
#endif
	return p + q;
}

ulong subtract(ulong p, ulong q, Logger* logger) {
#ifdef ENABLE_COPPER_ULONG_BOUNDS_CHECKS
	if ((q < 0) && (p > ULONG_MAX + q)) // detect overflow
	{
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "ULong-reduce overflows. Returning max value.");
		}
		return ULONG_MAX;
	}
	else if ((q > 0) && (p < q)) // detect underflow
	{
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "ULong-reduce underflows. Returning min value.");
		}
		return 0;
	}
#endif
	return p - q;
}

ulong multiply(ulong p, ulong q, Logger* logger) {
#ifdef ENABLE_COPPER_ULONG_BOUNDS_CHECKS
	if ( q > 0 && p > ULONG_MAX / q ) {
		logger->print(LogLevel::warning, "ULong-multiplication overflows. Returning max value.");
		return ULONG_MAX;
	}
#endif
	return p * q;
}

ulong divide(ulong p, ulong q, Logger* logger) {
	if ( iszero(q) ) {
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "ULong-division divisor is zero. Ignoring param value.");
		}
		return p;
	}
	return p / q;
}

ulong power(ulong p, ulong q, Logger* logger) {
	double r = pow((double)p, (double)q);
	return (ulong)r;
}

ulong mod(ulong p, ulong q, Logger* logger) {
	if ( iszero(q) ) {
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "ULong-modulus divisor is zero. Ignoring param value.");
		}
		return p;
	}
	return p % q;
}

ulong pick_max(ulong p, ulong q, Logger*) {
	return (p > q ? p : q);
}

ulong pick_min(ulong p, ulong q, Logger*) {
	return (p < q ? p : q);
}

void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames) {

	AreULong*			ulongAreULong	= new AreULong();
	Create*				ulongCreate		= new Create(&logger);
	Compare*			ulongEqual		= new Compare(&logger, isEqual);
	Compare*			ulongGreaterThanOrEq	= new Compare(&logger, isGreaterThanOrEqual);
	Compare*			ulongLessThanOrEq		= new Compare(&logger, isLessThanOrEqual);
	Compare*			ulongGreaterThan		= new Compare(&logger, isGreaterThan);
	Compare*			ulongLessThan			= new Compare(&logger, isLessThan);
	SingleOperation*	ulongSum = new SingleOperation(&logger, add);
	SingleOperation*	ulongRdc = new SingleOperation(&logger, subtract);
	SingleOperation*	ulongMul = new SingleOperation(&logger, multiply);
	SingleOperation*	ulongDiv = new SingleOperation(&logger, divide);
	SingleOperation*	ulongMod = new SingleOperation(&logger, mod);
	SingleOperation*	ulongPow = new SingleOperation(&logger, power);
	Avg*				ulongAvg = new Avg(&logger);
	SingleOperation*	ulongMax = new SingleOperation(&logger, pick_max);
	SingleOperation*	ulongMin = new SingleOperation(&logger, pick_min);

	Unimplemented* ulongUnimplemented = new Unimplemented(&logger);

	engine.addForeignFunction(util::String("are_ulong"), ulongAreULong);
	engine.addForeignFunction(util::String("ulong"),	ulongCreate);

	if ( useShortNames ) {
		engine.addForeignFunction(util::String("equal"),ulongEqual);
		engine.addForeignFunction(util::String("gte"),	ulongGreaterThanOrEq);
		engine.addForeignFunction(util::String("lte"),	ulongLessThanOrEq);
		engine.addForeignFunction(util::String("gt"),	ulongGreaterThan);
		engine.addForeignFunction(util::String("lt"),	ulongLessThan);
		engine.addForeignFunction(util::String("+"),	ulongSum);
		engine.addForeignFunction(util::String("-"),	ulongRdc);
		engine.addForeignFunction(util::String("*"),	ulongMul);
		engine.addForeignFunction(util::String("/"),	ulongDiv);
		engine.addForeignFunction(util::String("%"),	ulongMod);
		engine.addForeignFunction(util::String("pow"),	ulongPow);
		engine.addForeignFunction(util::String("avg"),	ulongAvg);
		engine.addForeignFunction(util::String("max"),	ulongMax);
		engine.addForeignFunction(util::String("min"),	ulongMin);
		FFI::simpleFunctionAdd(engine, util::String("abs"),	get_abs, &logger);

		engine.addForeignFunction(util::String("sin"),	ulongUnimplemented);
		engine.addForeignFunction(util::String("cos"),	ulongUnimplemented);
		engine.addForeignFunction(util::String("tan"),	ulongUnimplemented);
	} else {
		engine.addForeignFunction(util::String("ulong_equal"),	ulongEqual);
		engine.addForeignFunction(util::String("ulong_gte"),	ulongGreaterThanOrEq);
		engine.addForeignFunction(util::String("ulong_lte"),	ulongLessThanOrEq);
		engine.addForeignFunction(util::String("ulong_gt"),		ulongGreaterThan);
		engine.addForeignFunction(util::String("ulong_lt"),		ulongLessThan);
		engine.addForeignFunction(util::String("ulong+"),		ulongSum);
		engine.addForeignFunction(util::String("ulong-"),		ulongRdc);
		engine.addForeignFunction(util::String("ulong*"),		ulongMul);
		engine.addForeignFunction(util::String("ulong/"),		ulongDiv);
		engine.addForeignFunction(util::String("ulong%"),		ulongMod);
		engine.addForeignFunction(util::String("ulong_pow"),	ulongPow);
		engine.addForeignFunction(util::String("ulong_avg"),	ulongAvg);
		engine.addForeignFunction(util::String("ulong_max"),	ulongMax);
		engine.addForeignFunction(util::String("ulong_min"),	ulongMin);
		FFI::simpleFunctionAdd(engine, util::String("ulong_abs"),	get_abs, &logger);
	}

	ulongAreULong->deref();
	ulongCreate->deref();
	ulongEqual->deref();
	ulongGreaterThanOrEq->deref();
	ulongLessThanOrEq->deref();
	ulongGreaterThan->deref();
	ulongLessThan->deref();
	ulongSum->deref();
	ulongRdc->deref();
	ulongMul->deref();
	ulongDiv->deref();
	ulongMod->deref();
	ulongPow->deref();
	ulongAvg->deref();
	ulongMax->deref();
	ulongMin->deref();
	ulongUnimplemented->deref();
}


ULong::ULong(const String& pInitValue)
		: BasicPrimitive( BasicPrimitive::Type::_ulong )
		, value(0)
{
	std::istringstream iss(pInitValue.c_str());
	if ( !( iss >> value ) )
		value = 0;
}

bool ULong::equal(const ULong& pOther) {
	return value == pOther.value;
}

void ULong::writeToString(String& out) const {
	std::ostringstream os;
	if ( os << value )
		out = os.str().c_str();
}

int ULong::getAsInt() const {
	if ( value >= (ulong)INT_MAX )
		return INT_MAX;
	return (int)value;
}

unsigned int ULong::getAsUnsignedInt() const {
	if ( value >= 0 ) {
		if ( value > (ulong)UINT_MAX ) {
			return UINT_MAX;
		}
		return (unsigned int)value;
	}
	return 0;
}

unsigned long ULong::getAsUnsignedLong() const {
	return value;
}

float ULong::getAsFloat() const {
	return (float)value;
}

double ULong::getAsDouble() const {
	return (double)value;
}

bool AreULong::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	bool is = true;
	if ( paramsIter.has() ) {
		do {
			if ( ! isObjectULong(**paramsIter) ) {
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
	ulong value = 0;
	// Use only the first param
	if ( params.size() != 1 ) {
		print(LogLevel::warning, "ULong-creation requires one param.");
	} else {
		if ( ! getULong( *paramsIter, value ) ) {
			if ( util::equals( (*paramsIter)->typeName(), "number" ) ) {
				result.setWithoutRef( new ULong( ((ObjectNumber*)*paramsIter)->getRawValue() ) );
				return true;
			} else {
				print(LogLevel::warning, "ULong-creation parameter was not a compatible number type.");
			}
		}
	}
	result.setWithoutRef(new ULong(value));
	return true;
}

bool Unimplemented::call( const List<Object*>& params, RefPtr<Object>& result ) {
	print(LogLevel::warning, "ULong function not implemented.");
	result.setWithoutRef(new ULong(0));
	return true;
}

bool Compare::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	ulong value = 0;
	ulong other_value = 0;
	bool match = true;
	if ( params.size() < 2 ) {
		print(LogLevel::warning, "ULong-comparison function should have at least two parameters.");
	} else {
		if ( ! getULong(*paramsIter, value) ) {
			print(LogLevel::warning, "ULong-comparison function should have only numeric parameters.");
		}
		while ( ++paramsIter ) {
			if ( ! getULong(*paramsIter, other_value) ) {
				print(LogLevel::warning, "ULong-comparison function should have only numeric parameters.");
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
	ulong total = 0;
	ulong indie = 0;
	if ( paramsIter.has() ) {
		// First param is for the total
		if ( ! getULong(*paramsIter, total) ) {
			total = 0;
			print(LogLevel::warning, "ULong math function should have only numeric parameters.");
		}
		while( ++paramsIter ) {
			if ( ! getULong(*paramsIter, indie) ) {
				indie = 0;
				print(LogLevel::warning, "ULong math function should have only numeric parameters.");
			}
			total = operation(total, indie, logger);
		}
	}
	// Create a zeroed instance
	result.setWithoutRef(new ULong(total));
	return true;
}
/*
bool SingleParamOperation::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	ulong total = 0;
	if ( paramsIter.has() ) {
		if ( ! getULong(*paramsIter, total) ) {
			total = 0;
			print(LogLevel::warning, "ULong math function should have a single numeric parameter.");
		} else {
			total = operation(total, logger);
		}
	}
	// Create a zeroed instance
	result.setWithoutRef(new ULong(total));
	return true;
}*/

bool Avg::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	ulong total = 0;
	ulong indie = 0;
	ulong used_count = 0;
	if ( paramsIter.has() ) {
		do {
			if ( getULong(*paramsIter, indie) ) {
				total += indie;
				used_count++;
			} else {
				print(LogLevel::warning, "ULong-avg function should have only numeric parameters. Ignoring param value.");
			}
		} while( ++paramsIter );
	}
	// Create a zeroed instance
	if ( used_count > 0 )
		total /= used_count;
	result.setWithoutRef(new ULong(total));
	return true;
}

bool get_abs( const List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	List<Object*>::ConstIter paramsIter = params.constStart();
	if ( ! paramsIter.has() ) {
		result.setWithoutRef(new ULong(0));
		return true;
	}
	ulong value;
	if ( ! getULong(*paramsIter, value) ) {
		FFI::printWarning(logger, "ULong-abs function requires numeric parameter.");
	}
	result.setWithoutRef(new ULong(value));
	return true;
}


}}}
