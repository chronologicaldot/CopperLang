#include "../src/Copper.h"
#include <cstdio>

namespace CuStd {

using util::List;
using Cu::RefPtr;
using Cu::Object;

/*
A printer class for printing the basics:
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

	virtual bool call(const List<Object*>& params, RefPtr<Object>& result) {
		util::String out;
		List<Object*>::ConstIter paramsIter = params.constStart();
		if ( paramsIter.has() )
		do {
			if ( isNull(*paramsIter) ) {
				//std::fprintf(writeFile, "\nPRINT ERROR: Null parameter.\n");
				continue;
			}
			(*paramsIter)->writeToString(out);
			std::fprintf(writeFile, "%s", out.c_str());
		} while( ++paramsIter );
		return false;
	}
};

}
