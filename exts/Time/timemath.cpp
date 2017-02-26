// Copyright 2017 Nicolaus Anderson
#include "timemath.h"
#include <sstream>

namespace Cu {
  namespace MSecTime {

void MSecTime::writeToString(String& out) const {
    std::ostringstream ss;
    if ( ss << value )
      out = ss.str().c_str();
}

void MSecTime::fromUnsignedLong(unsigned long p) {
  // It is possible by hopefully unlikely that long long int is merely the same number of
  // bytes as unsigned long, which would create undefined behavior here.
  if ( sizeof(unsigned long) == sizeof(long long int) ) {
    value = static_cast<long long int>(p & (unsigned long)LLONG_MAX);
  } else {
    value = static_cast<ms_time>(p);
  }
}

void MSecTime::fromNativeTime(time_t p) {
#ifdef __linux__
  // glibc makes clock_t long int, which records in millionths of a second.
  value = (static_cast<ms_time>(p) * 1000) / CLOCKS_PER_SEC; // Could still have wrapping problems
#else
  // Convert to long double since we have no idea what this is
  long double ld = (static_cast<long double>(p) * 1000) / CLOCKS_PER_SEC;
  value = static_cast<ms_time>(ld); // Could still have wrapping problems, but deal with that later.
#endif
}

long int MSecTime::linuxTime() {
  if ( value - (ms_time)INT_MAX > 0 ) {
    return INT_MAX;
  }
  return (long int) value; 
}

unsigned long MSecTime::getAsUnsignedLong() const {
  if ( value < 0 ) {
    return 0;
  }
  if ( value > static_cast<ms_time>(ULONG_MAX) ) {
    return ULONG_MAX;
  }
  return static_cast<unsigned long>(value);
}

void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames) {
  FFI::simpleFunctionAdd(engine, util::String("MSecTime"), create, &logger);
  FFI::simpleFunctionAdd(engine, util::String("MSecTime_to_number"), mstimeToNumber, &logger);
  FFI::simpleFunctionAdd(engine, util::String("MSecTime_sum"), mstimeAdd, &logger);
  FFI::simpleFunctionAdd(engine, util::String("MSecTime_rdc"), mstimeSubtract, &logger);
  FFI::simpleFunctionAdd(engine, util::String("MSec_ClockTime"), clockTime);
}

bool isObjectMSecTime(Object* obj) {
  return isObjectOfType(*obj, MSecTime_TYPENAME);
}

bool getTimeValue(constParamsIter& iter, MSecTime*& out) {
  if ( ! iter.has() )
    return false;
  if ( ! isObjectMSecTime(*iter) ) {
    return false;
  }
  out = (MSecTime*)(*iter);
  return true;
}

bool create(constParamsList& params, CallResult& result, Logger* logger) {
  if ( ! params.has() ) {
    result.setWithoutRef(new MSecTime());
    return true;
  }
  // Can create time from a number or another time
  MSecTime* m;
  constParamsIter start = params.constStart();
  if ( getTimeValue(start, m) ) {
    result.setWithoutRef(new MSecTime(*m));
    return true;
  }
  if ( isObjectNumber(**start) ) {
    m = new MSecTime();
    m->setRaw(
      ((Number*)(*start))->getAsUnsignedLong()
    );
    result.setWithoutRef(m);
  }
  return true;
}

bool mstimeToNumber(constParamsList& params, CallResult& result, Logger* logger) {
  if ( !params.has() ) {
    FFI::printWarning(logger, "MSecTime-to-number function requires a parameter.");
    result.setWithoutRef(new ObjectNumber());
    return true;
  }
  MSecTime* m;
  constParamsIter start = params.constStart();
  if ( ! getTimeValue(start, m) ) {
    FFI::printWarning(logger, "MSecTime-to-number function requires a time.");
    result.setWithoutRef(new ObjectNumber());
    return true;
  }
  result.setWithoutRef(new ObjectNumber(m->getAsUnsignedLong()));
  return true;
}

bool mstimeAdd(constParamsList& params, CallResult& result, Logger* logger) {
  if ( !params.has() ) {
    FFI::printWarning(logger, "MSecTime-add function requires parameters.");
    result.setWithoutRef(new MSecTime());
    return true;
  }
  constParamsIter paramsIter = params.constStart();
  MSecTime* start = REAL_NULL;
  MSecTime* addition = REAL_NULL;
  do {
    if ( getTimeValue(paramsIter, addition) ) {
      if ( isNull(start) ) {
		start = (MSecTime*)(addition->copy());
      } else {
		start->addTime(*addition);
      }
    }
  } while ( ++paramsIter );
  if ( notNull(start) ) {
    result.setWithoutRef(start);
    return true;
  } else {
    return false;
  }
}

bool mstimeSubtract(constParamsList& params, CallResult& result, Logger* logger) {
  if ( !params.has() ) {
    FFI::printWarning(logger, "MSecTime-subtract function requires parameters.");
    result.setWithoutRef(new MSecTime());
    return true;
  }
  constParamsIter paramsIter = params.constStart();
  MSecTime* start = REAL_NULL;
  MSecTime* removal = REAL_NULL;
  do {
    if ( getTimeValue(paramsIter, removal) ) {
      if ( isNull(start) ) {
		start = (MSecTime*)(removal->copy());
      } else {
		start->subtractTime(*removal);
      }
    }
  } while ( ++paramsIter );
  if ( notNull(start) ) {
    result.setWithoutRef(start);
    return true;
  } else {
    return false;
  }
}

bool clockTime(constParamsList& params, CallResult& result) {
  MSecTime* m = new MSecTime();
  m->fromNativeTime(clock());
  result.setWithoutRef(m);
  return true;
}

  }
}
