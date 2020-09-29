// (c) 2019 Nicolaus Anderson
#ifndef CU_STRING_BASICS_H
#define CU_STRING_BASICS_H

#include "../../Copper/src/Copper.h"

namespace Cu {
namespace StringLib {

namespace Basics {

	void
	addFunctionsToEngine( Engine& );

}

ForeignFunc::Result
Overwrite( FFIServices& );

ForeignFunc::Result
Length( FFIServices& );

ForeignFunc::Result
CharAt( FFIServices& );

}}

#endif
