#ifndef COPPER_ENGINE_MESSAGE_TO_STRING
#define COPPER_ENGINE_MESSAGE_TO_STRING

#include "../src/Copper.h"

namespace Cu {

using Cu::EngineMessage;

struct EngineErrorLevel {
	enum Value {
		none,
		error,
		system,
		cpp
	};
};

const char* getStringFromEngineMessage(const EngineMessage::Value& msg, EngineErrorLevel::Value& errLevel) {
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
	// Stream ended before comment ended.
	case EngineMessage::StreamInteruptedInComment:
		errLevel = EngineErrorLevel::error;
		return "Stream ended before comment ended.";

	// ERROR
	// Stream ended before string ended.
	case EngineMessage::StreamInteruptedInString:
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
	/* Function body has been closed without first being opened.
	(Should only happen at the global scope.) */
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
	// Bad token found at the start of a function (after "fn").
	case EngineMessage::InvalidTokenAtFunctionStart:
		errLevel = EngineErrorLevel::error;
		return "Invalid token following \"fn\" keyword.";

	// WARNING
	/* Unused assignment symbol in the function construction parameters.
	This can result from accidentally adding the end-statement symbol before it. */
	case EngineMessage::UnusedAsgnInFunctionBuildParams:
		return "Unused assignment symbol in function construction.";

	// WARNING
	/* Unused pointer-assignment/creation symbol in the function construction parameters.
	This can result from accidentally adding the end-statement symbol before it. */
	case EngineMessage::UnusedPtrAsgnInFunctionBuildParams:
		return "Unused pointer-assignment symbol in function construction.";

	// ERROR
	/* Unused token in function construction parameters.
	This can result from forgetting a token. */
	case EngineMessage::UnusedTokenInFunctionBuildParams:
		errLevel = EngineErrorLevel::error;
		return "Stray token found in function construction parameters.";

	// ERROR
	// Bad token found when collecting the parameter value.
	case EngineMessage::InvalidParamValueToken:
		errLevel = EngineErrorLevel::error;
		return "Invalid token used for parameter value.";

	// ERROR
	// Attempting to assign a pointer a non-named item.
	case EngineMessage::PtrAssignedNonName:
		errLevel = EngineErrorLevel::error;
		return "Invalid token used for parameter value of a pointer. Variable expected.";

	// ERROR
	// Attempting to open the scope of a system function.
	case EngineMessage::OpeningSystemFuncScope:
		errLevel = EngineErrorLevel::error;
		return "Invalid opening of system function scope.";

	// ERROR
	// Attempting to open the scope of an extension function. Currently, this is unsupported.
	case EngineMessage::OpeningExtensionFuncScope:
		errLevel = EngineErrorLevel::error;
		return "Invalid opening of extension function scope.";

	// ERROR
	// Trying to create more bodies within a function than feasible.
	case EngineMessage::ExceededBodyCountLimit:
		errLevel = EngineErrorLevel::error;
		return "Body creation max reached.";

	// ERROR
	// Trying to assign the wrong kind of token to a variable.
	case EngineMessage::InvalidAsgn:
		errLevel = EngineErrorLevel::error;
		return "Invalid token used for assignment value.";

	// ERROR
	// Trying to make a variable a pointer to a non-object via wrong kind of token.
	case EngineMessage::InvalidPtrAsgn:
		errLevel = EngineErrorLevel::error;
		return "Invalid token used for pointer assignment value. Variable expected.";

	// ERROR
	// Non-name followed member accessor (.).
	case EngineMessage::InvalidTokenAfterMemberAccessor:
		errLevel = EngineErrorLevel::error;
		return "Invalid token following member accessor token. Name expected.";

	// ERROR - SERIOUS!
	// Task Variable does not exist.
	case EngineMessage::TaskVarIsNull:
		errLevel = EngineErrorLevel::system;
		return "Task variable is null.";

	// ERROR
	// Trying to create more parameter bodies within a function header than feasible.
	case EngineMessage::ExceededParamBodyCountLimit:
		errLevel = EngineErrorLevel::error;
		return "Parameter body creation max reached.";

	// WARNING
	/* Missing parameter for a function call.
	This results in a default value (of empty function) for the parameter. */
	case EngineMessage::MissingFunctionCallParam:
		return "Missing parameter for function call. Defaulting to empty function.";

	// ERROR, C++
	// Last-object pointer was not set before object was to be assigned to a variable.
	case EngineMessage::UnsetLastObjectInAsgn:
		errLevel = EngineErrorLevel::cpp;
		return "Last Object not set before used in assignment.";

	// WARNING
	// An attempt was made to assign a new function to a name reserved for a built-in function.
	case EngineMessage::AttemptToOverrideBuiltinFunc:
		return "Attempt to override built-in function name.";

	// WARNING
	// An attempt was made to assign a new function to a name reserved for an external function.
	case EngineMessage::AttemptToOverrideExternalFunc:
		return "Attempt to override external function name.";

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

	// WARNING
	/* No boolean result came from the condition body given to the if-statement.
	The statement automatically defaults to "false".
	If this happens for successive if and elif statements, the else block is still executed. */
	case EngineMessage::ConditionlessIf:
		return "Missing boolean result in \"if\" structure condition. Defaulting to false.";

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
	// An incorrect number of params (!=2) was passed to a system function.
	case EngineMessage::SystemFunctionWrongParamCount:
		errLevel = EngineErrorLevel::error;
		return "Wrong number of parameters passed to a system function.";

	// ERROR
	// An incorrect parameter was passed to a system function.
	case EngineMessage::SystemFunctionBadParam:
		errLevel = EngineErrorLevel::error;
		return "Bad parameter passed to a system function.";

	// ERROR
	// member() was given the wrong number of parameters.
	case EngineMessage::MemberWrongParamCount:
		errLevel = EngineErrorLevel::error;
		return "Wrong number of parameters passed to member(). Should be 2.";

	// ERROR
	// First parameter passed to member() was not a function.
	case EngineMessage::MemberParam1NotFunction:
		errLevel = EngineErrorLevel::error;
		return "1st parameter of member() was not a function.";

	// ERROR
	// Destroyed function passed to member() function. Parameter was probably a pointer.
	case EngineMessage::DestroyedFuncAsMemberParam:
		errLevel = EngineErrorLevel::error;
		return "Destroyed function passed to member().";

	// ERROR
	// Second parameter passed to member() was not a string.
	case EngineMessage::MemberParam2NotString:
		errLevel = EngineErrorLevel::error;
		return "2nd parameter passed to member() was not a string.";

	// ERROR
	// Invalid name passed to member() function.
	case EngineMessage::MemberFunctionInvalidNameParam:
		errLevel = EngineErrorLevel::error;
		return "Invalid name passed as 2nd parameter to member().";

	// ERROR
	// member_count() was given the wrong number of parameters.
	case EngineMessage::MemberCountWrongParamCount:
		errLevel = EngineErrorLevel::error;
		return "No parameters passed to member_count(). Default return is zero.";

	// ERROR
	// First parameter passed to member_count() was not a function.
	case EngineMessage::MemberCountParam1NotFunction:
		errLevel = EngineErrorLevel::error;
		return "1st parameter of member_count() was not a function.";

	// ERROR
	// Destroyed function passed to member_count() function. Parameter was probably a pointer.
	case EngineMessage::DestroyedFuncAsMemberCountParam:
		errLevel = EngineErrorLevel::error;
		return "Destroyed function passed to member_count().";

	// ERROR
	// is_member() was given the wrong number of parameters.
	case EngineMessage::IsMemberWrongParamCount:
		errLevel = EngineErrorLevel::error;
		return "Wrong number of parameters passed to is_member(). Should be 2.";

	// ERROR
	// First parameter passed to is_member() was not a function.
	case EngineMessage::IsMemberParam1NotFunction:
		errLevel = EngineErrorLevel::error;
		return "1st parameter of is_member() was not a function.";

	// ERROR
	// Destroyed function passed to is_member() function. Parameter was probably a pointer.
	case EngineMessage::DestroyedFuncAsIsMemberParam:
		errLevel = EngineErrorLevel::error;
		return "Destroyed function passed to is_member().";

	// ERROR
	// Second parameter passed to is_member() was not a string.
	case EngineMessage::IsMemberParam2NotString:
		errLevel = EngineErrorLevel::error;
		return "2nd parameter of is_member() was not a string.";

	// ERROR
	// set_member() was given the wrong number of parameters.
	case EngineMessage::SetMemberWrongParamCount:
		errLevel = EngineErrorLevel::error;
		return "Wrong number of parameters passed to set_member(). Should be 3.";

	// ERROR
	// First parameter passed to set_member() was not a function.
	case EngineMessage::SetMemberParam1NotFunction:
		errLevel = EngineErrorLevel::error;
		return "1st parameter of set_member() was not a function.";

	// ERROR
	// Second parameter passed to set_member() was not a string.
	case EngineMessage::SetMemberParam2NotString:
		errLevel = EngineErrorLevel::error;
		return "2nd parameter of set_member() was not a string.";

	// ERROR
	// Destroyed function passed to set_member() function. Parameter was probably a pointer.
	case EngineMessage::DestroyedFuncAsSetMemberParam:
		errLevel = EngineErrorLevel::error;
		return "Destroyed function passed to set_member().";

	// WARNING
	// Destroyed function passed to union() function. Parameter was probably a pointer.
	case EngineMessage::DestroyedFuncAsUnionParam:
		return "Destroyed function passed to union().";

	// WARNING
	// A parameter passed to the union() function was not a function.
	case EngineMessage::NonFunctionAsUnionParam:
		return "Param passed to union() was not a function.";

	case EngineMessage::COUNT:
		return "INFO: tick.";
		break;

	default: return "";
	}
}

}

#endif
