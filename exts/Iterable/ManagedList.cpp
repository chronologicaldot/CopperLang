// Copyright 2017 Nicolaus Anderson
#include "../../Copper/src/Strings.h"
#include "ManagedList.h"
#include <climits>

namespace Cu {
namespace ManagedList {

bool isList(const Object& pObject) {
	return (pObject.getType() == List::StaticType());
}

bool isIterator(const Object& pObject) {
	return (pObject.getType() == Iter::StaticType());
}

void addFunctionsToEngine(
	Engine& engine,
	bool useShortNames
) {
	addForeignFuncInstance<AreList> 		(engine, "AreList");
	addForeignFuncInstance<Create> 			(engine, "List");

	if ( useShortNames ) {
		addForeignFuncInstance<Build> 			(engine, "Build");
		addForeignFuncInstance<Length> 			(engine, "Length");
		addForeignFuncInstance<HasNodes>		(engine, "Has_nodes");
		addForeignFuncInstance<PushTop>			(engine, "Push_top");
		addForeignFuncInstance<PushBottom>		(engine, "Push_bottom");
		addForeignFuncInstance<PopTop> 			(engine, "Pop_top");
		addForeignFuncInstance<PopBottom>		(engine, "Pop_bottom");
		//addForeignFuncInstance<Foreach>		(engine, "Foreach");
		addForeignFuncInstance<Iter_Top> 		(engine, "Top");
		addForeignFuncInstance<Iter_Bottom>		(engine, "Bottom");
		addForeignFuncInstance<Iter_Tied> 		(engine, "Are_tied");
		addForeignFuncInstance<Iter_GoUp>		(engine, "Go_up");
		addForeignFuncInstance<Iter_GoDown>		(engine, "Go_down");
		addForeignFuncInstance<Iter_Elem> 		(engine, "Elem");
		addForeignFuncInstance<Iter_InsertAbove>(engine, "Insert_above");
		addForeignFuncInstance<Iter_InsertBelow>(engine, "Insert_below");
		addForeignFuncInstance<Iter_Extract> 	(engine, "Extract");
		addForeignFuncInstance<Iter_Replace>	(engine, "Replace");
	} else {
		addForeignFuncInstance<Build> 			(engine, "List_build");
		addForeignFuncInstance<Length> 			(engine, "List_length");
		addForeignFuncInstance<HasNodes>		(engine, "List_has_nodes");
		addForeignFuncInstance<PushTop>			(engine, "List_push_top");
		addForeignFuncInstance<PushBottom>		(engine, "List_push_bottom");
		addForeignFuncInstance<PopTop> 			(engine, "List_pop_top");
		addForeignFuncInstance<PopBottom>		(engine, "List_pop_bottom");
		//addForeignFuncInstance<Foreach>		(engine, "List_foreach");
		addForeignFuncInstance<Iter_Top> 		(engine, "List_top");
		addForeignFuncInstance<Iter_Bottom>		(engine, "List_bottom");
		addForeignFuncInstance<Iter_Tied> 		(engine, "List_are_tied");
		addForeignFuncInstance<Iter_GoUp>		(engine, "List_go_up");
		addForeignFuncInstance<Iter_GoDown>		(engine, "List_go_down");
		addForeignFuncInstance<Iter_Elem> 		(engine, "List_elem");
		addForeignFuncInstance<Iter_InsertAbove>(engine, "List_insert_above");
		addForeignFuncInstance<Iter_InsertBelow>(engine, "List_insert_below");
		addForeignFuncInstance<Iter_Extract> 	(engine, "List_extract");
		addForeignFuncInstance<Iter_Replace>	(engine, "List_replace");
	}
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
	: Object(	StaticType())
	, list(		pList		)
	, node(		pNode		)
{
	if ( list )
		list->ref();
	if ( node )
		node->ref();
}

Iter::Iter(List* pList, bool top)
	: Object(	StaticType())
	, list(		pList		)
	, node(		REAL_NULL	)
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

void Iter::get_elem(
	RefPtr<Object>& result
) {
	if ( isNull(node) ) {
		result.setWithoutRef(new FunctionContainer());
	} else {
		result.set(node->getElem());
	}
}

void Iter::set_result_to_elem(
	FFIServices& ffi
) {
	if ( notNull(node) ) {
		ffi.setResult(node->getElem());
	}
	// else | Result-defaulting-to-empty-function is automatically handled by the engine.
	//Object* result = new FunctionContainer();
	//ffi.setResult(result);
	//result->deref();
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
	return true;
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

bool getList(
	FFIServices& ffi,
	List*& list
) {
	if ( ! ffi.hasMoreArgs() ) {
		return false;
	}
	Object* arg = ffi.getNextArg();
	if ( ! isList(*arg) ) {
		return false;
	}
	list = (List*)arg;
	return true;
}

bool getIter(
	FFIServices& ffi,
	Iter*& iter
) {
	if ( ! ffi.hasMoreArgs() ) {
		return false;
	}
	Object* arg = ffi.getNextArg();
	if ( ! isIterator(*arg) ) {
		return false;
	}
	iter = (Iter*)arg;
	return true;
}

bool
AreList::call(
	FFIServices& ffi
) {
	bool result = true;
	while( ffi.hasMoreArgs() ) {
		result = isList( *(ffi.getNextArg()) );
		if ( !result ) break;
	}
	ObjectBool* out = new ObjectBool(result);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
Create::call(
	FFIServices& ffi
) {
	List* list = new List();
	while( ffi.hasMoreArgs() ) {
		list->push_top(ffi.getNextArg());
	}
	ffi.setResult(list);
	list->deref();
	return true;
}

bool
Build::call(
	FFIServices& ffi
) {
	List* list = new List();
	Object* arg = REAL_NULL;
	while( ffi.hasMoreArgs() ) {
		arg = ffi.getNextArg();
		if ( isList(*arg) ) {
			build_appendList(list, (List*)arg);
		} else {
			ffi.printWarning("List-build function received non-list argument. Ignoring...");
		}
	}
	ffi.setResult(list);
	list->deref();
	return true;
};

void build_appendList(
	List* outList,
	List* src
) {
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

bool
Length::call(
	FFIServices& ffi
) {
	List* list = (List*)ffi.getNextArg();
	ObjectNumber* result = new ObjectNumber( list->getSize() );
	ffi.setResult(result);
	result->deref();
	return true;
}

bool
HasNodes::call(
	FFIServices& ffi
) {
	List* list = (List*)ffi.getNextArg();
	ObjectBool* result = new ObjectBool( list->has() );
	ffi.setResult(result);
	result->deref();
	return true;
}

bool
PushTop::call(
	FFIServices& ffi
) {
	List* list;
	if (!getList(ffi, list))
	{
		ffi.printError("List push_top function requires a list as the first parameter.");
		return false;
	}
	while( ffi.hasMoreArgs() ) {
		list->push_top( ffi.getNextArg() );
	}
	ffi.setResult(list);
	return true;
}

bool
PushBottom::call(
	FFIServices& ffi
) {
	List* list;
	if (!getList(ffi, list))
	{
		ffi.printError("List push_bottom function requires a list as the first parameter.");
		return false;
	}
	while( ffi.hasMoreArgs() ) {
		list->push_top( ffi.getNextArg() );
	}
	ffi.setResult(list);
	return true;
}

bool
PopTop::call(
	FFIServices& ffi
) {
	List* list;
	if (!getList(ffi, list))
	{
		ffi.printError("List pop_top function requires a list as the first parameter.");
		return false;
	}
	RefPtr<Object> result;
	list->pop_top(result);
	ffi.setResult(result.raw());
	return true;
}

bool
PopBottom::call(
	FFIServices& ffi
) {
	List* list;
	if (!getList(ffi, list))
	{
		ffi.printError("List pop_bottom function requires a list as the first parameter.");
		return false;
	}
	RefPtr<Object> result;
	list->pop_bottom(result);
	ffi.setResult(result.raw());
	return true;
}

	// TODO - Implement
/*bool
Foreach::call(
	FFIServices& ffi
) {
};*/

bool
Iter_Top::call(
	FFIServices& ffi
) {
	List* list = (List*)ffi.getNextArg();
	Iter* out = new Iter(list, true);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
Iter_Bottom::call(
	FFIServices& ffi
) {
	List* list = (List*)ffi.getNextArg();
	Iter* out = new Iter(list, false);
	ffi.setResult(out);
	out->deref();
	return true;
}

bool
Iter_Tied::call(
	FFIServices& ffi
) {
	List* list = (List*)ffi.getNextArg();
	bool areTied = true;
	Iter* iter;
	while( ffi.hasMoreArgs() ) {
		if ( getIter(ffi, iter) ) {
			areTied = iter->tied(list);
			if ( !areTied )
				break;
		} else {
			ffi.printError("List/Iter-are-tied function was given non-iterator. Defaulting to false.");
			areTied = false;
			break;
		}
	}
	ObjectBool* result = new ObjectBool(areTied);
	ffi.setResult(result);
	result->deref();
	return true;
}

bool
Iter_GoUp::call(
	FFIServices& ffi
) {
	Iter* iter = (Iter*)ffi.getNextArg();
	bool moved = iter->go_up();
	ObjectBool* result = new ObjectBool(moved);
	ffi.setResult(result);
	result->deref();
	return true;
}

bool
Iter_GoDown::call(
	FFIServices& ffi
) {
	Iter* iter = (Iter*)ffi.getNextArg();
	bool moved = iter->go_down();
	ObjectBool* result = new ObjectBool(moved);
	ffi.setResult(result);
	result->deref();
	return true;
}

bool
Iter_Elem::call(
	FFIServices& ffi
) {
	Iter* iter = (Iter*)ffi.getNextArg();
	iter->set_result_to_elem(ffi);
	return true;
}

bool
Iter_InsertAbove::call(
	FFIServices& ffi
) {
	Iter* iter = (Iter*)ffi.getNextArg();
	bool jobDone = true;
	while ( ffi.hasMoreArgs() ) {
		jobDone = iter->insert_above( ffi.getNextArg() );
		if ( ! jobDone ) break;
	}
	ObjectBool* result = new ObjectBool(jobDone);
	ffi.setResult(result);
	result->deref();
	return true;
}

bool
Iter_InsertBelow::call(
	FFIServices& ffi
) {
	Iter* iter = (Iter*)ffi.getNextArg();
	bool jobDone = true;
	while ( ffi.hasMoreArgs() ) {
		jobDone = iter->insert_below( ffi.getNextArg() );
		if ( ! jobDone ) break;
	}
	ObjectBool* result = new ObjectBool(jobDone);
	ffi.setResult(result);
	result->deref();
	return true;
}

bool
Iter_Extract::call(
	FFIServices& ffi
) {
	Iter* iter = (Iter*)ffi.getNextArg();
	iter->extract();
	return true;
}

bool
Iter_Replace::call(
	FFIServices& ffi
) {
	Iter* iter = (Iter*)ffi.getNextArg();
	if ( ! ffi.hasMoreArgs() ) {
		ffi.printError("Iter-replace function requires two arguments.");
		return false;
	}
	iter->replace( ffi.getNextArg() );
	return true;
}

/*
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
*/

}}
