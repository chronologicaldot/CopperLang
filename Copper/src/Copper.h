// Copyright 2016-2017 Nicolaus Anderson

#ifndef COPPER_LANG_H
#define COPPER_LANG_H

// ******* Debug *******

//#define COPPER_REF_LEVEL_MESSAGES
//#define COPPER_VAR_LEVEL_MESSAGES
//#define COPPER_FFI_LEVEL_MESSAGES
//#define COPPER_SCOPE_LEVEL_MESSAGES
//#define COPPER_TASK_LEVEL_MESSAGES
//#define COPPER_DEBUG_ENGINE_MESSAGES
//#define COPPER_USE_DEBUG_NAMES
//#define COPPER_DEBUG_LOOP_STRUCTURE // Limits loops to 100 process() cycles

//#define COPPER_PRINT_ENGINE_PROCESS_TOKENS
//#include <cstdio>


// ******* Null *******

#ifdef REAL_NULL
#undef REAL_NULL
#endif

	// Note: "__cplusplus" indicates the version on linux, at least in the standard library.
	// See /usr/include/c++/#.#/numeric, line 126
	// I don't know what other operating systems use.
	// However, the user might want to continue using 0 for null. Otherwise, you can use the following line:
//#if (defined(__cplusplus) && __cplusplus >= 201103L)
#ifdef COMPILE_COPPER_FOR_C_PLUS_PLUS_11

	// For ensuring items in RefPtr inherit Ref.
#include <type_traits> // Requires C++11 support
#define REAL_NULL nullptr
#else
// Using zero for checking pointers.
// Because compilers lie about their C++ support, we can't guarantee nullptr
#define REAL_NULL 0

#endif // endif use C++11

#ifdef isNull
#undef isNull
#endif
#define isNull(x) (x == REAL_NULL)

#ifdef notNull
#undef notNull
#endif
#define notNull(x) (x != REAL_NULL)

// ******* Virtual machine version *******

#define COPPER_INTERPRETER_VERSION 0.18
#define COPPER_INTERPRETER_BRANCH 2

// ******* Language version *******

#define COPPER_LANG_VERSION 2.01

// ******* Language modifications *******

// Uncomment to prevent the creation of non-ASCII strings
//#define COPPER_PURGE_NON_PRINTABLE_ASCII_INPUT_STRINGS

// Uncomment to allow callback functions to have their own persistent scope.
// Enabling this feature means you must save each callback FunctionContainer to a variable
// so that its Function is not destroyed with "this" when the scope is cleared from the stack.
// Example: [code]
// Variable var; var.setFunc( callbackFunctionContainer );
//#define COPPER_ENABLE_CALLBACK_THIS_POINTER

// Uncomment to allow names to contain the following special characters:
// + - * / % ! ?
#define COPPER_ENABLE_EXTENDED_NAME_SET

#ifdef UNDEF_COPPER_ENABLE_EXTENDED_NAME_SET
#undef COPPER_ENABLE_EXTENDED_NAME_SET
#endif

// Uncomment to enable names to contain numbers.
// This can be overridden by a name filter function given to the engine.
#define COPPER_ENABLE_NUMERIC_NAMES

#ifdef UNDEF_COPPER_ENABLE_NUMERIC_NAMES
#undef COPPER_ENABLE_NUMERIC_NAMES
#endif

// ******* Error templates *******

template<typename T>
class UnimplementedFunction {};

template<typename T>
class BadParameterException {};

class RandomNullByteInStream {};


// ******* System includes *******

#include "utilList.h"
#include "RHHash.h"
#include "Strings.h"

namespace Cu {

using util::List;
using util::CharList;
using util::String;
using util::RobinHoodHash;

//-----------------

struct OSInfo {
	enum OSType {
		OS_Windows32,
		OS_Windows64,
		OS_Apple_Mac,
		OS_Linux,
		OS_Unix,
		OS_POSIX
	};

	OSInfo()
	{}

	/* Checks the system's endianness.
	Idea thanks to http://www.codeproject.com/Articles/4804/Basic-concepts-on-Endianness */
	static bool isSystemBigEndian()
	{
		short w = 0x0201;
		return ((char*)&w)[0] != 0x01;
	}

	static OSType getOS()
	{
//#ifdef _MSC_VER
#ifdef _WIN32
		// Defined for both 32-bit and 64-bit versions, so check for 64-bit
	#ifdef _WIN64
		return OS_Windows64;
	#else
		return OS_Windows32;
	#endif
#elif __APPLE__
		return OS_Apple_Mac;
#elif __linux__
		return OS_Linux;
#elif __unix__
		return OS_Unix;
#elif defined(_POSIX_VERSION)
		return OS_POSIX;
#else
		return OS_Unknown;
#endif
	}
};

//-------------

	// Engine Messages
/*
Note: Those marked with ERROR indicate problems that cause the engine to halt running.
Those marked with WARNING are problems but merely produce messages.
The engine attempts to compensate for the issues that have arisen, but this may result in
unwanted activity, so the user is informed.
*/
/*
It would probably be better if I reduced these to simple numbers.
That's annoying, but at least I can see what the errors are numerically.
At the same rate, it would be annoying when trying to insert new errors and not break code.
*/
struct EngineMessage {
	enum Value {

	Running=0,

	// C++ ERROR
	ZeroSizeToken,

	// ERROR
	// Stack has reached its maximum amount.
	StackOverflow,

	// WARNING
	/* Function does not exist when trying to access it via FunctionContainer.
	This can occur when trying to access via a pointer. */
	NoFunctionInContainer,

	// ERROR
	/* The special character (used for hiding other characters within a string) has been
	found outside of a string. */
	EscapeCharMisplaced,

	// ERROR
	// The given token is too long. Tokens are limited to 255 characters (bytes) in length.
	LongToken,

	// ERROR
	// Token could not be resolved
	UnresolvedToken,

	// ERROR
	// A token failed to be parsed. Usually, this means a zero-size token.
	MalformedToken,

	// ERROR
	// A token with an unpermitted arrangement of characters was encountered.
	UnknownToken,

	// ERROR
	// A token that starts with a numeric letter has been found to be not a number.
	MalformedNumber,

	// ERROR
	InvalidName,

	// ERROR
	// Stream ended before comment ended.
	StreamInteruptedInComment,

	// ERROR
	// Stream ended before string ended.
	StreamInteruptedInString,

	// ERROR
	// Parameter body has been opened without being preceded by a name.
	OrphanParamBodyOpener,

	// ERROR
	// Parameter body has been closed without being opened.
	OrphanParamBodyCloser,

	// ERROR
	/* Function body has been closed without first being opened.
	(Should only happen at the global scope.) */
	OrphanBodyCloser,

	// ERROR
	// "elif" used without first being preceded by "if"
	OrphanElif,

	// ERROR
	// "else" used without first being preceded by "if" or "elif"
	OrphanElse,

	// ERROR
	// Token could not be handled. This results when the system has not implemented it.
	// i.e. my bad.
	TokenNotHandled,

	// ERROR
	// For compatibility reasons, this error remains.
	InvalidTokenAtFunctionStart,

	// WARNING
	/* Unused assignment symbol in the function construction parameters.
	This can result from accidentally adding the end-statement symbol before it. */
	UnusedAsgnInFunctionBuildParams,

	// WARNING
	/* Unused pointer-assignment/creation symbol in the function construction parameters.
	This can result from accidentally adding the end-statement symbol before it. */
	UnusedPtrAsgnInFunctionBuildParams,

	// ERROR
	/* Unused token in function construction parameters.
	This can result from forgetting a token. */
	UnusedTokenInFunctionBuildParams,

	// ERROR
	// Bad token found when collecting the parameter value.
	InvalidParamValueToken,

	// ERROR
	// Attempting to assign a pointer a non-named item.
	PtrAssignedNonName,

