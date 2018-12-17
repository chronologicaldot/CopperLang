// Copyright 2016 Nicolaus Anderson

#ifndef COPPER_LANG_H
#define COPPER_LANG_H

// ******* Debug *******

//#define COPPER_REF_LEVEL_MESSAGES
//#define COPPER_VAR_LEVEL_MESSAGES
//#define COPPER_FFI_LEVEL_MESSAGES
//#define COPPER_SCOPE_LEVEL_MESSAGES
//#define COPPER_TASK_LEVEL_MESSAGES
//#define COPPER_DEBUG_ADDRESS_MESSAGES
//#define COPPER_PARSER_LEVEL_MESSAGES
//#define COPPER_DEBUG_ADDRESS
//#define COPPER_DEBUG_ENGINE_MESSAGES
//#define COPPER_USE_DEBUG_NAMES
//#define COPPER_DEBUG_STACK

// Shows the type of each opcode that is executed
//#define COPPER_OPCODE_DEBUGGING

//----- Strict Checks

// To force the interpreter to perform checks where the state machine should fulfill assumptions.
// These checks should not be necessary for safety of the final VM, but they aid in debugging.

// IF YOU MODIFY THE PARSER, YOU SHOULD ENABLE THIS FLAG!
//#define COPPER_STRICT_CHECKS

// Checks the "box" of instances of Variable.
//#define COPPER_VAR_STRICT_CHECKS

//-----

//#define COPPER_PRINT_ENGINE_PROCESS_TOKENS
//#include <cstdio>

//#define COPPER_SPEED_PROFILE

#ifdef COPPER_SPEED_PROFILE
#include <ctime>
#include <sys/time.h>
#include <cstdio>
#endif

// Strict check for debug, ensuring variable scopes are never null
//#define COMPILE_WITH_SCOPE_HASH_NULL_CHECKING

// For getting rid of the unused argument complaints by GCC.
#ifdef __GNUC__
# define CU_UNUSED_ARG(x) x __attribute__((__unused__))
#else
# define CU_UNUSED_ARG(x) x
#endif


// ******* Null *******

#ifdef REAL_NULL
#undef REAL_NULL
#endif

	// Note: "__cplusplus" indicates the version on linux, at least in the standard library.
	// See /usr/include/c++/#.#/numeric, line 126
	// I don't know what other operating systems use.
	// However, the user might want to continue using 0 for null. Otherwise, you can use the following line:
//#if (defined(__cplusplus) && __cplusplus >= 201103L)
	//... instead of...
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

#define COPPER_INTERPRETER_VERSION 0.621
#define COPPER_INTERPRETER_BRANCH 6

// ******* Language version *******

#define COPPER_LANG_VERSION 3.2

// ******* Language modifications *******

// Uncomment to prevent the creation of non-ASCII strings
//#define COPPER_PURGE_NON_PRINTABLE_ASCII_INPUT_STRINGS

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

//! Open-body token cap
// While the engine technically supports the full range of uint,
// the actual memory available to the program is limited and hampered by the fact
// that many things need to be created for each function call. Consequently, the number
// of active open function parameter bodies and execution bodies must be limited.
#define PARSER_OPENBODY_MAX_COUNT 350

//! Initial size of function scope
#define CU_FUNCTION_SCOPE_SIZE 10

//! Initial size of stack frame scope
#define CU_STACK_FRAME_SCOPE_SIZE 100

//! Allows for bounds-checking on integers
// Slow but safe. Requires <limits>, however.
//#define ENABLE_COPPER_NUMERIC_BOUNDS_CHECKS


// ******* Error templates *******

template<typename T>
class UnimplementedFunction {};

template<typename T>
class BadParameterException {};


// ******* System includes *******

#include "utilList.h"
#include "RHHash.h"
#include "Strings.h"

namespace Cu {

// ******* External Classes in Use ********

using util::uint;
using util::List;
using util::CharList;
using util::String;
using util::RobinHoodHash;

// ******* Virtual Machine Types ********

typedef unsigned int	UInteger;
typedef long int		Integer;
typedef double			Decimal;

static const Decimal DECIMAL_ROUNDING_ERROR = 0.0000001;

// ******* Operating System Info ********

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
//ifdef _MSC_VER
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
	/* Function does not exist when trying to access it via FunctionObject.
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
	// Stream ended before parsing completed.
	StreamHaltedParsingNotDone,

	// ERROR
	// Stream ended before comment ended.
	StreamHaltedInComment,

	// ERROR
	// Stream ended before string ended.
	StreamHaltedInString,

	// ERROR
	// Parameter body has been opened without being preceded by a name.
	OrphanParamBodyOpener,

	// ERROR
	// Parameter body has been closed without being opened.
	OrphanParamBodyCloser,

	// ERROR
	// Object body was started but the stream ended.
	OrphanObjectBodyOpener,

	// ERROR
	// Object body has not been opened before but a closing token has been encountered.
	OrphanObjectBodyCloser,

	// ERROR
	// Function body has been closed without first being opened.
	// (Should only happen at the global scope.)
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
	// Invalid token encountered in the object body construction.
	ObjectBodyInvalid,

	// ERROR
	// Unfinished or invalid parameter assignment in an object body construction.
	ObjectBodyInvalidParamAsgn,

	// ERROR
	// Unfinished or invalid parameter pointer assignment in an object body construction.
	ObjectBodyInvalidParamPtrAsgn,

	// ERROR
	// Unfinished execution body.
	ExecBodyUnfinished,

	// ERROR
	// Trying to create more bodies within a function than feasible.
	ExceededBodyCountLimit,

	// ERROR
	// Trying to assign the wrong kind of token to a variable.
	// May also be stream ended before assignment completed.
	InvalidAsgn,

	// ERROR
	// Trying to make a variable a pointer to a non-object via wrong kind of token.
	// May also be stream ended before pointer assignment completed.
	InvalidPtrAsgn,

	// ERROR
	// Non-name followed member accessor (.).
	// May also be stream ended before member access completed.
	InvalidTokenAfterMemberAccessor,

	// ERROR
	// Trying to create more parameter bodies within a function header than feasible.
	ExceededParamBodyCountLimit,

	// ERROR
	// Stream ended before arguments could be collected for a function call.
	ArgBodyIncomplete,

	// ERROR
	// Invalid arguments passed to a function. (Parsing stage ONLY)
	InvalidFunctionArguments,

	// WARNING
	/* Missing parameter for a function call.
	This results in a default value (of empty function) for the parameter. */
	MissingFunctionCallArg,

	// ERROR
	// Attempting to treat a built-in function like a variable.
	SystemFuncInvalidAccess,

	// Warning
	// Attempting to treat a foreign function like a variable.
	ForeignFuncInvalidAccess,

	// ERROR
	// Last-object pointer was not set before object was to be assigned to a variable.
	// Unused but preserved in case it is to be used later.
	UnsetLastObjectInAsgn,

	// ERROR
	// Last-object pointer was not set before a pointer-to-it was to be assigned to a variable.
	// Unused but preserved in case it is to be used later.
	UnsetLastObjectInPtrAsgn,

	// ERROR - May be changed to WARNING
	// An attempt was made to assign raw data to a variable via the pointer assignment token.
	// NOTE: This is a run-time error because the data may be the return of a function call.
	// Unused but preserved in case it is to be used later.
	DataAssignedToPtr,

	// ERROR
	// After the if-statement, a condition (parameter body opener) did not immediately follow.
	InvalidTokenAtIfStart,

	// ERROR
	// If-structure won't have a boolean in its condition or condition contamination will occur.
	// e.g.: true if() { print("runs") }
	IfConditionContaminated,

	// ERROR
	// No boolean result came from the condition body given to the if-statement.
	// If this happens for successive if and elif statements, the else block is still executed.
	ConditionlessIf,

	// ERROR
	// A different token was found while searching for the body-opener token of an if-statment.
	InvalidTokenBeforeIfBody,

	// ERROR
	// A stray porameter was found while searching for the body of a loop.
	StrayTokenInLoopHead,

	// ERROR
	// A stray loop-stop token was found.
	// To allow such stops would contaminate the opcode execution process.
	UselessLoopStopper,

	// ERROR
	// A stray loop-skip token was found.
	// To allow such skips would contaminate the opcode execution process.
	UselessLoopSkipper,

	// WARNING
	// The "own" control structure is disabled.
	PointerNewOwnershipDisabled,

	// ERROR
	// A token other than a parameter body opener was found after "own" or "is_ptr".
	InvalidTokenBeforePtrHandlerParamBody,

	// WARNING
	// A variable was not created before being used in an "own" or "is_ptr" structure, making the structure needless.
	// Unused but preserved in case it is desired later.
	NonexistentVariablePassedToPtrHandler,

	// Warning
	// Invalid name passed to "own", "is_ptr", or "is_owner".
	NonVariablePassedToPtrHandler,

	// ERROR
	// Encountered a non-string token when searching for one in "own" or "is_ptr" structure.
	NonNameFoundInPtrHandlerParamBody,

	// ERROR
	// A token other than the parameter body ending was found before "own" or "is_ptr" ended (after parameter collection).
	// For Copper branch 4, this is the same as breaking a member-link chain, i.e. is_ptr(a.)
	InvalidTokenForEndingPtrHandlerParamBody,

	// User-induced error
	AssertionFailed,

	// ERROR
	// Wrong number of arguments passed to a function
	WrongArgCount,

	// ERROR
	// Wrong argument type of argument passed to a function
	WrongArgType,

	// ERROR
	// Destroyed function as argument
	DestroyedFuncAsArg,

	// ERROR
	// An critical argument was excluded, making the results of the function meaningless.
	MissingArg,

	// WARNING
	// The provided index (for a list) was out of bounds. The index will be substituted by a valid value.
	IndexOutOfBounds,

	// ERROR
	// Accessing non-existent argument for a foreign function
	BadArgIndexInForeignFunc,

	// ERROR
	// An invalid arg was passed to a system function.
	// Could have a number of causes.
	SystemFunctionBadArg,

