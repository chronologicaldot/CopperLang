// Copyright 2017 Nicolaus Anderson
#ifndef CU_MS_TIME_H
#define CU_MS_TIME_H
#include "../ExtTypeValues.h"
#include "../../Copper/src/Copper.h"
#include <ctime>
#include <limits.h>

namespace Cu {
  namespace MSecTime {

typedef long long int ms_time;

void addFunctionsToEngine(Engine& engine, bool useShortNames);

class MSecTime : public Object {
  ms_time value;

public:
  static const char* StaticTypeName() { return "mstime"; }

  static ObjectType::Value
  StaticType() {
    return (ObjectType::Value)MSTIME_OBJECT_TYPE_VALUE;
  }

  MSecTime()
    : Object( StaticType() )
	, value(0)
  {}
  
  MSecTime(const MSecTime& pOther)
    : Object( StaticType() )
    , value(pOther.value)
  {}

  void setRaw(ms_time p) {
    value = p;
  }

  ms_time getRaw() {
    return value;
  }

  virtual Object* copy() {
    return new MSecTime(*this);
  }

  virtual void writeToString(String& out) const;

  virtual const char* typeName() const {
    return StaticTypeName();
  }

  void fromUnsignedLong(unsigned long);

  void fromNativeTime(time_t p);
  
  // glibc makes clock_t long int.
  long int linuxTime();

  virtual unsigned long getAsUnsignedLong() const;

  void addTime(const MSecTime& pOther) {
    value += pOther.value;
  }

  void subtractTime(const MSecTime& pOther) {
    value -= pOther.value;
  }
};

bool isObjectMSecTime(Object*);

struct Create : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

struct ToNumber : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual ObjectType::Value
	getParameterType( unsigned int index ) const {
		if ( index == 0 )
			return MSecTime::StaticType();
		return ObjectType::Function;
	}

	virtual unsigned int
	getParameterCount() const {
		return 1;
	}
};

struct Add : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

struct Subtract : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

struct ClockTime : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

  }
}

#endif
