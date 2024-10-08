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

#ifndef CU_CLOCK_TIME_TYPE
#define CU_CLOCK_TIME_TYPE 10
#endif

	// You should really just use me. :)
/*
	Creates the following functions in the Copper engine:

get_time()
	Returns the current time of the application's running, not the system time.

get_seconds( [clocktime] )
	Returns the seconds of the given clock time.

get_milliseconds( [clocktime] )
	Returns the milliseconds of the given clock time.

get_nanoseconds( [clocktime] )
	Returns the nanoseconds of the given clock time.
*/
void
addFunctionsToEngine(
	Engine&		engine
);

class ClockTime : public NumericObject {

	timespec  data;

public:

	ClockTime()
		: NumericObject()
		, data()
	{}

	ClockTime( const timespec&  pData ) // was there some reason timespec was supposed to be non const?
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

	static const char*
	StaticTypeName() {
		return "time";
	}

	virtual const char*
	typeName() const {
		return StaticTypeName();
	}

	virtual bool
	supportsInterface( ObjectType::Value  typeValue ) const;

	static ObjectType::Value
	StaticClockTimeType() {
		return static_cast<ObjectType::Value>( CU_CLOCK_TIME_TYPE );
	}

	static bool
	isCastable( NumericObject&  arg ) {
		return arg.supportsInterface( ClockTime::StaticClockTimeType() );
	}

	Decimal
	getNanosecondsValue() const {
		return (Integer)data.tv_nsec;
		// Should also add data.tv_sec*1000000
	}

	Decimal
	getMillisecondsValue() const {
		return (Integer)data.tv_sec * 1000 + (Integer)data.tv_nsec / 1000000;
	}

	Integer
	getSecondsValue() const {
		return (Integer)data.tv_sec;
	}

	void
	setValue( timespec  newValue ) {
		data.tv_sec = newValue.tv_sec;
		data.tv_nsec = newValue.tv_nsec;
	}

		// NumericObject methods

	virtual void
	setValue( const NumericObject& );

	virtual Integer
	getIntegerValue() const {
		return (Integer) (data.tv_sec);
	}

	virtual Decimal
	getDecimalValue() const {
		return (Decimal)data.tv_sec + (Decimal)data.tv_nsec/1000000000;
	}

	virtual bool
	isEqualTo(const NumericObject&  other );

	virtual bool
	isGreaterThan(const NumericObject&  other );

	virtual bool
	isGreaterOrEqual(const NumericObject&  other );

	virtual NumericObject*
	absValue() const;

	virtual NumericObject*
	add(const NumericObject&  other );

	virtual NumericObject*
	subtract(const NumericObject&  other );

	virtual NumericObject*
	multiply(const NumericObject&  other );

	virtual NumericObject*
	divide(const NumericObject&  other );

	virtual NumericObject*
	modulus(const NumericObject&  other );

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char*
	getDebugName() const {
		return "ClockTime";
	}
#endif
protected:
	bool isEqualTo(const timespec&);
	bool isGreaterThan(const timespec&);
	bool isGreaterOrEqual(const timespec&);
	NumericObject* add(const timespec&);
	NumericObject* subtract(const timespec&);
};

struct GetClockTime : public ForeignFunc {
	virtual ForeignFunc::Result call( FFIServices& );
};

ForeignFunc::Result GetSeconds( FFIServices& );
ForeignFunc::Result GetMilliseconds( FFIServices& );
ForeignFunc::Result GetNanoseconds( FFIServices& );

 }
}

#endif
