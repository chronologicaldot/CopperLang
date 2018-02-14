// Copyright 2018 Nicolaus Anderson
#include "basicmath.h"
#include <math.h>
//#include <climits>
#include <limits>
#include <sstream>

namespace Cu {
namespace Numeric {

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
	addForeignFuncInstance<Add>						(engine, "+");
	addForeignFuncInstance<Subtract>				(engine, "-");
	addForeignFuncInstance<Multiply>				(engine, "*");
	addForeignFuncInstance<Divide>					(engine, "/");
	addForeignFuncInstance<Avg>						(engine, "avg");
	addForeignFuncInstance<Get_abs>					(engine, "abs");
	addForeignFuncInstance<Pick_min>				(engine, "min");
	addForeignFuncInstance<Pick_max>				(engine, "max");

	// Integer-return only
	addForeignFuncInstance<Modulus>					(engine, "%");

	// Decimal return only
	addForeignFuncInstance<Power>					(engine, "pow");

	// Integer only
	addForeignFuncInstance<Incr>					(engine, "++");
	addForeignFuncInstance<Decr>					(engine, "--");

/*
	// Decimal only (Integers are casted)
	addForeignFuncInstance<Unimplemented>			(engine, "sin");
	addForeignFuncInstance<Unimplemented>			(engine, "cos");
	addForeignFuncInstance<Unimplemented>			(engine, "tan");
	addForeignFuncInstance<Unimplemented>			(engine, "floor");
	addForeignFuncInstance<Unimplemented>			(engine, "ceiling");
*/
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
	Object*  obj;
	IntDeciUnion  startValue;
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
	Integer  nextValue;
#endif
	startValue.i_val = 0;

	if ( ffi.hasMoreArgs() ) {
		obj = ffi.getNextArg();
		switch( obj->getType() )
		{
		case ObjectType::Integer:
			startValue.i_val = obj->getIntegerValue();
			while ( ffi.hasMoreArgs() ) {
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
				nextValue = ffi.getNextArg()->getIntegerValue();
				// detect overflow
				if ((nextValue > 0) && (startValue.i_val > std::numeric_limits<Integer>::max() - nextValue))
				{
					ffi.printWarning("Integer-summation overflows. Returning max value.");
					startValue.i_val = std::numeric_limits<Integer>::max();
					break;
				}
				// detect underflow
				else if ((nextValue < 0) && (startValue.i_val < std::numeric_limits<Integer>::min() - nextValue))
				{
					ffi.printWarning("Integer-summation underflows. Returning min value.");
					startValue.i_val = std::numeric_limits<Integer>::min();
					break;
				}
				startValue.i_val += nextValue;
#else
				startValue.i_val += ffi.getNextArg()->getIntegerValue();
#endif
			}
			obj = new ObjectInteger( startValue.i_val );
			ffi.setResult(obj);
			obj->deref();
			return true;

		case ObjectType::Decimal:
			startValue.d_val = obj->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				startValue.d_val += ffi.getNextArg()->getDecimalValue();
			}
			obj = new ObjectDecimal( startValue.d_val );
			ffi.setResult(obj);
			obj->deref();
			return true;

		default:
			ffi.printWarning("Summation not given numeric initial argument. Defaulting to empty function.");
			break;
		}
	}
	return false;
}

