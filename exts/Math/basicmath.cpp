// Copyright 2018 Nicolaus Anderson
#include "basicmath.h"
#include <math.h>
#include <climits>
#include <sstream>

namespace Cu {
namespace Numeric {
/*
bool
getInteger(
	const Object*	pObject,
	Integer&		pValue
) {
	if ( isObjectInteger( *pObject ) ) {
		pValue = ((ObjectInteger*)pObject)->getIntegerValue();
		return true;
	}
	if ( isObjectDecimal( *pObject ) ) {
		pValue = static_cast<Integer>(((ObjectDecimal*)pObject)->getDecimalValue());
		return true;
	}
	pValue = 0;
	return false;
}

bool
getDataAsInteger(
	const Object*	pObject,
	Integer&		pValue
) {
	if ( getInteger( pObject, pValue ) )
		return true;

	if ( ! isObjectString( *pObject ) ) {
		pValue = 0;
		return false;
	}
	// Assume a string and try to extract a numeric value
	String sval;
	pObject->writeToString(sval);
	std::istringstream iss(sval.c_str());
	if ( !( iss >> pValue ) )
		pValue = 0;
	return true;
}

bool
getDecimal(
	const Object*	pObject,
	Decimal&		pValue
) {
	if ( isObjectDecimal( *pObject ) ) {
		pValue = ((ObjectDecimal*)pObject)->getDecimalValue();
		return true;
	}
	if ( isObjectInteger( *pObject ) ) {
		pValue = static_cast<Decimal>(((ObjectInteger*)pObject)->getIntegerValue());
		return true;
	}
	pValue = 0;
	return false;
}

bool
getDataAsDecimal(
	const Object*	pObject,
	Decimal&		pValue
) {
	if ( ! getDecimal( pObject, pValue ) ) {
		pValue = 0;
		return false;
	}
	// Assume a string
	String sval;
	pObject->writeToString(sval);
	std::istringstream iss(sval.c_str());
	if ( !( iss >> pValue ) )
		pValue = 0;
	return true;
}
*/
bool
iszero(
	Decimal		p
) {
	// Check based on rounding error
	return p < DECIMAL_ROUNDING_ERROR && p > - DECIMAL_ROUNDING_ERROR;
}

void
addFunctionsToEngine(
	Engine&		engine
) {
	addForeignFuncInstance<IntegerCast>	(engine, "int");
	addForeignFuncInstance<DecimalCast>	(engine, "dcml");
	addForeignFuncInstance<ToString> (engine, "num_to_str");

	addForeignFuncInstance<AreZero>					(engine, "are_zero");
	addForeignFuncInstance<AreEqual>				(engine, "equal");
	addForeignFuncInstance<IsGreaterThan>			(engine, "gt");
	addForeignFuncInstance<IsLessThan>				(engine, "lt");
	addForeignFuncInstance<IsGreaterThanOrEqual>	(engine, "gte");
	addForeignFuncInstance<IsLessThanOrEqual>		(engine, "lte");
/*
	addForeignFuncInstance<Add>						(engine, "+");
	addForeignFuncInstance<Subtract>				(engine, "-");
	addForeignFuncInstance<Multiply>				(engine, "*");
	addForeignFuncInstance<Divide>					(engine, "/");
	addForeignFuncInstance<Modulus>					(engine, "%");
	addForeignFuncInstance<Power>					(engine, "pow");
	addForeignFuncInstance<Avg>						(engine, "avg");
	addForeignFuncInstance<Get_abs>					(engine, "abs");
*/
	addForeignFuncInstance<Incr>					(engine, "++");
	//addForeignFuncInstance<Pick_min>				(engine, "min");
	//addForeignFuncInstance<Pick_max>				(engine, "max");

	// Decimal only
/*
	addForeignFuncInstance<Unimplemented>			(engine, "sin");
	addForeignFuncInstance<Unimplemented>			(engine, "cos");
	addForeignFuncInstance<Unimplemented>			(engine, "tan");
*/
	// TODO: for Decimal
	// floor, ceiling
}

/*
	REFERENCE for a string operations lib
void Int::writeToString(
	String& out
) const {
	std::ostringstream os;
	if ( os << value )
		out = os.str().c_str();
}
*/

bool
IntegerCast::call(
	FFIServices&  ffi
) {
	// Only accepts at most 1 arg, but may have zero
	Object*  arg;
	Integer  value = 0;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		value = arg->getIntegerValue();
	}
	ObjectInteger*  out = new ObjectInteger(value);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
DecimalCast::call(
	FFIServices&  ffi
) {
	// Only accepts at most 1 arg, but may have zero
	Object*  arg;
	Decimal  value = 0;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		value = arg->getDecimalValue();
	}
	ObjectDecimal*  out = new ObjectDecimal(value);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
ToString::call(
	FFIServices& ffi
) {
	Object*  arg;
	std::stringstream  sstream;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		switch( arg->getType() ) {
		case ObjectType::Integer:
			sstream << arg->getIntegerValue();
			break;
		case ObjectType::Decimal:
			sstream << arg->getDecimalValue();
			break;
		default:
			ffi.printWarning("Attempted to use numeric to-string function on incompatible type.");
			break;
		}
	}
	ObjectString*  out = new ObjectString(sstream.str().c_str());
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
Unimplemented::call(
	FFIServices&  ffi
) {
	ffi.printError("Unimplemented function for number type.");
	return false;
}

/*
bool
IsZero::call(
	FFIServices&  ffi
) {
	Object*  arg;
	Integer  iValue = (ObjectInteger*)(ffi.getNextArg())->getIntegerValue();
	ObjectBool*  out = new ObjectBool( iValue == 0 );
	ffi.setResult(out);
	out->deref();
	return true;
}

const char*
IsZero::getParameterType( UInteger index ) {
	return ObjectInteger::StaticType();
}

UInteger
IsZero::getParameterCount() {
	return 1;
}
*/

bool
AreZero::call(
	FFIServices&  ffi
) {
	Object*  arg;
	bool  is_zero = true;
	ObjectBool*  out = REAL_NULL;
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		switch( arg->getType() )
		{
		case ObjectType::Integer:
			is_zero = ( 0 == arg->getIntegerValue() );
			break;

		case ObjectType::Decimal:
			is_zero = iszero( arg->getDecimalValue() );
			break;

		default:
			// Default return is an empty function
			// Should this be a warning and default to true? Doesn't seem right.
			ffi.printError("are_zero was not given a built-in number type.");
			return false;
		}
		if ( !is_zero ) break;
	}
	out = new ObjectBool(is_zero);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
AreEqual::call(
	FFIServices&  ffi
) {
	Object*  arg;
	bool  are_equal = true;
	IntDeciUnion  startValue;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		switch ( arg->getType() )
		{
		case ObjectType::Integer:
			startValue.i_val = arg->getIntegerValue();
			while ( ffi.hasMoreArgs() ) {
				if ( startValue.i_val != ffi.getNextArg()->getIntegerValue() ) {
					are_equal = false;
					break;
				}
			}
			break;
		case ObjectType::Decimal:
			startValue.d_val = arg->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				if ( startValue.d_val != ffi.getNextArg()->getDecimalValue() ) {
					are_equal = false;
					break;
				}
			}
			break;
		default:
			ffi.printWarning("equal function given non-numeric argument to begin. Defaulting to false...");
			break;
		}
	}
	ObjectBool*  out = new ObjectBool(are_equal);
	ffi.setResult(out);
	out->deref();
	return true;
};

