#include "../src/Copper.h"
#include <cstdio>

namespace CuStd {

using util::List;
using Cu::RefPtr;
using Cu::ObjectType;
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
		Object*  arg;
		util::String  out;
		while ( ffi.hasMoreArgs() ) {
			arg = ffi.getNextArg();
			switch( arg->getType() )
			{
			case ObjectType::Integer:
				std::fprintf(writeFile, "%ld", arg->getIntegerValue());
				break;

			case ObjectType::Decimal:
				std::fprintf(writeFile, "%lf", arg->getDecimalValue());
				break;

			default:
				arg->writeToString(out);
				std::fprintf(writeFile, "%s", out.c_str());
				break;
			}
		}
		return true;
	}

	virtual bool isVariadic() const {
		return true;
	}
};

}