bool
Subtract::call(
	FFIServices& ffi
) {
	Object*  obj;
	IntDeciUnion  startValue;
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
	Integer  nextValue;
#endif
	startValue.i_val = 0;

	if ( ffi.hasMoreArgs() ) {
		obj = ffi.getNextArg();
		switch( obj->getType() )
		{
		case ObjectType::Integer:
			startValue.i_val = obj->getIntegerValue();
			while ( ffi.hasMoreArgs() ) {
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
				nextValue = ffi.getNextArg()->getIntegerValue();
				// detect overflow
				if ((nextValue < 0) && (startValue.i_val > std::numeric_limits<Integer>::max() + nextValue))
				{
					ffi.printWarning("Integer-subtraction/reduce overflows. Returning max value.");
					startValue.i_val = std::numeric_limits<Integer>::max();
					break;
				}
				// detect underflow
				else if ((nextValue > 0) && (startValue.i_val < std::numeric_limits<Integer>::min() + nextValue))
				{
					ffi.printWarning("Integer-subtraction/reduce underflows. Returning min value.");
					startValue.i_val = std::numeric_limits<Integer>::min();
					break;
				}
				startValue.i_val -= nextValue;
#else
				startValue.i_val -= ffi.getNextArg()->getIntegerValue();
#endif
			}
			obj = new ObjectInteger( startValue.i_val );
			ffi.setResult(obj);
			obj->deref();
			return true;

		case ObjectType::Decimal:
			startValue.d_val = obj->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				startValue.d_val -= ffi.getNextArg()->getDecimalValue();
			}
			obj = new ObjectDecimal( startValue.d_val );
			ffi.setResult(obj);
			obj->deref();
			return true;

		default:
			ffi.printWarning("Subtraction not given numeric initial argument. Defaulting to empty function.");
			break;
		}
	}
	return false;
}

bool
Multiply::call(
	FFIServices& ffi
) {
	Object*  obj;
	IntDeciUnion  startValue;
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
	Integer  nextValue;
	Integer  nextValueAbs;
#endif
	startValue.i_val = 0;

	if ( ffi.hasMoreArgs() ) {
		obj = ffi.getNextArg();
		switch( obj->getType() )
		{
		case ObjectType::Integer:
			startValue.i_val = obj->getIntegerValue();
			while ( ffi.hasMoreArgs() ) {
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
				nextValue = ffi.getNextArg()->getIntegerValue();
				nextValueAbs = (nextValue > 0 ? nextValue : -nextValue);
				if ( ! iszero(nextValueAbs) ) {
					if ( startValue.i_val > std::numeric_limits<Integer>::max() / nextValueAbs ) {
						ffi.printWarning("Integer-multiplication overflows. Returning max value.");
						startValue.i_val = std::numeric_limits<Integer>::max();
						break;
					}
					if ( startValue.i_val < std::numeric_limits<Integer>::min() / nextValueAbs ) {
						ffi.printWarning("Integer-multiplication underflows. Returning min value.");
						startValue.i_val = std::numeric_limits<Integer>::min();
						break;
					}
				}
				startValue.i_val *= nextValue;
#else
				startValue.i_val *= ffi.getNextArg()->getIntegerValue();
#endif
			}
			obj = new ObjectInteger( startValue.i_val );
			ffi.setResult(obj);
			obj->deref();
			return true;

		case ObjectType::Decimal:
			startValue.d_val = obj->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				startValue.d_val *= ffi.getNextArg()->getDecimalValue();
			}
			obj = new ObjectDecimal( startValue.d_val );
			ffi.setResult(obj);
			obj->deref();
			return true;

		default:
			ffi.printWarning("Multiplication not given numeric initial argument. Defaulting to empty function.");
			break;
		}
	}
	return false;
}