	// ERROR
	// Attempting to open the scope of a system function.
	OpeningSystemFuncScope,

	// ERROR
	// Attempting to open the scope of an extension function. Currently, this is unsupported.
	OpeningExtensionFuncScope,

	// ERROR
	// Trying to create more bodies within a function than feasible.
	ExceededBodyCountLimit,

	// ERROR
	// Trying to assign the wrong kind of token to a variable.
	InvalidAsgn,

	// ERROR
	// Trying to make a variable a pointer to a non-object via wrong kind of token.
	InvalidPtrAsgn,

	// ERROR
	// Non-name followed member accessor (.).
	InvalidTokenAfterMemberAccessor,

	// ERROR - SERIOUS!
	// Task Variable does not exist. This would be my bad.
	TaskVarIsNull,

	// ERROR
	// Trying to create more parameter bodies within a function header than feasible.
	ExceededParamBodyCountLimit,

	// WARNING
	/* Missing parameter for a function call.
	This results in a default value (of empty function) for the parameter. */
	MissingFunctionCallParam,

	// ERROR
	// Last-object pointer was not set before object was to be assigned to a variable.
	UnsetLastObjectInAsgn,

	// WARNING
	// An attempt was made to assign a new function to a name reserved for a built-in function.
	AttemptToOverrideBuiltinFunc,

	// WARNING
	// An attempt was made to assign a new function to a name reserved for an external function.
	AttemptToOverrideExternalFunc,

	// ERROR
	// Last-object pointer was not set before a pointer-to-it was to be assigned to a variable.
	UnsetLastObjectInPtrAsgn,

	// ERROR - May be changed to WARNING
	// An attempt was made to assign raw data to a variable via the pointer assignement token.
	DataAssignedToPtr,

	// ERROR
	// After the if-statement, a condition (parameter body opener) did not immediately follow.
	InvalidTokenAtIfStart,

	// WARNING
	/* No boolean result came from the condition body given to the if-statement.
	The statement automatically defaults to "false".
	If this happens for successive if and elif statements, the else block is still executed. */
	ConditionlessIf,

	// ERROR
	// A different token was found while searching for the body-opener token of an if-statment.
	InvalidTokenBeforeIfBody,

	// ERROR
	// A stray porameter was found while searching for the body of a loop.
	StrayTokenInLoopHead,

	// WARNING
	// The "own" control structure is disabled.
	PointerNewOwnershipDisabled,

	// ERROR
	// A token other than a parameter body opener was found after "own" or "is_ptr".
	InvalidTokenBeforePtrHandlerParamBody,

	// WARNING
	// A variable was not created before being used in an "own" or "is_ptr" structure, making the structure needless.
	NonexistentVariablePassedToPtrHandler,

	// ERROR
	// Encountered a non-string token when searching for one in "own" or "is_ptr" structure.
	NonNameFoundInPtrHandlerParamBody,

	// ERROR
	// A token other than the parameter body ending was found before "own" or "is_ptr" ended (after parameter collection).
	InvalidTokenForEndingPtrHandlerParamBody,

	// User-induced error
	AssertionFailed,

	// ERROR
	// An incorrect number of params was passed to a system function.
	SystemFunctionWrongParamCount,

	// ERROR
	// An incorrect parameter was passed to a system function.
	SystemFunctionBadParam,

	// ERROR
	// member() was given the wrong number of parameters.
	MemberWrongParamCount,

	// ERROR
	// First parameter passed to member() was not a function.
	MemberParam1NotFunction,

	// ERROR
	// Destroyed function passed to member() function. Parameter was probably a pointer.
	DestroyedFuncAsMemberParam,

	// ERROR
	// Second parameter passed to member() was not a string.
	MemberParam2NotString,

	// ERROR
	// Invalid member name passed to member() function.
	MemberFunctionInvalidNameParam,

	// ERROR
	// member_count() was given the wrong number of parameters.
	MemberCountWrongParamCount,

	// ERROR
	// First parameter passed to member_count() was not a function.
	MemberCountParam1NotFunction,

	// ERROR
	// Destroyed function passed to member_count() function. Parameter was probably a pointer.
	DestroyedFuncAsMemberCountParam,

	// ERROR
	// is_member() was given the wrong number of parameters.
	IsMemberWrongParamCount,

	// ERROR
	// First parameter passed to is_member() was not a function.
	IsMemberParam1NotFunction,

	// ERROR
	// Destroyed function passed to is_member() function. Parameter was probably a pointer.
	DestroyedFuncAsIsMemberParam,

	// ERROR
	// Second parameter passed to is_member() was not a string.
	IsMemberParam2NotString,

	// ERROR
	// set_member() was given the wrong number of parameters.
	SetMemberWrongParamCount,

	// ERROR
	// First parameter passed to set_member() was not a function.
	SetMemberParam1NotFunction,

	// ERROR
	// Destroyed function passed to set_member() function. Parameter was probably a pointer.
	DestroyedFuncAsSetMemberParam,

	// ERROR
	// Second parameter passed to set_member() was not a string.
	SetMemberParam2NotString,

	// WARNING
	// Destroyed function passed to union() function. Parameter was probably a pointer.
	DestroyedFuncAsUnionParam,

	// WARNING
	// A parameter passed to the union() function was not a function.
	NonFunctionAsUnionParam,

	// Total number of engine messages
	COUNT

	};
};

//--------------

struct ObjectType {
	enum Value {
	Function, // Also handles pointers
	Data
	};
};

struct Result {
	enum Value {
	Ok = 0,
	Error = 1
	};
};

struct EngineResult {
	enum Value {
		Ok,
		Error,
		Done		// Used for indicating end-main/end processing
	};
};

//-------------------

/*
This MUST be a buffered stream - that is, one possessing another byte.
atEOS() is to return true if another byte is in the buffer or guaranteed to be (such as from stdin).
*/
struct ByteStream {
	virtual char getNextByte()=0;
	virtual bool atEOS()=0;
};

//-------------------

enum TokenType {
	/* Unknown
	Tokens of this type should never be sent to the processor. They are wrong. */
	TT_unknown=0,

	/* Mal-formed
	A mal-formed token name, perhaps from someone trying to mess up the system. */
	TT_malformed,

	/* End-of-main
	Indicates end-of-stream, end-of-file, or end-of-main-program.
	This token can be directly passed to process() to initiate cleanup. */
	TT_end_main,

	/* Exit
	This token ends processing. */
	TT_exit,

	/* Comment
	Should not be included in process: it results in an error.
	These are filtered out by run(). */
	TT_comment,

	/* End segment
	This is used to divide elements in a list. */
	TT_end_segment,

	/* Assignment */
	TT_assignment,

	/* Pointer assignment */
	TT_pointer_assignment,

	/* Scope open
	Attached to the end of a name which opens the persistent scope. */
	TT_member_link,

	/* Param body open and close
	These are used for collecting parameters for a function call.
	These are defined to be parentheses (). */
	TT_parambody_open,
	TT_parambody_close,

	/* Function body open and close
	Used exclusively for creating function bodies. There are four valid ways to create a function:
	a = 5		// Constant-return function, created from a chunk of data.
	a = {}		// Basic, parameter-less function, created from body brackets.
	a = [] {}	// Basic, parameter-less function, created from "[]".
	a = []		// Basic, parameter-less function, created from "[]". */
	TT_body_open,
	TT_body_close,

	/* Object-body open and close
	Used for associating values with persistent scope variables in a function construction
	and/or for listing the names of parameters passed to the function.
	These are defined to be square brackets []. */
	TT_objectbody_open,
	TT_objectbody_close,

	/* Immediately run the function
	Parameter-less call of a function. */
	TT_immediate_run,

	/* End function run
	This is passed to all tasks that are still active when a function ends. */
	TT_function_end_run,

