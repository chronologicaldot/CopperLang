// Copyright 2018 Nicolaus Anderson

#ifndef _CU_STRING_IN_STREAM_H_
#define _CU_STRING_IN_STREAM_H_

#include "../src/Strings.h"
#include "../src/Copper.h"

namespace Cu {

//! String-In-Stream
/* A class for containing a constant string of characters meant to act as code for the engine. */
class StringInStream : public ByteStream {
	const util::String  source;
	UInteger  index;
	UInteger  line;
	UInteger  column;

public:
	StreamInStream( const char* );
	StreamInStream( const util::String& );

	virtual char getNextByte();
	virtual bool atEOS();

	UInteger getLine();
	UInteger getColumn();
};

}

#endif
