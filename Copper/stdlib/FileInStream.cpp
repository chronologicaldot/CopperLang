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
	int c = ' ';
	if ( stream.is_open() && stream.good() && ! stream.eof() ) {
		c = stream.get();
		if ( c == '\n' ) {
			++line;
			column = 0;
		}
		++index;
		++column;
	}
	if ( c < 0 ) // End of file
		return ' ';
	return (char)c;
}

bool
FileInStream::atEOS() {
	return stream.eof() || stream.fail();
}

UInteger
FileInStream::getLine() {
	return line;
}

UInteger
FileInStream::getColumn() {
	return column;
}

}
