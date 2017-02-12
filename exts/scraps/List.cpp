// Copyright 2017 Nicolaus Anderson
#include "List.h"
#include "../../Copper/src/Strings.h"

namespace Cu {
namespace List {

bool isObjectList(const Object& pObject) {
	return util::equals(pObject.getType(), LIST_TYPENAME);
}

ObjectList::~ObjectList() {
	List<Object*>::Iter iter = list.start();
	if ( list.has() )
	do {
		(*iter)->deref();
	} while ( ++iter );
	// List clearing is called on destruction of list
}

void ObjectList::append(Object* pData) {
	if ( notNull(pData) ) {
		pData->ref();
		list.push_back(pData);
	}
}

void ObjectList::remove(Object* pData) {
	if ( ! list.has() )
		return;
	util::List<Object*>::Iter iter = list.start();
	do {
		if ( *iter == pData ) {
			pData->deref();
			list.remove(iter);
			return;
		}
	} while ( ++iter );
}

void ObjectList::remove(unsigned long index) {
	if ( ! list.has() )
		return;
	util::List<Object*>::Iter iter = list.start();
	unsigned long i = 0;
	for (; i < index; i++) {
		if ( ! ++iter ) // Don't go beyond what the iterator allows
			return;
	}
	(*iter)->deref();
	list.remove(iter);
}

bool ObjectList::itemAt(unsigned long index, RefPtr<Object>& storage) {
	if ( index > list.size() )
		return false;
	storage.set(list[index]);
	return true;
}

unsigned long ObjectList::getSize() {
	return list.size();
}

void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames) {
	simpleFunctionAdd(engine, String("are_list"), areList);
	simpleFunctionAdd(engine, String("List"), create);
	if ( useShortNames ) {
		simpleFunctionAdd(engine, String("Len"), length);
		simpleFunctionAdd(engine, String("Append"), append);
		simpleFunctionAdd(engine, String("Remove"), remove);
	} else {
		simpleFunctionAdd(engine, String("List_len"), length);
		simpleFunctionAdd(engine, String("List_append"), append);
		simpleFunctionAdd(engine, String("List_remove"), remove);
	}
}

bool areList( const List<Object*>& params, RefPtr<Object>& result ) {
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	if ( params.has() )
	do {
		if ( ! isObjectList(**paramsIter) ) {
			result.setWithoutRef(new ObjectBool(false));
		}
	} while ( ++paramsIter );
	result.setWithoutRef(new ObjectBool(true));
	return true;
}

bool create( const List<Object*>& params, RefPtr<Object>& result ) {
	util::List* list = new util::List();
	// Make a list with the given parameters
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	if ( paramsIter.has() )
	do {
		if ( notNull(*paramsIter) ) {
			(*paramsIter)->ref();
			list.push_back( *paramsIter );
		}
	} while ( ++paramsIter );
	result.setWithoutRef(new ObjectList(list));
	return true;
}

bool length( const List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	if ( ! params.has() ) {
		return false;
	}
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	// Only use one parameter
	if ( isObjectList(**paramsIter) ) {
		result.setWithoutRef(new ObjectNumber( ((ObjectList*)(*paramsIter))->getSize() ));
		return true;
	} else {
		if ( notNull(logger) ) {
			logger->print(LogLevel::Warning, "List length function given non-list as parameter.");
		}
	}
	return false;
}

bool append( const List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	if ( ! params.has() )
		return false;
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	if ( ! isObjectList(**paramsIter) ) {
		if ( notNull(logger) ) {
			logger->print(LogLevel::Warning, "List append function not given a list.");
		}
		return false;
	}
	ObjectList* list = (ObjectList*)(*paramsIter);
	while ( ++paramsIter ) {
		(*paramsIter)->ref();
		list->push_back(*paramsIter);
	}
	return false;
}

bool remove( const List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	if ( ! params.has() )
		return false;
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	if ( ! isObjectList(**paramsIter) ) {
		if ( notNull(logger) ) {
			logger->print(LogLevel::Warning, "List remove function not given a list.");
		}
		return false;
	}
	ObjectList* list = (ObjectList*)(*paramsIter);
	unsigned int idx=0;
	while( ++paramsIter ) {
		if ( isObjectNumber(**paramsIter) ) {
			idx = (unsigned long) (*paramsIter)->getRawValue().toUnsignedLong();
			list->list->remove(idx);
		}
	}
	return false;
}

bool item( const List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	if ( ! params.has() )
		return false;
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	if ( ! isObjectList(**paramsIter) ) {
		if ( notNull(logger) ) {
			logger->print(LogLevel::Warning, "List item function not given a list.");
		}
		return false;
	}
	ObjectList* list = (ObjectList*)(*paramsIter);
	// Next parameter needs to be a number
	if ( ! ++paramsIter ) {
		if ( notNull(logger) ) {
			logger->print(LogLevel::Warning, "List item function requires a second parameter.");
		}
		return false;
	}
	if ( !isObjectNumber(**paramsIter) ) {
		if ( notNull(logger) ) {
			logger->print(LogLevel::Warning, "List item function second parameter must be a number.");
		}
		return false;
	}
}

bool foreach( const List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {

}

}}
