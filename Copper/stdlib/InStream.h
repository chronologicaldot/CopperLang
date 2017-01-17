// Copyright 2016 Nicolaus Anderson

#include <cstdio>

namespace CuStd {

class InStream : public ByteStream {
	std::FILE* inFile;
	bool atEOS;

public:
	InStream()
		: inFile(std::stdin)
		, atEOS(false)
	{}

	void setInputFile(std::FILE* pFile) {
		if ( pFile ) {
			inFile = pFile;
			atEOF = false;
		} else {
			inFile = std::stdin;
		}
	}

	virtual char getNextByte() {
		int c = std::fgetc(inFile);
		// I/O validity is only relevant when obtaining data from the file, so set atEOF here.
		if ( c != std::EOF ) {
			columns++;
			if ( c == '\n' ) {
				// Addendum
				if ( inFile == stdin )
					atEOF = true;
			}
			return char(c);
		}
		atEOF = true;
		return ' ';
	}

	virtual bool atEOS() {
		bool val = atEOF;
		if ( inFile == stdin )
			atEOF = false;
		return val;
	}
}

}
