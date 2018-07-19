// Copyright 2018 Nicolaus Anderson

#include "FileInStream.h"

namespace Cu {

FileInStream::FileInStream( const char*  filename )
	: stream()
	, index(0)
	, line(1)
	, column(0)
{
	stream.open(filename);
}

char
FileInStream::getNextByte() {
	char c = ' ';
	if ( stream.good() && ! stream.eof() ) {
		c = stream.get();
		if ( c == '\n' ) {
			++line;
			column = 0;
		}
		++index;
		++column;
	}
	return c;
}

bool
FileInStream::atEOS() {
	return stream.eof();
}

FileInStream::getLine() {
	return line;
}

FileInStream::getColumn() {
	return column;
}

}
