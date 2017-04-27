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

void addFunctionsToEngine(
	Engine& engine,
	bool useShortNames
) {
	if ( useShortNames ) {
		addForeignFuncInstance<Create>	(engine, "MSec");
		addForeignFuncInstance<ToNumber>(engine, "MSec_to_number");
		addForeignFuncInstance<Add>		(engine, "MSec+");
		addForeignFuncInstance<Subtract>(engine, "MSec-");
		addForeignFuncInstance<ClockTime>(engine, "MSec_ClockTime");
	} else {
		addForeignFuncInstance<Create>	(engine, "MSecTime");
		addForeignFuncInstance<ToNumber>(engine, "MSecTime_to_number");
		addForeignFuncInstance<Add>		(engine, "MSecTime+");
		addForeignFuncInstance<Subtract>(engine, "MSecTime-");
		addForeignFuncInstance<ClockTime>(engine, "MSec_ClockTime");
	}
}

bool isObjectMSecTime(
	Object* obj
) {
	return isObjectOfType(*obj, MSecTime::StaticTypeName());
}

bool
Create::call(
	FFIServices& ffi
) {
	MSecTime* m;
	Object* arg;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( isObjectOfType(*arg, ObjectNumber::StaticTypeName()) ) {
			m = new MSecTime();
			m->setRaw( (long long int) ((ObjectNumber*)arg)->getAsUnsignedLong() );
		} else {
			m = new MSecTime();
			ffi.printWarning("MSecTime-creation was not given a numeric starting argument. Defaulting to zero.");
			ffi.setResult(m);
			m->deref();
			return false;
		}
	} else {
		m = new MSecTime();
	}
	ffi.setResult(m);
	m->deref();
	return true;
}

bool
ToNumber::call(
	FFIServices& ffi
) {
	MSecTime* m = (MSecTime*)(ffi.getNextArg());
	ObjectNumber* out = new ObjectNumber(m->getAsUnsignedLong());
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
Add::call(
	FFIServices& ffi
) {
	MSecTime* m;
	MSecTime* firstM;
	MSecTime* secondM;
	Object* arg;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( isObjectMSecTime(arg) ) {
			firstM = new MSecTime( *((MSecTime*)arg) );
		} else {
			ffi.printError("MSecTime-addition was not given an MSecTime object as the first argument.");
			return false;
		}
	}
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( isObjectMSecTime(arg) ) {
			secondM = (MSecTime*)arg;
			firstM->addTime(*secondM);
		} else {
			ffi.printWarning("MSecTime-addition arg was not of type MSecTime. Ignoring arg...");
		}
	}
	ffi.setResult(firstM);
	firstM->deref();
	return true;
}

bool
Subtract::call(
	FFIServices& ffi
) {
	MSecTime* firstM;
	MSecTime* secondM;
	Object* arg;
	if ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( isObjectMSecTime(arg) ) {
			firstM = new MSecTime( *((MSecTime*)arg) );
		} else {
			ffi.printError("MSecTime-subtraction was not given an MSecTime object as the first argument.");
			return false;
		}
	}
	while ( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( isObjectMSecTime(arg) ) {
			secondM = (MSecTime*)arg;
			firstM->subtractTime(*secondM);
		} else {
			ffi.printWarning("MSecTime-subtraction arg was not of type MSecTime. Ignoring arg...");
		}
	}
	ffi.setResult(firstM);
	firstM->deref();
	return true;
}

bool
ClockTime::call(
	FFIServices& ffi
) {
	MSecTime* m = new MSecTime();
	m->fromNativeTime(clock());
	ffi.setResult(m);
	m->deref();
	return true;
}

  }
}
