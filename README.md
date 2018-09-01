# Copper Lang Interpreter
This repository contains an interpreter that runs code of the Copper language - a statically-typed, loose syntax programming language. For more information, see the [blog](https://copperlang.wordpress.com) or the [online documentation](https://chronologicaldot.github.io/CopperLang/).

## Table of Contents
1. Basic Introduction to the Language
2. Project Structure and Building
3. Incorporating into other Projects
4. Legal

## Basic Introduction to the Language

The language is described in Copper/docs/CopperLangGuide.html and on the [Copper blog](http://copperlang.wordpress.com).

Copper is a statically typed, loose-syntax language revolving around the concept of function-objects. All variables are instantiated upon declaration and can contain only functions, but functions can return any object. There is no nil/null/void, so the default return from a function is an empty function. The language uses if/elif/else and loop structures.

Quick example:
```Copper
fibonacci = {
	ret([a=0 b=1]{
		this.peek = +(this.a: this.b:)
		this.a = this.b
		this.b = this.peek
		ret(this.peek:)
	})
}
fibber = fibonacci()
loop {
	a = fibber()
	if ( gte(a: 100) ) {
		stop
	}
	if ( equal(a: 13) ) {
		skip
	} elif ( equal(a:, 21) ) {
		print("Still young!")
	}
	print("Value = ", a: "\n")
}
```


## Interpreter Features

This is a tiny, light-weight, cross-platform interpreter (roughly 250kb) providing only certain basic features essential to the language. The bare-bones interpreter includes functions for boolean comparisons and some for numbers and strings. An extension has been provided for useful numeric functions. Many features have been deliberately excluded from the engine to keep it small and also to allow users to have more control over the permitted flow of data. For example, basic strings are merely byte strands, but the user may create a separate object type to enforce Unicode.

The interpreter does not use a garbage collector. The language doesn't require one.

There is only one system dependency in the core code: climits. Other includes are primarily for printing and debugging but aren't necessary.

The code-base is written in cross-platform, standard-agnostic C++ and has been compiled successfully for C++98 and C++11 with GCC.


## Project Structure and Building

There are two layers of folders: The inner folder "Copper" contains the basic interpreter itself (with basic printer and logger extensions). The outer folder includes this and the "ext" folder where you can store your extensions to the interpreter.

There are Premake files provided for making project files for the basic console application and debugging. You don't need them, but they may be helpful. A premake file is provided in the Copper folder. From Premake files, you can create a g++ make file or a Visual Studio file (2015 at highest; see the Premake documentation for other supported versions). The one inside the "Copper" folder can create make and VS project files for the basic interpreter. The one in the outer folder can create make and VS project files for the interpreter with all of the extensions provided in the "ext" folder. You may add your files without modifying the Premake file, but you must rerun the Premake file if you want to create the project files (make or VS) with these new files added (that is, if you don't want to manually add them yourself).

The Premake files are not necessary for the project as it is simply enough to include the necessary files manually.

The following files must be included for the basic interpreter (without the printer):

* utilList.h
* RRHash.h
* String.h
* String.cpp
* Copper.h
* Copper.cpp

These files themselves have no dependencies, as opposed to the Printer and InStreamLogger files.
In addition, there is a file called EngMsgToStr.h that contains useful string translations of the messages from the interpreter (and it is used in InStreamLogger.h).

The basic console application could be build with sh using a build.sh file containing:
```bash
g++ ./console.cpp src/Copper.h src/Copper.cpp src/utilList.h src/utilPipe.h src/Strings.h src/RRHash.h src/Strings.cpp stdlib/Printer.h stdlib/InStreamLogger.h -Wall -Wfatal-errors -O3 -o bin/Copper.exe
```

For debugging on ARM, you may need to add -funwind-tables to your gcc compiler flags.


### Extensions Build Notes

Extensions for Copper are provided in the ext folder. These can be include in your main.cpp file along with Copper, and their functionality can be added to the engine by calling their respective addFunctionsToEngine(Engine&) functions.

The numbers extension provided ("basicmath.h") can optionally be compiled using the flag #define ENABLE_COPPER_INTEGER_BOUNDS_CHECKS for ensuring numbers don't exceed their cap and produce undefined behavior. Compiling without this flag creates a warning that the logger in the function is not used. This is not an error nor does it affect functionality, but if you are picky about having clean builds, you should use the aforementioned flag.


### Alternate C++ Build Notes

Optionally, you may define the compiler flag COMPILE_COPPER_FOR_C_PLUS_PLUS_11. All this does is change a few lines of code to use null_ptr and some functions to be deleted rather than set to empty functions. This should not effect functionality, and it is not C++11-specific. If you define this flag, you should use null_ptr for setters. If you do not define this flag, you should use the number 0 for setting pointers instead.


### Build Flags List

* Defining COMPILE_COPPER_FOR_C_PLUS_PLUS_11 forces use of null_ptr for null checks rather than zero.
* Defining COPPER_ENABLE_CALLBACK_THIS_POINTER causes functions that are called via Engine::runFunction (usually for callbacks for hooks) to be given a "this" pointer but requires they use a guard (usually a variable) to claim ownership, otherwise using the this-pointer causes a self-reset.
* Defining COPPER_ENABLE_EXTENDED_NAME_SET allows names to contain the following special characters:
	* "+"
	* "-"
	* "*"
	* "/"
	* "%"
	* "!"
	* "?"
* Defining UNDEF_COPPER_ENABLE_NUMERIC_NAMES prevents names from being allowed to contain numbers.


### Build Warnings

This project has been compiled with -Wall and -Wfatal-errors on GCC.
A warning for an unused variable ("index") in the FFIServices has been silenced with a macro.


#### Extension Flags

* Defining ENABLE_COPPER_INTEGER_BOUNDS_CHECKS enables bounds checks for the unsigned long extension.


## Incorporating into other Projects

You need to include the core interpreter files, as given in section <i>Project Structure and Building</i>.

All interaction with the interpreter occurs through the small Cu::Engine API.
For running a command-line-based version, the setup process is as follows:
* creating the engine
* setting the logger
* adding foreign functions
* calling run (and passing it an implementation of the byte stream interface)

For example:
```C++
Cu::Engine engine;
Cu::InStreamLogger insl;
engine.setLogger(&insl);
Cu::Printer printer;
engine.addForeignFunction(util::String("print"), &printer);

while( engine.run(insl) == EngineResult::Ok );
```

### Adding Functionality

New functionality can be added through objects inheriting the class Cu::ForeignFunc, which are then passed to the engine via Engine::addForeignFunction(). This class contains a single method - "call()" - that is to return true if the "result" parameter is set.

The method "call()" is called whenever the name associated with the function (the string passed to Engine::addForeignFunction()) is called in Copper.

A number of helper classes have been provided for adding functions and class methods directly (so that there is no need to implement Cu::ForeignFunc).


## Legal

The Copper Virtual Machine as given in the files included are copyright 2017-2018 Nicolaus Anderson.
License details are provided in license.txt.
