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
		return "Handling for this token has not been implemented. Are you using beta version?";

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
	case EngineMessage::MissingFunctionCallParam:
		return "Missing parameter for function call. Defaulting to empty function.";

	// WARNING
	// Attempting to treat a built-in function like a variable.
	case EngineMessage::SystemFuncInvalidAccess:
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

	// WARNING
	// No boolean result came from the condition body given to the if-statement.
	// The statement automatically defaults to "false".
	// If this happens for successive if and elif statements, the else block is still executed.
	case EngineMessage::ConditionlessIf:
		return "Missing boolean condition in \"if\" structure condition. Defaulting to false.";

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
		return "Invalid token found in pointer-handling function parameter list.";

	// ERROR
	// A token other than the parameter body ending was found before "own" or "is_ptr" ended (after parameter collection).
	case EngineMessage::InvalidTokenForEndingPtrHandlerParamBody:
		errLevel = EngineErrorLevel::error;
		return "Invalid token found before end of pointer-handling function parameter.";

	// User-induced error
	case EngineMessage::AssertionFailed:
		errLevel = EngineErrorLevel::error;
		return "Assertion failed!";

	// ERROR
	// An incorrect number of args (!=2) was passed to a system function.
	case EngineMessage::SystemFunctionWrongArgCount:
		errLevel = EngineErrorLevel::error;
		return "Wrong number of parameters passed to a system function.";

	// ERROR
	// An incorrect arg was passed to a system function.
	case EngineMessage::SystemFunctionBadArg:
		errLevel = EngineErrorLevel::error;
		return "Bad parameter passed to a system function.";

	// ERROR
	// member() was given the wrong number of parameters.
	case EngineMessage::MemberWrongArgCount:
		errLevel = EngineErrorLevel::error;
		return "Wrong number of parameters passed to member(). Should be 2.";

	// ERROR
	// First parameter passed to member() was not a function.
	case EngineMessage::MemberArg1NotFunction:
		errLevel = EngineErrorLevel::error;
		return "1st parameter of member() was not a function.";

	// ERROR
	// Destroyed function passed to member() function. Parameter was probably a pointer.
	case EngineMessage::DestroyedFuncAsMemberArg:
		errLevel = EngineErrorLevel::error;
		return "Destroyed function passed to member().";

	// ERROR
	// Second parameter passed to member() was not a string.
	case EngineMessage::MemberArg2NotString:
		errLevel = EngineErrorLevel::error;
		return "2nd parameter passed to member() was not a string.";

	// ERROR
	// Invalid name passed to member() function.
	case EngineMessage::MemberFunctionInvalidNameArg:
		errLevel = EngineErrorLevel::error;
		return "Invalid name passed as 2nd parameter to member().";

	// ERROR
	// member_count() was given the wrong number of parameters.
	case EngineMessage::MemberCountWrongArgCount:
		errLevel = EngineErrorLevel::error;
		return "No parameters passed to member_count(). Default return is zero.";

	// ERROR
	// First parameter passed to member_count() was not a function.
	case EngineMessage::MemberCountArg1NotFunction:
		errLevel = EngineErrorLevel::error;
		return "1st parameter of member_count() was not a function.";

	// ERROR
	// Destroyed function passed to member_count() function. Parameter was probably a pointer.
	case EngineMessage::DestroyedFuncAsMemberCountArg:
		errLevel = EngineErrorLevel::error;
		return "Destroyed function passed to member_count().";

	// ERROR
	// is_member() was given the wrong number of parameters.
	case EngineMessage::IsMemberWrongArgCount:
		errLevel = EngineErrorLevel::error;
		return "Wrong number of parameters passed to is_member(). Should be 2.";

	// ERROR
	// First parameter passed to is_member() was not a function.
	case EngineMessage::IsMemberArg1NotFunction:
		errLevel = EngineErrorLevel::error;
		return "1st parameter of is_member() was not a function.";

	// ERROR
	// Destroyed function passed to is_member() function. Parameter was probably a pointer.
	case EngineMessage::DestroyedFuncAsIsMemberArg:
		errLevel = EngineErrorLevel::error;
		return "Destroyed function passed to is_member().";

	// ERROR
	// Second parameter passed to is_member() was not a string.
	case EngineMessage::IsMemberArg2NotString:
		errLevel = EngineErrorLevel::error;
		return "2nd parameter of is_member() was not a string.";

	// ERROR
	// set_member() was given the wrong number of parameters.
	case EngineMessage::SetMemberWrongArgCount:
		errLevel = EngineErrorLevel::error;
		return "Wrong number of parameters passed to set_member(). Should be 3.";

	// ERROR
	// First parameter passed to set_member() was not a function.
	case EngineMessage::SetMemberArg1NotFunction:
		errLevel = EngineErrorLevel::error;
		return "1st parameter of set_member() was not a function.";

	// ERROR
	// Second parameter passed to set_member() was not a string.
	case EngineMessage::SetMemberArg2NotString:
		errLevel = EngineErrorLevel::error;
		return "2nd parameter of set_member() was not a string.";

	// ERROR
	// Destroyed function passed to set_member() function. Parameter was probably a pointer.
	case EngineMessage::DestroyedFuncAsSetMemberArg:
		errLevel = EngineErrorLevel::error;
		return "Destroyed function passed to set_member().";

	// WARNING
	// Destroyed function passed to union() function. Parameter was probably a pointer.
	case EngineMessage::DestroyedFuncAsUnionArg:
		return "Destroyed function passed to union().";

	// WARNING
	// A parameter passed to the union() function was not a function.
	case EngineMessage::NonFunctionAsUnionArg:
		return "Param passed to union() was not a function.";

	// ERROR
	// The wrong number of arguments was passed to a foreign function.
	// Aliases to "Argument count does not match foreign function header."
	case EngineMessage::ForeignFunctionWrongArgCount:
		errLevel = EngineErrorLevel::error;
		return "Wrong number of arguments passed to a foreign function.";

	// ERROR
	// An argument passed to a foreign function did not match the requested type.
	// Aliases to "Argument types do not match foreign function header."
	case EngineMessage::ForeignFunctionBadArg:
		errLevel = EngineErrorLevel::error;
		return "Argument passed to a foreign function was not of the requested type.";

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

}
