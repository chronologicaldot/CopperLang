// Copyright 2016 Nicolaus Anderson

#include "../src/Copper.h"
#include "EngMsgToStr.h"
#include <cstdio>
#include <cstring> // For strlen

namespace CuStd {

using Cu::UInteger;
using Cu::LogLevel;
using Cu::EngineMessage;
using Cu::EngineErrorLevel;
using Cu::LogMessage;

class InStreamLogger : public Cu::Logger, public Cu::ByteStream {

	std::FILE* inFile;
	std::FILE* outFile;
	bool atEOF;
	unsigned long lines;
	unsigned long columns;
	unsigned long prev_lines;
	unsigned long prev_columns;
	EngineErrorLevel::Value errLevel;

public:

	bool enabled;
	bool showInfo;
	bool showWarnings;
	bool showErrors;
	bool printNaturalTokens;

	InStreamLogger()
		: inFile(stdin)
		, outFile(stdout)
		, atEOF(false)
		, lines(1)
		, columns(0)
		, prev_lines(1)
		, prev_columns(0)
		, errLevel(EngineErrorLevel::error)
		, enabled(true)
		, showInfo(true)
		, showWarnings(true)
		, showErrors(true)
		, printNaturalTokens(true)
	{}

	// Stream functions

	void setInputFile(std::FILE* pFile) {
		if ( pFile ) {
			inFile = pFile;
			atEOF = false;
		} else {
			inFile = stdin;
		}
	}

	virtual char getNextByte() {
		int c = std::fgetc(inFile);
		prev_lines = lines;
		prev_columns = columns;
		if ( c != EOF ) {
			++columns;
			if ( c == '\n' ) {
				++lines;
				columns = 0;
				// Addendum
				if ( inFile == stdin )
					atEOF = true;
			}
			return char(c);
		}
		atEOF = true;
		lines = 1;
		columns = 0;
		prev_lines = 1;
		prev_columns = 0;
		return ' ';
	}

	virtual bool atEOS() {
		bool val = atEOF;
		if ( inFile == stdin )
			atEOF = false;
		return val;
	}

	// Logger functions

	void setLogFile(std::FILE* pFile ) {
		if ( pFile ) {
			outFile = pFile;
		} else {
			outFile = stdout;
		}
	}

	bool willShow(LogLevel::Value logLevel) {
		switch( logLevel ) {
		case LogLevel::info:
			return showInfo;
		case LogLevel::warning:
			return showWarnings;
		case LogLevel::error:
			return showErrors;
		case LogLevel::debug:
			return true;
		default: // Defensive programming
			return true;
		}
	}

protected:
	void printInfo(const char* msg) {
		//std::fprintf(outFile, "\033[0;46m%s\033[0m\n", msg);
		std::fprintf(outFile, "%s\n", msg);
	}

	void printWarning(const char* msg) {
		if ( outFile == stdout )
			std::printf("\033[34m\033[103m Warning \33[0m\33[44m (%lu:%lu) \033[0m\033[0m \033[93m%s\033[0m\n", prev_lines, prev_columns, msg);
		else
			std::fprintf(outFile, " Warning  (%lu:%lu): %s\n", prev_lines, prev_columns, msg);
	}

	void printError(const char* msg) {
		if ( outFile == stdout )
			std::printf("\33[41m ERROR \33[44m (%lu:%lu) \033[0m\33[91m %s\33[0m\n", prev_lines, prev_columns, msg);
		else
			std::fprintf(outFile, " ERROR  (%lu:%lu): %s\n", prev_lines, prev_columns, msg);
	}

	void printSystemError(const char* msg) {
		if ( outFile == stdout )
			std::printf("\033[45m System Error \033[44m (%lu:%lu) \033[0m\33[35m%s\33[0m\n", prev_lines, prev_columns, msg);
		else
			std::fprintf(outFile, " System Error  (%lu:%lu): %s\n", prev_lines, prev_columns, msg);
	}

	void printCPPError(const char* msg) {
		if ( outFile == stdout )
			std::printf("\033[100m C++ Error \033[44m (%lu:%lu) \033[0m\033[91m%s\033[0m\n", prev_lines, prev_columns, msg);
		else
			std::fprintf(outFile, " C++ Error  (%lu:%lu): %s\n", prev_lines, prev_columns, msg);
	}

	void printDebug(const char* msg) {
		if ( outFile == stdout )
			std::printf("\033[45m Debug \033[44m (%lu:%lu) \033[0m\033[35m%s\033[0m\n", prev_lines, prev_columns, msg);
		else
			std::fprintf(outFile, " Debug  (%lu:%lu): %s\n", prev_lines, prev_columns, msg);
	}

public:
	virtual void print(const LogLevel::Value logLevel, const char* msg) {
		if ( !enabled || !willShow(logLevel) ) return;

		switch( logLevel ) {
		case LogLevel::info:
			printInfo(msg);
			break;

		case LogLevel::warning:
			printWarning(msg);
			break;

		case LogLevel::error:
			switch( errLevel ) {
				case EngineErrorLevel::error:
					printError(msg);
					break;

				case EngineErrorLevel::system:
					printSystemError(msg);
					break;

				case EngineErrorLevel::cpp:
					printCPPError(msg);
					break;

				default: break;
			}
			break;

		case LogLevel::debug:
			printDebug(msg);
			break;

		default: break;
		}
		errLevel = EngineErrorLevel::error;
	}

