#include "../src/Copper.h"
#include <cstdio>

namespace CuStd {

using util::List;
using Cu::RefPtr;
using Cu::Object;
using Cu::FFIServices;

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

	virtual bool call( FFIServices& ffi ) {
		util::String out;
		while ( ffi.hasMoreArgs() ) {
			ffi.getNextArg()->writeToString(out);
			std::fprintf(writeFile, "%s", out.c_str());
		}
		return true;
	}

	virtual bool isVariadic() {
		return true;
	}
};

}
