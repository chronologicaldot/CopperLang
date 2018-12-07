// Copyright 2017 Nicolaus Anderson

#include "EngMsgToStr.h"

namespace Cu {

using Cu::EngineMessage;

const char*
getStringFromEngineMessage(
	const EngineMessage::Value&		msg,
	EngineErrorLevel::Value&		errLevel
) {
	errLevel = EngineErrorLevel::none;

	switch( msg ) {

	case EngineMessage::Running:
		return "INFO: Engine is running.";

	case EngineMessage::ZeroSizeToken:
		errLevel = EngineErrorLevel::error;
		return "Zero size token encountered.";

	// ERROR
	case EngineMessage::StackOverflow:
		errLevel = EngineErrorLevel::error;
		return "Stack overflow.";

	// WARNING
	/* Function does not exist when trying to access it via FunctionContainer.
	This can occur when trying to access via a pointer. */
	case EngineMessage::NoFunctionInContainer:
		return "No function in container.";

	// ERROR
	/* The special character (used for hiding other characters within a string) has been
	found outside of a string. */
	case EngineMessage::EscapeCharMisplaced:
		errLevel = EngineErrorLevel::error;
		return "Encountered stray escape character.";

	// ERROR
	// The given token is too long. Tokens are limited to 255 characters (bytes) in length.
	case EngineMessage::LongToken:
		errLevel = EngineErrorLevel::error;
		return "Given token is too long. Must not exceed 255 bytes in length.";

	// ERROR
	// Token could not be resolved
	case EngineMessage::UnresolvedToken:
		errLevel = EngineErrorLevel::error;
		return "Given token could not be resolved.";

	// ERROR
	// A token failed to be parsed. Usually, this means a zero-size token.
	case EngineMessage::MalformedToken:
		return "Malformed token encountered.";

	// ERROR
	// A token with an unpermitted arrangement of characters was encountered.
	case EngineMessage::UnknownToken:
		errLevel = EngineErrorLevel::error;
		return "Unknown token encountered.";

	// ERROR
	// A token that starts with a numeric letter has been found to be not a number.
	case EngineMessage::MalformedNumber:
		errLevel = EngineErrorLevel::error;
		return "Token starting with a numeric character is not a number.";

	// ERROR
	// The encountered token began like a name but wasn't valid.
	case EngineMessage::InvalidName:
		errLevel = EngineErrorLevel::error;
		return "Invalid name encountered.";

	// ERROR
	// The stream ended before the parsing was completed.
	case EngineMessage::StreamHaltedParsingNotDone:
		errLevel = EngineErrorLevel::error;
		return "Stream ended before parsing completed.";

	// ERROR
	// Stream ended before comment ended.
	case EngineMessage::StreamHaltedInComment:
		errLevel = EngineErrorLevel::error;
		return "Stream ended before comment ended.";

	// ERROR
	// Stream ended before string ended.
	case EngineMessage::StreamHaltedInString:
		errLevel = EngineErrorLevel::error;
		return "Stream ended before string ended.";

	// ERROR
	// Parameter body has been opened without being preceded by a name.
	case EngineMessage::OrphanParamBodyOpener:
		errLevel = EngineErrorLevel::error;
		return "Stray parameter-body-opener token encountered.";

	// ERROR
	// Parameter body has been closed without being opened.
	case EngineMessage::OrphanParamBodyCloser:
		errLevel = EngineErrorLevel::error;
		return "Stray parameter-body-closer token encountered.";

	// ERROR
	// Object body was started but the stream ended.
	case EngineMessage::OrphanObjectBodyOpener:
		errLevel = EngineErrorLevel::error;
		return "Object-body incomplete.";

	// ERROR
	// Object body has not been opened before but a closing token has been encountered.
	case EngineMessage::OrphanObjectBodyCloser:
		errLevel = EngineErrorLevel::error;
		return "Stray object-body-closer token encountered.";

	// ERROR
	// Function body has been closed without first being opened.
	// (Should only happen at the global scope.)
	case EngineMessage::OrphanBodyCloser:
		errLevel = EngineErrorLevel::error;
		return "Stray body-closer token encountered.";

	// ERROR
	// "elif" used without first being preceded by "if"
	case EngineMessage::OrphanElif:
		errLevel = EngineErrorLevel::error;
		return "Stray \"elif\" keyword encountered.";

	// ERROR
	// "else" used without first being preceded by "if" or "elif"
	case EngineMessage::OrphanElse:
		errLevel = EngineErrorLevel::error;
		return "Stray \"else\" keyword encountered.";

	// ERROR
	// Token could not be handled. This results when the system has not implemented it.
	case EngineMessage::TokenNotHandled:
		errLevel = EngineErrorLevel::error;
		return "Unhandled token.";

	// ERROR
	// Invalid token encountered in the object body construction.
	// Aliases as "Stray token encountered while collecting names in object body."
	case EngineMessage::ObjectBodyInvalid:
		errLevel = EngineErrorLevel::error;
		return "Invalid token in object body. (Or stream may have ended.)";

	// ERROR
	// Unfinished or invalid parameter assignment in an object body construction.
	// Aliases as "Stray token encountered while collecting names in object body."
	case EngineMessage::ObjectBodyInvalidParamAsgn:
		errLevel = EngineErrorLevel::error;
		return "Invalid token used for parameter value.";

	// ERROR
	// Unfinished or invalid parameter pointer assignment in an object body construction.
	case EngineMessage::ObjectBodyInvalidParamPtrAsgn:
		errLevel = EngineErrorLevel::error;
		return "Invalid token used for pointer parameter value.";

	// ERROR
	// Unfinished execution body.
	case EngineMessage::ExecBodyUnfinished:
		errLevel = EngineErrorLevel::error;
		return "Execution body did not finish before stream ended.";

	// ERROR
	// Trying to create more bodies within a function than feasible.
	case EngineMessage::ExceededBodyCountLimit:
		errLevel = EngineErrorLevel::error;
		return "Body creation max reached.";

	// ERROR
	// Trying to assign the wrong kind of token to a variable.
	// May also be stream ended before assignment completed.
	case EngineMessage::InvalidAsgn:
		errLevel = EngineErrorLevel::error;
		return "Invalid token used for assignment value. (Or stream may have ended.)";

	// ERROR
	// Trying to make a variable a pointer to a non-object via wrong kind of token.
	// May also be stream ended before pointer assignment completed.
	case EngineMessage::InvalidPtrAsgn:
		errLevel = EngineErrorLevel::error;
		return "Invalid token used for pointer assignment value. Variable expected. (Or stream may have ended.)";

	// ERROR
	// Non-name followed member accessor (.).
	// May also be stream ended before member access completed.
	case EngineMessage::InvalidTokenAfterMemberAccessor:
		errLevel = EngineErrorLevel::error;
		return "Invalid token following member accessor token. Name expected.";

	// ERROR
	// Trying to create more parameter bodies within a function header than feasible.
	case EngineMessage::ExceededParamBodyCountLimit:
		errLevel = EngineErrorLevel::error;
		return "Parameter body creation max reached.";

	// ERROR
	// Stream ended before arguments could be collected for a function call.
	case EngineMessage::ArgBodyIncomplete:
		errLevel = EngineErrorLevel::error;
		return "Stream ended before argument collection finished for function call.";

	// ERROR
	// Invalid arguments passed to a function.
	// Aliases as "Invalid token in function call parameter parsing."
	case EngineMessage::InvalidFunctionArguments:
		errLevel = EngineErrorLevel::error;
		return "Invalid arguments passed to a function.";

	// WARNING
	/* Missing parameter for a function call.
	This results in a default value (of empty function) for the parameter. */
	case EngineMessage::MissingFunctionCallArg:
		return "Missing argument for function call. Defaulting to empty function.";

	// ERROR
	// Attempting to treat a built-in function like a variable.
	case EngineMessage::SystemFuncInvalidAccess:
		errLevel = EngineErrorLevel::error;
		return "Attempting to treat a built-in function like a variable";

	// WARNING
	// Attempting to treat a foreign function like a variable.
	case EngineMessage::ForeignFuncInvalidAccess:
		return "Attempting to treat a foreign function like a variable";

	// ERROR, C++
	// Last-object pointer was not set before object was to be assigned to a variable.
	case EngineMessage::UnsetLastObjectInAsgn:
		errLevel = EngineErrorLevel::cpp;
		return "Last Object not set before used in assignment.";

	// ERROR, C++
	// Last-object pointer was not set before a pointer-to-it was to be assigned to a variable.
	case EngineMessage::UnsetLastObjectInPtrAsgn:
		errLevel = EngineErrorLevel::cpp;
		return "Last Object not set before used in pointer assignment.";

	// ERROR - May be changed to WARNING
	// An attempt was made to assign raw data to a variable via the pointer assignment token.
	case EngineMessage::DataAssignedToPtr:
		errLevel = EngineErrorLevel::error;
		return "Variable pointer-assigned raw data.";

	// ERROR
	// After the if-statement, a condition (parameter body opener) did not immediately follow.
	case EngineMessage::InvalidTokenAtIfStart:
		errLevel = EngineErrorLevel::error;
		return "Invalid token following \"if\" keyword.";

	case EngineMessage::IfConditionContaminated:
		errLevel = EngineErrorLevel::error;
		return "Contamination in \"if\" structure condition.";

	// ERROR
	// No boolean result came from the condition body given to the if-statement.
	// The statement automatically defaults to "false".
	// If this happens for successive if and elif statements, the else block is still executed.
	case EngineMessage::ConditionlessIf:
		errLevel = EngineErrorLevel::error;
		return "Missing boolean condition in \"if\" structure condition.";

	// ERROR
	// A different token was found while searching for the body-opener token of an if-statment.
	case EngineMessage::InvalidTokenBeforeIfBody:
		errLevel = EngineErrorLevel::error;
		return "Invalid token encountered before \"if\" structure body.";

	// ERROR
	// A stray porameter was found while searching for the body of a loop.
	case EngineMessage::StrayTokenInLoopHead:
		errLevel = EngineErrorLevel::error;
		return "Invalid token encountered before \"loop\" structure body.";

	// ERROR
	// A stray loop-stop token was found.
	case EngineMessage::UselessLoopStopper:
		errLevel = EngineErrorLevel::error;
		return "Useless loop stop found. May disrupt processing flow.";

	// ERROR
	// A stray loop-skip token was found.
	case EngineMessage::UselessLoopSkipper:
		errLevel = EngineErrorLevel::error;
		return "Useless loop skip found. May disrupt processing flow.";

	// WARNING
	// The "own" control structure is disabled.
	case EngineMessage::PointerNewOwnershipDisabled:
		return "Pointer-ownership-changing has been disabled";

	// ERROR
	// A token other than a parameter body opener was found after "own" or "is_ptr".
	case EngineMessage::InvalidTokenBeforePtrHandlerParamBody:
		errLevel = EngineErrorLevel::error;
		return "Invalid token following pointer-handling function name.";

	// WARNING
	// A variable was not created before being used in an "own" or "is_ptr" structure, making the structure needless.
	case EngineMessage::NonexistentVariablePassedToPtrHandler:
		return "Non-existent variable passed to pointer-handling function.";

	// Warning
	// Invalid name passed to "own", "is_ptr", or "is_owner".
	case EngineMessage::NonVariablePassedToPtrHandler:
		return "Non-variable passed to pointer-handling function. Could be built-in or foreign function name.";

	// ERROR
	// Encountered a non-string token when searching for one in "own" or "is_ptr" structure.
	case EngineMessage::NonNameFoundInPtrHandlerParamBody:
		errLevel = EngineErrorLevel::error;
		return "Invalid token found in pointer-handling function argument list.";

	// ERROR
	// A token other than the parameter body ending was found before "own" or "is_ptr" ended (after parameter collection).
	case EngineMessage::InvalidTokenForEndingPtrHandlerParamBody:
		errLevel = EngineErrorLevel::error;
		return "Invalid token found before end of pointer-handling function argument.";

	// User-induced error
	case EngineMessage::AssertionFailed:
		errLevel = EngineErrorLevel::error;
		return "Assertion failed!";

	// ERROR
	// Wrong number of arguments passed to a function
	case EngineMessage::WrongArgCount:
		errLevel = EngineErrorLevel::error;
		return "Wrong argument count.";

	// ERROR
	// Wrong argument type of argument passed to a function
	case EngineMessage::WrongArgType:
		errLevel = EngineErrorLevel::error;
		return "Wrong argument type.";

	// ERROR
	// Destroyed function as argument
	case EngineMessage::DestroyedFuncAsArg:
		errLevel = EngineErrorLevel::error;
		return "Argument was a destroyed function.";

	// ERROR
	case EngineMessage::MissingArg:
		errLevel = EngineErrorLevel::error;
		return "Missing argument.";

	// WARNING
	case EngineMessage::IndexOutOfBounds:
		return "Index out of bounds. A valid value will substitute.";

	// ERROR
	case EngineMessage::BadArgIndexInForeignFunc:
		errLevel = EngineErrorLevel::error;
		return "Index for accessing argument is out-of-bounds.";

	// ERROR
	// An incorrect arg was passed to a system function.
	case EngineMessage::SystemFunctionBadArg:
		errLevel = EngineErrorLevel::error;
		return "Bad argument passed to a system function.";

	// ERROR
	// The user-function contains errors.
	// Usually, the other error will be printed. This just indicates that the problem is within a function body.
	case EngineMessage::UserFunctionBodyError:
		errLevel = EngineErrorLevel::error;
		return "Function body contains errors.";

	case EngineMessage::COUNT:
		return "INFO: tick.";
		break;

	default: return "";
	}
}

const char*
getSystemFunctionDefaultName(
	SystemFunction::Value  v
) {
	switch( v ) {

	case SystemFunction::_unset:
		return "[unset]";

	case SystemFunction::_return:
		return "ret";

	case SystemFunction::_not:
		return "not";

	case SystemFunction::_all:
		return "all";

	case SystemFunction::_any:
		return "any";

	case SystemFunction::_nall:
		return "nall";

	case SystemFunction::_none:
		return "none";

	case SystemFunction::_are_fn:
		return "are_fn";

	case SystemFunction::_are_empty:
		return "are_empty";

	case SystemFunction::_are_same:
		return "are_same";

	case SystemFunction::_member:
		return "member";

	case SystemFunction::_member_count:
		return "member_count";

	case SystemFunction::_is_member:
		return "is_member";

	case SystemFunction::_set_member:
		return "set_member";

	case SystemFunction::_member_list:
		return "member_list";

	case SystemFunction::_union:
		return "union";

	case SystemFunction::_type:
		return "type_of";

	case SystemFunction::_are_same_type:
		return "are_same_type";

	case SystemFunction::_are_bool:
		return "are_bool";

	case SystemFunction::_are_string:
		return "are_string";

	case SystemFunction::_are_list:
		return "are_list";

	case SystemFunction::_are_number:
		return "are_number";

	case SystemFunction::_are_integer:
		return "are_int";

	case SystemFunction::_are_decimal:
		return "are_deci";

	case SystemFunction::_assert:
		return "assert";

	case SystemFunction::_copy:
		return "copy";

	case SystemFunction::_execute_with_alt_super:
		return "xwsv";


	case SystemFunction::_make_list:
		return "list";

	case SystemFunction::_list_size:
		return "length";

	case SystemFunction::_list_append:
		return "append";

	case SystemFunction::_list_prepend:
		return "prepend";

	case SystemFunction::_list_insert:
		return "insert";

	case SystemFunction::_list_get_item:
		return "item_at";

	case SystemFunction::_list_remove:
		return "erase";

	case SystemFunction::_list_clear:
		return "dump";

	case SystemFunction::_list_swap:
		return "swap";

	case SystemFunction::_list_replace:
		return "replace";

	case SystemFunction::_list_sublist:
		return "sublist";


	case SystemFunction::_string_match:
		return "matching";

	case SystemFunction::_string_concat:
		return "concat";


	case SystemFunction::_num_equal:
		return "equal";

	case SystemFunction::_num_greater_than:
		return "gt";

	case SystemFunction::_num_greater_or_equal:
		return "gte";

	case SystemFunction::_num_abs:
		return "abs";

	case SystemFunction::_num_add:
		return "+"; // "add";

	case SystemFunction::_num_subtract:
		return "-"; // "subtract";

	case SystemFunction::_num_multiply:
		return "*"; // "multiply";

	case SystemFunction::_num_divide:
		return "/"; // "divide";

	case SystemFunction::_num_modulus:
		return "%"; // "modulus";

	case SystemFunction::_num_incr:
		return "++"; // "increment";

	case SystemFunction::_num_decr:
		return "--"; // "decrement";

	default:
		return "";
	}
}

}