	// ERROR
	// The user-function contains errors.
	// Usually, the other error will be printed. This just indicates that the problem is within a function body.
	UserFunctionBodyError,

	// UNKNOWN
	CustomMessage,

	// Total number of engine messages
	COUNT

	};
};

//--------------

struct ObjectType {
	enum Value {
		Function	= 0, // Also handles pointers

		// Built-in data
		Bool,
		String,
		List,
		Numeric, // Parent class
		Integer,
		DecimalNum,

		// For non-usable data
		UnknownData,

		// Forces 32-bit compilation so that any user integer can be used to extend the enum
		FORCE_32BIT = 0x7fffffff
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

struct ExecutionResult {
	enum Value {
		Ok,
		Error,
		Done,		// Used for indicating end-main/end processing
		Reset		// Used for resetting the operation strand parsing (for user functions)
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
	TT_execbody_open,
	TT_execbody_close,

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

	/* String (bit sequence)
	Given between two quotation marks, this is a bit sequence that can be treated as a string. */
	TT_string,

	/* Integer
	Contains only the numbers 0 through 9. */
	TT_num_integer,

	/* Decimal
	Contains only the numbers 0 through 9 and a decimal place. */
	TT_num_decimal,

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
	//TT_binary, // Not yet implemented
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
	_xall,
	_are_fn,
	_are_empty,
	_are_same, // function/pointer and pointer are for the same object (C++ pointer equality)
	_member,
	_member_count,
	_is_member,
	_set_member,
	_member_list,
	_union,
	_type,
	_are_same_type,
	_are_bool,
	_are_string,
	_are_list,
	_are_number,
	_are_integer,
	_are_decimal,
	_assert,
	_copy,
	_execute_with_alt_super, // "xwsv"
	_share_body,

	_make_list,		// "list"
	_list_size,		// "length"
	_list_append,	// "append"
	_list_prepend,	// "prepend"
	_list_insert,	// "insert"
	_list_get_item,	// "item_at"
	_list_remove,	// "erase"
	_list_clear, 	// "dump"
	_list_swap,		// "swap"
	_list_replace,	// "replace"
	_list_sublist,	// "sublist"

	_string_match,	// "matching"
	_string_concat,	// "concat"

	_num_equal,				// "equal"
	_num_greater_than,		// "gt"
	_num_greater_or_equal,	// "gte", greater than or equal
	_num_less_than,			// "lt"
	_num_less_or_equal,		// "lte", less than or equal
	_num_abs,				// "abs", absolute value
	_num_add,				// "add" or "+"
	_num_subtract,			// "subtract" or "sbtr" or "-"
	_num_multiply,			// "multiply" or "mul" or "mult" or "*"
	_num_divide,			// "divide" or "divd" or "/"
	_num_modulus,			// "modulus" or "mod" or "%"
	_num_incr,				// "incr" or "++"
	_num_incr_2,			// "incr" or "++" with 2 or more arguments
	_num_decr,				// "decr" or "--"
	_num_decr_2,			// "decr" or "--" with 2 or more arguments
	};
};

//------------

// Execution Task Types
struct TaskType {
	enum Value {
		FuncBuild=0,
		FuncFound
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

// Log Levels for filtering information to be printed
struct LogLevel {
	enum Value {
	info,
	warning,
	error,
	debug
	};
};

// Information pertaining to log messages
// Example Usage in the engine:
// print( LogMessage.create(LogLevel::error).MessageId(EngineMessage::WrongArgCount) )
struct LogMessage {
	LogLevel::Value  level;
	EngineMessage::Value  messageId;
	String  functionName;
	SystemFunction::Value  systemFunctionId;
	UInteger  argIndex;
	UInteger  argCount;
	ObjectType::Value  givenArgType;
	ObjectType::Value  expectedArgType;
	String  givenArgTypeName;
	String  expectedArgTypeName;
	UInteger  customCode; // For user applications and foreign functions

	LogMessage(
		LogLevel::Value  pLevel
	)
		: level(pLevel)
		, messageId(EngineMessage::Running)
		, functionName()
		, systemFunctionId(SystemFunction::_unset)
		, argIndex(0)
		, argCount(0)
		, givenArgType(ObjectType::UnknownData)
		, expectedArgType(ObjectType::UnknownData)
		, customCode(0)
	{}

	static LogMessage
	create( LogLevel::Value  pLevel ) {
		return LogMessage(pLevel);
	}

	LogMessage&
	Message( EngineMessage::Value  m ) {
		messageId = m;
		return *this;
	}

	LogMessage&
	FunctionName( const String  name ) {
		functionName = name;
		return *this;
	}

	LogMessage&
	SystemFunctionId( SystemFunction::Value  v ) {
		systemFunctionId = v;
		return *this;
	}

	LogMessage&
	ArgIndex( UInteger  i ) {
		argIndex = i;
		return *this;
	}

	LogMessage&
	ArgCount( UInteger  i ) {
		argCount = i;
		return *this;
	}

	LogMessage&
	GivenArgType( ObjectType::Value  v ) {
		givenArgType = v;
		return *this;
	}

	LogMessage&
	ExpectedArgType( ObjectType::Value  v ) {
		expectedArgType = v;
		return *this;
	}

	LogMessage&
	GivenArgTypeName( const char*  v ) {
		givenArgTypeName = v;
		return *this;
	}

	LogMessage&
	ExpectedArgTypeName( const char*  v ) {
		expectedArgTypeName = v;
		return *this;
	}

	LogMessage&
	CustomCode( UInteger  code ) {
		messageId = EngineMessage::CustomMessage;
		customCode = code;
		return *this;
	}
};

// Interface for a logger
struct Logger {
	~Logger() {}
	virtual void print(const LogLevel::Value, const char*)=0;
	virtual void print(const LogLevel::Value, const EngineMessage::Value)=0;
	virtual void print(LogMessage) = 0;
	virtual void printTaskTrace( TaskType::Value  taskType, const String&  taskName, UInteger  taskNumber )=0;
	virtual void printStackTrace( const String&  frameName, UInteger  frameNumber )=0;
	virtual void printToken( const Token& ) {} // Optional. Used for debugging
};

//----------

// Pre-declaration
class Ref;

struct BadReferenceCountingException {
	int refs;
	const Ref* object;

