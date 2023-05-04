// (C) 2021 Nicolaus Anderson

#ifndef CU_STRING_CASTS_H
#define CU_STRING_CASTS_H
#include "../../Copper/src/Copper.h"

/*
Why we need this:

In many binary files, numbers are stored directly in binary form rather than being converted to strings. However, Copper only loads strings. Thus, to interpret the loaded string as a number, we must cast it.
*/

namespace Cu {
namespace StringLib {

namespace Casts {
/* Creates the following functions and adds them to the engine.

cast_str_to_int()
	Converts the first four bytes of a string to an Integer.

cast_str_to_dcml()
	Converts the bytes of a string to a Decimal Number (Double).
*/
	void addFunctionsToEngine( Engine& );
}

Integer
convert_bytestr_to_int4( const String& s );

ForeignFunc::Result
ByteStrToInt_convert( FFIServices& ); // Uses convert_bytestr_to_int4()

Decimal
convert_bytestr_to_dcml( const String& s );

ForeignFunc::Result
ByteStrToDcml_convert( FFIServices& ); // Uses convert_bytestr_to_dcml()

}
}

#endif // CU_STRING_CASTS_H