bool
IsGreaterThan::call(
	FFIServices& ffi
) {
	bool  result = false;
	Object*  out;
	Object*  arg;
	IntDeciUnion  startValue;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		switch( arg->getType() )
		{
		case ObjectType::Integer:
			result = true;
			startValue.i_val = arg->getIntegerValue();
			while ( ffi.hasMoreArgs() ) {
				result = startValue.i_val > ffi.getNextArg()->getIntegerValue();
				if ( !result ) break;
			}
			break;

		case ObjectType::Decimal:
			result = true;
			startValue.d_val = arg->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				result = startValue.d_val > ffi.getNextArg()->getDecimalValue();
				if ( !result ) break;
			}
			break;

		default:
			ffi.printWarning("is-greater-than function did not receive numeric argument. Defaulting to false.");
			break;
		}
	}
	out = new ObjectBool(result);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
IsLessThan::call(
	FFIServices& ffi
) {
	// All same as GreaterThan::call code but with the comparison and warning message changed
	bool  result = false;
	Object*  out;
	Object*  arg;
	IntDeciUnion  startValue;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		switch( arg->getType() )
		{
		case ObjectType::Integer:
			result = true;
			startValue.i_val = arg->getIntegerValue();
			while ( ffi.hasMoreArgs() ) {
				result = startValue.i_val < ffi.getNextArg()->getIntegerValue();
				if ( !result ) break;
			}
			break;

		case ObjectType::Decimal:
			result = true;
			startValue.d_val = arg->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				result = startValue.d_val < ffi.getNextArg()->getDecimalValue();
				if ( !result ) break;
			}
			break;

		default:
			ffi.printWarning("is-less-than function did not receive numeric argument. Defaulting to false.");
			break;
		}
	}
	out = new ObjectBool(result);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
