// (C) 2022 Nicolaus Anderson

#ifndef CU_STRING_LIST_CONCAT_H
#define CU_STRING_LIST_CONCAT_H

#include "../../Copper/src/Strings.h"
#include "../../Copper/src/Copper.h"

namespace Cu {
namespace StringLib {

/*
	A foreign function that takes a list of strings and combines them into one string.
*/
class StringListConcat
	: public ForeignFunc
{
	StringListConcat()
	{}

public:

	virtual Result call( FFIServices& );

	static void addToEngine( Engine& engine )
	{
		StringListConcat* s = new StringListConcat();
		engine.addForeignFunction("str_list_concat", s);
		s->deref();
	}
};

}
}

#endif