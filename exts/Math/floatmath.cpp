// Copyright 2016 Nicolaus Anderson
#include "floatmath.h"
#include <math.h>
#include <climits>
#include <sstream>

namespace Cu {
namespace Numeric {
namespace Float {

bool isObjectFloat(const Object* pObject) {
	return ( util::equals( pObject->typeName(), BASIC_PRIMITIVE_TYPENAME )
		&& ((BasicPrimitive*)pObject)->isPrimitiveType( BasicPrimitive::Type::_float ) );
}

bool getFloat(const Object* pObject, float& pValue) {
	if ( util::equals( pObject->typeName(), BASIC_PRIMITIVE_TYPENAME ) ) {
		pValue = ((BasicPrimitive*)pObject)->getAsFloat();
		return true;
	} else
	if ( ! util::equals( pObject->typeName(), NUMBER_TYPENAME ) ) {
		pValue = 0;
		return false;
	}
	unsigned long ui = ((Number*)(pObject))->getAsUnsignedLong();
	pValue = static_cast<float>(ui);
	return true;
}

bool iszero(float p) {
	return (p > -0.00001f) && (p < 0.00001f);
}

bool isEqual(float p, float q) {
	return p - q < 0.00001f && q - p < 0.00001f;
}

bool isGreaterThan(float p, float q) {
	return p > q;
}

bool isLessThan(float p, float q) {
	return p < q;
}

bool isGreaterThanOrEqual(float p, float q) {
	return p >= q;
}

bool isLessThanOrEqual(float p, float q) {
	return p <= q;
}

float add(float p, float q, Logger* logger) {
	return p + q;
}

float subtract(float p, float q, Logger* logger) {
	return p - q;
}

float multiply(float p, float q, Logger* logger) {
	return p * q;
}

float divide(float p, float q, Logger* logger) {
	if ( iszero(q) ) {
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "Float-division divisor is zero. Ignoring param value.");
		}
		return p;
	} else {
		return p / q;
	}
}

float power(float p, float q, Logger* logger) {
	return powf(p, q);
}
/*
float mod(float p, float q, Logger* logger) {
	int p0 = (int)p;
	int q0 = (int)q;
	if ( q0 == 0 ) {
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, "Float-mod divisor is zero. Ignoring param value.");
		}
		return p;
	} else {
		return (float) (p0 % q0);
	}
}
*/
float sine(float p, Logger* logger) {
	return sinf(p);
}

float cosine(float p, Logger* logger) {
	return cosf(p);
}

float tangent(float p, Logger* logger) {
	return tanf(p);
}

void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames) {

	AreFloat*			floatAreFloat	= new AreFloat();
	Create*				floatCreate		= new Create(&logger);
	Compare*			floatEqual		= new Compare(&logger, isEqual);
	Compare*			floatGreaterThanOrEq	= new Compare(&logger, isGreaterThanOrEqual);
	Compare*			floatLessThanOrEq		= new Compare(&logger, isLessThanOrEqual);
	Compare*			floatGreaterThan		= new Compare(&logger, isGreaterThan);
	Compare*			floatLessThan			= new Compare(&logger, isLessThan);
	SingleOperation*	floatSum = new SingleOperation(&logger, add);
	SingleOperation*	floatRdc = new SingleOperation(&logger, subtract);
	SingleOperation*	floatMul = new SingleOperation(&logger, multiply);
	SingleOperation*	floatDiv = new SingleOperation(&logger, divide);
	//SingleOperation*	floatMod = new SingleOperation(&logger, mod);
	SingleOperation*	floatPow = new SingleOperation(&logger, power);
	Avg*				floatAvg = new Avg(&logger);
	SingleParamOperation*	floatSin = new SingleParamOperation(&logger, sine);
	SingleParamOperation*	floatCos = new SingleParamOperation(&logger, cosine);
	SingleParamOperation*	floatTan = new SingleParamOperation(&logger, tangent);

	Unimplemented* floatUnimplemented = new Unimplemented(&logger);

	engine.addForeignFunction(util::String("are_float"), floatAreFloat);
	engine.addForeignFunction(util::String("float"),	floatCreate);

	if ( useShortNames ) {
		engine.addForeignFunction(util::String("equal"),	floatEqual);
		engine.addForeignFunction(util::String("gte"),		floatGreaterThanOrEq);
		engine.addForeignFunction(util::String("lte"),		floatLessThanOrEq);
		engine.addForeignFunction(util::String("gt"),		floatGreaterThan);
		engine.addForeignFunction(util::String("lt"),		floatLessThan);
		engine.addForeignFunction(util::String("sum"),		floatSum);
		engine.addForeignFunction(util::String("rdc"),		floatRdc);
		engine.addForeignFunction(util::String("mul"),		floatMul);
		engine.addForeignFunction(util::String("div"),		floatDiv);
		engine.addForeignFunction(util::String("mod"),		floatUnimplemented);
		engine.addForeignFunction(util::String("pow"),		floatPow);
		engine.addForeignFunction(util::String("avg"),		floatAvg);
		engine.addForeignFunction(util::String("sin"),		floatSin);
		engine.addForeignFunction(util::String("cos"),		floatCos);
		engine.addForeignFunction(util::String("tan"),		floatTan);
	} else {
		engine.addForeignFunction(util::String("float_equal"),	floatEqual);
		engine.addForeignFunction(util::String("float_gte"),	floatGreaterThanOrEq);
		engine.addForeignFunction(util::String("float_lte"),	floatLessThanOrEq);
		engine.addForeignFunction(util::String("float_gt"),		floatGreaterThan);
		engine.addForeignFunction(util::String("float_lt"),		floatLessThan);
		engine.addForeignFunction(util::String("float_sum"),	floatSum);
		engine.addForeignFunction(util::String("float_rdc"),	floatRdc);
		engine.addForeignFunction(util::String("float_mul"),	floatMul);
		engine.addForeignFunction(util::String("float_div"),	floatDiv);
		//engine.addForeignFunction(util::String("float_mod"),	floatMod); // Only for return auto conversion
		engine.addForeignFunction(util::String("float_pow"),	floatPow);
		engine.addForeignFunction(util::String("float_avg"),	floatAvg);
		engine.addForeignFunction(util::String("float_sin"),	floatSin);
		engine.addForeignFunction(util::String("float_cos"),	floatCos);
		engine.addForeignFunction(util::String("float_tan"),	floatTan);
	}

	floatAreFloat->deref();
	floatCreate->deref();
	floatEqual->deref();
	floatGreaterThanOrEq->deref();
	floatLessThanOrEq->deref();
	floatGreaterThan->deref();
	floatLessThan->deref();
	floatSum->deref();
	floatRdc->deref();
	floatMul->deref();
	floatDiv->deref();
	//floatMod->deref();
	floatPow->deref();
	floatAvg->deref();
	floatSin->deref();
	floatCos->deref();
	floatTan->deref();
	floatUnimplemented->deref();
}