IsGreaterThanOrEqual::call(
	FFIServices& ffi
) {
	// All same as GreaterThan::call code but with the comparison and warning message changed
	bool  result = false;
	Object*  out;
	Object*  arg;
	IntDeciUnion  startValue;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		switch( arg->getType() )
		{
		case ObjectType::Integer:
			result = true;
			startValue.i_val = arg->getIntegerValue();
			while ( ffi.hasMoreArgs() ) {
				result = startValue.i_val >= ffi.getNextArg()->getIntegerValue();
				if ( !result ) break;
			}
			break;

		case ObjectType::Decimal:
			result = true;
			startValue.d_val = arg->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				result = startValue.d_val >= ffi.getNextArg()->getDecimalValue();
				if ( !result ) break;
			}
			break;

		default:
			ffi.printWarning("is-greater-than-or-equal function did not receive numeric argument. Defaulting to false.");
			break;
		}
	}
	out = new ObjectBool(result);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
IsLessThanOrEqual::call(
	FFIServices& ffi
) {
	// All same as GreaterThan::call code but with the comparison and warning message changed
	bool  result = false;
	Object*  out;
	Object*  arg;
	IntDeciUnion  startValue;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		switch( arg->getType() )
		{
		case ObjectType::Integer:
			result = true;
			startValue.i_val = arg->getIntegerValue();
			while ( ffi.hasMoreArgs() ) {
				result = startValue.i_val <= ffi.getNextArg()->getIntegerValue();
				if ( !result ) break;
			}
			break;

		case ObjectType::Decimal:
			result = true;
			startValue.d_val = arg->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				result = startValue.d_val <= ffi.getNextArg()->getDecimalValue();
				if ( !result ) break;
			}
			break;

		default:
			ffi.printWarning("is-less-than-or-equal function did not receive numeric argument. Defaulting to false.");
			break;
		}
	}
	out = new ObjectBool(result);
	ffi.setResult(out);
	out->deref();
	return true;
}

// Thanks to pmg for the bounds tests for integers.
// https://stackoverflow.com/questions/199333/how-to-detect-integer-overflow-in-c-c
// and thanks to Franz D. for observing more detection is needed for multiplication.

bool
Add::call(
	FFIServices& ffi
) {
/*
	Object* arg;
	int total = 0;
	int indie = 0;
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( getInt(*arg, indie) ) {
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
			if ((indie > 0) && (total > INT_MAX - indie)) // detect overflow
			{
				ffi.printWarning("Int-sum overflows. Returning max value.");
				total = INT_MAX;
				break;
			}
			else if ((indie < 0) && (total < INT_MIN - indie)) // detect underflow
			{
				ffi.printWarning("Int-sum underflows. Returning min value.");
				total = INT_MIN;
				break;
			}
#endif
			total += indie;
		}
	}
	Int* out = new Int(total);
	ffi.setResult(out);
	out->deref();
*/
	return true;
}

bool
Subtract::call(
	FFIServices& ffi
) {
/*
	Object* arg;
	int total = 0;
	int indie = 0;
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( getInt(*arg, indie) ) {
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
			if ((indie < 0) && (total > INT_MAX + indie)) // detect overflow
			{
				ffi.printWarning("Int-subtraction/reduce overflows. Returning max value.");
				total = INT_MAX;
				break;
			}
			else if ((indie > 0) && (total < INT_MIN + indie)) // detect underflow
			{
				ffi.printWarning("Int-subtraction/reduce underflows. Returning min value.");
				total = INT_MIN;
				break;
			}
#endif
			total -= indie;
		}
	}
	Int* out = new Int(total);
	ffi.setResult(out);
	out->deref();
*/
	return true;
}

bool
Multiply::call(
	FFIServices& ffi
) {
/*
	Object* arg;
	int total = 1;
	int indie = 0;
	int indieAbs = 0;
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( getInt(*arg, indie) ) {
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
			indieAbs = (indie > 0 ? indie : -indie);
			if ( ! iszero(indieAbs) ) {
				if ( total > INT_MAX / indieAbs ) {
					ffi.printWarning("Int-multiplication overflows. Returning max value.");
					total = INT_MAX;
					break;
				}
				if ( total < INT_MIN / indieAbs ) {
					ffi.printWarning("Int-multiplication underflows. Returning min value.");
					total = INT_MIN;
					break;
				}
			}
#endif
			total *= indie;
		}
	}
	Int* out = new Int(total);
	ffi.setResult(out);
	out->deref();
*/
	return true;
}

