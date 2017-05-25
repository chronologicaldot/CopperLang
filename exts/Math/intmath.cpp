// Copyright 2016-2017 Nicolaus Anderson
#include "intmath.h"
#include <math.h>
#include <climits>
#include <sstream>

namespace Cu {
namespace Numeric {
namespace Int {

bool isObjectInt(const Object* pObject) {
	return ( isBasicPrimitive( *pObject )
		&& ((BasicPrimitive*)pObject)->isPrimitiveType( BasicPrimitive::Type::_int ) );
}

bool getInt(const Object* pObject, int& pValue) {
	if ( isBasicPrimitive( *pObject ) ) {
		pValue = ((BasicPrimitive*)pObject)->getAsInt();
		return true;
	}
	// If not a number, return false
	if ( ! isObjectNumber( *pObject ) ) {
		pValue = 0;
		return false;
	}
	unsigned long ui = ((ObjectNumber*)(pObject))->getAsUnsignedLong();
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

void addFunctionsToEngine(
	Engine&		engine,
	bool		useShortNames
) {
	addForeignFuncInstance<AreInt>	(engine, "are_int");
	addForeignFuncInstance<Create>	(engine, "int");

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
		addForeignFuncInstance<AreZero>					(engine, "int_are_zero");
		addForeignFuncInstance<AreEqual>				(engine, "int_equal");
		addForeignFuncInstance<IsGreaterThan>			(engine, "int_gt");
		addForeignFuncInstance<IsLessThan>				(engine, "int_lt");
		addForeignFuncInstance<IsGreaterThanOrEqual>	(engine, "int_gte");
		addForeignFuncInstance<IsLessThanOrEqual>		(engine, "int_lte");
		addForeignFuncInstance<Add>						(engine, "int+");
		addForeignFuncInstance<Subtract>				(engine, "int-");
		addForeignFuncInstance<Multiply>				(engine, "int*");
		addForeignFuncInstance<Divide>					(engine, "int/");
		addForeignFuncInstance<Modulus>					(engine, "int%");
		addForeignFuncInstance<Power>					(engine, "int_pow");
		addForeignFuncInstance<Avg>						(engine, "int_avg");
		addForeignFuncInstance<Get_abs>					(engine, "int_abs");
		addForeignFuncInstance<Incr>					(engine, "int++");
		addForeignFuncInstance<Pick_min>				(engine, "int_min");
		addForeignFuncInstance<Pick_max>				(engine, "int_max");
	}
}

Int::Int(
	const String& pInitValue
)
		: BasicPrimitive( BasicPrimitive::Type::_int )
		, value(0)
{
	std::istringstream iss(pInitValue.c_str());
	if ( !( iss >> value ) )
		value = 0;
}

bool Int::equal(
	const Int& pOther
) {
	return value == pOther.value;
}

void Int::writeToString(
	String& out
) const {
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

bool
AreInt::call(
	FFIServices& ffi
) {
	bool is = false;
	while ( ffi.hasMoreArgs() ) {
		is = isObjectInt(*(ffi.getNextArg()));
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
	int value = 0;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( ! getInt( *arg, value ) ) {
			ffi.printWarning("Int-creation argument was not a compatible number type.");			
		}
	}
	Int* out = new Int(value);
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
	int value = 0;
	bool is_zero = false;
	ObjectBool* out = REAL_NULL;
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( getInt(*arg, value) ) {
			is_zero = (value == 0);
		} else {
			// Default return is an empty function
			// Should this be a warning and default to true? Doesn't seem right.
			ffi.printError("Int-is-zero was not given a parsable number.");
			return false;
		}
		if ( is_zero ) break;
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
	int startValue = 0;
	int value = 0;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( ! getInt(*arg, startValue) ) {
			ffi.printWarning("Int are-equal function given non-numeric argument. Cancelling calculation...");
			return false;
		}
	}
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( getInt(*arg, value) ) {
			are_equal = startValue == value;
			if ( !are_equal )
				break;
		} else {
			ffi.printWarning("Int are-equal function given non-numeric argument. Cancelling calculation...");
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
		arg->getAsInt() > arg2->getAsInt()
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
		arg->getAsInt() < arg2->getAsInt()
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
		arg->getAsInt() >= arg2->getAsInt()
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
		arg->getAsInt() <= arg2->getAsInt()
	);
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
	return true;
}

bool
Subtract::call(
	FFIServices& ffi
) {
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
	return true;
}

bool
Multiply::call(
	FFIServices& ffi
) {
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
	return true;
}

bool
Divide::call(
	FFIServices& ffi
) {
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
	return true;
}


bool
Power::call(
	FFIServices& ffi
) {
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
	return true;
}

bool
Modulus::call(
	FFIServices& ffi
) {
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
	return true;
}

bool
Pick_min::call(
	FFIServices& ffi
) {
	BasicPrimitive* arg = (BasicPrimitive*)(ffi.getNextArg());
	int argValue1 = arg->getAsInt();
	arg = (BasicPrimitive*)(ffi.getNextArg());
	int argValue2 = arg->getAsInt();
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
	int argValue1 = arg->getAsInt();
	arg = (BasicPrimitive*)(ffi.getNextArg());
	int argValue2 = arg->getAsInt();
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
	return true;
}

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

bool
Incr::call(
	FFIServices& ffi
) {
	Object* arg;
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( isObjectInt(*arg) ) {
			((Int*)arg)->incr();
		} else {
			ffi.printWarning("Int increment argument was not of type Int. Ignoring...");
		}
	}
	return true;
}

}}}
