// Copyright 2018 Nicolaus Anderson

#ifndef _CU_FILE_IN_STREAM_H_
#define _CU_FILE_IN_STREAM_H_

#include <fstream>
#include "../src/Copper.h"

namespace Cu {

//! File-In-Stream
/* A safe, convenient reader for a stream. */
class FileInStream : public ByteStream {
	std::ifstream  stream;
	UInteger  index;
	UInteger  line;
	UInteger  column;

public:
	FileInStream( const char*  filename );
	virtual char getNextByte();
	virtual bool atEOS();
	UInteger getLine();
	UInteger getColumn();
};

}

#endif