	virtual void print(const LogLevel::Value logLevel, const EngineMessage::Value msg) {
		if ( !enabled || !willShow(logLevel) )
			return;

		print(logLevel, Cu::getStringFromEngineMessage(msg, errLevel));
	}

	virtual void print(LogMessage  logMsg)
	{
		if ( !enabled || !willShow(logMsg.level) )
			return;

		const char*  msg = Cu::getStringFromEngineMessage(logMsg.messageId, errLevel);
		Cu::String  format = "Function %s, arg %u of %u: %s";
		size_t  msgSize = 0;

		if ( logMsg.functionName.size() == 0 ) {
			if ( logMsg.systemFunctionId != Cu::SystemFunction::_unset ) {
				logMsg.functionName = Cu::getSystemFunctionDefaultName(logMsg.systemFunctionId);
			} else {
				logMsg.functionName = "global";
			}
		}

		if ( logMsg.givenArgType.size() > 0 ) {
			format = "Function %s, arg %u of %u: %s\nExpected: %s, Given: %s";
			msgSize = (size_t)format.size()
					+ (size_t)logMsg.functionName.size()
					+ strlen(msg)
					+ (size_t)logMsg.givenArgType.size()
					+ (size_t)logMsg.expectedArgType.size()
					+ sizeof(Cu::UInteger)*2;
		}
		else {
			msgSize = (size_t)format.size()
					+ (size_t)logMsg.functionName.size()
					+ strlen(msg)
					+ sizeof(Cu::UInteger)*2;
		}

		char msgFull[ msgSize ];

		if ( logMsg.givenArgType.size() > 0 ) {
			snprintf(msgFull, msgSize, format.c_str(),
					logMsg.functionName.c_str(), logMsg.argIndex, logMsg.argCount, msg,
					logMsg.expectedArgType.c_str(), logMsg.givenArgType.c_str());
		} else {
			snprintf(msgFull, msgSize, format.c_str(),
					logMsg.functionName.c_str(), logMsg.argIndex, logMsg.argCount, msg);
		}

		switch ( logMsg.level ) {
		case LogLevel::info:
			//fprintf(outFile, "In %s, arg %u: %s\n", logMsg.functionName.c_str(), logMsg.argIndex, msg);
			printInfo(msgFull);
			break;

		case LogLevel::warning:
			//fprintf(outFile, "WARNING: In %s, arg %u: %s\n", logMsg.functionName.c_str(), logMsg.argIndex, msg);
			printWarning(msgFull);
			break;

		case LogLevel::error:
			//fprintf(outFile, "ERROR: In %s, arg %u: %s\n", logMsg.functionName.c_str(), logMsg.argIndex, msg);
			printError(msgFull);
			break;

		case LogLevel::debug:
			//fprintf(outFile, "DEBUG: In %s, arg %u: %s\n", logMsg.functionName.c_str(), logMsg.argIndex, msg);
			printDebug(msgFull);
			break;

		default: break;
		}
	}

	virtual void printTaskTrace( Cu::TaskType::Value  taskType, const Cu::String&  taskName, UInteger  taskNumber ) {
		if ( !enabled )
			return;

		if ( taskType == Cu::TaskType::FuncFound ) {
			if ( outFile == stdout )
				fprintf(outFile, "\33[46m TASK TRACE %u \33[0m \33[96m[CALL] %s\33[0m\n", taskNumber, taskName.c_str());
			else
				fprintf(outFile, " TASK TRACE %u : %s\n", taskNumber, taskName.c_str());
		} else {
			if ( outFile == stdout )
				fprintf(outFile, "\33[46m TASK TRACE %u \33[0m \33[96m[BUILD] %s\33[0m\n", taskNumber, taskName.c_str());
			else
				fprintf(outFile, " TASK TRACE %u : %s\n", taskNumber, taskName.c_str());
		}
	}

	virtual void printStackTrace( const Cu::String&  frameName, UInteger  frameNumber ) {
		if ( !enabled )
			return;

		if ( outFile == stdout )
			fprintf(outFile, "\33[44m STACK TRACE %u \33[0m \33[94m %s\33[0m\n", frameNumber, frameName.c_str());
		else
			fprintf(outFile, " STACK TRACE %u : %s\n", frameNumber, frameName.c_str());
	}

	virtual void printToken( const Cu::Token&  token ) {
		if ( !enabled )
			return;

		if ( printNaturalTokens ) {
			if ( outFile == stdout )
				fprintf(outFile, "\33[45m%s \33[0m", token.name.c_str());
			else
				fprintf(outFile, "%s ", token.name.c_str());
		} else {
			if ( outFile == stdout )
				fprintf(outFile, "\33[45m Type = %u\33 [0m", (UInteger)token.type);
			else
				fprintf(outFile, " Type = %u ", (UInteger)token.type);
		}
	}

};

}
