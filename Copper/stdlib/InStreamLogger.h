// Copyright 2016 Nicolaus Anderson

#include "../src/Copper.h"
#include "EngMsgToStr.h"
#include <cstdio>
#include <cstring> // For strlen

namespace CuStd {

using Cu::ObjectType;
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
	unsigned nesting;
	bool needInputLine;

public:

	bool enabled;
	bool showInfo;
	bool showWarnings;
	bool showErrors;
	bool printNaturalTokens;
	bool showInputLine;

	InStreamLogger();

	// Stream functions

	void setInputFile(std::FILE* pFile);
	virtual char getNextByte();
	virtual bool atEOS();

	// Logger functions

	void setLogFile(std::FILE* pFile );
	bool willShow(LogLevel::Value logLevel);
	util::String  getObjectTypeNameFromType( ObjectType::Value  type );

	void printInputLine();

protected:
	void printInfo(const char* msg);
	void printWarning(const char* msg);
	void printError(const char* msg);
	void printSystemError(const char* msg);
	void printCPPError(const char* msg);
	void printDebug(const char* msg);

public:
	virtual void print(const LogLevel::Value logLevel, const char* msg);
	virtual void print(const LogLevel::Value logLevel, const EngineMessage::Value msg);
	virtual void print(LogMessage  logMsg);
	virtual void printTaskTrace( Cu::TaskType::Value  taskType, const Cu::String&  taskName, UInteger  taskNumber );
	virtual void printStackTrace( const Cu::String&  frameName, UInteger  frameNumber );
	virtual void printToken( const Cu::Token&  token );

};

}
