// Copyright 2017 Nicolaus Anderson
#include "../../Copper/src/Copper.h"
#include <time.h>
#include <ctime>
#include <limits.h>
#include "../FFIBase.h"

namespace Cu {
  namespace MSecTime {

using FFI::constParamsList;
using FFI::constParamsIter;
using FFI::CallResult;

typedef long long int ms_time;
static const char* MSecTime_TYPENAME = "mstime";

void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames);

class MSecTime : public Object {
  ms_time value;

public:
  MSecTime()
    : value(0)
  {}
  
  MSecTime(const MSecTime& pOther)
    : value(pOther.value)
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
    return MSecTime_TYPENAME;
  }

  void fromUnsignedLong(unsigned long);

  void fromNativeTime(clock_t p);
  
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
bool getTimeValue(constParamsIter&, MSecTime*&);

bool create(constParamsList&, CallResult&, Logger*);
bool mstimeToNumber(constParamsList&, CallResult&, Logger*);
bool mstimeAdd(constParamsList&, CallResult&, Logger*);
bool mstimeSubtract(constParamsList&, CallResult&, Logger*);
bool clockTime(constParamsList&, CallResult&);

  }
}
