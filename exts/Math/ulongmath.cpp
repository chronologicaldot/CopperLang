// Copyright 2017 Nicolaus Anderson
#include "ulongmath.h"
#include <math.h>
#include <climits>
#include <sstream>

namespace Cu {
namespace Numeric {
namespace ULong {

bool isObjectULong(const Object* pObject) {
	return ( isBasicPrimitive( *pObject )
		&& ((BasicPrimitive*)pObject)->isPrimitiveType( BasicPrimitive::Type::_ulong ) );
}

bool getULong(const Object* pObject, ulong& pValue) {
	if ( isBasicPrimitive( *pObject ) ) {
		pValue = ((BasicPrimitive*)pObject)->getAsUnsignedLong();
		return true;
	} else
	if ( ! isObjectNumber( *pObject ) ) {
		pValue = 0;
		return false;
	}
	pValue = ((ObjectNumber*)(pObject))->getAsUnsignedLong();
	return true;
}

bool iszero(ulong p) {
	return p == 0;
}

void addFunctionsToEngine(
	Engine&		engine,
	bool		useShortNames
) {
	addForeignFuncInstance<AreULong>	(engine, "are_ulong");
	addForeignFuncInstance<Create>		(engine, "ulong");

	if ( useShortNames ) {
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
		addForeignFuncInstance<Modulus>					(engine, "%");
		addForeignFuncInstance<Power>					(engine, "pow");
		addForeignFuncInstance<Avg>						(engine, "avg");
		addForeignFuncInstance<Get_abs>					(engine, "abs");
		addForeignFuncInstance<Incr>					(engine, "++");
		addForeignFuncInstance<Pick_min>				(engine, "min");
		addForeignFuncInstance<Pick_max>				(engine, "max");

		addForeignFuncInstance<Unimplemented>			(engine, "sin");
		addForeignFuncInstance<Unimplemented>			(engine, "cos");
		addForeignFuncInstance<Unimplemented>			(engine, "tan");
	} else {
		addForeignFuncInstance<AreZero>					(engine, "ulong_are_zero");
		addForeignFuncInstance<AreEqual>				(engine, "ulong_equal");
		addForeignFuncInstance<IsGreaterThan>			(engine, "ulong_gt");
		addForeignFuncInstance<IsLessThan>				(engine, "ulong_lt");
		addForeignFuncInstance<IsGreaterThanOrEqual>	(engine, "ulong_gte");
		addForeignFuncInstance<IsLessThanOrEqual>		(engine, "ulong_lte");
		addForeignFuncInstance<Add>						(engine, "ulong+");
		addForeignFuncInstance<Subtract>				(engine, "ulong-");
		addForeignFuncInstance<Multiply>				(engine, "ulong*");
		addForeignFuncInstance<Divide>					(engine, "ulong/");
		addForeignFuncInstance<Modulus>					(engine, "ulong%");
		addForeignFuncInstance<Power>					(engine, "ulong_pow");
		addForeignFuncInstance<Avg>						(engine, "ulong_avg");
		addForeignFuncInstance<Get_abs>					(engine, "ulong_abs");
		addForeignFuncInstance<Incr>					(engine, "ulong++");
		addForeignFuncInstance<Pick_min>				(engine, "ulong_min");
		addForeignFuncInstance<Pick_max>				(engine, "ulong_max");
	}
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
	if ( value > (ulong)UINT_MAX ) {
		return UINT_MAX;
	}
	return (unsigned int)value;
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

bool
AreULong::call(
	FFIServices& ffi
) {
	bool is = false;
	while ( ffi.hasMoreArgs() ) {
		is = isObjectULong(*(ffi.getNextArg()));
		if ( !is ) {
			break;
		}
	}
	ObjectBool* out = new ObjectBool(is);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
Create::call(
	FFIServices& ffi
) {
	// Only accepts at most 1 arg, but may have zero
	Object* arg;
	ulong value = 0;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( ! getULong( *arg, value ) ) {
			ffi.printWarning("ULong-creation argument was not a compatible number type.");
		}
	}
	ULong* out = new ULong(value);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
Unimplemented::call(
	FFIServices& ffi
) {
	ffi.printError("Unimplemented function for integer.");
	return false;
}

bool
AreZero::call(
	FFIServices& ffi
) {
	Object* arg;
	ulong value = 0;
	bool is_zero = true;
	ObjectBool* out = REAL_NULL;
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( getULong(*arg, value) ) {
			is_zero = (value == 0);
		} else {
			// Default return is an empty function
			// Should this be a warning and default to true? Doesn't seem right.
			ffi.printError("ULong-is-zero was not given a parsable number.");
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
	FFIServices& ffi
) {
	Object* arg;
	bool are_equal = true;
	ulong startValue = 0;
	ulong value = 0;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( ! getULong(*arg, startValue) ) {
			ffi.printWarning("ULong are-equal function given non-numeric argument. Cancelling calculation...");
			return false;
		}
	}
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( getULong(*arg, value) ) {
			are_equal = startValue == value;
			if ( !are_equal )
				break;
		} else {
			ffi.printWarning("ULong are-equal function given non-numeric argument. Cancelling calculation...");
			return false;
		}
	}
	ObjectBool* out = new ObjectBool(are_equal);
	ffi.setResult(out);
	out->deref();
	return true;
};

bool
IsGreaterThan::call(
	FFIServices& ffi
) {
	BasicPrimitive* arg = (BasicPrimitive*)(ffi.getNextArg());
	BasicPrimitive* arg2 = (BasicPrimitive*)(ffi.getNextArg());
	ObjectBool* out = new ObjectBool(
		arg->getAsUnsignedLong() > arg2->getAsUnsignedLong()
	);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
IsLessThan::call(
	FFIServices& ffi
) {
	BasicPrimitive* arg = (BasicPrimitive*)(ffi.getNextArg());
	BasicPrimitive* arg2 = (BasicPrimitive*)(ffi.getNextArg());
	ObjectBool* out = new ObjectBool(
		arg->getAsUnsignedLong() < arg2->getAsUnsignedLong()
	);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
IsGreaterThanOrEqual::call(
	FFIServices& ffi
) {
	BasicPrimitive* arg = (BasicPrimitive*)(ffi.getNextArg());
	BasicPrimitive* arg2 = (BasicPrimitive*)(ffi.getNextArg());
	ObjectBool* out = new ObjectBool(
		arg->getAsUnsignedLong() >= arg2->getAsUnsignedLong()
	);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
IsLessThanOrEqual::call(
	FFIServices& ffi
) {
	BasicPrimitive* arg = (BasicPrimitive*)(ffi.getNextArg());
	BasicPrimitive* arg2 = (BasicPrimitive*)(ffi.getNextArg());
	ObjectBool* out = new ObjectBool(
		arg->getAsUnsignedLong() <= arg2->getAsUnsignedLong()
	);
	ffi.setResult(out);
	out->deref();
	return true;
}

// Thanks to pmg for the bounds tests for integers.
// https://stackoverflow.com/questions/199333/how-to-detect-integer-overflow-in-c-c
// and thanks to Franz D. for observing more detection is needed for multiplication.
// Tests have been modified for unsigned long.

bool
Add::call(
	FFIServices& ffi
) {
	Object* arg;
	ulong total = 0;
	ulong indie = 0;
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( getULong(*arg, indie) ) {
#ifdef ENABLE_COPPER_ULONG_BOUNDS_CHECKS
			if ((indie > 0) && (total > ULONG_MAX - indie)) // detect overflow
			{
				ffi.printWarning("ULong-sum overflows. Returning max value.");
				total = INT_MAX;
				break;
			}
#endif
			total += indie;
		}
	}
	ULong* out = new ULong(total);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
Subtract::call(
	FFIServices& ffi
) {
	Object* arg;
	ulong total = 0;
	ulong indie = 0;
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( getULong(*arg, indie) ) {
#ifdef ENABLE_COPPER_ULONG_BOUNDS_CHECKS
			if ((indie > 0) && (total < indie)) // detect underflow
			{
				ffi.printWarning("ULong-subtraction/reduce underflows. Returning min value.");
				total = 0;
				break;
			}
#endif
			total -= indie;
		}
	}
	ULong* out = new ULong(total);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
Multiply::call(
	FFIServices& ffi
) {
	Object* arg;
	ulong total = 1;
	ulong indie = 0;
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( getULong(*arg, indie) ) {
#ifdef ENABLE_COPPER_ULONG_BOUNDS_CHECKS
			if ( ! iszero(indie) ) {
				if ( total > ULONG_MAX / indie ) {
					ffi.printWarning("ULong-multiplication overflows. Returning max value.");
					total = ULONG_MAX;
					break;
				}
				if ( total < 0 / indie ) {
					ffi.printWarning("ULong-multiplication underflows. Returning min value.");
					total = 0;
					break;
				}
			}
#endif
			total *= indie;
		}
	}
	ULong* out = new ULong(total);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
Divide::call(
	FFIServices& ffi
) {
	Object* arg;
	ulong total = 1;
	ulong indie = 0;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		// Get the first arg and save it as the total.
		// Failure results in termination
		if ( ! getULong(*arg, total) ) {
			ffi.printError("ULong division first argument was not a number.");
			return false;
		}
	}
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( getULong(*arg, indie) ) {
			if ( iszero(indie) ) {
				ffi.printWarning("ULong division divisor is zero. Returning maximized value instead.");
				total = (total > 0 ? ULONG_MAX : 0);
				break;
			}
			// else
			total /= indie;
		}
	}
	ULong* out = new ULong(total);
	ffi.setResult(out);
	out->deref();
	return true;
}


bool
Power::call(
	FFIServices& ffi
) {
	BasicPrimitive* arg = (BasicPrimitive*)(ffi.getNextArg());
	ulong argValue1 = arg->getAsUnsignedLong();
	arg = (BasicPrimitive*)(ffi.getNextArg());
	ulong argValue2 = arg->getAsUnsignedLong();
	ulong total = 1;
	double r = pow((double)argValue1, (double)argValue2);
#ifdef ENABLE_COPPER_ULONG_BOUNDS_CHECKS
	if ( r > INT_MAX ) {
		ffi.printWarning("ULong power result is greater than ulong max. Returning max value.");
		total = INT_MAX;
	}
	else if ( r < 0 ) {
		ffi.printWarning("ULong power result is less than ulong min. Returning min value.");
		total = 0;
	} else
#endif
		total = ulong(r);

	ULong* out = new ULong(total);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
Modulus::call(
	FFIServices& ffi
) {
	BasicPrimitive* arg = (BasicPrimitive*)(ffi.getNextArg());
	ulong argValue1 = arg->getAsUnsignedLong();
	arg = (BasicPrimitive*)(ffi.getNextArg());
	ulong argValue2 = arg->getAsUnsignedLong();
	ulong total = 0;
	if ( iszero(argValue2) ) {
		ffi.printWarning("ULong modulus divisor is zero. Ignoring argument value.");
		total = argValue1;
	} else {
		total = argValue1 % argValue2;
	}
	ULong* out = new ULong(total);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
Pick_min::call(
	FFIServices& ffi
) {
	BasicPrimitive* arg = (BasicPrimitive*)(ffi.getNextArg());
	ulong argValue1 = arg->getAsUnsignedLong();
	arg = (BasicPrimitive*)(ffi.getNextArg());
	ulong argValue2 = arg->getAsUnsignedLong();
	ObjectBool* out = new ObjectBool(
		argValue1 > argValue2 ? argValue2 : argValue1
	);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
Pick_max::call(
	FFIServices& ffi
) {
	BasicPrimitive* arg = (BasicPrimitive*)(ffi.getNextArg());
	ulong argValue1 = arg->getAsUnsignedLong();
	arg = (BasicPrimitive*)(ffi.getNextArg());
	ulong argValue2 = arg->getAsUnsignedLong();
	ObjectBool* out = new ObjectBool(
		argValue1 > argValue2 ? argValue1 : argValue2
	);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
Avg::call(
	FFIServices& ffi
) {
	ulong total = 0;
	ulong indie = 0;
	ulong used_count = 0;
	while ( ffi.hasMoreArgs() ) {
		if ( getULong(ffi.getNextArg(), indie) ) {
			total += indie;
			used_count++;
		} else {
			ffi.printWarning("ULong average function given non-numeric arg. Cancelling calculation...");
			return false;
		}
	}
	if ( used_count > 0 )
		total /= used_count;
	ULong* i = new ULong(total);
	ffi.setResult(i);
	i->deref();
	return true;
}

bool
Get_abs::call(
	FFIServices& ffi
) {
	ulong value = 0;
	getULong(ffi.getNextArg(), value);
	ULong* i = new ULong(value);
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

bool
Incr::call(
	FFIServices& ffi
) {
	Object* arg;
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( isObjectULong(*arg) ) {
			((ULong*)arg)->incr();
		} else {
			ffi.printWarning("ULong increment argument was not of type ULong. Ignoring...");
		}
	}
	return true;
}

}}}