Float::Float(const String& pInitValue)
	: BasicPrimitive( BasicPrimitive::Type::_float )
	, value( 0 )
{
	std::istringstream iss(pInitValue.c_str());
	if ( !( iss >> value ) )
		value = 0;
}

bool Float::equal(const Float& pOther) {
	return value == pOther.value;
}

void Float::writeToString(String& out) const {
	std::ostringstream os;
	if ( os << value )
		out = os.str().c_str();
}

int Float::getAsInt() const {
	if ( value > (float)INT_MAX )
		return INT_MAX;
	return (int)value;
}

unsigned int Float::getAsUnsignedInt() const {
	if ( value < 0 )
		return 0;
	if ( value > (float)UINT_MAX )
		return UINT_MAX;
	return (unsigned int)value;
}

unsigned long Float::getAsUnsignedLong() const {
	if ( value < 0 )
		return 0;
	if ( value > (float)ULONG_MAX )
		return ULONG_MAX;
	return (unsigned long)value;
}

float Float::getAsFloat() const {
	return value;
}

double Float::writeToDouble() const {
	return value;
}

bool AreFloat::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	bool is = true;
	if ( paramsIter.has() ) {
		do {
			if ( ! isObjectFloat(**paramsIter) ) {
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
	float value = 0;
	// Use only the first param
	if ( params.size() != 1 ) {
		print(LogLevel::warning, "Float-creation requires one param.");
	} else {
		// Write float value
		if ( ! getFloat(*paramsIter, value) ) {
			if ( util::equals( (*paramsIter)->typeName(), "number" ) ) {
				result.setWithoutRef( new Float( ((ObjectNumber*)*paramsIter)->getRawValue() ) );
				return true;
			} else {
				print(LogLevel::warning, "Float-creation parameter was not a compatible number type.");
			}
		}
	}
	result.setWithoutRef(new Float(value));
	return true;
}

bool Unimplemented::call( const List<Object*>& params, RefPtr<Object>& result ) {
	print(LogLevel::warning, "Int function not implemented.");
	result.setWithoutRef(new Float(0));
	return true;
}

bool Compare::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	float value = 0;
	float other_value = 0;
	bool match = true;
	if ( params.size() < 2 ) {
		print(LogLevel::warning, "Float-comparison function should have at least two parameters.");
	} else {
		if ( ! getFloat(*paramsIter, value) ) {
			value = 0;
			print(LogLevel::warning, "Float-comparison function should have only numeric parameters.");
		}
		while ( ++paramsIter ) {
			if ( ! getFloat(*paramsIter, other_value) ) {
				print(LogLevel::warning, "Float-comparison function should have only numeric parameters.");
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
	float total = 0;
	float indie = 0;
	if ( paramsIter.has() ) {
		// First param is for the total
		if ( ! getFloat(*paramsIter, total) ) {
			total = 0;
			print(LogLevel::warning, "Float math function should have only numeric parameters.");
		}
		while( ++paramsIter ) {
			if ( ! getFloat(*paramsIter, indie) ) {
				indie = 0;
				print(LogLevel::warning, "Float math function should have only numeric parameters.");
			}
			total = operation(total, indie, logger);
		}
	}
	// Create a zeroed instance
	result.setWithoutRef(new Float(total));
	return true;
}

bool SingleParamOperation::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	float total = 0;
	if ( paramsIter.has() ) {
		if ( ! getFloat(*paramsIter, total) ) {
			total = 0;
			print(LogLevel::warning, "Float math function should have a single numeric parameter.");
		} else {
			total = operation(total, logger);
		}
	}
	// Create a zeroed instance
	result.setWithoutRef(new Float(total));
	return true;
}

bool Avg::call( const List<Object*>& params, RefPtr<Object>& result )
{
	List<Object*>::ConstIter paramsIter = params.constStart();
	float total = 0;
	float indie = 0;
	float used_count = 0;
	if ( paramsIter.has() ) {
		do {
			if ( getFloat(*paramsIter, indie) ) {
				total += indie;
				used_count++;
			} else {
				print(LogLevel::warning, "Float-avg function should have only numeric parameters. Ignoring param value.");
			}
		} while( ++paramsIter );
	}
	// Create a zeroed instance
	if ( used_count > 0 )
		total /= used_count;
	result.setWithoutRef(new Float(total));
	return true;
}

}}}
