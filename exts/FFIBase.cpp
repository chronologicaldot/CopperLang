// Copyright 2017 Nicolaus Anderson
#include "FFIBase.h"

namespace Cu {
namespace FFI {

void printWarning(Logger* logger, const char* msg) {
	if ( notNull(logger) ) {
		logger->print(LogLevel::warning, msg);
	}
}

void simpleFunctionAdd(
			Engine& engine,
			const String& name,
			bool (*callable)(const util::List<Object*>&, RefPtr<Object>&) )
{
	Wrapper* wrapper = new Wrapper(callable);
	engine.addForeignFunction(name, wrapper);
	wrapper->deref();
}

void simpleFunctionAdd(
			Engine& engine,
			const String& name,
			bool (*callable)(const util::List<Object*>&, RefPtr<Object>&, Logger*),
			Logger* logger
		)
{
	WrapperWithPrinter* wrapper = new WrapperWithPrinter(callable,logger);
	engine.addForeignFunction(name, wrapper);
	wrapper->deref();
}

}}
