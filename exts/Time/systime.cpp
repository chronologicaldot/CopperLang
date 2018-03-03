// Copyright 2018 Nicolaus Anderson

#include "systime.h"

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
	// Add together time if more than one arg
	Object*  arg;
	Integer  totalTime = 0;
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( ClockTime::isSameType(arg->typeName()) ) {
			totalTime += ((ClockTime*)arg)->getSecondsValue();
		}
	}
	ffi.setNewResult( new ObjectInteger( totalTime ) );
	return true;
}

bool
GetMilliseconds( FFIServices& ffi ) {
	// Add together time if more than one arg
	Object*  arg;
	Decimal  totalTime = 0;
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( ClockTime::isSameType(arg->typeName()) ) {
			totalTime += ((ClockTime*)arg)->getMillisecondsValue();
		}
	}
	ffi.setNewResult( new ObjectDecimal( totalTime ) );
	return true;
}

bool
GetNanoseconds( FFIServices& ffi ) {
	// Add together time if more than one arg
	Object*  arg;
	Decimal  totalTime = 0;
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( ClockTime::isSameType(arg->typeName()) ) {
			totalTime += ((ClockTime*)arg)->getMillisecondsValue();
		}
	}
	ffi.setNewResult( new ObjectDecimal( totalTime ) );
	return true;
}

}
}
