// Copyright 2017 Nicolaus Anderson
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

const char*
getStringFromEngineMessage(
	const EngineMessage::Value&		msg,
	EngineErrorLevel::Value&		errLevel
);

const char*
getSystemFunctionDefaultName(
	SystemFunction::Value
);

}

#endif