	/* Name
	Obeys name character limit (255 max). Begins with a letter and contains no special characters.
	It may have unicode characters, but not reserved or special characters. */
	TT_name,

	// System command structures
	/* If-statements */
	TT_if,
	TT_elif,
	TT_else,

	/* Loops */
	TT_loop,
	TT_endloop,
	TT_skip,

	/* Boolean
	True and false. These are the only tokens that can be the final value of conditional statements.
	All other types return an error. */
	TT_boolean_true,
	TT_boolean_false,

	/* Number
	Contains only numbers 0 through 9 as letters. */
	TT_number,

	/* String (bit sequence)
	Given between two quotation marks, this is a bit sequence that can be treated as a string. */
	TT_string,

	/* Hide character
	Used for hiding a single character. It is used only in run(). Process will give an error. */
	TT_escape_character,

	/* Hex value
	Given in the format 0xNNNN..., this is a bit sequence.
	This seems less useful since I will have binary. */
	//TT_hex,

	/* Binary
	Given in the format nnnnnb where n=0 or 1, this is a bit sequence.
	ex: 10010100b
	An error is thrown if the bit sequence length is not evenly divisible by 8. */
	TT_binary // Not yet implemented
};

struct TaskResult {
	enum Value {
	_none,		// Task may be waiting for something (possibly an object)
	_pop_loop,	// Task is done, so remove from task stack and continue with other tasks
	_cycle,		// Task consumed the token and a new run cycle must start
							// (task will pop itself if necessary)
	_skip,		// Task has handled the token and set the last object, so skip interpret()
	_error,
	_done		// Something has ended all processing.
	};
};

struct SystemFunction {
	enum Value {
	_unset = 0, // NOT A FUNCTION. Indicates the system function has not been set.
	_return,
	_not, // "not" is an alternate token for "!" in standard C++
	_all,
	_any,
	_nall,
	_none,
	_are_fn,
	_are_empty,
	_are_same, // function/pointer and pointer are for the same object (C++ pointer equality)
	_member,
	_member_count,
	_is_member,
	_set_member,
	_union,
	_type,
	_are_same_type,
	_are_bool,
	_are_string,
	_are_number,
	_assert,

		// Built-in control structures that use a different task
	_own,
	_is_ptr,
	_is_owner,	
	};
};


//----------

struct Token {
	const String name;
	const TokenType type;

	Token( TokenType pType, const String& pName ) : name(pName), type(pType) {}
	Token( TokenType pType ) : name(), type(pType) {}
};

//--------------

struct LogLevel {
	enum Value {
	info,
	warning,
	error,
	debug
	};
};

// Interface for a logger
struct Logger {
	virtual void print(const LogLevel::Value& logLevel, const char* msg)=0;
	virtual void print(const LogLevel::Value& logLevel, const EngineMessage::Value& msg)=0;
	virtual void printFunctionError(unsigned int functionId, unsigned int tokenIndex, const TokenType& tokenType)=0;
};

//----------

// Pre-declaration
class Ref;

struct BadReferenceCountingException {
	int refs;
	const Ref* object;

	BadReferenceCountingException(int pCount, const Ref* pObject)
		: refs(pCount)
		, object(pObject)
	{}
};

// Class Ref
// Used for tracking reference counting for certain types of C++ objects
class Ref {
	int refs;

public:
	Ref() : refs(1) {
#ifdef COPPER_REF_LEVEL_MESSAGES
		std::printf("[REF = [%p]\n", (void*)this);
#endif
	}

	virtual ~Ref() {
		// Some people gawk at this, but it's actually useful in debugging.
#ifdef COPPER_REF_LEVEL_MESSAGES
		std::printf("!! Deletion refs: %i, ptr = %p\n", refs, (void*)this);
		if ( refs != 0 ) {
			std::printf("Bad ref count in dstor.\n");
			throw BadReferenceCountingException(refs, this);
		}
#endif
	}

	void ref() {
		++refs;
#ifdef COPPER_REF_LEVEL_MESSAGES
		std::printf("++refs: %i, ptr = %p\n", refs, (void*)this);
		//std::printf("++refs: %u\n", refs);
#endif
	}

	void deref() {
		--refs;
#ifdef COPPER_REF_LEVEL_MESSAGES
		std::printf("--refs: %i, ptr = %p\n", refs, (void*)this);
		//std::printf("--refs: %u\n", refs);
#endif
		if ( refs < 0 ) {
#ifdef COPPER_REF_LEVEL_MESSAGES
			std::printf("Bad ref count in deref.\n");
#endif
			throw BadReferenceCountingException(refs, this);
		}
		if ( refs == 0 )
			delete this;
	}

	// Should be a DEBUG-only thing
	int getRefCount() {
		return refs;
	}

	// Used when an instance has been created as a stack variable rather than on the heap
	void dropRef() {
		refs -= 1;
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "Ref";
	}
#endif
};

template<typename T>
class RefPtr
#ifdef COMPILE_COPPER_FOR_C_PLUS_PLUS_11
	: std::enable_if<std::is_base_of<Ref, T::value>> // Requires C++11 support
#endif
{
	T* obj;
public:
	RefPtr()
		: obj(REAL_NULL)
	{}

	RefPtr(const RefPtr& pOther)
		: obj(REAL_NULL)
	{
		set(pOther.obj);
	}

	~RefPtr() {
		if ( notNull(obj) )
			obj->deref();
	}

	void set(T* pObject)
	{
		if ( notNull(obj) ) {
			obj->deref();
		}
		obj = pObject;
		if ( notNull(obj) ) {
			obj->ref();
		}
	}

	// Allows for directly setting with a "new" object
	void setWithoutRef(T* pObject)
	{
		if ( notNull(obj) ) {
			obj->deref();
		}
		obj = pObject;
	}

	bool obtain(T*& pStorage) {
		if ( isNull(obj) ) {
			pStorage = REAL_NULL;
			return false;
		}
		pStorage = obj;
		return true;
	}

	T* raw() const {
		return obj;
	}
};

//--------------------
class Object : public Ref {
protected:
	ObjectType::Value type;
public:
	Object()
		: type(ObjectType::Data)
	{}

	virtual ~Object() {}

	ObjectType::Value getType() const {
		return type;
	}

	operator Object*() {
		return (Object*)this;
	}

	// Meant to be overridden.
	// If you return "this", you must call this->ref() to ensure proper memory manangement.
	virtual Object* copy() = 0;

	virtual void writeToString(String& out) const {
		out = "{object}";
	}

	// Name of the data
	/* The Data class can be extended to point to other types of data, such as huge numbers,
	matrices, etc., and complemented by extension functions. */
	virtual const char* typeName() const =0;

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "Object";
	}
#endif
};

//-------------------

class Body : public Ref {
	List<Token> tokens; // This should probably be List<const Token>
public:
	Body() {
#ifdef COPPER_VAR_LEVEL_MESSAGES
		std::printf("[DEBUG: Body constructor [%p]\n", (void*)this);
#endif
	}

	~Body() {
		tokens.clear();
	}

	void addToken(const Token& pToken) {
		tokens.push_back(pToken);
	}

	List<Token>::ConstIter getIterator() {
		return tokens.constStart();
	}

	unsigned int size() const {
		return tokens.size();
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "Body";
	}
#endif
};

//-------------------

class Scope; // predeclaration - Not enough. Function methods need to be in a separate CPP file.

struct Function : public Ref {
	bool constantReturn; // If this function always returns the same, static value (allows skipping run)
	RefPtr<Body> body;
	List<String> params; // Should probably be a pointer so it can be easily set
	RefPtr<Object> result; // Used only for constant-return functions

private:
	Scope* persistentScope;

public:
	Function();
	Function(const Function& pOther); // Do NOT use directly. Use set() for copying.
	~Function();
	Scope& getPersistentScope();
	void set( Function& other );

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "Function";
	}
