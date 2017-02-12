// Copyright 2017 Nicolaus Anderson
#ifndef COPPER_OBJECT_LIST_H
#define COPPER_OBJECT_LIST_H
#include "../../Copper/src/utilList.h"
#include "../../Copper/src/Copper.h"
#include "../FFIBase.h"

namespace Cu {
namespace ManagedList {

using FFI::Base;

static const char* MGD_LIST_TYPENAME = "mgdlist";
static const char* MGD_LIST_ITER_TYPENAME = "mgditer";

	// You should really just use me. :)
void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames);

bool isIterator(const Object& pObject);
bool isList(const Object& pObject);

// Exceptions
class EmptyNodeException {};
class DisconnectedListException {};

// Predeclaration
class List;

class Node : public Ref {
	Object* elem;

public:
	Node* above;
	Node* below;
	bool inList;
	// Without the inList flag, it would not be possible to tell if the node was in the list,
	// and thus we would not know how to apply proper reference counting to it.

	Node(Object* pElem);
	~Node();

	Object* getElem();
	void setElem(Object* pData);
	void delink();
};

class Iter : public Data {
	List* list;
	Node* node;

public:
	explicit Iter(List* pList, Node* pNode);
	explicit Iter(List* pList, bool top);
	~Iter();

	virtual Object* copy();

	bool has_node();
	bool tied(List*);
	bool go_up();
	bool go_down();
	void get_elem(RefPtr<Object>& result);
	bool insert_above(Object* pData);
	bool insert_below(Object* pData);
	bool extract();
	bool replace(Object* pData);

	virtual const char* typeName() const {
		return MGD_LIST_ITER_TYPENAME;
	}
};

class List : public Data {
protected:
	friend Iter;
	Node* bottom;
	Node* top;

public:
	List()
		: bottom(REAL_NULL)
		, top(REAL_NULL)
	{}

	~List();

	virtual Object* copy();

	void push_top(Object* pData);
	void push_bottom(Object* pData);
	void pop_top(RefPtr<Object>& result);		// always sets the result
	void pop_bottom(RefPtr<Object>& result);	// always sets the result

	bool has();
	unsigned long getSize();

	virtual const char* typeName() const {
		return MGD_LIST_TYPENAME;
	}
};

/*
- Are_list		( [object,..] )
- List			( [item,..] )
- Build			(List [,List,..]) // Creates a new copy of the list by merging the given lists
- Length		(List)
- Has_nodes		(List)
- Push_top		(List, [item,..] )
- Push_bottom	(List, [item,..] )
- Pop_top		(List)
- Pop_bottom	(List)
- Foreach		(List, fn)
- Iter			(List) // Replaced by "Top" and "Bottom".
- Top			(List)
- Bottom		(List)
- Are_tied		(List, Iter)
- Go_up			(Iter)
- Go_down		(Iter)
- Elem			(Iter)
- Insert_above	(Iter, item [,item,..] )
- Insert_below	(Iter, item [,item,..] )
- Extract		(Iter)
- Replace		(Iter, item)

// to do later...
- Item			(List) ... maybe

- Above			(Iter) // Creates a new list from all items at or above the given iterator
- Below			(Iter) // Creates a new list from all items at or below the given iterator

// Items that may not be needed:

# Doesn't work if the data is a function, so you have to store functions as child members. #
Swap = fn(iter_a iter_b) {
	a = Elem(iter_b:)
	Replace(iter_b: Elem(iter_a:))
	Replace(iter_a: a:)
}

Relocate = fn(iter newIter){
	Extract(iter:)
	Replace(newIter: Elem(iter:))
}
*/

inline bool getList( const util::List<Object*>::ConstIter& paramsIter, List*& list, Logger* logger, const char* failMsg );
inline bool getIter( const util::List<Object*>::ConstIter& paramsIter, Iter*& list, Logger* logger, const char* failMsg );

bool areList( const util::List<Object*>& params, RefPtr<Object>& result );
bool create( const util::List<Object*>& params, RefPtr<Object>& result );
bool build( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
void build_appendList( List* outList, List* src ); // used by bool build()
bool length( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool has_nodes( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool push_top( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool push_bottom( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool pop_top( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool pop_bottom( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );

class Foreach : public ForeignFunc {
	Engine* engine;
	Logger* logger;

public:
	Foreach(Engine* pEngine, Logger* pLogger)
		: engine(pEngine)
		, logger(pLogger)
	{}

	virtual bool call( const util::List<Object*>& params, RefPtr<Object>& result );
};

//bool iterator( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool iter_top( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool iter_bottom( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool iter_tied( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool iter_go_up( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool iter_go_down( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool iter_elem( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool iter_insert_above( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool iter_insert_below( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool iter_extract( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );
bool iter_replace( const util::List<Object*>& params, RefPtr<Object>& result, Logger* logger );

}}

#endif
