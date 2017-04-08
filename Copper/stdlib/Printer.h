#include "../src/Copper.h"
#include <cstdio>

namespace CuStd {

using util::List;
using Cu::RefPtr;
using Cu::Object;
using Cu::ForeignFunctionInterface;

/*
A printer class for printing the return of writeToString, including:
> function info
> booleans
> strings
> numbers
*/
class Printer : public Cu::ForeignFunc {
	std::FILE* writeFile;

public:
	Printer()
		: writeFile(stdout)
	{}

	void setWriteFile(std::FILE* pFile) {
		if ( pFile ) {
			writeFile = pFile;
		} else {
			writeFile = stderr;
		}
	}

	virtual bool call( ForeignFunctionInterface& ffi ) {
		util::String out;
		while ( ffi.hasMoreParams() ) {
			ffi.getNextParam()->writeToString(out);
			std::fprintf(writeFile, "%s", out.c_str());
		}
		return true;
	}

	virtual bool isVariadic() {
		return true;
	}

	virtual const char* getParameterName( unsigned int index ) {
		return "";
	}

	virtual unsigned int getParameterCount() {
		return 0;
	}
};

}
