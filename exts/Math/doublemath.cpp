// Copyright 2016 Nicolaus Anderson
#include "doublemath.h"
#include <math.h>
#include <cmath>
#include <climits>
#include <sstream>

namespace Cu {
namespace Numeric {
namespace Double {

bool isObjectDouble(const Object* pObject) {
	return ( util::equals( pObject->typeName(), BASIC_PRIMITIVE_TYPENAME )
		&& ((BasicPrimitive*)pObject)->isPrimitiveType( BasicPrimitive::Type::_double ) );
}

bool getDouble(const Object* pObject, double& pValue) {
	if ( util::equals( pObject->typeName(), BASIC_PRIMITIVE_TYPENAME ) ) {
		pValue = ((BasicPrimitive*)pObject)->getAsDouble();
		return true;
	} else
	if ( ! util::equals( pObject->typeName(), NUMBER_TYPENAME ) ) {
		pValue = 0;
		return false;
	}
	unsigned long ui = ((Number*)(pObject))->getAsUnsignedLong();
	pValue = static_cast<double>(ui);
	return true;
}

bool iszero(double p) {
	return (p > -0.00000001) && (p < 0.00000001);
}

bool isEqual(double p, double q) {
	return p - q < 0.00000001 && q - p < 0.00000001;
}

bool isGreaterThan(double p, double q) {
	return p > q;
}

bool isLessThan(double p, double q) {
	return p < q;
}

bool isGreaterThanOrEqual(double p, double q) {
	return p >= q;
}

bool isLessThanOrEqual(double p, double q) {
	return p <= q;
}

double add(double p, double q, Logger* logger) {
	return p + q;
}

double subtract(double p, double q, Logger* logger) {
	return p - q;
}

double multiply(double p, double q, Logger* logger) {
	return p * q;
}

double divide(double p, double q, Logger* logger) {
	if ( iszero(q) ) {
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "Double-division divisor is zero. Ignoring param value.");
		}
		return p;
	} else {
		return p / q;
	}
}

double power(double p, double q, Logger* logger) {
	return powf(p, q);
}

double pick_max(double p, double q, Logger*) {
	return (p > q ? p : q);
}

double pick_min(double p, double q, Logger*) {
	return (p < q ? p : q);
}

double abs_(double d) {
	return ( d < 0 ? -d : d);
}

void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames) {

	AreDouble*			doubleAreDouble			= new AreDouble();
	Create*				doubleCreate			= new Create(&logger);
	Compare*			doubleEqual				= new Compare(&logger, isEqual);
	Compare*			doubleGreaterThanOrEq	= new Compare(&logger, isGreaterThanOrEqual);
	Compare*			doubleLessThanOrEq		= new Compare(&logger, isLessThanOrEqual);
	Compare*			doubleGreaterThan		= new Compare(&logger, isGreaterThan);
	Compare*			doubleLessThan			= new Compare(&logger, isLessThan);
	SingleOperation*	doubleSum = new SingleOperation(&logger, add);
	SingleOperation*	doubleRdc = new SingleOperation(&logger, subtract);
	SingleOperation*	doubleMul = new SingleOperation(&logger, multiply);
	SingleOperation*	doubleDiv = new SingleOperation(&logger, divide);
	SingleOperation*	doublePow = new SingleOperation(&logger, power);
	Avg*				doubleAvg = new Avg(&logger);
	SingleOperation*	doubleMax = new SingleOperation(&logger, pick_max);
	SingleOperation*	doubleMin = new SingleOperation(&logger, pick_min);
	SingleParamOperation*	doubleSin = new SingleParamOperation(&logger, sin);
	SingleParamOperation*	doubleCos = new SingleParamOperation(&logger, cos);
	SingleParamOperation*	doubleTan = new SingleParamOperation(&logger, tan);
	SingleParamOperation*	doubleFloor = new SingleParamOperation(&logger, floor);
	SingleParamOperation*	doubleCeiling = new SingleParamOperation(&logger, ceil);
	SingleParamOperation*	doubleAbs = new SingleParamOperation(&logger, abs_);

	Unimplemented* doubleUnimplemented = new Unimplemented(&logger);

	engine.addForeignFunction(util::String("are_double"), doubleAreDouble);
	engine.addForeignFunction(util::String("double"),	doubleCreate);

	if ( useShortNames ) {
		engine.addForeignFunction(util::String("equal"),	doubleEqual);
		engine.addForeignFunction(util::String("gte"),		doubleGreaterThanOrEq);
		engine.addForeignFunction(util::String("lte"),		doubleLessThanOrEq);
		engine.addForeignFunction(util::String("gt"),		doubleGreaterThan);
		engine.addForeignFunction(util::String("lt"),		doubleLessThan);
		engine.addForeignFunction(util::String("+"),		doubleSum);
		engine.addForeignFunction(util::String("-"),		doubleRdc);
		engine.addForeignFunction(util::String("*"),		doubleMul);
		engine.addForeignFunction(util::String("/"),		doubleDiv);
		engine.addForeignFunction(util::String("%"),		doubleUnimplemented);
		engine.addForeignFunction(util::String("pow"),		doublePow);
		engine.addForeignFunction(util::String("avg"),		doubleAvg);
		engine.addForeignFunction(util::String("max"),		doubleMax);
		engine.addForeignFunction(util::String("min"),		doubleMin);
		engine.addForeignFunction(util::String("sin"),		doubleSin);
		engine.addForeignFunction(util::String("cos"),		doubleCos);
		engine.addForeignFunction(util::String("tan"),		doubleTan);
		engine.addForeignFunction(util::String("floor"),	doubleFloor);
		engine.addForeignFunction(util::String("ceiling"),	doubleCeiling);
		engine.addForeignFunction(util::String("abs"),		doubleAbs);
	} else {
		engine.addForeignFunction(util::String("double_equal"),	doubleEqual);
		engine.addForeignFunction(util::String("double_gte"),	doubleGreaterThanOrEq);
		engine.addForeignFunction(util::String("double_lte"),	doubleLessThanOrEq);
		engine.addForeignFunction(util::String("double_gt"),	doubleGreaterThan);
		engine.addForeignFunction(util::String("double_lt"),	doubleLessThan);
		engine.addForeignFunction(util::String("double+"),		doubleSum);
		engine.addForeignFunction(util::String("double-"),		doubleRdc);
		engine.addForeignFunction(util::String("double*"),		doubleMul);
		engine.addForeignFunction(util::String("double/"),		doubleDiv);
		//engine.addForeignFunction(util::String("double_mod"),	doubleMod); // Only for return auto conversion
		engine.addForeignFunction(util::String("double_pow"),	doublePow);
		engine.addForeignFunction(util::String("double_avg"),	doubleAvg);
		engine.addForeignFunction(util::String("double_max"),	doubleMax);
		engine.addForeignFunction(util::String("double_min"),	doubleMin);
		engine.addForeignFunction(util::String("double_sin"),	doubleSin);
		engine.addForeignFunction(util::String("double_cos"),	doubleCos);
		engine.addForeignFunction(util::String("double_tan"),	doubleTan);
		engine.addForeignFunction(util::String("double_floor"),	doubleFloor);
		engine.addForeignFunction(util::String("double_ceiling"),	doubleCeiling);
		engine.addForeignFunction(util::String("double_abs"),		doubleAbs);
	}

	doubleAreDouble->deref();
	doubleCreate->deref();
	doubleEqual->deref();
	doubleGreaterThanOrEq->deref();
	doubleLessThanOrEq->deref();
	doubleGreaterThan->deref();
	doubleLessThan->deref();
	doubleSum->deref();
	doubleRdc->deref();
	doubleMul->deref();
	doubleDiv->deref();
	//doubleMod->deref();
	doublePow->deref();
	doubleAvg->deref();
	doubleMax->deref();
	doubleMin->deref();
	doubleSin->deref();
	doubleCos->deref();
	doubleTan->deref();
	doubleFloor->deref();
	doubleCeiling->deref();
	doubleAbs->deref();
	doubleUnimplemented->deref();
}