	BadReferenceCountingException(Integer pCount, const Ref* pObject)
		: refs(pCount)
		, object(pObject)
	{}
};

// Class Ref
// Used for tracking reference counting for certain types of C++ objects
class Ref {
	int refs;
#ifdef COPPER_REF_LEVEL_MESSAGES
	bool isStackBased;
#endif

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
		if ( refs != 0 && !isStackBased ) {
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
	//void dropRef() {
	//	refs -= 1;
	//}

#ifdef COPPER_REF_LEVEL_MESSAGES
	void declareStackBased() {
		isStackBased = true;
	}
#endif

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
		: obj(pOther.obj)
	{
		if ( notNull(obj) ) {
			obj->ref();
		}
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

	RefPtr<T>& operator= (const RefPtr<T> pOther) {
		set(pOther.obj);
		return *this;
	}

	bool obtain(T*& pStorage) {
		pStorage = obj;
		return notNull(obj);
	}

	T* raw() const {
		return obj;
	}
};

//--------------------
class Object : public Ref {
protected:
	// Enumeration is intended to be extended to other types
	ObjectType::Value type;
public:
	Object( ObjectType::Value t = ObjectType::UnknownData )
		: type(t)
	{}

	virtual ~Object() {}

	ObjectType::Value
	getType() const {
		return type;
	}

	operator Object*() {
		return (Object*)this;
	}

	// Meant to be overridden.
	// Meant to return a "new" (heap) object or one with an extra reference count.
	// If you return "this", you must call this->ref() to ensure proper memory management.
	virtual Object*
	copy() =0;

	virtual void
	writeToString(String& out) const {
		out = "{object}";
	}

	// Meant to return the whether or not the internally-stored data is the same.
	// Can be used to compare data pointers even if the wrapper (Copper Object) is different.
	virtual bool
	isSameData( Object* obj ) {
		return this == obj;
	}

	//! Name of the data
	// The class can be extended to point to other types of data, such as huge numbers,
	// matrices, etc., and complemented by extension functions.
	virtual const char*
	typeName() const =0;

	//! Indicate interface support
	// Returns true if this object can be casted to objects of the given type.
	virtual bool
	supportsInterface( ObjectType::Value ) const {
		return false;
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char*
	getDebugName() const {
		return "Object";
	}
#endif
};

// *********** OPERATION PROCESSING BASE COMPONENTS **********

struct Task : public Ref {
	TaskType::Value name;

	Task(TaskType::Value pName) : name(pName) {}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "Task";
	}
#endif
};

class TaskContainer {
	Task* task;
public:
	// Expected to be passed: new Task() (e.g. "new FuncFoundTask()")
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

typedef List<Token>			TokenQueue;
typedef List<Token>::Iter	TokenQueueIter;

//typedef List<String> 			VarAddress;
//typedef List<String>::Iter 		VarAddressIter;
//typedef List<String>::ConstIter VarAddressConstIter;

// Exceptions

class VarAddressException {

public:
	enum Value {
		is_empty,
		index_out_of_bounds,
		bad_iterator
	};

private:
	Value  v;

public:
	VarAddressException( Value  pValue )
		: v(pValue)
	{}

	Value
	getErrorValue() const {
		return v;
	}
};

class InvalidOpcodeInit {};
class CopyBodyOpcodeException {};
class InvalidBodyOpcodeAccess {};
class NullGotoOpcodeException {};

//---------- Predeclarations for OpcodeContainer and Opcode

class Body; // pre-declaration

class Opcode; // pre-declaration

//-----------

//! Address of a Variable
/*
	Addresses are constant after being built and are only iterated over from start-to-finish,
	so there is no need for a doubly-linked list.
*/
class VarAddress : public Ref {

	struct Node {
		const String data;
		Node* post;

		Node( const String&  pData )
			: data(pData)
			, post(REAL_NULL)
		{}
	};

	Node*  head;
	Node*  tail;

public:
	// For access only
	class Iterator {
		friend VarAddress;
		Node* start;
		Node* curr;

	protected:
		Iterator( Node* pStart )
			: start( pStart )
			, curr( pStart )
		{}

	public:
		Iterator( const Iterator&  pOther )
			: start( pOther.start )
			, curr( pOther.start )
		{}

		bool
		atEnd() const {
			return isNull(curr);
		}

		bool
		atLast() const {
			if ( notNull(curr) )
				return curr->post == REAL_NULL;
			return false;
		}

		bool
		next() {
#ifdef COPPER_STRICT_CHECKS
			if ( !curr )
				throw VarAddressException( VarAddressException::index_out_of_bounds );
#endif
			curr = curr->post;
			return notNull(curr);
		}

		const String&
		get() const {
#ifdef COPPER_STRICT_CHECKS
			if ( !curr )
				throw VarAddressException( VarAddressException::bad_iterator );
#endif
			return curr->data;
		}

		void
		reset() {
			curr = start;
		}
	};


	VarAddress()
		: head(REAL_NULL)
		, tail(REAL_NULL)
	{}

	VarAddress( const VarAddress&  pOther )
		: head(REAL_NULL)
		, tail(REAL_NULL)
	{
		if ( isNull(pOther.head) )
			return;

		Node* n = pOther.head;
		Node* m = REAL_NULL;
		head = new Node( n->data );
		tail = head;
		while ( n->post ) {
			n = n->post;
			m = new Node( n->data );
			tail->post = m;
			tail = m;
		}
	}

	~VarAddress() {
		Node* n;
		while ( head ) {
			n = head;
			head = head->post;
			delete n;
		}
	}

	bool
	has() const {
		return notNull(head);
	}

	bool
	hasOne() const {
		if ( notNull(head) )
			return head->post == REAL_NULL;
		return false;
	}

	void
	push_back( const String&  part ) {
		if ( !tail ) {
			tail = new Node( part );
			head = tail;
		} else {
			tail->post = new Node( part );
			tail = tail->post;
		}
	}

	const String&
	first() const {
#ifdef COPPER_STRICT_CHECKS
		if ( !head )
			throw VarAddressException( VarAddressException::is_empty );
#endif
		return head->data;
	}

	Iterator
	iterator() const {
		return Iterator(head);
	}

#ifdef COPPER_DEBUG_ADDRESS
	void print() const {
		std::printf("[DEBUG: Address = ");
		Iterator i = iterator();
		while ( ! i.atEnd() ) {
			std::printf("%s ", i.get().c_str());
			i.next();
		}
		std::printf("\n");
	}
#endif
};

//----------
// Moved from AFTER Opcode

class OpcodeContainer {
	// DO NOT SHARE
	Opcode* code;

public:
	explicit OpcodeContainer( Opcode* pCode );

	OpcodeContainer( const OpcodeContainer& pOther ); // Used only for transmission in lists

	~OpcodeContainer();

	//const Opcode*
	Opcode* // non-const required for referencing addresses
	getOp();

	//static OpcodeContainer
	//indie( Opcode* pCode );
};

//typedef List<OpcodeContainer> OpStrand;
//typedef List<OpcodeContainer>::Iter OpStrandIter;

struct OpStrand : public List<OpcodeContainer>, public Ref {};
typedef OpStrand::Iter	OpStrandIter;

//----------

struct Opcode : public Ref {
	enum Type {
		Exit = 0,
		FuncBuild_start,
		FuncBuild_createRegularParam,
		FuncBuild_assignToVar,
		FuncBuild_pointerAssignToVar,
		FuncBuild_execBody,
		FuncBuild_end,

		FuncFound_access,
		FuncFound_assignment,
		FuncFound_pointerAssignment,
		FuncFound_call,
		FuncFound_setParam,
		FuncFound_finishCall,

		Terminal,
		Goto,
		ConditionalGoto,

		Own,
		Is_owner,
		Is_pointer,

		CreateBoolTrue,
		CreateBoolFalse,
		CreateString,
		CreateInteger,
		CreateDecimal
	};

	enum DataType {
		ODT_Unset=0,
		ODT_Name,
		ODT_Address,
		ODT_Integer,
		ODT_Decimal,
		ODT_Body,
		ODT_CodeIndex,
	};

protected:
	Type type;
	DataType dtype;

	// Non-trivial destructors
	String name;
	VarAddress* address;
	union {
		Body*			body;
		//CodeIndex		codeIndex; // or GotoWrap
		OpStrandIter*	target;
		Decimal			decimal;
		Integer			integer;
	} data;

	/*
	Memory allocation appearance:
	unsigned | pointer | pointer | pointer | unsigned | [int/f64/pointer/pointer]
	*/

public:

	Opcode( Opcode::Type pType );

	Opcode( Opcode::Type pType, const String&  pStrValue, bool  onAddress );

	//Opcode( Opcode::Type pType, const VarAddress&  pAddress );
	Opcode( Opcode::Type pType, VarAddress*  pAddress );

	Opcode( const Opcode& pOther );

	~Opcode();

	Type
	getType() const;

	void setType( Opcode::Type pType );

	void
	appendAddressData( const String&  pString );

	//const VarAddress*
	//getAddressData() const;

	VarAddress*
	getAddressData();

	// Used for function parameters and so forth
	String
	getNameData() const;

	void
	setIntegerData( Integer value );

	Integer
	getIntegerData() const;

	void
	setDecimalData( Decimal value );

	Decimal
	getDecimalData() const;

	void
	addToken( const Token& pToken );

	Body*
	getBody() const;

	void
	setTarget( const OpStrandIter pTarget );

	OpStrandIter&
	getOpStrandIter();
};

class BadOpcodeException {
	const Opcode::Type v;
	
public:
	BadOpcodeException(const Opcode::Type code)
		: v(code)
	{}

	Opcode::Type getOpcodeType() const {
		return v;
	}
};

class OpStrandContainer {
	OpStrand* s;
	OpStrandIter i; // Should be const but stuff may act through it. Find out what does.
	bool is_const;

public:
	OpStrandContainer( OpStrand* strand, bool immutable = false )
		: s(strand)
		, i(strand->start())
		, is_const(immutable)
	{
		s->ref();
	}

	OpStrandContainer(const OpStrandContainer& pOther)
		: s(pOther.s)
		, i(pOther.i)
		, is_const(pOther.is_const)
	{
		s->ref();
	}

	~OpStrandContainer() {
		s->deref();
	}

	OpStrandIter& getCurrOp() {
		return i;
	}

	OpStrand* getCurrStrand() {
		return s;
	}

	void removeAllUpToCurrentCode() {
		if (!is_const)
			s->removeUpTo(i);
	}
};

typedef List<OpStrandContainer> OpStrandStack;
typedef List<OpStrandContainer>::Iter OpStrandStackIter;

// Attempting to use or access a strand that is null
class EmptyOpstrandException {};
// Attempting to save an empty opcode
class NullOpcodeException {};


// ============= Bodies ==============
class TokenPushToFinishedBodyException {};

// predeclaration
class Engine;

// Some of this body code is in the C++ file because
// it depends on functions defined later.
struct Body : public Ref {
	enum State {
		Raw,
		HasErrors,
		Ready
	};

private:
	State state;
	TokenQueue tokens;
	OpStrand* codes;

public:
	Body();

	~Body();

	void addToken(const Token& pToken);

	State getState();

	// Attempts to convert code. Returns true if succeeded.
	bool compile(Engine* engine);

	OpStrand* getOpcodeStrand();

	bool isEmpty() const;

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "Body";
	}
#endif

protected:
	bool compile_internal(Engine* engine);
};


//********* FUNCTIONS **********

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
	Function& operator=(const Function& pOther);
	Scope& getPersistentScope();
	void set( Function& other, bool copyScope=true );
	void addParam( const String pName );

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "Function";
	}
#endif
};

//-------------------

class FunctionObject; // Pre-declaration

//class RefNullFunctionInContainerException {};
class BadFunctionObjectOwnerException {};
class SettingNullFunctionInContainerException {};

// Base class for ownership over a FunctionObject
struct Owner {
	virtual ~Owner() {}

	operator Owner* () {
		return (Owner*)this;
	}

	virtual bool owns( FunctionObject*  container ) const = 0;
};

/*
	The idea:

Rather than having to keep track of two types of variables (functions and pointers), it's easier
to treat them as one and have them all point to the same container. However, only one of these
objects is the owner. Only the owner can destroy the data. 

When the owner changes its function, this container deletes the function but keeps itself.
This prevents cyclic memory loops but prevents pointers from accessing dead memory.
Pointers that wish to access the function will reference-count this object rather than the function.

Interestly enough, this (instead of Function) has to inherit Object since this is what is stored
by variables and passed around the system.
*/
class FunctionObject : public Object {
	RefPtr<Function> funcBox;
	Owner* owner;
	UInteger ID;

public:
	static const ObjectType::Value object_type = ObjectType::Function;

	explicit FunctionObject(Function* pFunction, UInteger id=0);

	FunctionObject();

	// Should only be used for data transfer where the old copy immediately dies
	FunctionObject(const FunctionObject& pOther);
//private:
	~FunctionObject();
//public:
	void own( Owner* pGrabber );

	void disown( Owner* pDropper );

	bool isOwned() const;

	bool isOwner( const Owner* pVariable ) const;

	void changeOwnerTo( Owner* pNewOwner );

	// Should store in a better storage variable, not some open pointer
	bool getFunction( Function*& storage );

	// Used for assignment, which only requires copying
	void setFrom( FunctionObject& pOther );

	// Createa a new function from the body of the given one (also copies constant return)
	void createFromBody( Function& );

	// FunctionObject* copy()
	virtual Object* copy();

	// Create a function container initialized to a copy of the given data
	static FunctionObject* createInitialized(Object* pData) {
		FunctionObject* fc = new FunctionObject();
		Function* f;
		fc->getFunction(f);
		if ( notNull(pData) ) {
			f->result.setWithoutRef(pData->copy());
			f->constantReturn = true;
		}
		return fc;
	}