#endif
};

//-------------------

// Predeclaration
class Variable;

//class RefNullFunctionInContainerException {};
class BadFunctionContainerOwnerException {};
class SettingNullFunctionInContainerException {};
/*
	The idea:

Rather than having to keep track of two types of variables (functions and pointers), it's easier
to treat them as one and have them all point to the same container. However, only one of these
objects is the owner. Only the owner can destroy the data. 

When the owner changes its function, this container deletes the function but keeps itself.
This prevents cyclic memory loops but prevents pointers from accessing dead memory.
Pointers that wish to access the function will reference-count this object rather than the function.

Oddly enough, this has to inherit Object (instead of Function) since this is what is passed along.
*/
class FunctionContainer : public Object {
	RefPtr<Function> funcBox;
	Variable* owner;
	unsigned int ID;

public:
	explicit FunctionContainer(Function* pFunction, unsigned int id=0);

	FunctionContainer();

	// Should only be used for data transfer where the old copy immediately dies
	FunctionContainer(const FunctionContainer& pOther);
//private:
	~FunctionContainer();
//public:
	void own(Variable* pGrabber);

	void disown(Variable* pDropper);

	bool isOwned() const;

	bool isOwner( const Variable* pVariable ) const;

	void changeOwnerTo(Variable* pNewOwner);

	// Should store in a better storage variable, not some open pointer
	bool getFunction(Function*& storage);

	// Used for assignment, which only requires copying
	void setFrom( FunctionContainer& pOther );

	// FunctionContainer* copy()
	virtual Object* copy();

	// Create a function container initialized to the given data
	static FunctionContainer* createInitialized(Object* pData) {
		FunctionContainer* fc = new FunctionContainer();
		Function* f;
		fc->getFunction(f);
		if ( notNull(pData) ) {
			f->result.setWithoutRef(pData->copy());
			f->constantReturn = true;
		}
		return fc;
	}

	virtual void writeToString(String& out) const {
		out = "{fn}";
	}

	virtual const char* typeName() const {
		return "fn";
	}

	unsigned int getID() {
		return ID;
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "FunctionContainer";
	}
#endif
};

//-------------------

class NullVariableException {};

/*
	Class Variable

	As a language rule, all variables MUST point to a function container.
*/
class Variable : public Ref {
	FunctionContainer* box;

public:
	Variable()
		: box(new FunctionContainer())
	{
#ifdef COPPER_VAR_LEVEL_MESSAGES
		std::printf("[DEBUG: Variable constructor [%p]\n", (void*)this);
#endif
		box->own(this);
	}

#ifdef COMPILE_COPPER_FOR_C_PLUS_PLUS_11
	Variable(const Variable& pOther) = delete;
#else
private:
	Variable(const Variable& pOther);
public:
#endif

	~Variable() {
#ifdef COPPER_VAR_LEVEL_MESSAGES
		std::printf("[DEBUG: Variable::~Variable [%p]\n", (void*)this);
#endif
		box->disown(this);
		box->deref();
	}

	void reset() {
#ifdef COPPER_VAR_LEVEL_MESSAGES
		std::printf("[DEBUG: Variable::reset [%p]\n", (void*)this);
#endif
		if ( isNull(box) )
			throw NullVariableException();

		box->disown(this);
		box->deref();
		box = new FunctionContainer();
		box->own(this);
	}

	void set( Variable* pOther, bool pReuseStorage ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
		std::printf("[DEBUG: Variable::set [%p]\n", (void*)this);
#endif
		if ( notNull(pOther) )
			setFunc( pOther->box, pReuseStorage );
		else
			throw NullVariableException();
	}

	void setFunc( FunctionContainer* pContainer, bool pReuseStorage ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
		std::printf("[DEBUG: Variable::setFunc [%p]\n", (void*)this);
#endif
		if ( isNull(pContainer) ) {
			throw BadParameterException<Variable>();
		}
		if ( isNull(box) )
			throw NullVariableException();

		FunctionContainer* fc = REAL_NULL;

		if ( pReuseStorage ) {
			pContainer->ref();
			box->disown(this);
			box->deref();
			box = pContainer;
			box->own(this);
		} else {
			// Data is either not owned or this is supposed to make a copy anyways.
			// Delinks from pointers (which is desired).
			// Copy occurs here in case of assigning box's contents to itself.
			// (See changelog.txt: 2017/1/24 v 0.12)
			fc = (FunctionContainer*)(pContainer->copy());
			box->disown(this);
			box->deref();
			box = fc;
			box->own(this);
		}
	}

	// Used only for data
	void setFuncReturn( Object* pData ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
		std::printf("[DEBUG: Variable::setFuncReturn [%p]\n", (void*)this);
#endif
		if ( isNull(box) )
			throw NullVariableException();

		Function* f = REAL_NULL;
		if ( box->getFunction(f) ) {
//std::printf("[DEBUG: Variable::setFuncReturn: using current function\n");
			f->result.set(pData->copy());
			f->constantReturn = true;
		} else {
//std::printf("[DEBUG: Variable::setFuncReturn: creating new container\n");
			box->disown(this);
			box->deref();
			box = FunctionContainer::createInitialized(pData);
			box->own(this);
		}
	}

	// Used for FuncFound_processRunVariable
	Function* getFunction(Logger* logger) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
		std::printf("[DEBUG: Variable::getFunction [%p]\n", (void*)this);
#endif
		Function* f = REAL_NULL;

		if ( isNull(box) )
			throw NullVariableException();

		if ( box->getFunction(f) ) {
			return f;
		}
		// else:
		// I need to print the variable name (which is only in the scope now)
		// and it would be good to have all the stack info (which can be obtained otherwise).
		if ( notNull(logger) ) {
			logger->print(LogLevel::warning, EngineMessage::NoFunctionInContainer);
		}
		// Conveniently reset this variable if the function no longer exists.
		// This happens when this variable is a pointer and the variable-it-points-to changes.
		box->disown(this);
		box->deref();
		box = new FunctionContainer();
		box->own(this);
		box->getFunction(f);
		return f;
	}

	bool isPointer() const {
#ifdef COPPER_VAR_LEVEL_MESSAGES
		std::printf("[DEBUG: Variable::isPointer [%p]\n", (void*)this);
#endif
		if ( isNull(box) )
			throw NullVariableException();
		return ! box->isOwner(this);
	}

	FunctionContainer* getRawContainer() {
#ifdef COPPER_VAR_LEVEL_MESSAGES
		std::printf("[DEBUG: Variable::getRawContainer [%p]\n", (void*)this);
#endif
		if ( isNull(box) )
			throw NullVariableException();
		return box;
	}

	Variable* getCopy() {
#ifdef COPPER_VAR_LEVEL_MESSAGES
		std::printf("[DEBUG: Variable::getCopy [%p]\n", (void*)this);
#endif
		Variable* var = new Variable();
		if ( isPointer() ) {
			box->ref();
			var->box = box;
			var->box->own(var);
			return var;
		} // else
		FunctionContainer* fc = (FunctionContainer*)(box->copy());
		var->box = fc;
		var->box->own(var);
		return var;
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "Variable";
	}
#endif
};

//-------------------
// ****** DATA CLASSES ******

struct Data : public Object {

	Data()
	{
		type = ObjectType::Data;
	}

	virtual ~Data() {}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "Data";
	}
#endif
};

class ObjectBool : public Data {
	bool value;
public:
	explicit ObjectBool(bool b)
		: Data()
		, value(b)
	{}

	virtual Object* copy() {
		return new ObjectBool(value);
	}

	bool getValue() const {
		return value;
	}

	virtual const char* typeName() const {
		return "bool";
	}

	virtual void writeToString(String& out) const {
		out = value?"true":"false";
	}
};


static const char* OBJECTSTRING_TYPENAME = "string";

