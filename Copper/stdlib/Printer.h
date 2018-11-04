// (C) 2018 Nicolaus Anderson
#include "../src/Copper.h"
#include <cstdio>

namespace CuStd {

using util::List;
using Cu::RefPtr;
using Cu::ObjectType;
using Cu::Object;
using Cu::NumericObject;
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
			writeFile = stdout;
		}
	}

	virtual bool call( FFIServices& ffi ) {
		Object*  arg;
		util::String  out;
		while ( ffi.hasMoreArgs() ) {
			arg = ffi.getNextArg();
			if ( writeFile == stdout ) {
				switch( arg->getType() )
				{
				case ObjectType::Bool:
					arg->writeToString(out);
					std::fprintf(writeFile, "\33[92m%s\33[0m", out.c_str());
					break;

				case ObjectType::Numeric:
					switch ( ((NumericObject*)arg)->getSubType() ) {
					case NumericObject::SubType::Integer:
						std::fprintf(writeFile, "\33[96m%ld\33[0m", ((NumericObject*)arg)->getIntegerValue());
						break;
					case NumericObject::SubType::DecimalNum:
					default:
						std::fprintf(writeFile, "\33[95m%lf\33[0m", ((NumericObject*)arg)->getDecimalValue());
						break;
					}
					break;

				default:
					arg->writeToString(out);
					std::fprintf(writeFile, "\33[93m%s\33[0m", out.c_str());
					break;
				}
			} else {
				switch( arg->getType() )
				{
				case ObjectType::Bool:
					arg->writeToString(out);
					std::fprintf(writeFile, "%s", out.c_str());
					break;

				case ObjectType::Numeric:
					switch ( ((NumericObject*)arg)->getSubType() ) {
					case NumericObject::SubType::Integer:
						std::fprintf(writeFile, "%ld", ((NumericObject*)arg)->getIntegerValue());
						break;
					case NumericObject::SubType::DecimalNum:
					default:
						std::fprintf(writeFile, "%lf", ((NumericObject*)arg)->getDecimalValue());
						break;
					}
					break;

				default:
					arg->writeToString(out);
					std::fprintf(writeFile, "%s", out.c_str());
					break;
				}
			}
		}
		return true;
	}

	virtual bool isVariadic() const {
		return true;
	}
};

}