	// Create a function container initialized to the given data
	static FunctionObject* createInitializedNoCopy(Object* pData) {
		FunctionObject* fc = new FunctionObject();
		Function* f;
		fc->getFunction(f);
		if ( notNull(pData) ) {
			f->result.set(pData);
			f->constantReturn = true;
		}
		return fc;
	}

	virtual void
	writeToString(String& out) const {
		out = "{fn}";
	}

	virtual const char*
	typeName() const {
		return "fn";
	}

	virtual bool
	supportsInterface( ObjectType::Value  typeValue ) const {
		return typeValue == object_type;
	}

	static const char*
	StaticTypeName() {
		return "fn";
	}

	static ObjectType::Value
	StaticType() {
		return ObjectType::Function;
	}

	UInteger getID() {
		return ID;
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "FunctionObject";
	}
#endif
};

//-------------------

class NullVariableException {};

/*
	Class Variable

	As a language rule, all variables MUST point to a function container.
*/
class Variable : public Ref, public Owner {
	FunctionObject* box;

public:
	Variable();

#ifdef COMPILE_COPPER_FOR_C_PLUS_PLUS_11
	Variable(const Variable& pOther) = delete;
#else
private:
	Variable(const Variable& pOther);
public:
#endif

	~Variable();

	void reset();

	void set( Variable* pOther, bool pReuseStorage );

	void setFunc( FunctionObject* pContainer, bool pReuseStorage );

	// Used only for data
	void setFuncReturn( Object* pData, bool pPerformCopy=true );

	// Used for FuncFound_processRunVariable
	Function* getFunction(Logger* logger);

	bool isPointer() const;

	FunctionObject* getRawContainer();

	virtual bool owns( FunctionObject*  container ) const;

	Variable* getCopy();

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "Variable";
	}
#endif
};

//-------------------
// ****** DATA CLASSES ******

class BoolObject : public Object {
	bool value;
public:
	static const ObjectType::Value object_type = ObjectType::Bool;

	explicit BoolObject(bool b)
		: Object( BoolObject::object_type )
		, value(b)
	{}

	virtual Object*
	copy() {
		return new BoolObject(value);
	}

	bool
	getValue() const {
		return value;
	}

	virtual Integer
	getIntegerValue() const {
		return (Integer)value;
	}

	virtual Decimal
	getDecimalValue() const {
		return (Decimal)value;
	}

	virtual const char*
	typeName() const {
		return StaticTypeName();
	}

	virtual bool
	supportsInterface( ObjectType::Value  typeValue ) const {
		return typeValue == BoolObject::object_type;
	}

	static const char*
	StaticTypeName() {
		return "bool";
	}

	static ObjectType::Value
	StaticType() {
		return BoolObject::object_type;
	}

	virtual void
	writeToString(String& out) const {
		out = value?"true":"false";
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "BoolObject";
	}
#endif
};


class StringObject : public Object {
	String value;

public:
	static const ObjectType::Value object_type = ObjectType::String;

	StringObject()
		: Object( StringObject::object_type )
		, value()
	{}

	explicit StringObject( const String& pValue )
		: Object( ObjectType::String )
		, value(pValue)
	{}

	explicit StringObject( const char* pValue )
		: Object( ObjectType::String )
		, value(pValue)
	{}

	virtual Object*
	copy() {
		return new StringObject(value);
	}

	virtual const char*
	typeName() const {
		return StaticTypeName();
	}

	virtual bool
	supportsInterface( ObjectType::Value  typeValue ) const {
		return typeValue == StringObject::object_type;
	}

	static const char*
	StaticTypeName() {
		return "string";
	}

	static ObjectType::Value
	StaticType() {
		return StringObject::object_type;
	}

	String&
	getString() {
		return value;
	}

	virtual Integer
	getIntegerValue() const {
		return value.toInt();
	}

	virtual Decimal
	getDecimalValue() const {
		return value.toDouble();
	}

#ifdef COPPER_PURGE_NON_PRINTABLE_ASCII_INPUT_STRINGS
	void
	purgeNonPrintableASCII() {
		value.purgeNonPrintableASCII();
	}
#endif

//#ifdef COPPER_PURGE_NON_UTF8_INPUT_STRINGS
	// Clears all non-UTF-8 characters from the string
	//void purgeNonUTF8() {
	//	value.purgeNonUTF8();
	//}
//#endif

	virtual void
	writeToString(String& out) const {
		out = value;
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "StringObject";
	}
#endif
};

/*
	A base object for all numbers, allowing interaction
*/
struct NumericObject : public Object {

	static const ObjectType::Value object_type = ObjectType::Numeric;

	// Please extend
	struct SubType {
	enum Value {
		Integer = 0,	// Integer class
		DecimalNum,		// Decimal Number class
		Unknown,
		FORCE32 = 0x7fffffff // Allows extensions
	};};

	NumericObject();

	virtual ~NumericObject();

	static const char*
	StaticTypeName() {
		return "number";
	}

	static ObjectType::Value
	StaticType() {
		return NumericObject::object_type;
	}

	virtual const char*
	typeName() const {
		return StaticTypeName();
	}

	virtual bool
	supportsInterface( ObjectType::Value  typeValue ) const {
		return typeValue == NumericObject::object_type;
	}

	virtual SubType::Value
	getSubType() const = 0;

	virtual void
	writeToString(String& out) const {
		out = "{number}";
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "NumericObject";
	}
#endif

	virtual void
	setValue( const NumericObject& ) = 0;

	virtual Integer
	getIntegerValue() const = 0;

	virtual Decimal
	getDecimalValue() const = 0;

	virtual bool
	isEqualTo(const NumericObject& CU_UNUSED_ARG(other) ) = 0;

	virtual bool
	isGreaterThan(const NumericObject& CU_UNUSED_ARG(other) ) = 0;

	virtual bool
	isGreaterOrEqual(const NumericObject& CU_UNUSED_ARG(other) ) = 0;

	virtual NumericObject*
	absValue() const = 0;

	virtual NumericObject*
	add(const NumericObject& CU_UNUSED_ARG(other) ) = 0;

	virtual NumericObject*
	subtract(const NumericObject& CU_UNUSED_ARG(other) ) = 0;

	virtual NumericObject*
	multiply(const NumericObject& CU_UNUSED_ARG(other) ) = 0;

	virtual NumericObject*
	divide(const NumericObject& CU_UNUSED_ARG(other) ) = 0;

	virtual NumericObject*
	modulus(const NumericObject& CU_UNUSED_ARG(other) ) = 0;

/*
	// Example Settings

	virtual bool
	isEqualTo(const NumericObject& CU_UNUSED_ARG(other) ) {
		return false;
	}

	virtual bool
	isGreaterThan(const NumericObject& CU_UNUSED_ARG(other) ) {
		return false;
	}

	virtual bool
	isGreaterOrEqual(const NumericObject& CU_UNUSED_ARG(other) ) {
		return false;
	}

	// These functions are meant to return heap-created objects or objects with at least
	// 1 extra reference count since the object may be dropped.
	virtual NumericObject*
	absValue() { return (NumericObject*)copy(); }

	virtual NumericObject*
	add( NumericObject& CU_UNUSED_ARG(other) ) { return copy(); }
*/
};

/*
	Number Helper Classes
*/

bool
iszero(
	Decimal		p
);


/*
	Object type represeting integers
	Created with characters 0-9
*/
class IntegerObject : public NumericObject {
	Integer  value;

public:
	//! cstor
	IntegerObject()
		: NumericObject()
		, value(0)
	{}

	//! cstor
	IntegerObject( Integer newValue )
		: NumericObject()
		, value ( newValue )
	{}

	//! cstor
	IntegerObject( const IntegerObject& other )
		: NumericObject()
		, value( other.value )
	{}

	//! Copy
	virtual Object*
	copy() {
		return new IntegerObject(value);
	}

	static const char*
	StaticTypeName() {
		return "int";
	}

	virtual const char*
	typeName() const {
		return StaticTypeName();
	}

	virtual bool
	supportsInterface( ObjectType::Value  typeValue ) const;

	virtual NumericObject::SubType::Value
	getSubType() const {
		return NumericObject::SubType::Integer;
	}

	virtual void
	writeToString(String& out) const {
		out = "{integer}";
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "IntegerObject";
	}
#endif

	void
	setValue( Integer newValue ) {
		value = newValue;
	}

	virtual void
	setValue( const NumericObject&  );

	virtual Integer
	getIntegerValue() const {
		return value;
	}

	virtual Decimal
	getDecimalValue() const {
		return (Decimal)value;
	}

	virtual bool
	isEqualTo(const NumericObject& CU_UNUSED_ARG(other) );

	virtual bool
	isGreaterThan(const NumericObject& CU_UNUSED_ARG(other) );

	virtual bool
	isGreaterOrEqual(const NumericObject& CU_UNUSED_ARG(other) );

	virtual NumericObject*
	absValue() const;

	virtual NumericObject*
	add(const NumericObject& CU_UNUSED_ARG(other) );

	virtual NumericObject*
	subtract(const NumericObject& CU_UNUSED_ARG(other) );

	virtual NumericObject*
	multiply(const NumericObject& CU_UNUSED_ARG(other) );

	virtual NumericObject*
	divide(const NumericObject& CU_UNUSED_ARG(other) );

	virtual NumericObject*
	modulus(const NumericObject& CU_UNUSED_ARG(other) );
};

/*
	Object type represeting integers
	Created with characters 0-9 and a single decimal
*/
class DecimalNumObject : public NumericObject {
	Decimal  value;

public:
	//! cstor
	DecimalNumObject()
		: NumericObject()
		, value(0)
	{}

	//! cstor
	DecimalNumObject( Decimal newValue )
		: NumericObject()
		, value ( newValue )
	{}

	//! cstor
	DecimalNumObject( const DecimalNumObject& other )
		: NumericObject()
		, value( other.value )
	{}

