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
	bool  usePadding;

	Printer()
		: writeFile(stdout)
		, usePadding(true)
	{}

	void setWriteFile(std::FILE* pFile) {
		if ( pFile ) {
			writeFile = pFile;
		} else {
			writeFile = stdout;
		}
	}

	virtual Result call( FFIServices& ffi ) {
		Object*  arg;
		util::String  out;
		Cu::UInteger  i = 0;
		for (; i < ffi.getArgCount(); ++i) {
			arg = ffi.arg(i);
			switch( arg->getType() )
			{
			case ObjectType::Bool:
				arg->writeToString(out);
				printBooleanString(out);
				break;

			case ObjectType::Numeric:
				switch ( ((NumericObject*)arg)->getSubType() ) {
				case NumericObject::SubType::Integer:
					printInteger( ((NumericObject*)arg)->getIntegerValue() );
					break;
				case NumericObject::SubType::DecimalNum:
				default:
					printDecimalNum( ((NumericObject*)arg)->getDecimalValue() );
					break;
				}
				break;

			default:
				arg->writeToString(out);
				printString(out);
				break;
			}
		}
		return FINISHED;
	}

protected:
	void printBooleanString( const util::String&  value ) {
		if ( writeFile == stdout ) {
			if ( usePadding )
				std::fprintf(writeFile, "\t\33[92m%s\33[0m\n", value.c_str());
			else
				std::fprintf(writeFile, "\33[92m%s\33[0m", value.c_str());
		} else {
			std::fprintf(writeFile, "%s", value.c_str());
		}
	}

	void printInteger( Cu::Integer value ) {
		if ( writeFile == stdout ) {
			if ( usePadding )
				std::fprintf(writeFile, "\t\33[96m%ld\33[0m\n", value);
			else
				std::fprintf(writeFile, "\33[96m%ld\33[0m", value);
		} else {
			std::fprintf(writeFile, "%ld", value);
		}
	}

	void printDecimalNum( Cu::Decimal value ) {
		if ( writeFile == stdout ) {
			if ( usePadding )
				std::fprintf(writeFile, "\t\33[95m%lf\33[0m\n", value);
			else
				std::fprintf(writeFile, "\33[95m%lf\33[0m", value);
		} else {
			std::fprintf(writeFile, "%lf", value);
		}
	}

	void printString( const util::String&  value ) {
		if ( writeFile == stdout ) {
			if ( usePadding )
				std::fprintf(writeFile, "\t\33[93m%s\33[0m\n", value.c_str());
			else
				std::fprintf(writeFile, "\33[93m%s\33[0m", value.c_str());
		} else {
			std::fprintf(writeFile, "%s", value.c_str());
		}
	}
};

}
