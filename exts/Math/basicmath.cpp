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
	addNewForeignFunc( engine, "int", new IntegerCast() );
	addNewForeignFunc( engine, "dcml", new DecimalCast() );

	addForeignFuncInstance( engine, "num_to_str", ToString );
	addForeignFuncInstance( engine, "infinity", DecimalInfinity );

	addNewForeignFunc( engine, "are_zero", new AreZero() );
	addNewForeignFunc( engine, "equal", new AreEqual() );
	addNewForeignFunc( engine, "gt", new IsGreaterThan() );
	addNewForeignFunc( engine, "lt", new IsLessThan() );
	addNewForeignFunc( engine, "gte", new IsGreaterThanOrEqual() );
	addNewForeignFunc( engine, "lte", new IsLessThanOrEqual() );
	addNewForeignFunc( engine, "+", new Add() );
	addNewForeignFunc( engine, "-", new Subtract() );
	addNewForeignFunc( engine, "*", new Multiply() );
	addNewForeignFunc( engine, "/", new Divide() );
	addNewForeignFunc( engine, "avg", new Avg() );
	addNewForeignFunc( engine, "abs", new Get_abs() );
	addNewForeignFunc( engine, "min", new Pick_min() );
	addNewForeignFunc( engine, "max", new Pick_max() );

	// Integer-return only
	addNewForeignFunc( engine, "%", new Modulus() );

	// Decimal return only
	addNewForeignFunc( engine, "pow", new Power() );
	addNewForeignFunc( engine, "PI", new PI() );
	addNewForeignFunc( engine, "small_PI", new SmallPI() );

	// Integer only
	addNewForeignFunc( engine, "++", new Incr() );
	addNewForeignFunc( engine, "--", new Decr() );

	// Decimal only (Integers are casted)
	addNewForeignFunc(engine, "sin", new Sine() );
	addNewForeignFunc(engine, "cos", new Cosine() );
	addNewForeignFunc(engine, "tan", new Tangent() );
	addNewForeignFunc(engine, "floor", new Floor() );
	addNewForeignFunc(engine, "ceiling", new Ceiling() );
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
	ffi.setNewResult( new IntegerObject(value) );
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
	ffi.setNewResult( new DecimalNumObject(value) );
	return true;
}

bool ToString(
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
	ffi.setNewResult(
		new StringObject(sstream.str().c_str())
	);
	return true;
}

bool
DecimalInfinity(
	FFIServices& ffi
) {
	ffi.setNewResult(
		new DecimalNumObject( std::numeric_limits<double>::infinity() )
	);
	return true;
}

bool
Unimplemented(
	FFIServices&  ffi
) {
	ffi.printError("Unimplemented function for number type.");
	return false;
}

bool
AreZero::call(
	FFIServices&  ffi
) {
	Object*  arg;
	bool  is_zero = true;
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
	ffi.setNewResult(
		new BoolObject(is_zero)
	);
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
	ffi.setNewResult(
		new BoolObject(are_equal)
	);
	return true;
};

bool
IsGreaterThan::call(
	FFIServices& ffi
) {
	bool  result = false;
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
	ffi.setNewResult(
		new BoolObject(result)
	);
	return true;
}

bool
IsLessThan::call(
	FFIServices& ffi
) {
	// All same as GreaterThan::call code but with the comparison and warning message changed
	bool  result = false;
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
	ffi.setNewResult(
		new BoolObject(result)
	);
	return true;
}

bool
IsGreaterThanOrEqual::call(
	FFIServices& ffi
) {
	// All same as GreaterThan::call code but with the comparison and warning message changed
	bool  result = false;
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
	ffi.setNewResult(
		new BoolObject(result)
	);
	return true;
}