	//! Copy
	virtual Object*
	copy() {
		return new DecimalNumObject(value);
	}

	static const char*
	StaticTypeName() {
		return "dcml";
	}

	virtual const char*
	typeName() const {
		return StaticTypeName();
	}

	virtual bool
	supportsInterface( ObjectType::Value  typeValue ) const;

	virtual NumericObject::SubType::Value
	getSubType() const {
		return NumericObject::SubType::DecimalNum;
	}

	virtual void
	writeToString(String& out) const {
		out = "{decimal}";
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "DecimalNumObject";
	}
#endif

	void
	setValue( Decimal newValue ) {
		value = newValue;
	}

	virtual void
	setValue( const NumericObject&  );

	virtual Integer
	getIntegerValue() const {
		return (Integer)value;
	}

	virtual Decimal
	getDecimalValue() const {
		return value;
	}

	virtual bool
	isEqualTo(const NumericObject& CU_UNUSED_ARG(other) );

	virtual bool
	isGreaterThan(const NumericObject& CU_UNUSED_ARG(other) );

	virtual bool
	isGreaterOrEqual(const NumericObject& CU_UNUSED_ARG(other) );

	virtual NumericObject*
	absValue() const;

	virtual NumericObject*
	add(const NumericObject& CU_UNUSED_ARG(other) );

	virtual NumericObject*
	subtract(const NumericObject& CU_UNUSED_ARG(other) );

	virtual NumericObject*
	multiply(const NumericObject& CU_UNUSED_ARG(other) );

	virtual NumericObject*
	divide(const NumericObject& CU_UNUSED_ARG(other) );

	virtual NumericObject*
	modulus(const NumericObject& CU_UNUSED_ARG(other) );
};


//------------------

/* To enable hooks into the engine, some functionality for returning objects via lists is needed.
This interface stands in place for a more concrete list implementation, which would otherwise
prohibit flexibility. */
struct AppendObjectInterface {
	virtual ~AppendObjectInterface() {}
	// Append objects
	// Implementers of this function are expected to call ref() on each object passed in.
	virtual void append(Object*)=0;

	operator AppendObjectInterface* () {
		return (AppendObjectInterface*)this;
	}
};

//------------------

class ListObject : public Object, public AppendObjectInterface {

	struct Node : public Owner {
		Object*  item;
		Node* prior;
		Node* post;

		Node( Object* pItem )
			: item( pItem )
			, prior( REAL_NULL )
			, post( REAL_NULL )
		{
			item->ref();
			if ( item->getType() == ObjectType::Function )
				((FunctionObject*)item)->own(this);
		}

		virtual bool
		owns( FunctionObject*  container ) const {
			return (Object*)container == item;
		}

		void
		destroy() {
			if ( item->getType() == ObjectType::Function ) {
				((FunctionObject*)item)->disown(this);
			}
			item->deref();
			if ( prior )
				prior->post = post;
			if ( post )
				post->prior = prior;
			delete this;
		}

		void
		swapItem( Node& pOther ) {
			Object*  temp = pOther.item;
			pOther.item = item;
			item = temp;
			if ( item->getType() == ObjectType::Function )
				((FunctionObject*)item)->changeOwnerTo(this);
			if ( pOther.item->getType() == ObjectType::Function )
				((FunctionObject*)(pOther.item))->changeOwnerTo(&pOther);
		}

		void
		replace( Object*  pItem ) {
			if ( item ) {
				if ( item->getType() == ObjectType::Function )
					((FunctionObject*)item)->disown(this);
				item->deref();
			}
			item = pItem;
			if ( item ) {
				if ( item->getType() == ObjectType::Function )
					((FunctionObject*)item)->own(this);
				item->ref();
			}
		}

		bool
		isPointer() const {
			if ( item->getType() != ObjectType::Function )
				return false;

			return ! ((FunctionObject*)item)->isOwner(this);
		}
	};

	struct NodePtr {
		Node*  node;

		NodePtr()
			: node (REAL_NULL)
		{}

		Integer
		moveOff() {
			if ( !node )
				return 0;
			if ( node->prior ) {
				node = node->prior;
				return -1;
			}
			if ( node->post ) {
				node = node->post;
				return 1;
			}
			return 0;
		}

		void
		append( Object*  pItem ) {
			// Should throw if no node. Should also check if node->post exists.
			// Limit usage to contexts where node exists and node->post does not.
			node->post = new Node(pItem);
			node->post->prior = node;
			node = node->post;
		}

		void
		prepend( Object*  pItem ) {
			// Should throw if no node. Should also check if node->prior exists.
			// Limit usage to contexts where node exists and node->prior does not.
			node->prior = new Node(pItem);
			node->prior->post = node;
			node = node->prior;
		}

		void
		insert( Object*  pItem ) {
			// Does NOT check for null nodes.
			// Limit usage to contexts where node and node->post exist.
			Node* n = new Node(pItem);
			n->post = node->post;
			n->prior = node;
			node->post->prior = n;
			node->post = n;
		}
	};

	Integer  nodeCount;
	Integer  selectorIndex;
	NodePtr  selector;
	NodePtr  head;
	NodePtr  tail;

	// Copy-constructor forbidden
	ListObject( const ListObject&  pOther );

public:

	static const ObjectType::Value object_type = ObjectType::List;

	ListObject();

	~ListObject();

	virtual Object*
	copy();

	Integer
	size();

protected:

	bool
	gotoIndex( Integer  index );

public:
	void
	clear();

	virtual void
	append( Object*  pItem );

	void
	push_back( Object*  pItem );

	void
	push_front( Object* pItem );

	// Removes an object. Returns true upon success.
	bool
	remove( Integer  index );

	// Inserts an object. Returns true upon success.
	bool
	insert( Integer  index, Object*  pItem );

	// Swaps two objects. Returns true upon success.
	bool
	swap( Integer  index1, Integer  index2 );

	// Replaces an object. Returns true upon success.
	bool
	replace( Integer  index, Object*  pNewItem );

	// Gets an object. Returns REAL_NULL upon failure.
	Object*
	getItem( Integer  index );

	static const char*
	StaticTypeName() {
		return "list";
	}

	virtual const char*
	typeName() const {
		return StaticTypeName();
	}

	virtual bool
	supportsInterface( ObjectType::Value  typeValue ) const {
		return typeValue == ListObject::object_type;
	}

	static ObjectType::Value
	StaticType() {
		return ObjectType::List;
	}

	virtual void
	writeToString(String& out) const {
		out = "{list}";
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "ListObject";
	}
#endif
};

//*********** FOREIGN FUNCTION HANDLING *********

class FFIServices; // predeclaration

// Shows up when the user tries to add a null pointer as a foreign function
class NullForeignFunctionException {};

	// External/Foreign Functions
/*
For integrating libraries with this language, classes can directly inherit this interface.
If the function returns, use FFIServices::setResult(), which automatically calls
ref() on the passed object.
*/
class ForeignFunc : public Ref {
public:
	enum Result {
		FINISHED = 0,
		NONFATAL,
		FATAL,
		EXIT
	};

	virtual ~ForeignFunc() {}

	// Calls the function. Return "false" on error.
	//virtual bool call( FFIServices& ffi )=0;
	virtual Result call( FFIServices& ffi )=0;

	//operator ForeignFunc* () {
	//	return (ForeignFunc*)(*this);
	//}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "ForeignFunc";
	}
#endif
};

// Containers are required because
// 1) GCC can't handle nested templates.
// 2) Hash table data slots don't handle reference counting.
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
class RefVariableStorage {
	Variable* variable;

public:
	RefVariableStorage();

	RefVariableStorage(Variable* refedVariable);

	RefVariableStorage(RefVariableStorage& pOther);

	RefVariableStorage(const RefVariableStorage& pOther);

	~RefVariableStorage();

	RefVariableStorage&  operator= (RefVariableStorage&);

	Variable&  getVariable();

	RefVariableStorage  copy();
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
	Scope( UInteger pTableSize = CU_FUNCTION_SCOPE_SIZE );
	// Note: The Global Namespace would be better served having a list or tree (since it may be "infinite")
/*
	// What's the point of this dance?
#ifdef COMPILE_COPPER_FOR_C_PLUS_PLUS_11
	Scope(const Scope& pOther) = delete;
#else
private:
	Scope(const Scope& pOther);
public:
#endif
*/
	Scope(Scope& pOther); // For implicit copy construction. DO NOT USE FOR TRUE COPY

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
	// The pReuseStorage option may be ignored if the FunctionObject is not owned
	void setVariable(const String& pName, Variable* pSourceVariable, bool pReuseStorage);

	// Sets the variable, creating the variable if it does not exist
	void setVariableFrom(const String& pName, Object* pObject, bool pReuseStorage);

	// Appends to the given list all the names of the members in this scope.
	// Could be turned into List<Object> and return object strings but there is no built-in list class.
	void appendNamesByInterface(AppendObjectInterface* aoi);

	// Copies members from another scope to this one
	void copyMembersFrom(Scope& pOther);

	// Number of occupied storage slots / actual Variables (there may be more storage allocated)
	UInteger occupancy();

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "Scope";
	}
#endif
};


#ifdef COPPER_DEBUG_STACK
/*
Debug testing setup:
	std::printf("[DEBUG: Engine::execute() stack level = %d\n", stack.getCurrLevel());
	PrintScopeObjectNames pson;
	stack.getBottom().getScope().appendNamesByInterface(pson);
*/
struct PrintScopeObjectNames : public AppendObjectInterface {

	virtual void append( Object* object ) {
		String v;
		object->writeToString(v);
		std::printf( "Printing Scope Object Name: %s\n", v.c_str() );
	}
};
#endif


// Pre-declaration
class Stack;

class StackFrame : public Ref {
	friend Stack;

	StackFrame* parent; // Set by Stack
	Scope* scope;
	VarAddress* address;

public:
	StackFrame( VarAddress* pAddress );

	StackFrame( const StackFrame& pOther );

	~StackFrame();

	void
	replaceScope();

	Scope&
	getScope();

