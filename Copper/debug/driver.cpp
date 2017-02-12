// Copyright 2016 Nicolaus Anderson

#include "../src/Copper.h"
#include "../stdlib/Printer.h"
#include "../stdlib/InStreamLogger.h"

void testString();
void testVariable();
void testEngine();
void testScope();

int main() {
	//testString();
	//testVariable();
	testEngine();
	//testScope();

	//double a = 12, b = 3.14;
	//std::printf("> a = %f, b = %f, a + b = %f, a * b = %f\n\n", a, b, a+b, a*b);

	return 0;
}

void testEngine() {
	Cu::Engine engine;
	//CuStd::Printer printer;
	CuStd::Printer* printer = new CuStd::Printer(); // Only for debugging
	CuStd::InStreamLogger logger;

	std::setbuf(stdout,0);
	std::printf("\nTesting begins.\n");

	engine.setLogger(&logger);
	engine.addForeignFunction(util::String("print"), printer);
	engine.setStackTracePrintingEnabled(true);

	std::printf("\nSetup done.\n");

	try {
		while ( engine.run( logger ) == Cu::EngineResult::Ok );
	}
	catch( Cu::BadReferenceCountingException& brce ) {
		if ( brce.object ) {
			std::printf("\nBad Reference Counting Exception: [%p], refs = %i\n\n", brce.object, brce.refs);
		} else {
			std::printf("\nBad Reference Counting Exception. Object deleted.\n\n");
		}
	}

	printer->deref();

	std::printf("\nTesting ends.\n\n");
}


void testString() {
	std::setbuf(stdout,0);
	std::printf("\nTesting begins.\n");

	float value = 90;
	std::printf("\nFloat value = %f", value);
	util::CharList cl_value(value);
	util::String str_value(cl_value);
	//util::String str_value(value);
	std::printf("\nString value = %s", str_value.c_str());
	std::printf("\nString numeric value = %f", str_value.toFloat());

	std::printf("\nTesting ends.\n\n");
}


void testVariable() {
	std::setbuf(stdout,0);
	std::printf("\nTesting begins.\n");

	Cu::Variable* var = new Cu::Variable();
	Cu::Function* func;
	var->getRawContainer()->getFunction(func);

	std::printf("\nVar func address: %p", (void*)func);
	std::printf("\nKilling var...\n");

	var->deref();
	var = 0;

	std::printf("\nTesting ends.\n\n");
}


void testScope() {
	std::setbuf(stdout,0);
	std::printf("Setup.\n");

	// Since class Ref throws an error when deleting a non-zero counted object in debug,
	// we have to create a scope on the heap and deref it.
	Cu::Scope* scope = new Cu::Scope();
	Cu::String name_a("a");
	Cu::String name_b("b");
	Cu::String name_c("c");
	Cu::Variable* var_a;
	Cu::Variable* var_b;
	Cu::Variable* var_c;
	Cu::Function* func_a;
	unsigned int currentOccupancy = 0;

	std::printf("Adding variable 'a'.\n");

	scope->addVariable(name_a);

	std::printf("Checking if variable 'a' exists.\n");

	if ( scope->variableExists(name_a) ) {
		std::printf("Yes, variable 'a' exists.\n");
	}

	std::printf("Finding variable 'a'.\n");

	if ( scope->findVariable(name_a, var_a) ) {
		std::printf("Variable found. &=%p\n", (void*)var_a);
	}

	std::printf("Getting existing variable.\n");

	scope->getVariable(name_a, var_a);

	std::printf("Variable 'a' &=%p\n", (void*)var_a);

	std::printf("Checking if variable 'b' exists.\n");

	if ( scope->variableExists(name_b) ) {
		std::printf("Yes, variable 'b' exists.\n");
	}

	std::printf("Searching for non-existent variable 'b'.\n");

	if ( scope->findVariable(name_b, var_b) ) {
		std::printf("Found non-existent variable 'b'.\n");
	}

	std::printf("Getting non-existent variable 'b'.\n");

	scope->getVariable(name_b, var_b);

	std::printf("Variable 'b' found. &=%p\n", (void*)var_b);

	currentOccupancy = scope->occupancy();

	std::printf("Current occupancy = %u\n", currentOccupancy);

	if ( var_a ) {
		std::printf("Getting function container from 'a'. Container &=%p\n", var_a->getRawContainer());
		if ( var_a->getRawContainer() ) {
			std::printf("Opening raw container....\n");
			if ( var_a->getRawContainer()->getFunction(func_a) ) {
				std::printf("obtained function...: &=%p\n", func_a);

				std::printf("Getting non-existent variable 'c' from 'a'.\n");
				func_a->getPersistentScope().getVariable(name_c, var_c);
				std::printf("Variable 'c' from 'a', &=%p\n", var_c);
			} else {
				std::printf("No function.\n");
			}
		} else {
			std::printf("No function container.\n");
		}
	} else {
		std::printf("Tried to use non-existent variable 'a'.\n");
	}

	std::printf("Copying scope.\n");

	Cu::Scope* scope2 = new Cu::Scope();
	*scope2 = *scope;

	std::printf("Destroying scope.\n");

	scope->deref();

	std::printf("Destroying second scope.\n");

	scope2->deref();

	std::printf("Testing complete.\n");
}