bool
Divide::call(
	FFIServices& ffi
) {
/*
	Object* arg;
	int total = 1;
	int indie = 0;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		// Get the first arg and save it as the total.
		// Failure results in termination
		if ( ! getInt(*arg, total) ) {
			ffi.printError("Int division first argument was not a number.");
			return false;
		}
	}
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( getInt(*arg, indie) ) {
			if ( iszero(indie) ) {
				ffi.printWarning("Int division divisor is zero. Returning maximized value instead.");
				if ( total != 0 )
					total = (total > 0 ? INT_MAX : INT_MIN);
				break;
			}
			else if ( total == INT_MIN && indie == -1 ) {
				ffi.printWarning("Int division is integer min over -1. Returning max positive integer value instead.");
				total = INT_MAX;
				break;
			}
			// else
			total /= indie;
		}
	}
	Int* out = new Int(total);
	ffi.setResult(out);
	out->deref();
*/
	return true;
}


bool
Power::call(
	FFIServices& ffi
) {
/*
	BasicPrimitive* arg = (BasicPrimitive*)(ffi.getNextArg());
	int argValue1 = arg->getAsInt();
	arg = (BasicPrimitive*)(ffi.getNextArg());
	int argValue2 = arg->getAsInt();
	int total = 1;
	double r = pow((double)argValue1, (double)argValue2);
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
	if ( r > INT_MAX ) {
		ffi.printWarning("Int power result is greater than integer max. Returning max value.");
		total = INT_MAX;
	}
	else if ( r < INT_MIN ) {
		ffi.printWarning("Int power result is less than integer min. Returning min value.");
		total = INT_MIN;
	} else
#endif
		total = int(r);

	Int* out = new Int(total);
	ffi.setResult(out);
	out->deref();
*/
	return true;
}

bool
Modulus::call(
	FFIServices& ffi
) {
/*
	BasicPrimitive* arg = (BasicPrimitive*)(ffi.getNextArg());
	int argValue1 = arg->getAsInt();
	arg = (BasicPrimitive*)(ffi.getNextArg());
	int argValue2 = arg->getAsInt();
	int total = 0;
	if ( iszero(argValue2) ) {
		ffi.printWarning("Int modulus divisor is zero. Ignoring argument value.");
		total = argValue1;
	} else {
		total = argValue1 % argValue2;
	}
	Int* out = new Int(total);
	ffi.setResult(out);
	out->deref();
*/
	return true;
}

bool
Pick_min::call(
	FFIServices& ffi
) {
/*
	BasicPrimitive* arg = (BasicPrimitive*)(ffi.getNextArg());
	int argValue1 = arg->getAsInt();
	arg = (BasicPrimitive*)(ffi.getNextArg());
	int argValue2 = arg->getAsInt();
	ObjectBool* out = new ObjectBool(
		argValue1 > argValue2 ? argValue2 : argValue1
	);
	ffi.setResult(out);
	out->deref();
*/
	return true;
}

bool
Pick_max::call(
	FFIServices& ffi
) {
/*
	BasicPrimitive* arg = (BasicPrimitive*)(ffi.getNextArg());
	int argValue1 = arg->getAsInt();
	arg = (BasicPrimitive*)(ffi.getNextArg());
	int argValue2 = arg->getAsInt();
	ObjectBool* out = new ObjectBool(
		argValue1 > argValue2 ? argValue1 : argValue2
	);
	ffi.setResult(out);
	out->deref();
*/
	return true;
}

bool
Avg::call(
	FFIServices& ffi
) {
/*
	int total = 0;
	int indie = 0;
	int used_count = 0;
	while ( ffi.hasMoreArgs() ) {
		if ( getInt(ffi.getNextArg(), indie) ) {
			total += indie;
			used_count++;
		} else {
			ffi.printWarning("Int average function given non-numeric arg. Cancelling calculation...");
			return false;
		}
	}
	if ( used_count > 0 )
		total /= used_count;
	Int* i = new Int(total);
	ffi.setResult(i);
	i->deref();
*/
	return true;
}
/*
bool
Get_abs::call(
	FFIServices& ffi
) {
	int value = 0;
	if ( getInt(ffi.getNextArg(), value) ) {
		value = value < 0 ? -value : value;
	}
	Int* i = new Int(value);
	ffi.setResult(i);
	i->deref();
	return true;
}

ObjectType::Value
Get_abs::getParameterType(
	unsigned int index
) const {
	if ( index == 0 )
		return BasicPrimitive::StaticType();
	return ObjectType::Function;
}

unsigned int
Get_abs::getParameterCount() const {
	return 1;
}
*/
bool
Incr::call(
	FFIServices& ffi
) {
	Object* arg;
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( isObjectInteger(*arg) ) {
			// Should check bounds
			((ObjectInteger*)arg)->setValue(arg->getIntegerValue() + 1);
		} else {
			ffi.printWarning("Increment argument was not of type Integer. Ignoring...");
		}
	}
	return true;
}

}}