	VarAddress*
	getAddress();

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
	UInteger  size;
	// Singly Linked List from top-to-bottom
	StackFrame*  bottom;
	StackFrame*  top;

	// Bottom frame name
	VarAddress*  globalName;

public:
	Stack();
#ifdef COMPILE_COPPER_FOR_C_PLUS_PLUS_11
	Stack( const Stack& ) = delete;
#else
private:
	Stack( const Stack& );
public:
#endif
	~Stack();
	void clear();
	void clearNonGlobal();
	void clearGlobal();
	StackFrame& getBottom();
	StackFrame& getTop();
	UInteger getCurrLevel();
	StackPopReturn pop();
	void push( VarAddress* pAddress );
	void push( StackFrame* );
	void print( Logger* );
};


// ============= Execution Tasks, Opcode implementations, and Parse Tasks ==============

#ifdef COPPER_STRICT_CHECKS
// Used to check for invalid tokens triggering valid activity
class ParserTokenException {
	Token t;
public:
	ParserTokenException(const Token& pOffendingToken)
		: t(pOffendingToken)
	{}

	const Token& getOffendingToken() {
		return t;
	}
};
#endif

// Parse tasks are temporary containers for parser information.
// They are used to hold information when the parser must parse embedded expressions
// or must pause due to lack of tokens needed to complete an expression.
struct ParseTask : public Ref {
	struct Result {
		enum Value {
			need_more,
			task_done,
			interpret_token,
			syntax_error
		};
	};

	enum Type {
		FuncBuild,
		FuncFound,
		If,
		Loop,
		SRPS		// Single Raw Parameter Structure: own, is_owner, and is_ptr
	} type;

	ParseTask(Type t)
		: type(t)
	{}
};

struct ParseTaskContainer {
	ParseTask* task;

	ParseTaskContainer(ParseTask* t)
		: task(t)
	{
		t->ref();
	}

	ParseTaskContainer( const ParseTaskContainer& pOther )
		: task(pOther.task)
	{
		task->ref();
	}

	~ParseTaskContainer() {
		task->deref();
	}

	ParseTask* getTask() {
		return task;
	}
};

typedef List<ParseTaskContainer> ParseTaskList;
typedef List<ParseTaskContainer>::Iter ParseTaskIter;

// Thrown when attempting to get a token when no token source was set
// or the source is empty
class ParserContextEmptySourceException {};
class BadParserStateException {};


struct ParseResult {
	enum Value {
		More,
		Error,
		Done
	};
};


class ParserContext {
	TokenQueue* tokenSource;
	TokenQueueIter* currToken;
	TokenQueueIter* lastUsedToken;

public:
	OpStrand* outputStrand;
	ParseTaskList taskStack;

	ParserContext();
	~ParserContext();

	void setTokenSource( TokenQueue& source );

	void setCodeStrand( OpStrand* strand );

	// Indicates there are no more tokens available to process
	bool isFinished();

	// Internal handling for when errors occur
	void onError();

	// Moves the iterator to the first unused token.
	// Returns "true" if the move could be made.
	bool moveToFirstUnusedToken();

	// Gets the next token
	Token peekAtToken();
	bool moveToNextToken();
	bool moveToPreviousToken();

	// Triggers the context to save the state that all of the examined tokens were used
	void commitTokenUsage();

	// Deletes tokens that have been used upto the current one
	void clearUsedTokens();

	// Add new operation
	void addNewOperation( Opcode* newOp );

	// Add operation that has already been reference-counted
	void addOperation( Opcode* op );

	// TODO:
	// Replace addNewOperation() and addOperation() with methods that handle ALL possible
	// new opcode creation calls, thereby hiding the actual implementation of opcodes
	// from the parsing functions.
	// Currently, the problem is with dereferencing, which could possibly be solved
	// if opcodes were to become unions / not stored in the heap.
};

void
addNewParseTask(
	ParseTaskList&	taskStack,
	ParseTask*		newTask
);

struct FuncBuildTask : public Task {

	Function* function;

	FuncBuildTask()
		: Task(TaskType::FuncBuild)
		, function(new Function())
	{}

	~FuncBuildTask() {
		function->deref();
	}
};

// struct FuncBuildParseTask
// The first op-code has already been created.
// This is used to create related op-codes.
struct FuncBuildParseTask : public ParseTask {
	struct State {
		enum Value {
			FromObjectBody,
			CollectParams,
			AwaitAssignment,
			AwaitPointerAssignment,
			FromExecBody
		};
	};
	State::Value state;
	String paramName;

	FuncBuildParseTask(State::Value s)
		: ParseTask(ParseTask::FuncBuild)
		, state(s)
		, paramName()
	{}
};

typedef List<Object*>			ArgsList;
typedef List<Object*>::Iter		ArgsIter;

struct FuncFoundTask : public Task {
	//const VarAddress  varAddress; // MUST NOT BE A VarAddress&!!
	VarAddress*  varAddress;
	ArgsList  args;

	//explicit FuncFoundTask( const VarAddress& pVarAddress );
	explicit FuncFoundTask( VarAddress* pVarAddress );

	//FuncFoundTask( const FuncFoundTask& pOther );
	FuncFoundTask( FuncFoundTask& pOther );

	~FuncFoundTask();

	void
	addArg( Object* a );

	const VarAddress&
	getAddress() const {
		return *varAddress;
	}
};

struct FuncFoundParseTask : public ParseTask {
	enum State {
		Start,
		ValidateAssignment,
		ValidatePointerAssignment,
		CompleteAssignment,
		CompletePointerAssignment,
		VerifyParams,
		CollectParams,
	} state;

	Opcode* code;
	bool waitingOnAssignment;
	UInteger openBodies;

	FuncFoundParseTask( const String& pName )
		: ParseTask(ParseTask::FuncFound)
		, state(Start)
		, code(new Opcode(Opcode::FuncFound_access, pName, true))
		, waitingOnAssignment(false)
		, openBodies(1)
	{}

	~FuncFoundParseTask() {
		code->deref();
	}
};

class NullIfStructureConditionException {};

struct IfStructureParseTask : public ParseTask {
	OpStrandIter firstIter;
	UInteger openBodies; // used for both parameter and execution body token counting
	Opcode* conditionalGoto;
	List<Opcode*> finalGotos;
	bool atElse;

	enum State {
		Start,
		CreateCondition,
		CreateBody,
		PostBody
	} state;

	IfStructureParseTask( OpStrand* strand );

	// Queues a conditional goto meant to be set to the next terminal when added
	void queueNewConditionalJump( Opcode* jump );

	// Queues a new goto meant to be set to the last terminal when added
	void queueNewFinalJumpCode( Opcode* jump );

	void finalizeConditionalGoto( ParserContext& context );

	void finalizeGotos( ParserContext& context );
};

struct LoopStructureParseTask : public ParseTask {
	OpStrandIter firstIter;
	List<Opcode*> finalGotos;
	UInteger openBodies;

	enum State {
		Start,
		CollectBody,
		AwaitFinish
	} state;

	LoopStructureParseTask( OpStrand* strand );

	void setGotoOpcodeToLoopStart( Opcode* code );

	void queueFinalGoto( Opcode* code );

	void finalizeGotos( OpStrand* strand );
};

struct SRPSParseTask : ParseTask {
	const Opcode::Type codeType;

	SRPSParseTask(const Opcode::Type value)
		: ParseTask(ParseTask::SRPS)
		, codeType(value)
	{}
};


// Function Execution Return
// Engine::execute() is the only place where functions are run.
struct FuncExecReturn {
	enum Value {
		Ran,			// Function ran successfully
		Reset,			// Function is about to run and needs the opstrand iterator reset
		ErrorOnRun,		// Function ran but had an error
		ExitCalled,		// Function ran and met an "exit" token
		Return,			// "ret()" function called. The opcode stack should be popped.
		NoMatch			// No matching function was found for calling
	};
};

//------ Task Exceptions

class BadFuncFoundTaskException {};

// For when trying to access a task when the task stack is empty
class EmptyTaskStackException {};

//----------- Foreign Function Interface -------------

// Exceptions usually occur for things like calling arg() with a bad index.
class FFIMisuseException {};

// class Engine was declared before this

class FFIServices {
	Engine&			engine;
	RefPtr<Object>*	argsArray;
	UInteger		numArgs;
	String			who;

public:
	FFIServices( Engine& enginePtr, ArgsList& argsList, String  foreignFuncName );

	~FFIServices();

	//! Get the total number of arguments
	UInteger getArgCount() const;

	//! Demand a specific number of args
	/*
		Prints an error if the number of args does not match the given.
		\param imperative - Indicates whether not matching would cause an error.
		\return - true if the number of args matches the given.
	*/
	bool demandArgCount( UInteger  count, bool  imperative = true );

	//! Demand min and max number of args
	/*
		Demands a minimum and maximum number of args. Prints an error if the number of
		args does not fit in the range.
		\param imperative - Indicates whether not being in range would cause an error.
		\return - true if the number of args is in range.
	*/
	bool demandArgCountRange( UInteger  min, UInteger  max, bool  imperative = true );

	//! Demand min number of args
	/*
		Demands a minimum number of args. Prints an error if there are fewer args.
		\param imperative - Indicates whether not being in range would cause an error.
		\return - true if the number of args is in range.
	*/
	bool demandMinArgCount( UInteger  min, bool  imperative = true );

	//! Demand argument be of type
	/*
		Prints an error if the argument at the given index doesn't match a specific type.
		\return - true if the given argument matches the required type.
	*/
	bool demandArgType( UInteger  index, ObjectType::Value  type );

	//! Demand all args are of the given type
	/*
		Prints an error if at least one argument doesn't match the given type.
		\return - false only if there is an argument that does not match the given type.
			Defaults to "true" even for no arguments.
	*/
	bool demandAllArgsType( ObjectType::Value  type, bool  imperative = true );

	//! Get the arg at the given index
	//! throws an error if out-of-bounds
	Object& arg( UInteger  index );

		// Methods for printing miscellaneous messages

