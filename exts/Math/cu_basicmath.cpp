// Copyright 2018 Nicolaus Anderson

#define CU_MATH_USE_C_LIMITS 1
//#define CU_MATH_USE_STRING_CONVERSIONS 1

#include "cu_basicmath.h"
#include <math.h>
//#include <climits>

#ifdef CU_MATH_USE_C_LIMITS
#include <limits>
//#else
//#define DECIMAL_ROUNDING_ERROR 0.000001
#endif

namespace Cu {
namespace Numeric {

Integer
getIntegerValue( Object&  object ) {
	if ( isNumericObject(object) ) {
		return ((NumericObject&)object).getIntegerValue();
	}
	return 0;
}

Decimal
getDecimalValue( Object&  object ) {
	if ( isNumericObject(object) ) {
		return ((NumericObject&)object).getDecimalValue();
	}
	return 0;
}

bool
iszero(
	Decimal  p
) {
	// Check based on rounding error
	return p < DECIMAL_ROUNDING_ERROR && p > - DECIMAL_ROUNDING_ERROR;
}

void
addFunctionsToEngine(
	Engine&  engine
) {
	addNewForeignFunc( engine, "int", new IntegerCast() );
	addNewForeignFunc( engine, "dcml", new DecimalCast() );

	addForeignFuncInstance( engine, "num_to_str", ToString );
	addForeignFuncInstance( engine, "infinity", DecimalInfinity );

	addNewForeignFunc( engine, "avg", new Avg() );
	addNewForeignFunc( engine, "min", new PickMin() );
	addNewForeignFunc( engine, "max", new PickMax() );

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

ForeignFunc::Result
IntegerCast::call(
	FFIServices&  ffi
) {
	// Only accepts at most 1 arg, but may have zero
	if ( ! ffi.demandArgCountRange(0,1) )
		return ForeignFunc::NONFATAL;

	Integer  value = 0;
	if ( ffi.getArgCount() == 1 )
		value = getIntegerValue(ffi.arg(0));

	ffi.setNewResult( new IntegerObject( value ) );
	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
DecimalCast::call(
	FFIServices&  ffi
) {
	// Only accepts at most 1 arg, but may have zero
	if ( ! ffi.demandArgCountRange(0,1) )
		return ForeignFunc::NONFATAL;

	Decimal  value = 0;
	if ( ffi.getArgCount() == 1 )
		value = getDecimalValue(ffi.arg(1));

	ffi.setNewResult( new DecimalNumObject( value ) );
	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
ToString(
	FFIServices& ffi
) {
	if ( ! ffi.demandMinArgCount(1) || ! ffi.demandArgType(0, ObjectType::Numeric) )
		return ForeignFunc::NONFATAL;

	UInteger precision = 6;
	if ( ffi.getArgCount() == 2 ) {
		if ( ffi.arg(1).supportsInterface( ObjectType::Numeric ) ) {
			precision = ((NumericObject&)ffi.arg(1)).getIntegerValue();
		}
	}

	NumericObject& arg = (NumericObject&) ffi.arg(0);
	util::String  str;
	if ( arg.supportsInterface( ObjectType::DecimalNum ) ) {
		str.fromDouble( arg.getDecimalValue(), precision );
	} else {
		str.fromInt( arg.getIntegerValue() );
	}
	ffi.setNewResult(
		new StringObject(str)
	);
//#endif // CU_MATH_USE_STRING_CONVERSIONS
	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
DecimalInfinity(
	FFIServices& ffi
) {
#ifdef CU_MATH_USE_C_LIMITS
	ffi.setNewResult(
		new DecimalNumObject( std::numeric_limits<double>::infinity() )
	);
#endif
	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
AreZero::call(
	FFIServices&  ffi
) {
	if ( ! ffi.demandAllArgsType( ObjectType::Numeric ) )
		return NONFATAL;

	NumericObject*  arg;
	bool  is_zero = true;
	UInteger  index = 0;
	for (; index < ffi.getArgCount(); ++index) {
		arg = (NumericObject*)&(ffi.arg(index));
		if ( arg->supportsInterface( ObjectType::DecimalNum ) ) {
			is_zero = iszero( arg->getDecimalValue() );
		} else {
			is_zero = ( 0 == arg->getIntegerValue() );
		}
		if ( ! is_zero ) break;
	}
	ffi.setNewResult( new BoolObject(is_zero) );
	return FINISHED;
}

ForeignFunc::Result
Power::call(
	FFIServices& ffi
) {
	if ( ! ffi.demandAllArgsType( ObjectType::Numeric ) )
		return NONFATAL;

	Decimal  base;
	UInteger  index = 1;
	if ( ffi.getArgCount() >= 1 )
		base = ((NumericObject&)ffi.arg(index)).getDecimalValue();

	for (; index < ffi.getArgCount(); ++index) {
		base = pow( base, ((NumericObject&)ffi.arg(index)).getDecimalValue() );
	}
	ffi.setNewResult( new DecimalNumObject(base) );
	return FINISHED;
}

ForeignFunc::Result
PI::call(
	FFIServices& ffi
) {
	ffi.setNewResult(
		new DecimalNumObject(3.1415926535897932384626433832795028841971693993751)
	);
	return FINISHED;
}

ForeignFunc::Result
SmallPI::call(
	FFIServices& ffi
) {
	ffi.setNewResult(
		new DecimalNumObject(3.1415926)
	);
	return FINISHED;
}

ForeignFunc::Result
PickMin::call( FFIServices& ffi ) {
	if ( ! ffi.demandAllArgsType( NumericObject::object_type ) )
		return ForeignFunc::NONFATAL;

	if ( ! ffi.demandMinArgCount(1) )
		return ForeignFunc::NONFATAL;

	NumericObject*  current = (NumericObject*)&(ffi.arg(0));
	UInteger index = 1;
	for (; index < ffi.getArgCount(); ++index) {
		if ( current->isGreaterThan( (NumericObject&)ffi.arg(index) ) ) {
			current = (NumericObject*)&(ffi.arg(index));
		}
	}

	ffi.setNewResult( current->copy() );
	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
PickMax::call( FFIServices& ffi ) {
	if ( ! ffi.demandAllArgsType( NumericObject::object_type ) )
		return ForeignFunc::NONFATAL;

	if ( ! ffi.demandMinArgCount(1) )
		return ForeignFunc::NONFATAL;

	NumericObject*  current = (NumericObject*)&(ffi.arg(0));
	UInteger index = 1;
	for (; index < ffi.getArgCount(); ++index) {
		if ( ((NumericObject&)ffi.arg(index)).isGreaterThan(*current) ) {
			current = (NumericObject*)&(ffi.arg(index));
		}
	}

	ffi.setNewResult( current->copy() );
	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
Avg::call( FFIServices& ffi ) {
	if ( ! ffi.demandAllArgsType( ObjectType::Numeric ) ) {
		return ForeignFunc::NONFATAL;
	}
	if ( ! ffi.demandMinArgCount(1) )
		return ForeignFunc::NONFATAL;

	if ( ffi.getArgCount() == 1 ) {
		ffi.setNewResult( ffi.arg(0).copy() );
		return ForeignFunc::FINISHED;
	}

	UInteger  count = 1;
	NumericObject*  totalObject = (NumericObject*)&(ffi.arg(0));
	totalObject->ref();
	NumericObject*  nextObject = REAL_NULL;
	for (; count < ffi.getArgCount(); ++count) {
		nextObject = totalObject->add( (NumericObject&)ffi.arg(count) );
		totalObject->deref();
		totalObject = nextObject;
	}
	NumericObject*  countObject = new IntegerObject(count);
	nextObject = totalObject->divide( *countObject );
	ffi.setNewResult( nextObject );
	totalObject->deref();
	countObject->deref();

	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
Sine::call(
	FFIServices&  ffi
) {
	if ( ! ffi.demandArgCount(1) || ! ffi.demandArgType(0, ObjectType::Numeric) )
		return NONFATAL;

	ffi.setNewResult(
		new DecimalNumObject( sin( ((NumericObject&)ffi.arg(0)).getDecimalValue() ) )
	);
	return FINISHED;
}

ForeignFunc::Result
Cosine::call(
	FFIServices&  ffi
) {
	if ( ! ffi.demandArgCount(1) || ! ffi.demandArgType(0, ObjectType::Numeric) )
		return NONFATAL;

	ffi.setNewResult(
		new DecimalNumObject( cos( ((NumericObject&)ffi.arg(0)).getDecimalValue() ) )
	);
	return FINISHED;
}

ForeignFunc::Result
Tangent::call(
	FFIServices&  ffi
) {
	if ( ! ffi.demandArgCount(1) || ! ffi.demandArgType(0, ObjectType::Numeric) )
		return NONFATAL;

	ffi.setNewResult(
		new DecimalNumObject( tan( ((NumericObject&)ffi.arg(0)).getDecimalValue() ) )
	);
	return FINISHED;
}

ForeignFunc::Result
Ceiling::call(
	FFIServices&  ffi
) {
	if ( ! ffi.demandArgCount(1) || ! ffi.demandArgType(0, ObjectType::Numeric) )
		return NONFATAL;

	ffi.setNewResult(
		new DecimalNumObject( ceil( ((NumericObject&)ffi.arg(0)).getDecimalValue() ) )
	);
	return FINISHED;
}

ForeignFunc::Result
Floor::call(
	FFIServices&  ffi
) {
	if ( ! ffi.demandArgCount(1) || ! ffi.demandArgType(0, ObjectType::Numeric) )
		return NONFATAL;

	ffi.setNewResult(
		new DecimalNumObject( floor( ((NumericObject&)ffi.arg(0)).getDecimalValue() ) )
	);
	return FINISHED;
}

}}
