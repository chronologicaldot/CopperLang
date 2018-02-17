// Copyright 2018 Nicolaus Anderson
#ifndef CU_SYS_CLOCK
#define CU_SYS_CLOCK

#include "../../Copper/src/Copper.h"
#include <ctime>
#include <sys/time.h>

// Reference:
// http://pubs.opengroup.org/onlinepubs/7908799/xsh/time.h.html

namespace Cu {
 namespace Time {

	// You should really just use me. :)
void
addFunctionsToEngine(
	Engine&		engine
);

class ClockTime : public Object {

	timespec  data;

public:
	ClockTime()
		: data()
	{}

	ClockTime( timespec&  pData )
		: data( pData )
	{}

	virtual Object*
	copy() {
		return new ClockTime( data );
	}

	virtual void
	writeToString( String& out ) const {
		out = "{time}";
	}

	Integer
	getIntegerValue() const {
		return (Integer) (data.tv_sec);
	}

	Decimal
	getDecimalValue() const {
		return (Decimal)data.tv_sec + (Decimal)data.tv_nsec/1000000000;
	}

	Decimal
	getNanosecondsValue() const {
		return (Integer)data.tv_nsec;
	}

	Decimal
	getMillisecondsValue() const {
		return (Integer)data.tv_sec * 1000 + (Integer)data.tv_nsec / 1000000;
	}

	Integer
	getSecondsValue() const {
		return (Integer)data.tv_sec;
	}

	virtual const char*
	typeName() const {
		return StaticTypeName();
	}

	static const char*
	StaticTypeName() {
		return "time";
	}

	static bool
	isSameType( const char* pTypeName ) {
		return util::equals( StaticTypeName(), pTypeName );
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char*
	getDebugName() const {
		return "ClockTime";
	}
#endif
};

struct GetClockTime : public ForeignFunc {
	virtual bool call( FFIServices& );
};

bool GetSeconds( FFIServices& );
bool GetMilliseconds( FFIServices& );
bool GetNanoseconds( FFIServices& );

 }
}

#endif
