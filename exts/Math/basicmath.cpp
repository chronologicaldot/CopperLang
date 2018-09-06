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

	addNewForeignFunc( engine, "avg", new Avg() );
	addNewForeignFunc( engine, "min", new Pick_min() );
	addNewForeignFunc( engine, "max", new Pick_max() );

	// Decimal return only
	addNewForeignFunc( engine, "pow", new Power() );
	addNewForeignFunc( engine, "PI", new PI() );
	addNewForeignFunc( engine, "small_PI", new SmallPI() );

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
		case ObjectType::Numeric:
			switch ( ((NumericObject*)arg)->getSubType() ) {
			case NumericObject::SubType::Integer:
				sstream << arg->getIntegerValue();
				break;
			case NumericObject::SubType::DecimalNum:
				sstream << arg->getDecimalValue();
				break;
			default:
				ffi.printWarning("Could not cast number to string.");
			}
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
		case ObjectType::Numeric:
			switch ( ((NumericObject*)arg)->getSubType() ) {
			case NumericObject::SubType::Integer:
				is_zero = ( 0 == arg->getIntegerValue() );
				break;
			case NumericObject::SubType::DecimalNum:
				is_zero = iszero( arg->getDecimalValue() );
				break;
			default:
				is_zero = ( 0 == arg->getIntegerValue() );
				break;
			}
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

/*
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
*/

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
		case ObjectType::Numeric:
			switch ( ((NumericObject*)obj)->getSubType() ) {

			case NumericObject::SubType::DecimalNum:
				minValue.d_val = obj->getDecimalValue();
				while ( ffi.hasMoreArgs() ) {
					nextValue.d_val = ffi.getNextArg()->getDecimalValue();
					minValue.d_val = ( minValue.d_val <= nextValue.d_val ) ? minValue.d_val : nextValue.d_val;
				}
				ffi.setNewResult( new DecimalNumObject(minValue.d_val) );
				return true;

			case NumericObject::SubType::Integer:
			default:
				minValue.i_val = obj->getIntegerValue();
				while ( ffi.hasMoreArgs() ) {
					nextValue.i_val = ffi.getNextArg()->getIntegerValue();
					minValue.i_val = ( minValue.i_val <= nextValue.i_val ) ? minValue.i_val : nextValue.i_val;
				}
				ffi.setNewResult( new IntegerObject(minValue.i_val) );
				return true;
			}
			break;

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
		case ObjectType::Numeric:
			switch ( ((NumericObject*)obj)->getSubType() ) {

			case NumericObject::SubType::DecimalNum:
				maxValue.d_val = obj->getDecimalValue();
				while ( ffi.hasMoreArgs() ) {
					nextValue.d_val = ffi.getNextArg()->getDecimalValue();
					maxValue.d_val = ( maxValue.d_val >= nextValue.d_val ) ? maxValue.d_val : nextValue.d_val;
				}
				ffi.setNewResult( new DecimalNumObject(maxValue.d_val) );
				return true;

			case NumericObject::SubType::Integer:
			default:
				maxValue.i_val = obj->getIntegerValue();
				while ( ffi.hasMoreArgs() ) {
					nextValue.i_val = ffi.getNextArg()->getIntegerValue();
					maxValue.i_val = ( maxValue.i_val >= nextValue.i_val ) ? maxValue.i_val : nextValue.i_val;
				}
				ffi.setNewResult( new IntegerObject(maxValue.i_val) );
				return true;
			}
			break;

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
		case ObjectType::Numeric:
			switch ( ((NumericObject*)obj)->getSubType() ) {

			case NumericObject::SubType::DecimalNum:
				++valueCount;
				avgValue.d_val = obj->getDecimalValue();
				while ( ffi.hasMoreArgs() ) {
					++valueCount;
					nextValue.d_val = ffi.getNextArg()->getDecimalValue();
					avgValue.d_val += nextValue.d_val;
				}
				ffi.setNewResult( new DecimalNumObject(avgValue.d_val / (Decimal)valueCount) );
				return true;

			case NumericObject::SubType::Integer:
			default:
				++valueCount;
				avgValue.i_val = obj->getIntegerValue();
				while ( ffi.hasMoreArgs() ) {
					++valueCount;
					nextValue.i_val = ffi.getNextArg()->getIntegerValue();
					avgValue.i_val += nextValue.i_val;
				}
				ffi.setNewResult( new IntegerObject(avgValue.i_val / valueCount) );
				return true;
			}
			break;

		default:
			ffi.printWarning("Numeric-average given non-numeric initial argument. Defaulting to empty function.");
			break;
		}
	}
	return true; // Should be set to false if error-faulting enabled
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
