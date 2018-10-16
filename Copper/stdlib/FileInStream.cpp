// Copyright 2018 Nicolaus Anderson

#include "FileInStream.h"

namespace Cu {

FileInStream::FileInStream( const char*  filename )
	: rfile(REAL_NULL)
	, atEOF(false)
	, line(1)
	, column(0)
{
	if ( filename ) {
		rfile = fopen(filename, "r");
		if ( !rfile )
			atEOF = true;
	}
}

FileInStream::~FileInStream() {
	if ( rfile ) {
		fclose(rfile);
	}
}

char
FileInStream::getNextByte() {
	if ( ! rfile ) {
		atEOF = true;
		return ' ';
	}
	int c = fgetc(rfile);
	if ( c == EOF  ) {
		atEOF = true;
		return ' ';
	}
	if ( c == '\n' ) {
		++line;
		column = 0;
	}
	++column;
	return char(c);
}

bool
FileInStream::atEOS() {
	return atEOF;
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