bool
IsLessThanOrEqual::call(
	FFIServices& ffi
) {
	// All same as GreaterThan::call code but with the comparison and warning message changed
	bool  result = false;
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
	ffi.setNewResult(
		new BoolObject(result)
	);
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
			ffi.setNewResult( new IntegerObject( startValue.i_val ) );
			return true;

		case ObjectType::Decimal:
			startValue.d_val = obj->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				startValue.d_val += ffi.getNextArg()->getDecimalValue();
			}
			ffi.setNewResult( new DecimalNumObject( startValue.d_val ) );
			return true;

		default:
			ffi.printWarning("Summation not given numeric initial argument. Defaulting to empty function.");
			break;
		}
	}
	return true; // Should be set to false if error-faulting enabled
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
			ffi.setNewResult( new IntegerObject( startValue.i_val ) );
			return true;

		case ObjectType::Decimal:
			startValue.d_val = obj->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				startValue.d_val -= ffi.getNextArg()->getDecimalValue();
			}
			ffi.setNewResult( new DecimalNumObject( startValue.d_val ) );
			return true;

		default:
			ffi.printWarning("Subtraction not given numeric initial argument. Defaulting to empty function.");
			break;
		}
	}
	return true; // Should be set to false if error-faulting enabled
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
			ffi.setNewResult( new IntegerObject( startValue.i_val ) );
			return true;

		case ObjectType::Decimal:
			startValue.d_val = obj->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				startValue.d_val *= ffi.getNextArg()->getDecimalValue();
			}
			ffi.setNewResult( new DecimalNumObject( startValue.d_val ) );
			return true;

		default:
			ffi.printWarning("Multiplication not given numeric initial argument. Defaulting to empty function.");
			break;
		}
	}
	return true; // Should be set to false if error-faulting enabled
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
			ffi.setNewResult( new IntegerObject( startValue.i_val ) );
			return true;

		case ObjectType::Decimal:
			startValue.d_val = obj->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				nextValue = ffi.getNextArg()->getDecimalValue();
				if ( iszero( nextValue ) ) {
					ffi.printWarning("Decimal-division is zero. Returning infinity.");
					startValue.d_val = (startValue.d_val > 0 ?
							std::numeric_limits<Decimal>::infinity()
							: - std::numeric_limits<Decimal>::infinity());
							//std::numeric_limits<Decimal>::max()
							//: std::numeric_limits<Decimal>::lowest());
					break;
				}
				startValue.d_val /= nextValue;
			}
			ffi.setNewResult( new DecimalNumObject( startValue.d_val ) );
			return true;

		default:
			ffi.printWarning("Division not given numeric initial argument. Defaulting to empty function.");
			break;
		}
	}
	return true; // Should be set to false if error-faulting enabled
}

bool
Modulus::call(
	FFIServices& ffi
) {
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
	ffi.setNewResult( new IntegerObject(total) );
	return true;
}

bool
Power::call(
	FFIServices& ffi
) {
	Decimal  base;

	if ( ffi.hasMoreArgs() ) {
		base = ffi.getNextArg()->getDecimalValue();
		while ( ffi.hasMoreArgs() ) {
			base = pow( base, ffi.getNextArg()->getDecimalValue() );
		}
	}
	ffi.setNewResult( new DecimalNumObject(base) );
	return true;
}

bool
PI::call(
	FFIServices& ffi
) {
	ffi.setNewResult(
		new DecimalNumObject(3.1415926535897932384626433832795028841971693993751)
	);
	return true;
}

