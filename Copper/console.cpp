// Copyright 2016-2018 Nicolaus Anderson

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> // for strsignal()


#include "src/Copper.h"
#include "stdlib/Printer.h"
#include "stdlib/InStreamLogger.h"


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


int main() {
	Cu::Engine engine;
	CuStd::Printer printer;
	CuStd::InStreamLogger streamLogger;
	engine.setLogger(&streamLogger);
	engine.addForeignFunction(util::String("print"), &printer);
	//engine.setStackTracePrintingEnabled(true);
	engine.setPrintTokensWhenParsing(true);
	signal(SIGSEGV, handler);
	std::setbuf(stdout,0);
	std::printf("\33[92m\n \t\tCOPPER LANGUAGE\n\t\tConsole Application\n\t\t(c) 2016-2018 Nicolaus Anderson\n\33[4m                                                              \33[0m\n");

#ifdef COPPER_REF_LEVEL_MESSAGES
	printer.declareStackBased();
#endif

	//streamLogger.printInputLine();

	int err = 0;
	try {
		while ( engine.run( streamLogger ) == Cu::EngineResult::Ok );
	}
	catch( Cu::BadReferenceCountingException& brce ) {
		if ( brce.object ) {
			std::printf("\nBad Reference Counting Exception: [%p], refs = %i\n", brce.object, brce.refs);
			err = 1;
		} else {
			std::printf("\nBad Reference Counting Exception. Object deleted.\n\n");
			err = 1;
		}
	}

	return err;
}
