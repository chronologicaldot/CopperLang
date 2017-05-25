# Copper Virtual Machine (CopperVM)
This repository contains an interpreter that runs code of the Copper language - a statically-typed, loose syntax programming language that uses stack-frame-based variable lifetimes (no GC needed). For more information, see the [blog](http://copperlang.wordpress.com).

## Table of Contents
1. Basic Introduction to the Language
2. Project Structure and Building
3. Incorporating into other Projects
4. Legal

## Basic Introduction to the Language

The language is described in Copper/docs/CopperLangGuide.html and on the [Copper blog](http://copperlang.wordpress.com). What follows here is an overview.

Copper is a statically typed, loose-syntax language revolving around the concept of function-objects. All variables are instantiated upon declaration and can contain only functions, but functions can return any object. There is no nil/null/void, so the default return from a function is an empty function.

Variables can be directly assigned functions and other kinds of objects. Internally, these are all saved as function-objects.
```Copper
a = 5
b = { ret("Hello") }
```

Getting the object stored in a variable is also the same as calling a function.
```Copper
a = 5
print( b() )
```

A short-cut for calling a parameter-less function is the colon :
```Copper
print( b: )
```

Functions can be copied to other variables or pointed-to by other variables.
```Copper
a = { print("hello") }
b = a
c ~ a
```

Functions can have member variables, saved to a "persistent scope", which are copied during the assignment process.
These members can be accessed using the member operator (".").
```Copper
a.b = 12
c = a
print(c.a()) # Prints "12"
```

Function bodies are also copied to new functions during the assignment process.
``` Copper
a.b = { ret(5) }
c = a.b
print(c()) # Prints "5" #
```

Functions can take any parameters. They can optionally have a parameter body (where parameters are named) and can optionally have a body. One or the other must be present. The parameter body or the body can be empty.
```Copper
a = [p]{ ret(p:) }
a = [p]
a = { ret(5) }
```

To access the persistent scope of a function from inside the body, use the "this" pointer. Outside, use the parent variable's name.
```Copper
a = {
	this.child = 5
}
a:
print( a.child: )
```

The "super" pointer can be used within a function to access the parent function of the one whose scope is open.
```Copper
a.b = {
	super.c = 10
}
a.b:
print( a.c: ) # Prints 10 #
```

Data terminates an expression, so there is no need for an end-statement token, but for the sake of avoiding ambiguity, you can use "comma".
```Copper
a b = 5, c, d
```

If-structures allow for "elif" and "else" blocks but must be surrounded by curly brackets.
```Copper
i = true
if ( not(i:) ) {
	print("not i")
} elif ( i: ) {
	print("is i")
} else {
	print("not happening")
}
```

Loop structures begin with "loop", are stopped using "stop", and are forced to repeat using "skip".
```Copper
i = true
loop {
	if ( i: ) {
		i = false
		skip
	}
	stop
}
```


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


In addition, there is a EngMsgToStr.h file that contains useful string translations of the engine messages (and it is used in InStreamLogger.h).

These files themselves have no dependencies, as opposed to the Printer and InStreamLogger files.

The basic console application could be build with sh using a build.sh file containing:
```bash
g++ ./console.cpp src/Copper.h src/Copper.cpp src/utilList.h src/Strings.h src/Strings.cpp stdlib/Printer.h stdlib/InStreamLogger.h -Wall -Wfatal-errors -O3 -o bin/Copper.exe
```

For debugging on ARM, you may need to add -funwind-tables to your gcc compiler flags.


### Extensions Build Notes

A primitives (number) extension group has been provided. The specific classes (intmath.h, floatmath.h, etc.) are dependent on BasicPrimitive.h but do not all need to be included with each other in the project. (They each only depend on BasicPrimitive.h.)

The integer extension provided ("intmath.h") can optionally be compiled using the flag #define ENABLE_COPPER_INTEGER_BOUNDS_CHECKS for ensuring numbers don't exceed their cap and produce undefined behavior. Compiling without this flag creates a warning that the logger in the function is not used. This is not an error nor does it affect functionality, but if you are picky about having clean builds, you should use the aforementioned flag.


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


#### Extension Flags

* Defining ENABLE_COPPER_INTEGER_BOUNDS_CHECKS enableds bounds checks for the unsigned long extension.
* Defining ENABLE_COPPER_ULONG_BOUNDS_CHECKS enableds bounds checks for the unsigned long extension.


## Incorporating into other Projects

You need to include the core interpreter files, as given in section <i>Project Structure and Building</i>.

All interaction with the virtual occurs through the small Cu::Engine API.
A normal process is as follows:
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


## Legal

The Copper Virtual Machine as given in the files included are copyright 2017 Nicolaus Anderson.
License details are provided in license.txt.