class ObjectString : public Data {
	String value;
public:
	explicit ObjectString(const String& pValue)
		: Data()
		, value(pValue)
	{}

	virtual Object* copy() {
		return new ObjectString(value);
	}

	virtual const char* typeName() const {
		return OBJECTSTRING_TYPENAME;
	}

	String& getString() {
		return value;
	}

#ifdef COPPER_PURGE_NON_PRINTABLE_ASCII_INPUT_STRINGS
	void purgeNonPrintableASCII() {
		value.purgeNonPrintableASCII();
	}
#endif

//#ifdef COPPER_PURGE_NON_UTF8_INPUT_STRINGS
	// Clears all non-UTF-8 characters from the string
	//void purgeNonUTF8() {
	//	value.purgeNonUTF8();
	//}
//#endif

	virtual void writeToString(String& out) const {
		out = value;
	}
};

/*
	Interface for all numbers.
	This ensures all numbers will have the same functionality belonging to numbers.
*/
static const char* NUMBER_TYPENAME = "number";

struct Number : public Data {
	virtual ~Number() {}

	virtual unsigned long getAsUnsignedLong() const = 0;

	virtual const char* typeName() const {
		return NUMBER_TYPENAME;
	}
};

/*
	Object numbers are base-10 numbers saved as strings.
	This makes for easy integration with other libraries, like GNU MPC.
*/
class ObjectNumber : public Number {
	String value; // Numeric-only string

public:
	 // TODO: A constructor using CharList

	ObjectNumber()
		: value()
	{}

	explicit ObjectNumber(const unsigned long pValue)
		: value()
	{
		value = CharList(pValue);
	}

	explicit ObjectNumber(const String& pValue)
		: value()
	{
		// Slow constructor for long numbers, otherwise I have to create a filter in Engine::run()
		// TODO: It should throw optional errors or print some warning message for debug,
		// but it would be better to do that in the run() method.
		CharList numbers;
		unsigned int i=0;
		for ( ; i < pValue.size(); ++i) {
			if ( '1' <= pValue[i] && pValue[i] <= '9' ) {
				numbers.push_back(pValue[i]);
			}
			else if ( pValue[i] == '.' ) {
				numbers.push_back('.');
			}
			else if ( numbers.size() > 0 ) {
				if ( pValue[i] == '0' ) {
					numbers.push_back('0');
				}
			}
		}
		value = String(numbers);
	}

	virtual Object* copy() {
		return new ObjectNumber(value);
	}

	unsigned int getDigitCount() const {
		return value.size();
	}

	unsigned char getDigit(unsigned int index) const {
		// // Debating on this:
		// if ( value[index] == '.' ) ++index;
		return value[index]; // Bounds checking done in String class
	}

	virtual void writeToString(String& out) const {
		out = value.c_str();
	}

	String& getRawValue() {
		return value;
	}

	virtual unsigned long getAsUnsignedLong() const {
		return value.toUnsignedLong();
	}
};

struct NumberObjectFactory : public Ref {
	virtual Number* createNumber(const String& pValue) {
		return new ObjectNumber(pValue);
	}
};


//------------------

/* To enable hooks into the engine, some functionality for returning objects via lists is needed.
This interface stands in place for a more concrete list implementation, which would otherwise
prohibit flexibility. */
struct AppendObjectInterface {
	// Append objects
	// Implementers of this function are expected to call ref() on each object passed in.
	virtual void append(Object* pObject)=0;
};


//-------------------

	// External/Foreign Functions
/*
For integrating libraries with this language, classes can directly inherit this interface.
If the function returns, please set the "result" parameter to the resultant object (using set())
and return true. Return false if the result was not set.
*/
class ForeignFunc : public Ref {
public:
	virtual ~ForeignFunc() {}

	virtual bool call(const List<Object*>& params, RefPtr<Object>& result)=0;

	//operator ForeignFunc* () {
	//	return (ForeignFunc*)(*this);
	//}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "ForeignFunc";
	}
#endif
};

/*
Required because
1) GCC can't handle nested templates.
2) Hash table data slots don't handle reference counting.
*/
class ForeignFuncContainer {
	RefPtr<ForeignFunc> data;
public:
	ForeignFuncContainer(ForeignFunc* func)
	{
#ifdef COPPER_FFI_LEVEL_MESSAGES
		std::printf("[DEBUG: ForeignFuncContainer constructor 1 (ForeignFunc*) [%p]\n", (void*)this);
#endif
		data.set(func);
	}
/*
	ForeignFuncContainer(const ForeignFuncContainer& pOther)
	{
		data.set(pOther.data.raw());
	}
*/
	~ForeignFuncContainer() {
#ifdef COPPER_FFI_LEVEL_MESSAGES
		std::printf("[DEBUG: ~ForeignFunction() [%p]\n", (void*)this);
#endif
	}

	ForeignFunc* getForeignFunction() {
		return data.raw();
	}
};

//-------------------

// In order to keep reference counting working, this has to be done.
// Note that users may try to save variables outside the context of the engine.
// QUESTION: Should I leave the varPtr? It doesn't help anymore.
class RefVariableStorage {
	Variable* variable;

public:
	RefVariableStorage()
		: variable( new Variable() )
	{
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
		std::printf("[DEBUG: RefVariableStorage cstor 1 [%p]\n", (void*)this);
#endif
	}

	RefVariableStorage(Variable& refedVariable)
		: variable(REAL_NULL)
	{
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
		std::printf("[DEBUG: RefVariableStorage cstor 2 (Variable&) [%p]\n", (void*)this);
#endif
		// Note: Remove this line if there are copy problems.
		// Copying should be the default, but it is expensive.
		variable = refedVariable.getCopy();
	}

	RefVariableStorage(const RefVariableStorage& pOther)
		: variable(REAL_NULL)
	{
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
		std::printf("[DEBUG: RefVariableStorage cstor 3 (const RefVariableStorage&) [%p]\n", (void*)this);
#endif
		// Note: Remove this line if there are copy problems.
		// Copying should be the default, but it is expensive.
		variable = pOther.variable->getCopy();
	}

	~RefVariableStorage() {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
		std::printf("[DEBUG: RefVariableStorage::~RefVariableStorage [%p]\n", (void*)this);
#endif
		variable->deref();
	}

	Variable& getVariable() {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
		std::printf("[DEBUG: RefVariableStorage::getVariable [%p]\n", (void*)this);
#endif
		return *variable;
	}
};

//-------------------

// Exception classes
class UninitializedStackException {};
class UninitializedStackContextException {};
class NonExistentScopeTableException {};

#ifdef COMPILE_WITH_SCOPE_HASH_NULL_CHECKING
#define CHECK_SCOPE_HASH_NULL(x) if ( !x ) throw NonExistentScopeTableException();
#else
#define CHECK_SCOPE_HASH_NULL(x)
#endif


class Scope : public Ref {
	RobinHoodHash<RefVariableStorage>* robinHoodTable;

protected:
	void copyAsgnFromHashTable( RobinHoodHash<RefVariableStorage>& pTable );

public:
	// Should probably have variable table allocation size as a parameter
	// Or I could have an optional boolean parameter that requests a large variable size
	// The Global Namespace would be better served having a list or tree (since it may be "infinite")
	Scope();

	// What's the point of this dance?
#ifdef COMPILE_COPPER_FOR_C_PLUS_PLUS_11
	Scope(const Scope& pOther) = delete;
#else
private:
	Scope(const Scope& pOther);
public:
#endif

	~Scope();

	Scope& operator = (Scope& pOther);

	bool variableExists(const String& pName);

	// Adds an empty variable and returns a pointer to it
	Variable* addVariable(const String& pName);

	// Gets the variable, creating it if it does not exist
	void getVariable(const String& pName, Variable*& pVariable);

