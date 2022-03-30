// (C) 2022 Nicolaus Anderson

#include "cu_stringlistconcat.h"

namespace Cu {
namespace StringLib {

ForeignFunc::Result
StringListConcat::call( FFIServices& ffi )
{
	UInteger argCount = ffi.getArgCount();
	UInteger a = 0;
	Object* arg;
	util::CharList  charList;
	StringObject* stringObject;
	Integer la;
	ListObject* listObject;
	Object* item;

	for (; a < argCount; ++a) {
		arg = & ffi.arg(a);
		if ( arg->getType() == ObjectType::String ) {
			stringObject = (StringObject*)arg;
			charList.append( stringObject->getString() );
		}
		if ( arg->getType() == ObjectType::List ) {
			listObject = (ListObject*)arg;
			for (la=0; la < listObject->size(); ++la) {
				item = listObject->getItem(la);
				if ( item->getType() == ObjectType::String ) {
					stringObject = (StringObject*)item;
					charList.append( stringObject->getString() );
				}
			}
		}
	}
	ffi.setNewResult(new StringObject(String(charList)));
	return ForeignFunc::FINISHED;
}

}
}