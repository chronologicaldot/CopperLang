// (c) 2019 Nicolaus Anderson
#ifndef CU_STRING_BASICS_H
#define CU_STRING_BASICS_H

#include "../../Copper/src/Copper.h"

namespace Cu {
namespace StringLib {

namespace Basics {
/*
	Creates the following Copper functions in the given Copper Engine:

str_overwrite( [string], [int index], [string other] )
	Overwrites the first string with the second, beginning at index. Negative index is allowed.

str_byte_length( [string] )
	Returns the number of bytes in the given string.

str_char_at( [string], [int index] )
	Returns the byte character at the given index.

str_substr( [string], [int start_index], [string end_index] )
	Returns a copy substring of the given string between start and end indexes.
*/
	void
	addFunctionsToEngine( Engine& );

}

ForeignFunc::Result
Overwrite( FFIServices& );

ForeignFunc::Result
Length( FFIServices& );

ForeignFunc::Result
CharAt( FFIServices& );

ForeignFunc::Result
SubString( FFIServices& );

}}

#endif