	// Looks for the variable. If the function exists, it saves it to storage.
	// Returns "true" if the variable was found, false otherwise.
	// Shouldn't the string here be const?
	bool findVariable(const String& pName, Variable*& pStorage);

	// Sets the variable, creating the variable if it does not exist
	// The pReuseStorage option may be ignored if the FunctionContainer is not owned
	void setVariable(const String& pName, Variable* pSourceVariable, bool pReuseStorage);

	// Sets the variable, creating the variable if it does not exist
	void setVariableFrom(const String& pName, Object* pObject, bool pReuseStorage);

	// Appends to the given list all the names of the members in this scope.
	// Could be turned into List<Object> and return object strings but there is no built-in list class.
	void appendNamesByInterface(AppendObjectInterface* aoi);

	// Copies members from another scope to this one
	void copyMembersFrom(Scope& pOther);

	// Number of occupied storage slots / actual Variables (there may be more storage allocated)
	unsigned int occupancy();

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "Scope";
	}
#endif
};


// Pre-declaration
class Stack;

class StackFrame : public Ref {
	friend Stack;

	StackFrame* parent; // Set by Stack
	Scope* scope;

public:
	StackFrame()
		: parent(REAL_NULL)
		, scope(new Scope())
	{}

	StackFrame(const StackFrame& pOther)
		: parent(REAL_NULL)
		, scope(REAL_NULL)
	{
		// Copy, don't share
		*scope = *(pOther.scope);
	}

	~StackFrame() {
		scope->deref();
	}

	Scope& getScope() {
		return *scope;
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "StackFrame";
	}
#endif
};


enum StackPopReturn {
	STACK_POP_more,
	STACK_POP_done	// Nothing left on the stack
};

// Stack class
class Stack {
	unsigned int size;
	// Singly Linked List from top-to-bottom
	StackFrame* bottom;
	StackFrame* top;

public:
	Stack();
#ifdef COMPILE_COPPER_FOR_C_PLUS_PLUS_11
	Stack(const Stack& pOther) = delete;
#else
private:
	Stack(const Stack& pOther);
public:
#endif
	~Stack();
	void clear();
	StackFrame& getBottom();
	StackFrame& getTop();
	unsigned int getCurrLevel();
	StackPopReturn pop();
	void push();
	void push(StackFrame* pContext);
};


//========== Types of tasks and their states ============


struct TaskName {
	enum Value {
		_Function_Construct=0,
		_Function_Found,
		_if,
		_loop,
		_own,
		_is_ptr,
		_is_owner
	};
};

struct Task : public Ref {
	TaskName::Value name;

	Task(TaskName::Value pName) : name(pName) {}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "Task";
	}
#endif
};

class TaskContainer {
	Task* task;
public:
	// Expected to be passed: new Task() (e.g. "new TaskFunctionConstruct()")
	TaskContainer(Task* newTask)
		: task(newTask)
	{}

	TaskContainer(const TaskContainer& pOther)
		: task(pOther.task)
	{
		task->ref();
	}

	~TaskContainer() {
		task->deref();
	}

	// returning an address prevents deleting the pointer
	Task& getTask() { return (Task&)*task; }

	bool areSameTask(Task* pOther) {
		return task == pOther;
	}
};

enum TaskFunctionConstructState {
	TASK_FCS_param_collect,		// Collecting parameter names
	TASK_FCS_param_value,		// Collecting parameter value (for an instantiated parameter)
	TASK_FCS_param_ptr_value,	// Collecting parameter pointer value (for an instantiated parameter)
	TASK_FCS_end_param_collect,	// Closing parenthesis found and now searching for open body bracket
	TASK_FCS_from_bodystart		// Can act as a start state (note: since the open bracket has been obtained, this merely adds to the body)
};

struct TaskFunctionConstruct : public Task {
	TaskFunctionConstructState state;
	//RefPtr<Function> functionPtr; // Too much of just obtaining the func when nothing else accesses it
	Function* function;
	Body* body;
	unsigned int openBodies;
	bool hasOpenParam;
	String paramName;

	TaskFunctionConstruct(TaskFunctionConstructState pStartState)
		: Task(TaskName::_Function_Construct)
		, state(pStartState)
		, function( new Function() )
		, body( new Body() )
		, openBodies(0)
		, hasOpenParam(false)
		, paramName()
	{
#ifdef COPPER_TASK_LEVEL_MESSAGES
		std::printf("[DEBUG: TaskFunctionConstruct cstor 1 (TaskFunctionConstructState) [%p]\n", (void*)this);
#endif
		if ( state == TASK_FCS_from_bodystart )
			openBodies = 1;
		function->body.set(body);
	}

	TaskFunctionConstruct(const TaskFunctionConstruct& pOther)
		: Task(TaskName::_Function_Construct)
		, state( pOther.state )
		, function( pOther.function )
		, body( pOther.body )
		, openBodies(0)
		, hasOpenParam(false)
		, paramName()
	{
#ifdef COPPER_TASK_LEVEL_MESSAGES
		std::printf("[DEBUG: TaskFunctionConstruct cstor 2 (const TaskFunctionConstruct&) [%p]\n", (void*)this);
#endif
		if ( state == TASK_FCS_from_bodystart )
			openBodies = 1;
		function->ref();
		body->ref();
	}

	~TaskFunctionConstruct() {
#ifdef COPPER_TASK_LEVEL_MESSAGES
		std::printf("[DEBUG: TaskFunctionConstruct::~TaskFunctionConstruct [%p]\n", (void*)this);
#endif
		function->deref();
		body->deref();
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "TaskFunctionConstruct";
	}
#endif
};

enum TaskFunctionFoundState {
	TASK_FF_start,
	TASK_FF_assignment,
	TASK_FF_pointer_assignment,
	TASK_FF_find_member,
	TASK_FF_collect_params
};

enum TaskFunctionFoundType {
	TASK_FFTYPE_variable,
	TASK_FFTYPE_cside,
	TASK_FFTYPE_ext_cside // May be necessary as I may force saving a reference to the external
};

// Created when a function has been identified/created.
// It checks for assignment (copy and pointer) or calling, and returns either itself or its function return.
struct TaskFunctionFound : public Task {
	TaskFunctionFoundState state;
	TaskFunctionFoundType type;
	unsigned int openParamBodies; // Note: Copper.cpp line 893 (1961 now?) depends on max value being for uint
private:
	List<Object*> params;
public:
	RefPtr<Variable> varPtr;
	RefPtr<Variable> superPtr;
	ForeignFunc* extFuncPtr;
	SystemFunction::Value cSideCallback;

	TaskFunctionFound( Variable* pVar )
		: Task(TaskName::_Function_Found)
		, state(TASK_FF_start)
		, type(TASK_FFTYPE_variable)
		, openParamBodies(0)
		, params()
		, varPtr()
		, superPtr()
		, extFuncPtr(REAL_NULL)
		, cSideCallback(SystemFunction::_unset)
	{
#ifdef COPPER_TASK_LEVEL_MESSAGES
		std::printf("[DEBUG: TaskFunctionFound cstor 1 (Variable*) [%p]\n", (void*)this);
#endif
		varPtr.set(pVar);
	}

	TaskFunctionFound( ForeignFunc* pForeignFunc )
		: Task(TaskName::_Function_Found)
		, state(TASK_FF_start)
		, type(TASK_FFTYPE_ext_cside)
		, openParamBodies(0)
		, params()
		, varPtr()
		, superPtr()
		, extFuncPtr(pForeignFunc)
		, cSideCallback(SystemFunction::_unset)
	{
#ifdef COPPER_TASK_LEVEL_MESSAGES
		std::printf("[DEBUG: TaskFunctionFound cstor 2 (ForeignFunc*) [%p]\n", (void*)this);
#endif
	}

