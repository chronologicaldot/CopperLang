// Copyright 2018 Nicolaus Anderson

#include "StringInStream.h"

namespace Cu {

StringInStream::StringInStream( const char*  code )
	: source(code)
	, index(0)
	, line(1)
	, column(0)
{}

StringInStream::StringInStream( const util::String&  code )
	: source(code)
	, index(0)
	, line(1)
	, column(0)
{}

char
StringInStream::getNextByte() {
	if ( atEOS() )
		return ' ';

	char c = source[index];
	if ( c == '\n' ) {
		++line;
		column = 0;
	}
	++column;
	++index;
	return c;
}

bool
StringInStream::atEOS() {
	return !( source.size() > 0 && index < source.size());
}

UInteger
StringInStream::getLine() {
	return line;
}

UInteger
StringInStream::getColumn() {
	return column;
}

}
