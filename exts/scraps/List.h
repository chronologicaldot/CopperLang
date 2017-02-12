// Copyright 2017 Nicolaus Anderson
#ifndef COPPER_OBJECT_LIST_H
#define COPPER_OBJECT_LIST_H
#include "../../Copper/src/utilList.h"
#include "../../Copper/src/Copper.h"
#include "../FFIBase.h"

/*
	ATTENTION!

	This goes into archives in case I decide I really want it, which is possible
	because this one allows for indexing directly.
*/

namespace Cu {
namespace List {

using FFI::Base;

static const char* LIST_TYPENAME = "list";

	// You should really just use me. :)
void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames);

bool isObjectList(const Object& pObject);

class ObjectList : public Object {
	util::List<Object*>* list; // Could be replaced by something better in the future

public:
	ObjectList(util::List<Object*>* unrefedList)
		: list(unrefedList)
	{}

	~ObjectList();

	void append(Object* pData);
	void remove(Object* pData);
	void remove(unsigned long index);
	bool itemAt(unsigned long index, RefPtr<Object>& storage); // Returns true if the index is in range
	void clear();
	unsigned long getSize();
	virtual const char* typeName() const {
		return LIST_TYPENAME;
	}
};

bool areList( const List<Object*>& params, RefPtr<Object>& result );
bool create( const List<Object*>& params, RefPtr<Object>& result );
bool length( const List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool append( const List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool remove( const List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool item( const List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool foreach( const List<Object*>& params, RefPtr<Object>& result, Logger* logger );

// To do:
//bool insert:
//bool relocate: list, index, new index
//bool front: list, splitting index
//bool back: list, splitting index
//bool swap: list, index 1, index 2
//bool bring_to_front
//bool bring_to_back

}}

#endif