bool
Divide::call(
	FFIServices& ffi
) {
	Object*  obj;
	IntDeciUnion  startValue;
	Integer  nextValue;

	startValue.i_val = 0;

	if ( ffi.hasMoreArgs() ) {
		obj = ffi.getNextArg();
		switch( obj->getType() )
		{
		case ObjectType::Integer:
			startValue.i_val = obj->getIntegerValue();
			while ( ffi.hasMoreArgs() ) {
				nextValue = ffi.getNextArg()->getIntegerValue();
				if ( iszero(nextValue) ) {
					ffi.printWarning("Integer-division divisor is zero. Returning maximized value instead.");
					if ( startValue.i_val != 0 )
						startValue.i_val = (startValue.i_val > 0 ?
							std::numeric_limits<Integer>::max()
							: std::numeric_limits<Integer>::min());
					break;
				}
				else if ( startValue.i_val == std::numeric_limits<Integer>::min() && nextValue == -1 ) {
					ffi.printWarning("Integer-division is integer min over -1. Returning max positive integer value instead.");
					startValue.i_val = std::numeric_limits<Integer>::max();
					break;
				}
				// else
				startValue.i_val /= nextValue;
			}
			obj = new ObjectInteger( startValue.i_val );
			ffi.setResult(obj);
			obj->deref();
			return true;

		case ObjectType::Decimal:
			startValue.d_val = obj->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				nextValue = ffi.getNextArg()->getDecimalValue();
				if ( iszero( nextValue ) ) {
					ffi.printWarning("Decimal-division is zero. Returning infinity.");
					startValue.d_val = (startValue.d_val > 0 ?
							std::numeric_limits<Decimal>::max()
							: std::numeric_limits<Decimal>::min());
					break;
				}
				startValue.d_val /= nextValue;
			}
			obj = new ObjectDecimal( startValue.d_val );
			ffi.setResult(obj);
			obj->deref();
			return true;

		default:
			ffi.printWarning("Division not given numeric initial argument. Defaulting to empty function.");
			break;
		}
	}
	return false;
}

bool
Modulus::call(
	FFIServices& ffi
) {
	Object*  obj;
	Integer  total = 0;
	Integer  nextValue = 0;

	if ( ffi.hasMoreArgs() ) {
		total = ffi.getNextArg()->getIntegerValue();
		while ( ffi.hasMoreArgs() ) {
			nextValue = ffi.getNextArg()->getIntegerValue();
			if ( nextValue == 0 ) {
				ffi.printWarning("Integer-modulus divisor is zero. Ignoring argument value.");
			} else {
				total %= nextValue;
			}
		}
	}
	obj = new ObjectInteger(total);
	ffi.setResult(obj);
	obj->deref();
	return true;
}

bool
Power::call(
	FFIServices& ffi
) {
	Object*  obj;
	Decimal  base;

	if ( ffi.hasMoreArgs() ) {
		base = ffi.getNextArg()->getDecimalValue();
		while ( ffi.hasMoreArgs() ) {
			base = pow( base, ffi.getNextArg()->getDecimalValue() );
		}
	}
	obj = new ObjectDecimal(base);
	ffi.setResult(obj);
	obj->deref();
	return true;
}

bool
Pick_min::call(
	FFIServices& ffi
) {
	Object*  obj;
	IntDeciUnion  minValue;
	IntDeciUnion  nextValue;

	if ( ffi.hasMoreArgs() ) {
		obj = ffi.getNextArg();
		switch( obj->getType() )
		{
		case ObjectType::Integer:
			minValue.i_val = obj->getIntegerValue();
			while ( ffi.hasMoreArgs() ) {
				nextValue.i_val = ffi.getNextArg()->getIntegerValue();
				minValue.i_val = ( minValue.i_val <= nextValue.i_val ) ? minValue.i_val : nextValue.i_val;
			}
			obj = new ObjectInteger(minValue.i_val);
			ffi.setResult(obj);
			obj->deref();
			return true;

		case ObjectType::Decimal:
			minValue.d_val = obj->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				nextValue.d_val = ffi.getNextArg()->getDecimalValue();
				minValue.d_val = ( minValue.d_val <= nextValue.d_val ) ? minValue.d_val : nextValue.d_val;
			}
			obj = new ObjectDecimal(minValue.d_val);
			ffi.setResult(obj);
			obj->deref();
			return true;

		default:
			ffi.printWarning("Numeric-min given non-numeric initial argument. Defaulting to empty function.");
			break;
		}
	}
	return false;
}

