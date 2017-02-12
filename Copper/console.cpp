// Copyright 2016-2017 Nicolaus Anderson

#include "src/Copper.h"
#include "stdlib/Printer.h"
#include "stdlib/InStreamLogger.h"

int main() {
	Cu::Engine engine;
	CuStd::Printer printer;
	CuStd::InStreamLogger logger;
	engine.setLogger(&logger);
	engine.addForeignFunction(util::String("print"), &printer);
	engine.setStackTracePrintingEnabled(true);
	//std::setbuf(stdout,0);
	std::printf("\n>> \t\tCOPPER LANGUAGE\n>>\t\tConsole Application\n>>\t\t(c) 2016-2017 Nicolaus Anderson\n\n");

	int err = 0;
	try {
		while ( engine.run( logger ) == Cu::EngineResult::Ok );
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
