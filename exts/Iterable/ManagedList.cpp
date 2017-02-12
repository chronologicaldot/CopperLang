// Copyright 2017 Nicolaus Anderson
#include "../../Copper/src/Strings.h"
#include "ManagedList.h"
#include <climits>

namespace Cu {
namespace ManagedList {

using FFI::simpleFunctionAdd;

bool isList(const Object& pObject) {
	return util::equals(pObject.typeName(), MGD_LIST_TYPENAME);
}

bool isIterator(const Object& pObject) {
	return util::equals(pObject.typeName(), MGD_LIST_ITER_TYPENAME);
}

void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames) {
	simpleFunctionAdd(engine, String("Are_list"), areList);
	simpleFunctionAdd(engine, String("List"), create);
	Foreach* foreach = new Foreach(&engine, &logger);
	if ( useShortNames ) {
		simpleFunctionAdd(engine, String("Build"),			build, &logger);
		simpleFunctionAdd(engine, String("Length"),			length, &logger);
		simpleFunctionAdd(engine, String("Has_nodes"),		has_nodes, &logger);
		simpleFunctionAdd(engine, String("Push_top"),		push_top, &logger);
		simpleFunctionAdd(engine, String("Push_bottom"),	push_bottom, &logger);
		simpleFunctionAdd(engine, String("Pop_top"),		pop_top, &logger);
		simpleFunctionAdd(engine, String("Pop_bottom"),		pop_bottom, &logger);
		engine.addForeignFunction(String("Foreach"),		foreach);
		simpleFunctionAdd(engine, String("Bottom"),			iter_bottom, &logger);
		simpleFunctionAdd(engine, String("Top"),			iter_top, &logger);
		simpleFunctionAdd(engine, String("Are_tied"),		iter_tied, &logger);
		simpleFunctionAdd(engine, String("Go_up"),			iter_go_up, &logger);
		simpleFunctionAdd(engine, String("Go_down"),		iter_go_down, &logger);
		simpleFunctionAdd(engine, String("Elem"),			iter_elem, &logger);
		simpleFunctionAdd(engine, String("Insert_above"),	iter_insert_above, &logger);
		simpleFunctionAdd(engine, String("Insert_below"),	iter_insert_below, &logger);
		simpleFunctionAdd(engine, String("Extract"),		iter_extract, &logger);
		simpleFunctionAdd(engine, String("Replace"),		iter_replace, &logger);
	} else {
		simpleFunctionAdd(engine, String("List_build"),			build, &logger);
		simpleFunctionAdd(engine, String("List_length"),		length, &logger);
		simpleFunctionAdd(engine, String("List_has_nodes"),		has_nodes, &logger);
		simpleFunctionAdd(engine, String("List_push_top"),		push_top, &logger);
		simpleFunctionAdd(engine, String("List_push_bottom"),	push_bottom, &logger);
		simpleFunctionAdd(engine, String("List_pop_top"),		pop_top, &logger);
		simpleFunctionAdd(engine, String("List_pop_bottom"),	pop_bottom, &logger);
		engine.addForeignFunction(String("List_foreach"),		foreach);
		simpleFunctionAdd(engine, String("List_bottom"),		iter_bottom, &logger);
		simpleFunctionAdd(engine, String("List_top"),			iter_top, &logger);
		simpleFunctionAdd(engine, String("List_are_tied"),		iter_tied, &logger);
		simpleFunctionAdd(engine, String("Iter_go_up"),			iter_go_up, &logger);
		simpleFunctionAdd(engine, String("Iter_go_down"),		iter_go_down, &logger);
		simpleFunctionAdd(engine, String("Iter_elem"),			iter_elem, &logger);
		simpleFunctionAdd(engine, String("Iter_insert_above"),	iter_insert_above, &logger);
		simpleFunctionAdd(engine, String("Iter_insert_below"),	iter_insert_below, &logger);
		simpleFunctionAdd(engine, String("Iter_extract"),		iter_extract, &logger);
		simpleFunctionAdd(engine, String("Iter_replace"),		iter_replace, &logger);
	}
	foreach->deref();
}

Node::Node(Object* pElem)
	: elem(pElem)
	, above(REAL_NULL)
	, below(REAL_NULL)
	, inList(true)
{
	if ( !elem )
		throw EmptyNodeException();
	elem->ref();
}

Node::~Node() {
	elem->deref();
}

Object* Node::getElem() {
	return elem;
}

void Node::setElem(Object* pData) {
	if ( pData ) {
		pData->ref();
		elem->deref();
		elem = pData;
	}
}

void Node::delink() {
	Node* oldAbove = above;
	if ( above ) {
		above->below = below;
		above = REAL_NULL;
	}
	if ( below ) {
		below->above = oldAbove;
		below = REAL_NULL;
	}
}

Iter::Iter(List* pList, Node* pNode)
	: list(pList)
	, node(pNode)
{
	if ( list )
		list->ref();
	if ( node )
		node->ref();
}

Iter::Iter(List* pList, bool top)
	: list(pList)
	, node(REAL_NULL)
{
	if ( list )
		list->ref();
	if ( notNull(pList) ) {
		if ( top ) {
			node = pList->top;
		} else {
			node = pList->bottom;
		}
		if ( node )
			node->ref();
	}
}

Iter::~Iter() {
	if ( node )
		node->deref();
	if ( list )
		list->deref();
}

Object* Iter::copy() {
	return new Iter(list, node);
}

bool Iter::has_node() {
	return notNull(node);
}

bool Iter::tied(List* l) {
	return list == l;
}

bool Iter::go_up() {
	if ( isNull(node) ) {
		if ( list->has() ) {
			node = list->bottom;
			if ( notNull(node) )
				node->ref();
			return true;
		}
		return false;
	}
	if ( notNull(node->above) ) {
		node = node->above;
		node->ref();
		if ( isNull( node->below ) ) // Should never throw
			throw DisconnectedListException();
		node->below->deref();
		return true;
	}
	return false;
}

bool Iter::go_down() {
	if ( isNull(node) ) {
		if ( list->has() ) {
			node = list->bottom;
			if ( notNull(node) )
				node->ref();
			return true;
		}
		return false;
	}
	if ( notNull(node->below) ) {
		node = node->below;
		node->ref();
		if ( isNull( node->above ) ) // Should never throw
			throw DisconnectedListException();
		node->above->deref();
		return true;
	}
	return false;
}

void Iter::get_elem(RefPtr<Object>& result) {
	if ( isNull(node) ) {
		result.setWithoutRef(new FunctionContainer());
	} else {
		result.set(node->getElem());
	}
}

bool Iter::insert_above(Object* pData) {
	if ( isNull(node) || isNull(pData) )
		return false;
	Node* nn = new Node(pData);
	nn->below = node;
	if ( notNull( node->above ) ) {
		nn->above = node->above;
		node->above->below = nn;
	} else {
		if ( node->inList ) {
			list->top = nn;
		}
	}
	node->above = nn;
	return true;
}

bool Iter::insert_below(Object* pData) {
	if ( isNull(node) || isNull(pData) )
		return false;
	Node* nn = new Node(pData);
	nn->above = node;
	if ( notNull( node->below ) ) {
		nn->below = node->below;
		node->below->above = nn;
	} else {
		if ( node->inList ) {
			list->bottom = nn;
		}
	}
	node->below = nn;
	return true;
}

bool Iter::extract() {
	if ( isNull(node) )
		return false;
	if ( node->inList ) {
		if ( isNull(list) ) {
			throw DisconnectedListException();
		}
		// Fix the top and bottom first
		if ( list->bottom == node ) {
			list->bottom = node->above; // May be null
		}
		if ( list->top == node ) {
			list->top = node->below; // May be null
		}
		node->delink();
		node->deref(); // Remove the list's reference
		node->inList = false;
	}
	return true;
}

bool Iter::replace(Object* pData) {
	if ( isNull(node) )
		return false;
	node->setElem(pData);
}

List::~List() {
	// In case iterators added nodes to the bottom
	if ( notNull(bottom) ) {
		while( notNull( bottom->below ) ) {
			bottom = bottom->below;
		}
	}
	// Delink and dereference all nodes
	Node* next;
	while ( bottom ) {
		next = bottom->above;
		bottom->delink();
		bottom->deref();
		bottom = next;
	}
}

Object* List::copy() {
	List* list = new List();
	Node* n = bottom;
	while ( n ) {
		list->push_top(n->getElem()->copy());
		n = n->above;
	}
	return list;
}

void List::push_top(Object* pData) {
	if ( isNull(pData) )
		return;
	if ( isNull(top) ) {
		bottom = new Node(pData);
		top = bottom;
	}
	else {
		// In case iterators added nodes to the top
		while( notNull( top->above ) ) {
			top = top->above;
		}
		top->above = new Node(pData);
		top->above->below = top;
		top = top->above;
	}
}

void List::push_bottom(Object* pData) {
	if ( isNull(pData) )
		return;
	if ( isNull(bottom) ) {
		bottom = new Node(pData);
		top = bottom;
	}
	else {
		// In case iterators added nodes to the bottom
		while( notNull( bottom->below ) ) {
			bottom = bottom->below;
		}
		bottom->below = new Node(pData);
		bottom->below->above = bottom;
		bottom = bottom->below;
	}
}

void List::pop_top(RefPtr<Object>& result) {
	if ( isNull(top) ) {
		result.setWithoutRef(new FunctionContainer());
		return;
	}
	// In case iterators added nodes to the top
	while( notNull( top->above ) ) {
		top = top->above;
	}
	// In case iterators added nodes to the bottom
	while( notNull( bottom->below ) ) {
		bottom = bottom->below;
	}
	result.set(top->getElem());
	Node* n = top->below;
	if ( top == bottom ) {
		top->delink();
		top->deref();
		top = REAL_NULL;
		bottom = REAL_NULL;
	}
	else {
		top->delink();
		top->deref();
		top = n;
		if ( !top ) {
			if ( bottom ) {
				// Should print a warning
				throw DisconnectedListException();
			}
		}
	}
}

void List::pop_bottom(RefPtr<Object>& result) {
	if ( isNull(bottom) ) {
		result.setWithoutRef(new FunctionContainer());
		return;
	}
	// In case iterators added nodes to the top
	while( notNull( top->above ) ) {
		top = top->above;
	}
	// In case iterators added nodes to the bottom
	while( notNull( bottom->below ) ) {
		bottom = bottom->below;
	}
	result.set(bottom->getElem());
	Node* n = bottom->above;
	if ( top == bottom ) {
		bottom->delink();
		bottom->deref();
		bottom = REAL_NULL;
		top = REAL_NULL;
	}
	else {
		bottom->delink();
		bottom->deref();
		bottom = n;
		if ( !bottom ) {
			if ( top ) {
				// Should print a warning
				throw DisconnectedListException();
			}
		}
	}
}

unsigned long List::getSize() {
	unsigned long size = 0;
	Node* n = bottom;
	while ( n ) {
		if ( size == ULONG_MAX ) break; // or (unsigned long)-1
		size++;
		n = n->above;
	}
	return size;
}

bool List::has() {
	return notNull(bottom);
}

bool getList( const util::List<Object*>::ConstIter& paramsIter, List*& list, Logger* logger, const char* failMsg ) {
	if ( !paramsIter.has() ) {
		FFI::printWarning(logger, failMsg);
		return false;
	}
	if ( ! isList(**paramsIter) ) {
		FFI::printWarning(logger, failMsg);
		return false;
	}
	list = ((List*)*paramsIter);
	return true;
}

bool getIter( const util::List<Object*>::ConstIter& paramsIter, Iter*& iter, Logger* logger, const char* failMsg ) {
	if ( !paramsIter.has() ) {
		FFI::printWarning(logger, failMsg);
		return false;
	}
	if ( ! isIterator(**paramsIter) ) {
		FFI::printWarning(logger, failMsg);
		return false;
	}
	iter = ((Iter*)*paramsIter);
	return true;
}

bool areList( const util::List<Object*>& params, RefPtr<Object>& result ) {
	if ( ! params.has() ) {
		result.setWithoutRef(new ObjectBool(true));
		return true;
	}
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	bool is = true;
	do {
		is = isList(**paramsIter);
		if ( !is ) break;
	} while (++paramsIter);
	result.setWithoutRef(new ObjectBool(is));
	return true;
}

bool create( const util::List<Object*>& params, RefPtr<Object>& result ) {
	if ( ! params.has() ) {
		result.setWithoutRef(new List());
		return true;
	}
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	List* list = new List();
	do {
		list->push_top(**paramsIter);
	} while (++paramsIter);
	result.setWithoutRef(list);
	return true;
}

bool build( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	if ( ! params.has() ) {
		result.setWithoutRef(new List());
		return true;
	}
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	List* list = new List();
	List* cmpList; // Component list
	do {
		if ( isList(**paramsIter) ) {
			build_appendList(list, (List*)(*paramsIter));
		} else {
			FFI::printWarning(logger, "List build function ignoring non-list param.");
		}
	} while (++paramsIter);
	result.setWithoutRef(list);
	return true;
}

void build_appendList( List* outList, List* src ) {
	if ( ! src->has() )
		return;
	Iter srcIter(src, false);
	RefPtr<Object> obj;
	Object* dataCopy;
	do {
		srcIter.get_elem(obj);
		// Nodes are designed to only contain non-null objects, so this should be safe
		dataCopy = obj.raw()->copy();
		outList->push_top(dataCopy);
		dataCopy->deref();
	} while( srcIter.go_up() );
}

bool length( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	// Only one parameter is used
	if ( ! params.size() == 1 ) {
		FFI::printWarning(logger, "List length function only takes 1 parameter.");
		result.setWithoutRef(new ObjectNumber());
	}
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	if ( isList(**paramsIter) ) {
		result.setWithoutRef(new ObjectNumber( ((List*)*paramsIter)->getSize() ));
	} else {
		FFI::printWarning(logger, "List length function requires list as parameter.");
		result.setWithoutRef(new ObjectNumber());
	}
	return true;
}

bool has_nodes( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	if ( ! params.size() == 1 ) {
		FFI::printWarning(logger, "List has_nodes function only takes 1 parameter.");
		result.setWithoutRef(new ObjectNumber());
	}
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	if ( isList(**paramsIter) ) {
		result.setWithoutRef(new ObjectBool( ((List*)*paramsIter)->has() ));
	} else {
		FFI::printWarning(logger, "List has_nodes function requires list as parameter.");
		result.setWithoutRef(new ObjectBool(false));
	}
	return true;
}

bool push_top( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	List* list;
	if (!getList(paramsIter, list, logger, "List push_top function requires a list as the first parameter."))
	{
		return false;
	}
	while( ++paramsIter ) {
		list->push_top( *paramsIter );
	}
	result.set(list);
	return true;
}

bool push_bottom( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	List* list;
	if (!getList(paramsIter, list, logger, "List push_bottom function requires a list as the first parameter."))
	{
		return false;
	}
	while( ++paramsIter ) {
		list->push_bottom( *paramsIter );
	}
	result.set(list);
	return true;
}

bool pop_top( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	List* list;
	if (!getList(paramsIter, list, logger, "List pop_top function requires a list as the first parameter."))
	{
		return false;
	}
	list->pop_top( result );
	return true;
}

bool pop_bottom( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	List* list;
	if (!getList(paramsIter, list, logger, "List pop_bottom function requires a list as the first parameter."))
	{
		return false;
	}
	list->pop_bottom( result );
	return true;
}

bool Foreach::call( const util::List<Object*>& params, RefPtr<Object>& result ) {
	// Requires a list and an action function
	if ( params.size() != 2 ) {
		FFI::printWarning(logger, "List foreach function requires list and action parameters.");
		return false;
	}
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	List* list;
	if ( ! getList(paramsIter, list, logger, "List foreach function requires list as first parameter.") )
	{
		return false;
	}
	++paramsIter; // Safe because the params size == 2
	if ( ! isObjectFunction(**paramsIter) ) {
		FFI::printWarning(logger, "List foreach function requires function for action.");
		return false;
	}
	// List is empty, so short-circuit here
	if ( ! list->has() ) {
		return false;
	}
	FunctionContainer* fc = ((FunctionContainer*)*paramsIter);
	// In order to protect unowned functions from accidental deletion, simulate storage in a variable
	Variable guard;
	guard.setFunc(fc, true);
	util::List<Object*> foreachParams;
	Iter listContentIter(list, false);
	RefPtr<Object> elem;
	if ( list->has() )
	do {
		listContentIter.get_elem(elem);
		foreachParams.push_back(elem.raw());
		engine->runFunction(fc, foreachParams);
		foreachParams.clear();
	} while ( listContentIter.go_up() );
	return false;
}

bool iter_bottom( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	// Only one parameter is used
	if ( ! params.size() == 1 ) {
		FFI::printWarning(logger, "Iterator function only takes 1 parameter.");
		return false;
	}
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	if ( isList(**paramsIter) ) {
		result.setWithoutRef( new Iter((List*)*paramsIter, false) );
		return true;
	}
	return false;
}

bool iter_top( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	// Only one parameter is used
	if ( ! params.size() == 1 ) {
		FFI::printWarning(logger, "Iterator function only takes 1 parameter.");
		return false;
	}
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	if ( isList(**paramsIter) ) {
		result.setWithoutRef( new Iter((List*)*paramsIter, true) );
		return true;
	}
	return false;
}

bool iter_tied( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	// Requires 2 or more parameters
	if ( params.size() < 2 ) {
		FFI::printWarning(logger, "List/Iter tied function requires 2 or more parameters.");
	}
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	List* list;
	if ( ! getList(paramsIter, list, logger, "List/Iter tied function requires list as first parameter.") )
	{
		result.setWithoutRef(new ObjectBool(false));
		return true;
	}
	bool tied = true;
	Iter* iter;
	while( ++paramsIter ) {
		if ( getIter(paramsIter, iter, logger, "List/Iter tied function was passed non-iter. Skipping...") )
		{
			tied = iter->tied(list);
			if ( !tied ) break;
		}
	}
	result.setWithoutRef(new ObjectBool(tied));
	return true;
}

bool iter_go_up( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	Iter* iter;
	if ( ! getIter(paramsIter, iter, logger, "Iter go_up function requires iterator as first parameter.") )
	{
		result.setWithoutRef(new ObjectBool(false));
		return true;
	}
	bool moved = iter->go_up();
	result.setWithoutRef(new ObjectBool(moved));
	return true;
}

bool iter_go_down( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	Iter* iter;
	if ( ! getIter(paramsIter, iter, logger, "Iter go_down function requires iterator as first parameter.") )
	{
		result.setWithoutRef(new ObjectBool(false));
		return true;
	}
	bool moved = iter->go_down();
	result.setWithoutRef(new ObjectBool(moved));
	return true;
}

bool iter_elem( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	Iter* iter;
	if ( ! getIter(paramsIter, iter, logger, "Iter elem function requires iterator as first parameter.") )
	{
		result.setWithoutRef(new ObjectBool(false));
		return true;
	}
	iter->get_elem(result);
	return true;
}

bool iter_insert_above( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	Iter* iter;
	if ( ! getIter(paramsIter, iter, logger, "Iter insert_above function requires iterator as first parameter.") )
	{
		result.setWithoutRef(new ObjectBool(false));
		return true;
	}
	bool jobdone = true;
	while( ++paramsIter ) {
		jobdone = iter->insert_above(*paramsIter);
	}
	result.setWithoutRef(new ObjectBool(jobdone));
	return true;
}

bool iter_insert_below( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	Iter* iter;
	if (!getIter(paramsIter,iter,logger,"Iter insert_below function requires iterator as first parameter."))
	{
		result.setWithoutRef(new ObjectBool(false));
		return true;
	}
	bool jobdone = true;
	while( ++paramsIter ) {
		jobdone = iter->insert_below(*paramsIter);
	}
	result.setWithoutRef(new ObjectBool(jobdone));
	return true;
}

bool iter_extract( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	Iter* iter;
	if ( ! getIter(paramsIter, iter, logger, "Iter extract function requires iterator as first parameter.") )
	{
		return false;
	}
	// Could be wrapped in an if-statement if that had any meaning
	iter->extract();
	return false;
}

bool iter_replace( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger ) {
	if ( params.size() != 2 ) {
		FFI::printWarning(logger, "Iter replace function requires 2 parameters.");
		return false;
	}
	util::List<Object*>::ConstIter paramsIter = params.constStart();
	Iter* iter;
	if ( ! getIter(paramsIter, iter, logger, "Iter replace function requires iterator as first parameter.") )
	{
		return false;
	}
	++paramsIter;
	iter->replace(*paramsIter);
	return false;
}

}}