Double::Double(const String& pInitValue)
	: BasicPrimitive( BasicPrimitive::Type::_double )
	, value( 0 )
{
	std::istringstream iss(pInitValue.c_str());
	if ( !( iss >> value ) )
		value = 0;
}

bool Double::equal(const Double& pOther) {
	return value == pOther.value;
}

void Double::writeToString(String& out) const {
	std::ostringstream os;
	if ( os << value )
		out = os.str().c_str();
}

int Double::getAsInt() const {
	if ( value > (double)INT_MAX )
		return INT_MAX;
	return (int)value;
}

unsigned int Double::getAsUnsignedInt() const {
	if ( value < 0 )
		return 0;
	if ( value > (double)UINT_MAX )
		return UINT_MAX;
	return value;
}

unsigned long Double::getAsUnsignedLong() const {
	if ( value < 0 )
		return 0;
	if ( value > (double)ULONG_MAX )
		return ULONG_MAX;
	return value;
}

float Double::getAsFloat() const {
	return value;
}

double Double::writeToDouble() const {
	return value;
}

bool AreDouble::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	bool is = true;
	if ( paramsIter.has() ) {
		do {
			if ( ! isObjectDouble(**paramsIter) ) {
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
	double value = 0;
	// Use only the first param
	if ( params.size() != 1 ) {
		print(LogLevel::warning, "Double-creation requires one param.");
	} else {
		// Write double value
		if ( ! getDouble(*paramsIter, value) ) {
			if ( util::equals( (*paramsIter)->typeName(), "number" ) ) {
				result.setWithoutRef( new Double( ((ObjectNumber*)*paramsIter)->getRawValue() ) );
				return true;
			} else {
				print(LogLevel::warning, "Double-creation parameter was not a compatible number type.");
			}
		}
	}
	result.setWithoutRef(new Double(value));
	return true;
}

bool Unimplemented::call( const List<Object*>& params, RefPtr<Object>& result ) {
	print(LogLevel::warning, "Double function not implemented.");
	result.setWithoutRef(new Double(0));
	return true;
}

bool Compare::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	double value = 0;
	double other_value = 0;
	bool match = true;
	if ( params.size() < 2 ) {
		print(LogLevel::warning, "Double-comparison function should have at least two parameters.");
	} else {
		if ( ! getDouble(*paramsIter, value) ) {
			value = 0;
			print(LogLevel::warning, "Double-comparison function should have only numeric parameters.");
		}
		while ( ++paramsIter ) {
			if ( ! getDouble(*paramsIter, other_value) ) {
				print(LogLevel::warning, "Double-comparison function should have only numeric parameters.");
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
	double total = 0;
	double indie = 0;
	if ( paramsIter.has() ) {
		// First param is for the total
		if ( ! getDouble(*paramsIter, total) ) {
			total = 0;
			print(LogLevel::warning, "Double math function should have only numeric parameters.");
		}
		while( ++paramsIter ) {
			if ( ! getDouble(*paramsIter, indie) ) {
				indie = 0;
				print(LogLevel::warning, "Double math function should have only numeric parameters.");
			}
			total = operation(total, indie, logger);
		}
	}
	// Create a zeroed instance
	result.setWithoutRef(new Double(total));
	return true;
}

bool SingleParamOperation::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	double total = 0;
	if ( paramsIter.has() ) {
		if ( ! getDouble(*paramsIter, total) ) {
			total = 0;
			print(LogLevel::warning, "Double math function should have a single numeric parameter.");
		} else {
			total = operation(total);
		}
	}
	// Create a zeroed instance
	result.setWithoutRef(new Double(total));
	return true;
}

bool Avg::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	double total = 0;
	double indie = 0;
	double used_count = 0;
	if ( paramsIter.has() ) {
		do {
			if ( getDouble(*paramsIter, indie) ) {
				total += indie;
				used_count++;
			} else {
				print(LogLevel::warning, "Double-avg function should have only numeric parameters. Ignoring param value.");
			}
		} while( ++paramsIter );
	}
	// Create a zeroed instance
	if ( used_count > 0 )
		total /= used_count;
	result.setWithoutRef(new Double(total));
	return true;
}

}}}
