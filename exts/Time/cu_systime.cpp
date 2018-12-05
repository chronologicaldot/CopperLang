// Copyright 2018 Nicolaus Anderson

#include "cu_systime.h"

namespace Cu {
 namespace Time {

void
addFunctionsToEngine(
	Engine&		engine
) {
	addNewForeignFunc( engine, "get_time", new GetClockTime() );
	addForeignFuncInstance( engine, "get_seconds", GetSeconds );
	addForeignFuncInstance( engine, "get_milliseconds", GetMilliseconds );
	addForeignFuncInstance( engine, "get_nanoseconds", GetNanoseconds );
}

void
ClockTime::setValue( const NumericObject&  other ) {
	if ( other.getSubType() == ClockTime::StaticClockTimeType() ) {
		setValue( ((ClockTime&)other).data );
	}
	else {
		data.tv_sec = other.getIntegerValue();
	}
}

bool
ClockTime::isEqualTo(const NumericObject&  other ) {
	if ( other.getSubType() == ClockTime::StaticClockTimeType() ) {
		return isEqualTo( ((ClockTime&)other).data );
	}
	return getIntegerValue() == other.getIntegerValue();
}

bool
ClockTime::isGreaterThan(const NumericObject&  other ) {
	if ( other.getSubType() == ClockTime::StaticClockTimeType() ) {
		return isGreaterThan( ((ClockTime&)other).data );
	}
	return getIntegerValue() > other.getIntegerValue();
}

bool
ClockTime::isGreaterOrEqual(const NumericObject&  other ) {
	if ( other.getSubType() == ClockTime::StaticClockTimeType() ) {
		return isGreaterOrEqual( ((ClockTime&)other).data );
	}
	return getIntegerValue() >= other.getIntegerValue();
}

NumericObject*
ClockTime::absValue() const {
	return (NumericObject*) new ClockTime(data);
}

NumericObject*
ClockTime::add(const NumericObject&  other ) {
	if ( other.getSubType() == ClockTime::StaticClockTimeType() ) {
		return add( ((ClockTime&)other).data );
	}
	timespec  total;
	total.tv_sec = data.tv_sec + other.getIntegerValue();
	total.tv_nsec = data.tv_nsec;
	//return new IntegerObject( getIntegerValue() + other.getIntegerValue() );
	return new ClockTime( total );
}

NumericObject*
ClockTime::subtract(const NumericObject&  other ) {
	if ( other.getSubType() == ClockTime::StaticClockTimeType() ) {
		return subtract( ((ClockTime&)other).data );
	}
	timespec  total;
	if ( data.tv_sec < other.getIntegerValue() ) {
		total.tv_sec = 0;
	} else {
		total.tv_sec = data.tv_sec - other.getIntegerValue();
	}
	total.tv_nsec = data.tv_nsec;
	return new ClockTime( total );
}

NumericObject*
ClockTime::multiply(const NumericObject&  other ) {
	timespec  total;
	data.tv_sec = (time_t)(other.getDecimalValue() * data.tv_sec);
	data.tv_nsec = (long)(other.getDecimalValue() * data.tv_nsec);
	return new ClockTime( total );
}

NumericObject*
ClockTime::divide(const NumericObject&  other ) {
	timespec  total;
	data.tv_sec = (time_t)((Decimal) data.tv_sec / other.getDecimalValue());
	data.tv_nsec = (long)((Decimal) data.tv_nsec / other.getDecimalValue());
	return new ClockTime( total );
}

NumericObject*
ClockTime::modulus(const NumericObject&  other ) {
	timespec  total;
	data.tv_sec = (time_t)((Integer) data.tv_sec % other.getIntegerValue());
	data.tv_nsec = (long)((Integer) data.tv_nsec % other.getIntegerValue());
	return new ClockTime( total );
}

bool
ClockTime::isEqualTo(const timespec& other) {
	return data.tv_sec == other.tv_sec && data.tv_nsec == other.tv_nsec;
}

bool
ClockTime::isGreaterThan(const timespec& other) {
	if ( data.tv_sec > other.tv_sec )
		return true;
	if ( data.tv_sec == other.tv_sec )
		return data.tv_nsec > other.tv_nsec;
	return false;
}

bool
ClockTime::isGreaterOrEqual(const timespec& other) {
	if ( data.tv_sec > other.tv_sec )
		return true;
	if ( data.tv_sec == other.tv_sec )
		return data.tv_nsec >= other.tv_nsec;
	return false;
}

NumericObject*
ClockTime::add(const timespec& other) {
	timespec  total;
	total.tv_sec = data.tv_sec + other.tv_sec;
	total.tv_nsec = data.tv_nsec + other.tv_nsec;
	return new ClockTime(total);
}

NumericObject*
ClockTime::subtract(const timespec& other) {
	timespec  total;
	// Keep the minimum at zero
	if ( data.tv_sec < other.tv_sec ) {
		total.tv_sec = 0;
		total.tv_nsec = 0;
	}
	else {
		total.tv_sec = data.tv_sec - other.tv_sec;
		if ( data.tv_nsec < other.tv_nsec ) {
			if ( total.tv_sec > 0 ) {
				total.tv_sec -= 1;
				total.tv_nsec = data.tv_nsec + 1000000000 - other.tv_nsec;
			} else {
				total.tv_nsec = 0;
			}
		}
		else {
			total.tv_nsec = data.tv_nsec - other.tv_nsec;
		}
	}
	return new ClockTime(total);
}

bool
GetClockTime::call(
	FFIServices&  ffi
) {
	timespec currTime;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &currTime);
	ffi.setNewResult( new ClockTime(currTime) );
	return true;
}

bool
GetSeconds( FFIServices& ffi ) {
	if ( ! ffi.demandAllArgsType( ObjectType::Numeric, true ) )
		return false;

	NumericObject*  arg;
	Integer  totalTime = 0;
	UInteger  i = 0;

	for (; i < ffi.getArgCount(); ++i) {
		arg = (NumericObject*)&(ffi.arg(i));
		if ( arg->getSubType() == ClockTime::StaticClockTimeType() ) {
			totalTime += ((ClockTime*)arg)->getSecondsValue();
		}
	}

	ffi.setNewResult( new IntegerObject( totalTime ) );
	return true;
}

bool
GetMilliseconds( FFIServices& ffi ) {
	if ( ! ffi.demandAllArgsType( ObjectType::Numeric, true ) )
		return false;

	NumericObject*  arg;
	Integer  totalTime = 0;
	UInteger  i = 0;

	for (; i < ffi.getArgCount(); ++i) {
		arg = (NumericObject*)&(ffi.arg(i));
		if ( arg->getSubType() == ClockTime::StaticClockTimeType() ) {
			totalTime += ((ClockTime*)arg)->getMillisecondsValue();
		}
	}

	ffi.setNewResult( new IntegerObject( totalTime ) );
	return true;
}

bool
GetNanoseconds( FFIServices& ffi ) {
	if ( ! ffi.demandAllArgsType( ObjectType::Numeric, true ) )
		return false;

	NumericObject*  arg;
	Integer  totalTime = 0;
	UInteger  i = 0;

	for (; i < ffi.getArgCount(); ++i) {
		arg = (NumericObject*)&(ffi.arg(i));
		if ( arg->getSubType() == ClockTime::StaticClockTimeType() ) {
			totalTime += ((ClockTime*)arg)->getNanosecondsValue();
		}
	}

	ffi.setNewResult( new IntegerObject( totalTime ) );
	return true;
}

}
}
