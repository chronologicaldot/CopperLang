// Copyright 2016 Nicolaus Anderson

#include "../src/Copper.h"
#include "EngMsgToStr.h"


namespace CuStd {

using Cu::LogLevel;
using Cu::EngineErrorLevel;

/*
A better implementation would also be tied to the input stream and be able to inform me of the actual output.
*/
class BasicLogger : public Cu::Logger {
	FILE* outFile;

public:
	bool enabled;
	bool showInfo;
	bool showWarnings;
	bool showErrors;

	BasicLogger()
		: outFile(stdout)
		, enabled(true)
		, showInfo(true)
		, showWarnings(true)
		, showErrors(true)
	{}

	bool willShow(LogLevel logLevel) {
		switch( logLevel ) {
		case LOG_LEVEL_info:
			return showInfo;
		case LOG_LEVEL_warning:
			return showWarnings;
		case LOG_LEVEL_error:
			return showErrors;
		}
	}

	virtual void log(const LogLevel::Value& logLevel, const char* msg) {
		if ( !enabled || !willShow(logLevel) ) return;

		fprintf(outFile, "%s\n", msg);
	}

	virtual void log(const LogLevel::Value& logLevel, const EngineMessage::Value& msg) {
		if ( !enabled || !willShow(logLevel) ) return;

		EngineErrorLevel::Value errLevel;
		const char* strMsg = Cu::getStringFromEngineMessage(msg, errLevel);
		fprintf(outFile, "%s\n", strMsg);
	}
};

}
