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

#define CU_CLOCK_TIME_TYPE 1

	// You should really just use me. :)
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

	static const NumericObject::SubType::Value
	StaticClockTimeType() {
		return static_cast<NumericObject::SubType::Value>(NumericObject::SubType::Unknown + CU_CLOCK_TIME_TYPE);
	}

	virtual NumericObject::SubType::Value
	getSubType() const {
		return StaticClockTimeType();
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
	virtual bool call( FFIServices& );
};

bool GetSeconds( FFIServices& );
bool GetMilliseconds( FFIServices& );
bool GetNanoseconds( FFIServices& );

 }
}

#endif
