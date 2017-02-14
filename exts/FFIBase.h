// Copyright 2017 Nicolaus Anderson
#ifndef COPPER_FFI_BASE_H
#define COPPER_FFI_BASE_H
#include "../Copper/src/Copper.h"

namespace Cu {
namespace FFI {

void printWarning(Logger* logger, const char* msg);

typedef const util::List<Object*>				constParamsList;
typedef util::List<Object*>::ConstIter			constParamsIter;
typedef RefPtr<Object>							CallResult;


// Base foreign function class with build-in printing function
class Base : public ForeignFunc {
protected:
	Logger* logger;

public:
	Base(Logger* pLogger)
		: logger(pLogger)
	{}

	void print(LogLevel::Value logLevel, const char* msg) {
		if ( notNull(logger) ) {
			logger->print(logLevel, msg);
		}
	}
};

// Function-wrapper class
// Useful when the function needs to print warning messages.
// Beware: The wrapped function must check for a null logger/printer.
class WrapperWithPrinter : public ForeignFunc {
	bool (*calling)(const util::List<Object*>&, RefPtr<Object>&, Logger*);
	Logger* logger;

public:
	WrapperWithPrinter(
			bool (*callable)(const util::List<Object*>&, RefPtr<Object>&, Logger*),
			Logger* pLogger
		)
		: calling(callable)
		, logger(pLogger)
	{}

	virtual bool call(const util::List<Object*>& params, RefPtr<Object>& result) {
		return calling(params, result, logger);
	}
};

// Function-wrapper class
// Useful for functions that perform generic, fail-safe operations.
class Wrapper : public ForeignFunc {
	bool (*calling)(const util::List<Object*>&, RefPtr<Object>&);

public:
	Wrapper( bool (*callable)(const util::List<Object*>&, RefPtr<Object>&) )
		: calling(callable)
	{}

	virtual bool call(const util::List<Object*>& params, RefPtr<Object>& result) {
		return calling(params, result);
	}
};


// Convenience method for adding functions to the engine
void simpleFunctionAdd(
			Engine& engine,
			const String& name,
			bool (*callable)(const util::List<Object*>&, RefPtr<Object>&)
		);

// Convenience method for adding functions to the engine
void simpleFunctionAdd(
			Engine& engine,
			const String& name,
			bool (*callable)(const util::List<Object*>&, RefPtr<Object>&, Logger*),
			Logger* logger
		);
}}

#endif