	TaskFunctionFound( SystemFunction::Value pCallback )
		: Task(TaskName::_Function_Found)
		, state(TASK_FF_start)
		, type(TASK_FFTYPE_cside)
		, openParamBodies(0)
		, params()
		, varPtr()
		, superPtr()
		, extFuncPtr(REAL_NULL)
		, cSideCallback(pCallback)
	{
#ifdef COPPER_TASK_LEVEL_MESSAGES
		std::printf("[DEBUG: TaskFunctionFound cstor 3 (SystemFunction) [%p]\n", (void*)this);
#endif
	}

#ifdef COMPILE_COPPER_FOR_C_PLUS_PLUS_11
	TaskFunctionFound(const TaskFunctionFound& pOther) = delete;
#else
private:
	TaskFunctionFound(const TaskFunctionFound& pOther);
public:
#endif

	~TaskFunctionFound()
	{
#ifdef COPPER_TASK_LEVEL_MESSAGES
		std::printf("[DEBUG: TaskFunctionFound::~TaskFunctionFound [%p]\n", (void*)this);
#endif
		// Must dereference objects
		List<Object*>::Iter i = params.start();
		if ( params.has() )
		do {
			(*i)->deref();
		} while ( ++i );
		//params.clear(); // Should be automatically called
	}

	void addParam( Object* pObject ) {
#ifdef COPPER_TASK_LEVEL_MESSAGES
		std::printf("[DEBUG: TaskFunctionFound::addParam [%p]\n", (void*)this);
#endif
		// If I don't call some kind of ref(), then the parameters will die as they are
		// added to the list and lastObject is changed.
		if ( notNull(pObject) ) {
			pObject->ref();
			params.push_back(pObject);
		}
	}

	Object* getParam(unsigned int index) {
#ifdef COPPER_TASK_LEVEL_MESSAGES
		std::printf("[DEBUG: TaskFunctionFound::getParam [%p]\n", (void*)this);
#endif
		return params[index];
	}

	unsigned int getParamCount() const {
#ifdef COPPER_TASK_LEVEL_MESSAGES
		std::printf("[DEBUG: TaskFunctionFound::getParamCount = %u [%p]\n", params.size(), (void*)this);
#endif
		return params.size();
	}

	List<Object*>::Iter createParamsIterator() {
#ifdef COPPER_TASK_LEVEL_MESSAGES
		std::printf("[DEBUG: TaskFunctionFound::createParamsIterator [%p]\n", (void*)this);
#endif
		return params.start();
	}

	// Used only for external functions
	// May be removed or changed.
	const List<Object*>& getParamsList() {
#ifdef COPPER_TASK_LEVEL_MESSAGES
		std::printf("[DEBUG: TaskFunctionFound::getParamsList [%p]\n", (void*)this);
#endif
		return params;
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "TaskFunctionFound";
	}
#endif
};

// If-structures are handled by treating each if, elif, and else as a section of code meant to be run.
// if and elif are conditionally run whereas else always runs if no previous section had run
// Only if a body of code has not been run will the next section execute.
// Each section is scanned and must check for the "ran" to see if it should check the condition and
// possible execute its body of code.
enum TaskIfStructureState {
	TASK_IF_find_cond,
	TASK_IF_get_cond,
	TASK_IF_seek_cond_end,
	TASK_IF_seek_body,
	TASK_IF_run_body,
	TASK_IF_seek_body_end,
	TASK_IF_seek_else // or elif
};

struct TaskIfStructure : public Task {
	TaskIfStructureState state;
	bool ran;
	bool isConditionSet;
	bool condition;
	unsigned int openParamBodies;
	unsigned int openBodies;

	TaskIfStructure()
		: Task( TaskName::_if )
		, state( TASK_IF_find_cond )
		, ran(false)
		, isConditionSet(false)
		, condition(false)
		, openParamBodies(0)
		, openBodies(0)
	{}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "TaskIfStructure";
	}
#endif
};

enum TaskLoopStructureState {
	TASK_LOOP_find_body,
	TASK_LOOP_collect_body,
	TASK_LOOP_run
};

struct TaskLoopStructure : public Task {
	TaskLoopStructureState state;
	unsigned int openBodies;
	Body body;

	TaskLoopStructure()
		: Task( TaskName::_loop )
		, state(TASK_LOOP_find_body)
		, openBodies(0)
		, body()
	{
		body.dropRef(); // Bodies are created as constant, so we don't worry about reference counting here.
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "TaskLoopStructure";
	}
#endif
};

enum TaskProcessNamedState {
	TASK_PROC_NAMED_find_opening,
	TASK_PROC_NAMED_param_name,
	TASK_PROC_NAMED_find_closing,
	TASK_PROC_NAMED_find_member
};

struct TaskProcessNamed : public Task {
	TaskProcessNamedState state;
	RefPtr<Variable> varPtr;

	TaskProcessNamed(TaskName::Value pName)
		: Task( pName )
		, state(TASK_PROC_NAMED_find_opening)
		, varPtr()
	{}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "TaskProcessNamed";
	}
#endif
};

//-------------------------------
// Helper functions

bool isObjectFunction( const Object& pObject );
bool isObjectEmptyFunction( const Object& pObject );
bool isObjectBool( const Object& pObject );
bool getBoolValue( const Object& pObject );
bool isObjectString( const Object& pObject );
bool isObjectNumber( const Object& pObject );
	// Use this for checking your own types
bool isObjectOfType( const Object& pObject, const char* pTypeName );

//--------------------------------

struct EngineEndProcCallback {
	virtual void CuEngineDoneProcessing()=0;
};

//----------------------------------------

/*
Processing begins with run(), which takes a user string and tokenizes it.
It then sends it to process(), where the token causes changes in the state machine.
run() is used to filter out bad tokens, so passing any bad tokens to process() could be fatal.
You can use process() directly if you know what you are doing.
*/
class Engine {
	Logger* logger;
	Stack stack;
	List<TaskContainer> taskStack; // Needs a getLast() method
	RefPtr<Object> lastObject;
	unsigned int functionID;
	bool functionReturnMailbox; // Set by the function "return" task to inform a function of its ending
	bool loopEndMailbox;
	bool loopSkipMailbox;
	bool systemExitTrigger;
	EngineEndProcCallback* endMainCallback;
	RobinHoodHash<SystemFunction::Value> builtinFunctions;
	RobinHoodHash<ForeignFuncContainer> foreignFunctions;
	bool ownershipChangingEnabled;
	bool stackTracePrintingEnabled;
	RefPtr<NumberObjectFactory> numberObjectFactoryPtr;
	bool (* nameFilter)(const String& pName);

public:
	Engine(); // remember to initialize the logger

	// WARNING: Logger is never referenced or dropped, so don't delete the logger before the Engine!
	void setLogger( Logger* pLogger );

	// Print functions. Both of these call the logger (if it exists) or print to stdout (if enabled).
	void print( const LogLevel::Value& logLevel, const char* msg ) const;
	void print( const LogLevel::Value& logLevel, const EngineMessage::Value& msg ) const;

	// WARNING: endMainCallback is never referenced or dropped, so don't delete it before the Engine!
	void setEndofProcessingCallback( EngineEndProcCallback* pCallback ) {
		endMainCallback = pCallback;
	}

	// Print the stack trace when a function fails
	void setStackTracePrintingEnabled( bool on ) {
		stackTracePrintingEnabled = on;
	}

	// Note: You will need to call "deref" (on the factory) separately anyways.
	void setNumberObjectFactory( NumberObjectFactory* pFactory ) {
		numberObjectFactoryPtr.set(pFactory);
	}

	/* Set the filter used for checking the validity of names.
	The filter should return true if the name is valid.
	Such a filter can be used to check for different Unicode formats. */
	void setNameFilter( bool(*filter)(const String&) ) {
		nameFilter = filter;
	}

