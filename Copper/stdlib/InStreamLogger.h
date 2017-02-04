// Copyright 2016 Nicolaus Anderson

#include "../src/Copper.h"
#include "EngMsgToStr.h"
#include <cstdio>

namespace CuStd {

using Cu::LogLevel;
using Cu::EngineMessage;
using Cu::EngineErrorLevel;

class InStreamLogger : public Cu::Logger, public Cu::ByteStream {

	std::FILE* inFile;
	std::FILE* outFile;
	bool atEOF;
	unsigned long lines;
	unsigned long columns;
	unsigned long prev_lines;
	unsigned long prev_columns;

public:

	bool enabled;
	bool showInfo;
	bool showWarnings;
	bool showErrors;

	InStreamLogger()
		: inFile(stdin)
		, outFile(stdout)
		, atEOF(false)
		, lines(1)
		, columns(0)
		, prev_lines(1)
		, prev_columns(0)
		, enabled(true)
		, showInfo(true)
		, showWarnings(true)
		, showErrors(true)
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
			columns++;
			if ( c == '\n' ) {
				lines++;
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
		std::fprintf(outFile, "%s\n", msg);
	}

	void printWarning(const char* msg) {
		std::fprintf(outFile, "Warning (%lu:%lu): %s\n", prev_lines, prev_columns, msg);
	}

	void printError(const char* msg) {
		std::fprintf(outFile, "Error (%lu:%lu): %s\n", prev_lines, prev_columns, msg);
	}

	void printSystemError(const char* msg) {
		std::fprintf(outFile, "System Error (%lu:%lu): %s\n", prev_lines, prev_columns, msg);
	}

	void printCPPError(const char* msg) {
		std::fprintf(outFile, "C++ Error (%lu:%lu): %s\n", prev_lines, prev_columns, msg);
	}

public:
	virtual void print(const LogLevel::Value& logLevel, const char* msg) {
		if ( !enabled || !willShow(logLevel) ) return;

		fprintf(outFile, "%s\n", msg);
	}

	virtual void print(const LogLevel::Value& logLevel, const EngineMessage::Value& msg) {
		if ( !enabled || !willShow(logLevel) ) return;

		EngineErrorLevel::Value errLevel;
		const char* strMsg = Cu::getStringFromEngineMessage(msg, errLevel);

		switch( logLevel ) {
		case LogLevel::info:
			printInfo(strMsg);
			break;

		case LogLevel::warning:
			printWarning(strMsg);

		case LogLevel::error:
			switch( errLevel ) {
				case EngineErrorLevel::error:
					printError(strMsg);
					break;

				case EngineErrorLevel::system:
					printSystemError(strMsg);
					break;

				case EngineErrorLevel::cpp:
					printCPPError(strMsg);
					break;

				default: break;
			}
			break;

		default: break;
		}
	}

	virtual void printFunctionError(unsigned int functionId, unsigned int tokenIndex, const Cu::TokenType& tokenType) {
		fprintf(outFile, "STACK TRACE: fn( %u ) token( %u ):id(%u)\n", functionId, tokenIndex, (unsigned int)tokenType);
	}

};

}
