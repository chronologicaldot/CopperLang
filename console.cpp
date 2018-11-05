// Copyright 2016-2018 Nicolaus Anderson

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> // for strsignal()

#include "Copper/src/Copper.h"
#include "Copper/stdlib/Printer.h"
#include "Copper/stdlib/InStreamLogger.h"

#include "exts/Math/basicmath.h"
#include "exts/Time/systime.h"

// function by tgamblin
// https://stackoverflow.com/questions/77005/how-to-generate-a-stacktrace-when-my-gcc-c-app-crashes
void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  std::fprintf(stderr, "Error: signal %d: %s,\n", sig, strsignal(sig));
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

struct MethodSource {
	char a;

	MethodSource()
		: a('a')
	{}

	bool run( Cu::FFIServices& ffi ) {
		ffi.printInfo("Called method source");
		a++;
		const util::String c(a);
		ffi.printInfo( c.c_str() );
		return true;
	}
};

int main() {
	Cu::Engine engine;
	CuStd::Printer printer;
	CuStd::InStreamLogger streamLogger;
	engine.setLogger(&streamLogger);
	engine.addForeignFunction(util::String("print"), &printer);
	engine.setStackTracePrintingEnabled(true);
	engine.setPrintTokensWhenParsing(true);

	Cu::Numeric::addFunctionsToEngine(engine);
	Cu::Time::addFunctionsToEngine(engine);

	//Cu::CallbackWrapper ccw(engine, util::String("set_wrapper"));

	//MethodSource ms;
	//Cu::addForeignMethodInstance( engine, util::String("method"), &ms, &MethodSource::run );

	signal(SIGSEGV, handler);
	std::setbuf(stdout,0);
	//std::printf("\n>> \t\tCOPPER LANGUAGE\n>>\t\tConsole Application\n>>\t\t(c) 2016-2018 Nicolaus Anderson\n\n");
	// ATTENTION: Leave the spaces alone
	std::printf("\33[92m\n \t\tCOPPER LANGUAGE\n\t\tConsole Application\n\t\t(c) 2016-2018 Nicolaus Anderson\n\33[4m                                                              \33[0m\n");

	Cu::EngineResult::Value  result;

	int err = 0;
	try {
		do {
			result = engine.run( streamLogger ); 
		} while ( result == Cu::EngineResult::Ok );
		switch( result )
		{
		case Cu::EngineResult::Ok:
			std::printf("\nEngine terminated but with ok status.");
			break;
		case Cu::EngineResult::Error:
			std::printf("\nEngine terminated with error.");
			break;
		case Cu::EngineResult::Done:
		default:
			//std::printf("\nEngine finished.");
			break;
		}
	}
	catch( Cu::BadReferenceCountingException& brce ) {
		if ( brce.object ) {
			std::printf("\nBad Reference Counting Exception: [%p], refs = %d\n", brce.object, brce.refs);
			err = 1;
		} else {
			std::printf("\nBad Reference Counting Exception. Object deleted.\n\n");
			err = 1;
		}
	}
	std::printf("\n");
	return err;
}