bool
Pick_max::call(
	FFIServices& ffi
) {
	Object*  obj;
	IntDeciUnion  maxValue;
	IntDeciUnion  nextValue;

	if ( ffi.hasMoreArgs() ) {
		obj = ffi.getNextArg();
		switch( obj->getType() )
		{
		case ObjectType::Integer:
			maxValue.i_val = obj->getIntegerValue();
			while ( ffi.hasMoreArgs() ) {
				nextValue.i_val = ffi.getNextArg()->getIntegerValue();
				maxValue.i_val = ( maxValue.i_val >= nextValue.i_val ) ? maxValue.i_val : nextValue.i_val;
			}
			obj = new ObjectInteger(maxValue.i_val);
			ffi.setResult(obj);
			obj->deref();
			return true;

		case ObjectType::Decimal:
			maxValue.d_val = obj->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				nextValue.d_val = ffi.getNextArg()->getDecimalValue();
				maxValue.d_val = ( maxValue.d_val >= nextValue.d_val ) ? maxValue.d_val : nextValue.d_val;
			}
			obj = new ObjectDecimal(maxValue.d_val);
			ffi.setResult(obj);
			obj->deref();
			return true;

		default:
			ffi.printWarning("Numeric-max given non-numeric initial argument. Defaulting to empty function.");
			break;
		}
	}
	return false;
}

bool
Avg::call(
	FFIServices& ffi
) {
	Object*  obj;
	Integer  valueCount = 0;
	IntDeciUnion  avgValue;
	IntDeciUnion  nextValue;

	if ( ffi.hasMoreArgs() ) {
		obj = ffi.getNextArg();
		switch( obj->getType() )
		{
		case ObjectType::Integer:
			++valueCount;
			avgValue.i_val = obj->getIntegerValue();
			while ( ffi.hasMoreArgs() ) {
				++valueCount;
				nextValue.i_val = ffi.getNextArg()->getIntegerValue();
				avgValue.i_val += nextValue.i_val;
			}
			obj = new ObjectInteger(avgValue.i_val / valueCount);
			ffi.setResult(obj);
			obj->deref();
			return true;

		case ObjectType::Decimal:
			++valueCount;
			avgValue.d_val = obj->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				++valueCount;
				nextValue.d_val = ffi.getNextArg()->getDecimalValue();
				avgValue.d_val += nextValue.d_val;
			}
			obj = new ObjectDecimal(avgValue.d_val / (Decimal)valueCount);
			ffi.setResult(obj);
			obj->deref();
			return true;

		default:
			ffi.printWarning("Numeric-average given non-numeric initial argument. Defaulting to empty function.");
			break;
		}
	}
	return false;
}

bool
Get_abs::call(
	FFIServices& ffi
) {
	Object*  obj;
	IntDeciUnion  value;
	if ( ! ffi.hasMoreArgs() ) {
		ffi.printWarning("Numeric-abs called with no arguments.");
		return false;
	}

	obj = ffi.getNextArg();
	switch( obj->getType() )
	{
	case ObjectType::Decimal:
		value.d_val = obj->getDecimalValue();
		obj = new ObjectDecimal(value.d_val >= 0? value.d_val : -value.d_val);
		ffi.setResult(obj);
		obj->deref();
		break;

	default:
		value.i_val = obj->getIntegerValue();
		obj = new ObjectInteger(value.i_val >= 0? value.i_val : -value.i_val);
		ffi.setResult(obj);
		obj->deref();
		break;
	}
	return true;
}

bool
Incr::call(
	FFIServices& ffi
) {
	Object*  arg;

	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( isObjectInteger(*arg) ) {
			// Should check bounds
			((ObjectInteger*)arg)->setValue(arg->getIntegerValue() + 1);
		} else {
			ffi.printWarning("Increment argument was not of type Integer. Ignoring.");
		}
	}
	return true;
}

bool
Decr::call(
	FFIServices& ffi
) {
	Object*  arg;

	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( isObjectInteger(*arg) ) {
			// Should check bounds
			((ObjectInteger*)arg)->setValue(arg->getIntegerValue() - 1);
		} else {
			ffi.printWarning("Decrement argument was not of type Integer. Ignoring.");
		}
	}
	return true;
}

}}