	// Wrapper for Engine::print(LogLevel::info, const char*)
	void printInfo(const char* message);

	// Wrapper for Engine::print(LogLevel::warning, const char*)
	void printWarning(const char* message);

	// Wrapper for Engine::print(LogLevel::error, const char*)
	void printError(const char* message);

	//! Print custom message code
	void printCustomInfoCode( UInteger );
	void printCustomWarningCode( UInteger );
	void printCustomErrorCode( UInteger );

	//! Sets the Engine lastObject.
	void setResult(Object* obj);

	//! Sets the Engine lastObject with a new Object (calls deref)
	void setNewResult(Object* obj);
};

// ********** HELPER FUNCTIONS **********

// Accepts an address in string format and converts it to an address
// Can be used to reverse the effect of addressToString()
VarAddress createAddress( const char* textAddress );

// Accepts an address and returns a string
// Can be used to reverse the effect of createAddress()
String addressToString( const VarAddress& address );


inline bool
isFunctionObject(
	const Object& pObject
) {
	return ( pObject.getType() == ObjectType::Function );
}

inline bool
isEmptyFunctionObject(
	const Object& pObject
) {
	if ( pObject.getType() != ObjectType::Function ) {
		return false;
	}
	Function* function;
	if ( !((FunctionObject&)pObject).getFunction(function) ) {
		return false;
	}
	if ( function->constantReturn ) {
		if ( notNull(function->result.raw()) )
			return false;
	}
	return (
		function->body.raw()->isEmpty()
		&& function->params.size() == 0
		&& function->getPersistentScope().occupancy() == 0
	);
}

inline bool
isBoolObject(
	const Object& pObject
) {
	return ( pObject.getType() == ObjectType::Bool );
}

inline bool
getBoolValue(
	const Object& pObject
) {
	if ( isBoolObject( pObject ) ) {
		return ((BoolObject&)pObject).getValue();
	}
	return false;
}

inline bool
isStringObject(
	const Object& pObject
) {
	return ( pObject.getType() == ObjectType::String );
}

inline bool
isListObject(
	const Object& pObject
) {
	return ( pObject.getType() == ObjectType::List );
}

inline bool
isNumericObject(
	const Object& pObject
) {
	return pObject.getType() == ObjectType::Numeric;
}

inline bool
isIntegerObject(
	const Object& pObject
) {
	if ( pObject.getType() == ObjectType::Numeric ) {
		return ((NumericObject&)pObject).getSubType() == NumericObject::SubType::Integer;
	}
}

inline bool
isDecimalNumObject(
	const Object& pObject
) {
	if ( pObject.getType() == ObjectType::Numeric ) {
		return ((NumericObject&)pObject).getSubType() == NumericObject::SubType::DecimalNum;
	}
}

//--------------------------------

struct EngineEndProcCallback {
	virtual void CuEngineDoneProcessing()=0;
};

//----------------------------------------

#ifdef COPPER_STRICT_CHECKS
class NullOpcodeStrandException {};
#endif

/*
Processing begins with run(), which takes a user string and tokenizes it.
It then sends it to process(), where the token causes changes in the state machine.
run() is used to filter out bad tokens, so passing any bad tokens to process() could be fatal.
You can use process() directly if you know what you are doing.
*/

class Engine {
	friend FFIServices; // Not needed if you don't require the FFI to directly set the lastObject

	Logger* logger;
	Stack stack;
	List<TaskContainer> taskStack; // Needs a getLast() method
	RefPtr<Object> lastObject;
	List<Token> bufferedTokens;
	ParserContext globalParserContext;
	OpStrandStack opcodeStrandStack;
	OpStrandStack* activeOpcodeStrandStack;
	EngineEndProcCallback* endMainCallback;
	RobinHoodHash<SystemFunction::Value> builtinFunctions;
	RobinHoodHash<ForeignFuncContainer> foreignFunctions;
	bool ignoreBadForeignFunctionCalls;
	bool ownershipChangingEnabled;
	bool stackTracePrintingEnabled;
	bool printTokensWhenParsing;
	bool (* nameFilter)(const String& pName);

public:
	Engine(); // remember to initialize the logger

	// WARNING: Logger is never referenced or dropped, so don't delete the logger before the Engine!
	void setLogger( Logger* pLogger );

private:
	// Print functions. Both of these call the logger (if it exists) or print to stdout (if enabled).
	void print( const LogLevel::Value logLevel, const char* msg ) const;
	void print( const LogLevel::Value logLevel, const EngineMessage::Value msg ) const;
	void print( LogMessage ) const;

public:
	// WARNING: endMainCallback is never referenced or dropped, so don't delete it before the Engine!
	void setEndofProcessingCallback( EngineEndProcCallback* callback ) {
		endMainCallback = callback;
	}

	void setIgnoreBadForeignFunctionCalls( bool yes ) {
		ignoreBadForeignFunctionCalls = yes;
	}

	void setOwnershipChangingEnabled( bool yes ) {
		ownershipChangingEnabled = yes;
	}

	// Print the stack trace when a function fails
	void setStackTracePrintingEnabled( bool on ) {
		stackTracePrintingEnabled = on;
	}