bool
SmallPI::call(
	FFIServices& ffi
) {
	ffi.setNewResult(
		new DecimalNumObject(3.1415926)
	);
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
			ffi.setNewResult( new IntegerObject(minValue.i_val) );
			return true;

		case ObjectType::Decimal:
			minValue.d_val = obj->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				nextValue.d_val = ffi.getNextArg()->getDecimalValue();
				minValue.d_val = ( minValue.d_val <= nextValue.d_val ) ? minValue.d_val : nextValue.d_val;
			}
			ffi.setNewResult( new DecimalNumObject(minValue.d_val) );
			return true;

		default:
			ffi.printWarning("Numeric-min given non-numeric initial argument. Defaulting to empty function.");
			break;
		}
	}
	return true; // Should be set to false if error-faulting enabled
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
			ffi.setNewResult( new IntegerObject(maxValue.i_val) );
			return true;

		case ObjectType::Decimal:
			maxValue.d_val = obj->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				nextValue.d_val = ffi.getNextArg()->getDecimalValue();
				maxValue.d_val = ( maxValue.d_val >= nextValue.d_val ) ? maxValue.d_val : nextValue.d_val;
			}
			ffi.setNewResult( new DecimalNumObject(maxValue.d_val) );
			return true;

		default:
			ffi.printWarning("Numeric-max given non-numeric initial argument. Defaulting to empty function.");
			break;
		}
	}
	return true; // Should be set to false if error-faulting enabled
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
			ffi.setNewResult( new IntegerObject(avgValue.i_val / valueCount) );
			return true;

		case ObjectType::Decimal:
			++valueCount;
			avgValue.d_val = obj->getDecimalValue();
			while ( ffi.hasMoreArgs() ) {
				++valueCount;
				nextValue.d_val = ffi.getNextArg()->getDecimalValue();
				avgValue.d_val += nextValue.d_val;
			}
			ffi.setNewResult( new DecimalNumObject(avgValue.d_val / (Decimal)valueCount) );
			return true;

		default:
			ffi.printWarning("Numeric-average given non-numeric initial argument. Defaulting to empty function.");
			break;
		}
	}
	return true; // Should be set to false if error-faulting enabled
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
		ffi.setNewResult(
			new DecimalNumObject(value.d_val >= 0? value.d_val : -value.d_val)
		);
		break;

	default:
		value.i_val = obj->getIntegerValue();
		ffi.setNewResult(
			new IntegerObject(value.i_val >= 0? value.i_val : -value.i_val)
		);
		break;
	}
	return true;
}

bool
Incr::call(
	FFIServices& ffi
) {
	Object*  arg;
	Integer  curr;

	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		switch( arg->getType() )
		{
		case ObjectType::Integer:
			curr = arg->getIntegerValue();
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
			if ( curr == std::numeric_limits<Integer>::max() ) {
				ffi.printWarning("Increment integer already at maximum value.");
			} else {
				((IntegerObject*)arg)->setValue(arg->getIntegerValue() + 1);
			}
#else
			((IntegerObject*)arg)->setValue(arg->getIntegerValue() + 1);
#endif
			break;

		case ObjectType::Decimal:
			((DecimalNumObject*)arg)->setValue(arg->getDecimalValue() + 1.0);	

		default:
			ffi.printWarning("Increment argument was not a numeric type. Ignoring.");
			break;
		}
	}
	return true;
}

bool
Decr::call(
	FFIServices& ffi
) {
	Object*  arg;
	Integer curr;

	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		switch( arg->getType() )
		{
		case ObjectType::Integer:
			curr = arg->getIntegerValue();
#ifdef ENABLE_COPPER_INTEGER_BOUNDS_CHECKS
			if ( curr == std::numeric_limits<Integer>::min() ) {
				ffi.printWarning("Decrement integer already at minimum value.");
			} else {
				((IntegerObject*)arg)->setValue(arg->getIntegerValue() - 1);
			}
#else
			((IntegerObject*)arg)->setValue(arg->getIntegerValue() - 1);
#endif
			break;

		case ObjectType::Decimal:
			((DecimalNumObject*)arg)->setValue(arg->getDecimalValue() - 1.0);	

		default:
			ffi.printWarning("Decrement argument was not a numeric type. Ignoring.");
			break;
		}
	}
	return true;
}

bool
Sine::call(
	FFIServices&  ffi
) {
	ffi.setNewResult(
		new DecimalNumObject( sin( ffi.getNextArg()->getDecimalValue() ) )
	);
	return true;
}

bool
Cosine::call(
	FFIServices&  ffi
) {
	ffi.setNewResult(
		new DecimalNumObject( cos( ffi.getNextArg()->getDecimalValue() ) )
	);
	return true;
}

bool
Tangent::call(
	FFIServices&  ffi
) {
	ffi.setNewResult(
		new DecimalNumObject( tan( ffi.getNextArg()->getDecimalValue() ) )
	);
	return true;
}

bool
Ceiling::call(
	FFIServices&  ffi
) {
	ffi.setNewResult(
		new DecimalNumObject( ceil( ffi.getNextArg()->getDecimalValue() ) )
	);
	return true;
}

bool
Floor::call(
	FFIServices&  ffi
) {
	ffi.setNewResult(
		new DecimalNumObject( floor( ffi.getNextArg()->getDecimalValue() ) )
	);
	return true;
}

}}