	/* Add an external/foreign function to the virtual machine, accessible from within Copper.
	\param pName - The alias within Copper by which this function can be called.
	\param pFunction - Pointer to the function to be called.
	*/
	void addForeignFunction( const String& pName, ForeignFunc* pFunction );

	/*
	\param pName - The token's assume name.
	\return - Returns the token that this name resolves to or TT_unknown if it does not resolve. */
	TokenType resolveTokenType( const String& pName );

	/* Run Copper code.
	This accepts bytes from a byte stream and treats it as Copper code. */
	EngineResult::Value run( ByteStream& stream );

	/*
	\param pToken - The token to be processed.
	If the token is invalid, an error will be returned before any processing has been done.
	NOTE: Some errors are meant to be caught in tokenize() and will therefore create internal problems
	if not caught. */
	EngineResult::Value process( const Token& pToken );

	/*
	This is meant to be called by extensions that need to run functions passed to them.
	\param pFunction - the function container with the function whose body is to be run.
	This is an object that the external function may receive as a parameter, so there's no need to modify.
	\param pParams - the parameters that are passed to this function.
	\return - Signal indicating the success (EngineResult::Ok) or failure (EngineResult::Error)
		of processing. Return may also indicate the end of the VM running (EngineResult::Done).
	*/
	EngineResult::Value runFunction( FunctionContainer* pFunction, const List<Object*>& pParams );

protected:
	void clearStacks();
	void signalEndofProcessing();
	void printFunctionError( unsigned int id, unsigned int tokenIdx, const Token& token ) const;
	TaskResult::Value processTasksOnObjects();
	TaskResult::Value performObjProcessAndCycle(const Token& lastToken);
	Result::Value tokenize( CharList& tokenValue, List<Token>& tokens );
	Result::Value handleCommentsStringsAndSpecials(
						const TokenType& tokenType,
						CharList& tokenValue,
						List<Token>& tokens,
						ByteStream& stream );
	TaskResult::Value interpret( const Token& pToken );
	bool isWhitespace(const char c) const;
	bool isSpecialCharacter(const char c, TokenType& pTokenType);
	bool isEscapeCharacter(const char c) const;
	bool isValidToken( const TokenType& token ) const;
	bool isValidNameCharacter( const char c ) const;
	bool isValidName( const String& pName ) const;
	bool isCommentToken(const char c) const;
	bool isStringToken(const char c) const;
	Result::Value scanComment( ByteStream& stream );
	Result::Value collectString( ByteStream& stream, CharList& collectedValue );

	// For the individual token types
	void processBodyOpen();
	void constructFunctionFromObjBody();
	void processFunctionReturn();
	void constructBoolean(bool value);
	void constructString(const String& value);
	void constructNumber(const String& value);
	bool taskpushSystemFunction( const String& pName );
	bool taskpushExternalFunction( const String& pName );
	void taskpushUserFunction( const String& pName );

private:
	void				setupSystemFunctions();
	TaskResult::Value	processTask(Task& task, const Token& lastToken);
	TaskResult::Value	processTaskOnLastObject(Task& task);

	// Note: The actual order in the file is that related functions are one-after-another
	TaskResult::Value	processFunctionConstruct(TaskFunctionConstruct& task, const Token& lastToken);
	TaskResult::Value	processFunctionFound(TaskFunctionFound& task, const Token& lastToken);
	TaskResult::Value	processIfStatement(TaskIfStructure& task, const Token& lastToken);
	TaskResult::Value	processLoop(TaskLoopStructure& task, const Token& lastToken);
	TaskResult::Value	processNamed(TaskProcessNamed& task, const Token& lastToken);

	TaskResult::Value	processObjForFunctionConstruct(TaskFunctionConstruct& task);
	TaskResult::Value	processObjForFunctionFound(TaskFunctionFound& task);
	TaskResult::Value	processObjForIfStatement(TaskIfStructure& task);

	/*
		PLEASE NOTE:
		Task subprocesses should not have to clean up their data on failure.
		This should all be handled when the task itself is deleted, esp. by
		dropping references (reference counting).
	*/

	TaskResult::Value	FuncBuild_processAtParamCollect(TaskFunctionConstruct& task, const Token& lastToken);
	TaskResult::Value	FuncBuild_processCollectPointerValue(const Token& lastToken);
	TaskResult::Value	FuncBuild_processEndParamCollect(TaskFunctionConstruct& task, const Token& lastToken);
	TaskResult::Value	FuncBuild_processFromBodyStart(TaskFunctionConstruct& task, const Token& lastToken);
	unsigned int generateFunctionID();

	TaskResult::Value	FuncFound_processFromStart(TaskFunctionFound& task, const Token& lastToken);
	TaskResult::Value	FuncFound_processAssignment(const Token& lastToken);
	TaskResult::Value	FuncFound_processPointerAssignment(const Token& lastToken);
	TaskResult::Value	FuncFound_processFindMember(TaskFunctionFound& task, const Token& lastToken);
	TaskResult::Value	FuncFound_processCollectParams(TaskFunctionFound& task, const Token& lastToken);
	TaskResult::Value	FuncFound_processRun(TaskFunctionFound& task);

	TaskResult::Value	FuncFound_processRunVariable(TaskFunctionFound& task);
	TaskResult::Value	FuncFound_processRunSysFunction(TaskFunctionFound& task);
	void				process_sys_return(TaskFunctionFound& task);
	void				process_sys_not(TaskFunctionFound& task);
	void				process_sys_all(TaskFunctionFound& task);
	void				process_sys_any(TaskFunctionFound& task);
	void				process_sys_nall(TaskFunctionFound& task);
	void				process_sys_none(TaskFunctionFound& task);
	void				process_sys_are_fn(TaskFunctionFound& task);
	void				process_sys_are_empty(TaskFunctionFound& task);
	void				process_sys_are_same(TaskFunctionFound& task);
	EngineResult::Value	process_sys_member(TaskFunctionFound& task);
	EngineResult::Value	process_sys_member_count(TaskFunctionFound& task);
	EngineResult::Value	process_sys_is_member(TaskFunctionFound& task);
	EngineResult::Value	process_sys_set_member(TaskFunctionFound& task);
	void				process_sys_union(TaskFunctionFound& task);
	void				process_sys_type(TaskFunctionFound& task);
	void				process_sys_are_same_type(TaskFunctionFound& task);
	void				process_sys_are_bool(TaskFunctionFound& task);
	void				process_sys_are_string(TaskFunctionFound& task);
	void				process_sys_are_number(TaskFunctionFound& task);
	EngineResult::Value	process_sys_assert(TaskFunctionFound& task);


	// Please recall: running a function without a body results in a new empty function

	// Note when assigning (from =):
	// It must differentiate between assigning a function and assigning a regular object
	// since the former must be copied while the latter is saved as the direct return.
	// Pointer assignment (from ~) must ALWAYS be to a function, and should return an error when not.
	TaskResult::Value	FunctionFound_processAssignmentObj(TaskFunctionFound& task);
	TaskResult::Value	FunctionFound_processPointerAssignmentObj(TaskFunctionFound& task);

	TaskResult::Value	ifStatement_findCondition(TaskIfStructure& task, const Token& lastToken);
	TaskResult::Value	ifStatement_getCondition(TaskIfStructure& task, const Token& lastToken);
	TaskResult::Value	ifStatement_seekBody(TaskIfStructure& task, const Token& lastToken);
	TaskResult::Value	ifStatement_seekBodyEnd(TaskIfStructure& task, const Token& lastToken);
	TaskResult::Value	ifStatement_seekElse(TaskIfStructure& task, const Token& lastToken);

	TaskResult::Value	loopStructure_runBody(TaskLoopStructure& task);

	void				processNamed_collectVariable(TaskProcessNamed& task, const Token& lastToken);
	void				processNamed_collectMember(TaskProcessNamed& task, const Token& lastToken);
};

}

#endif