	// Set printing tokens when parsing
	void setPrintTokensWhenParsing( bool on ) {
		printTokensWhenParsing = on;
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
	void addForeignFunction(
		const String&	pName,
		ForeignFunc*	pFunction
	);

	/* Run Copper code.
	This accepts bytes from a byte stream and treats it as Copper code. */
	EngineResult::Value
	run( ByteStream& stream );

#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	void
	printGlobalStrand();
#endif

	/* Debugging feature, used for printing tokens when parsing. */
	void
	printTokens( TokenQueue& );

	void clearGlobals();

protected:
	void clearStacks();
	void signalEndofProcessing();
	void printTaskTrace();
	void printStackTrace();

	Scope&
	getGlobalScope();

	Scope&
	getCurrentTopScope();

	ParserContext&
	getGlobalParserContext();

	void
	setVariableByAddress(
		const VarAddress&	address,
		Object*				obj,
		bool				reuseStorage
	);

	ParseResult::Value
	lexAndParse( ByteStream& stream, bool srcDone );
	//lexAndParse( const CharList& byteQueue );

	/*
	\param pName - The token's assume name.
	\return - Returns the token that this name resolves to or TT_unknown if it does not resolve. */
	TokenType
	resolveTokenType( const String& pName );

	Result::Value
	tokenize(
		CharList& tokenValue,
		List<Token>& tokens
	);

	Result::Value
	handleCommentsStringsAndSpecials(
					const TokenType&	tokenType,
					CharList&			tokenValue,
					List<Token>&		tokens,
					ByteStream&			stream
	);

	bool isWhitespace(			const char c ) const;
	bool isSpecialCharacter(	const char c, TokenType& pTokenType );
	bool isEscapeCharacter(		const char c ) const;
	bool isValidToken(			const TokenType& token ) const;
	bool isValidNameCharacter(	const char c ) const;
	bool isValidName(			const String& pName ) const;
	bool isCommentToken(		const char c ) const;
	bool isStringToken(			const char c ) const;

	Result::Value
	scanComment( ByteStream& stream );

	Result::Value
	collectString(
		ByteStream& stream,
		CharList& collectedValue
	);

	void setupSystemFunctions();

	// ----- PARSING SYSTEM -----

	// 			HOW THE PARSING SYSTEM WORKS!
	// The new parse system is styled much like the previous one: first-in, first-processed.
	// That is due to the nature of the language.
	// However, this new system uses ParseTask to generate task information that is then converted into
	// operation codes, through which the execution of the code is performed, as opposed to executing the
	// code using Tasks.

	// The ParserContext must be passed in by whatever called this function.
public:
	ParseResult::Value
	parse(
		ParserContext&	context,
		bool			srcDone
	);

protected:
	ParseTask::Result::Value
	moveToFirstUnusedToken(
		ParserContext&		context,
		bool				srcDone
	);

	ParseResult::Value
	interpretToken(
		ParserContext&	context
	);

	ParseTask::Result::Value
	processParseTask(
		ParseTask*		task,
		ParserContext&	context,
		bool			srcDone
	);

	ParseTask::Result::Value
	parseFuncBuildTask(
		FuncBuildParseTask*	task,
		ParserContext&		context,
		bool				srcDone
	);

	ParseTask::Result::Value
	ParseFunctionBuild_FromObjectBody(
		FuncBuildParseTask*	task,
		ParserContext&		context,
		bool				srcDone
	);

	ParseTask::Result::Value
	ParseFunctionBuild_CollectParameters(
		FuncBuildParseTask*	task,
		ParserContext&		context,
		bool				srcDone
	);

	ParseTask::Result::Value
	ParseFunctionBuild_FromExecBody(
		ParserContext&		context,
		bool				srcDone
	);

	ParseTask::Result::Value
	parseFuncFoundTask(
		FuncFoundParseTask*	task,
		ParserContext&		context,
		bool				srcDone
	);

	ParseTask::Result::Value
	ParseFuncFound_ValidateAssignment(
		FuncFoundParseTask*	task,
		ParserContext&		context
	);

	ParseTask::Result::Value
	ParseFuncFound_ValidatePointerAssignment(
		FuncFoundParseTask*	task,
		ParserContext&		context
	);

	ParseTask::Result::Value
	ParseFuncFound_VerifyParams(
		FuncFoundParseTask*	task,
		ParserContext&		context,
		bool				srcDone
	);

	ParseTask::Result::Value
	ParseFuncFound_CollectParams(
		FuncFoundParseTask*	task,
		ParserContext&		context
	);

	ParseTask::Result::Value
	parseIfStructure(
		IfStructureParseTask*	task,
		ParserContext&			context,
		bool					srcDone
	);

	ParseTask::Result::Value
	ParseIfStructure_InitScan(
		IfStructureParseTask*	task,
		ParserContext&			context,
		bool					srcDone
	);

	ParseTask::Result::Value
	ParseIfStructure_ScanExecBody(
		IfStructureParseTask*	task,
		ParserContext&			context,
		bool					srcDone
	);

	ParseTask::Result::Value
	ParseIfStructure_CreateCondition(
		IfStructureParseTask*	task,
		ParserContext&			context,
		bool					srcDone
	);

	ParseTask::Result::Value
	ParseIfStructure_CreateBody(
		IfStructureParseTask*	task,
		ParserContext&			context,
		bool					srcDone
	);

	ParseTask::Result::Value
	ParseIfStructure_PostBody(
		IfStructureParseTask*	task,
		ParserContext&			context,
		bool					srcDone
	);

	ParseTask::Result::Value
	parseLoopStructure(
		LoopStructureParseTask*	task,
		ParserContext&			context,
		bool					srcDone
	);

	ParseTask::Result::Value
	ParseLoop_AwaitFinish(
		LoopStructureParseTask*	task,
		ParserContext&			context
	);

	ParseResult::Value
	ParseLoop_AddEndLoop(
		ParserContext&			context
	);

	ParseResult::Value
	ParseLoop_AddLoopSkip(
		ParserContext&			context
	);

	ParseTask::Result::Value
	parseSRPS(
		SRPSParseTask*			task,
		ParserContext&			context,
		bool					srcDone
	);

	// ----- EXECUTION SYSTEM -----

	inline void
	addNewTaskToStack(
		Task* t
	);

	inline Task*
	getLastTask();

	inline void
	popLastTask();

	inline void
	addOpStrandToStack(
		OpStrand*	strand
	);

public:
	EngineResult::Value
	execute();

	/*
	This is meant to be called by extensions that need to run functions passed to them.
	\param functionObject - the function container with the function whose body is to be run.
	This is an object that the external function may receive as a parameter, so there's no need to modify it.
	\param args - the arguments that are passed to this function.
	\return - Signal indicating the success (EngineResult::Ok) or failure (EngineResult::Error)
		of processing. Return may also indicate the end of the VM running (EngineResult::Done).
	*/
	EngineResult::Value
	runFunctionObject(
		FunctionObject*	 functionObject,
		List<Object*>*  args = REAL_NULL
	);

	/*
		This returns the address of the very last object processed by the engine.
		It is usually (but not always) the return of a function, so if you use runFunctionObject, you
		can use it to check the return of the function.
	*/
	Object*
	getLastObject() const;

protected:

	ExecutionResult::Value
	operate(
		OpStrandStackIter&	opStrandStackIter,
		OpStrandIter&		opIter
	);

	void
	addForeignFunctionArgsToStackFrame(
		StackFrame&  stackFrame,
		const List<String>&  functionParams,
		List<Object*>&  arguments
	);

	FuncExecReturn::Value
	setupFunctionExecution(
		FuncFoundTask& task,
		OpStrandStackIter&	opStrandStackIter
	);

	FuncExecReturn::Value
	setupBuiltinFunctionExecution(
		FuncFoundTask& task,
		OpStrandStackIter&	opStrandStackIter
	);

	FuncExecReturn::Value
	setupForeignFunctionExecution(
		FuncFoundTask& task
	);

	FuncExecReturn::Value
	setupUserFunctionExecution(
		FuncFoundTask& task,
		OpStrandStackIter&	opStrandStackIter
	);

	Variable*
	resolveVariableAddress(
		const VarAddress& address
	);

	ExecutionResult::Value
	run_Own(
		const VarAddress& address
	);

	bool
	is_var_pointer(
		const VarAddress& address
	);

	ExecutionResult::Value
	run_Is_owner(
		const VarAddress& address
	);

	ExecutionResult::Value
	run_Is_ptr(
		const VarAddress& address
	);

	void printSystemFunctionWrongArgCount(
		SystemFunction::Value  function,
		UInteger  argsGiven,
		UInteger  argCount
	);

	void printSystemFunctionWrongArg(
		SystemFunction::Value  function,
		UInteger  argIndex,
		UInteger  argCount,
		ObjectType::Value  given,
		ObjectType::Value  expected,
		bool isError = true
	);

	FuncExecReturn::Value	process_sys_return(			FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_not(			FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_all(			FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_any(			FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_nall(			FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_none(			FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_xall(			FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_are_fn(			FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_are_empty(		FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_are_same(		FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_member(			FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_member_count(	FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_is_member(		FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_set_member(		FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_member_list(	FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_union(			FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_type(			FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_are_same_type(	FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_are_bool(		FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_are_string(		FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_are_list(		FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_are_number(		FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_are_integer(	FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_are_decimal(	FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_assert(			FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_copy(			FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_execute_with_alt_super( FuncFoundTask& task, OpStrandStackIter&	opStrandStackIter );
	FuncExecReturn::Value   process_sys_share_body(		FuncFoundTask& task );

	void   shareBody( Function&  source, Function&  benefactor );

	// List functions
	FuncExecReturn::Value	process_sys_make_list(		FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_list_size(		FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_list_append(	FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_list_prepend(	FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_list_insert(	FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_list_get_item(	FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_list_remove(	FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_list_clear(		FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_list_swap(		FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_list_replace(	FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_list_sublist(	FuncFoundTask& task );

	// String functions
	FuncExecReturn::Value	process_sys_string_match(	FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_string_concat(	FuncFoundTask& task );

	// Numeric functions
	FuncExecReturn::Value
	process_sys_num_chain_num(
		FuncFoundTask&  task,
		NumericObject*  (*operation)(NumericObject&, NumericObject&),
		SystemFunction::Value  functionId
	);

	FuncExecReturn::Value
	process_sys_solo_num(
		FuncFoundTask&  task,
		NumericObject*  (*operation)(NumericObject&),
		SystemFunction::Value  functionId
	);

	FuncExecReturn::Value	process_sys_num_equal(FuncFoundTask& task );
	FuncExecReturn::Value	process_sys_num_greater_than(FuncFoundTask& task, bool flip );
	FuncExecReturn::Value	process_sys_num_greater_or_equal( FuncFoundTask& task, bool flip );
	FuncExecReturn::Value	process_sys_num_abs( FuncFoundTask& task );
	static NumericObject*	process_sys_num_add( NumericObject&, NumericObject& );
	static NumericObject*	process_sys_num_subtract( NumericObject&, NumericObject& );
	static NumericObject*	process_sys_num_multiply( NumericObject&, NumericObject& );
	static NumericObject*	process_sys_num_divide( NumericObject&, NumericObject& );
	static NumericObject*	process_sys_num_modulus( NumericObject&, NumericObject& );
	static NumericObject*	process_sys_num_incr( NumericObject& );
	static NumericObject*	process_sys_num_decr( NumericObject& );
	static NumericObject*	process_sys_num_incr2( NumericObject&, NumericObject& );
	static NumericObject*	process_sys_num_decr2( NumericObject&, NumericObject& );

#ifdef COPPER_SPEED_PROFILE
	timeval sp_startTime, sp_endTime;
	double fullTime;
#endif
};

// Independent Foreign Function Prototype
// "ForeignFuncPrototypePtr"
typedef  ForeignFunc::Result (*ForeignFuncPrototypePtr)( FFIServices& );


// Function for automatically creating foreign function instances and assigning them to the engine
// NOTE: Must be after the definition of Engine.
// WARNING: The typename MUST be of a type inheriting ForeignFunc.
// Usage: addForeignFuncInstance<ClassDecententOfForeignFunc> ( engine, "callname" )
template<typename ForeignFuncClass>
void addForeignFuncInstance( Engine& pEngine, const String& pName ) {
	ForeignFunc* f = new ForeignFuncClass();
	pEngine.addForeignFunction(pName, f);
	f->deref();
}

void
addNewForeignFunc(
	Engine&  pEngine,
	const String&  pName,
	ForeignFunc*  pFunction
);

//! Macro for creating foreign function structs.
#define CuForeignFuncStruct(name) \
struct name : public ::Cu::ForeignFunc { \
	virtual bool call( ::Cu::FFIServices& ); \
};

//! Class for wrapping variadic functions
class ForeignFuncWrapper : public ForeignFunc {

	ForeignFuncPrototypePtr  func;

public:
	ForeignFuncWrapper( ForeignFuncPrototypePtr  f )
		: func(f)
	{}

	virtual Result
	call( FFIServices& ffi ) {
		return func(ffi);
	}
};

void
addForeignFuncInstance(
	Engine&  pEngine,
	const String&  pName,
	ForeignFuncPrototypePtr  pFunction
);

//! Class for wrapping callback functions and using them
class CallbackWrapper : public ForeignFunc, public Owner {

	Engine& engine;
	FunctionObject*  callback;

public:
	CallbackWrapper( Engine&  pEngine, const String&  pName );

	~CallbackWrapper();

	bool
	run();

	virtual bool
	owns( FunctionObject*  container ) const;

	virtual Result
	call( FFIServices&  ffi );
};

//! Class for adding foreign methods
/*
	Example:
	struct MyClass {
		bool method( FFIServices& ffi );
	};
	//...
	MyClass mc;
	ForeignMethod<MyClass> fm( mc::method );
*/
template<class BaseClass>
class ForeignMethod : public ForeignFunc {
	typedef  Result (BaseClass::*ForeignMethodPrototypePtr)( FFIServices& );

	BaseClass*  base;
	ForeignMethodPrototypePtr  method;

public:
	ForeignMethod( BaseClass*  pBase, ForeignMethodPrototypePtr  pMethod )
		: base(pBase)
		, method(pMethod)
	{}

	virtual Result
	call( FFIServices& ffi ) {
		return (base->*method)(ffi); // Calling the method
	}
};

template<class BaseClass>
void addForeignMethodInstance(
	Engine&  pEngine,
	const String&  pName,
	BaseClass*  pBase,
	//ForeignMethod<BaseClass>::ForeignMethodPrototypePtr  pMethod // Doesn't work
	ForeignFunc::Result (BaseClass::*pMethod)( FFIServices& )
) {
	ForeignFunc* ff = new ForeignMethod<BaseClass>(pBase, pMethod);
	pEngine.addForeignFunction(pName, ff);
	ff->deref();
}

}

#endif
