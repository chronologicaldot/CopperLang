// Copyright 2017 Nicolaus Anderson
#ifndef COPPER_OBJECT_LIST_H
#define COPPER_OBJECT_LIST_H
#include "../ExtTypeValues.h"
#include "../../Copper/src/utilList.h"
#include "../../Copper/src/Copper.h"

// Require that an object type value be set
#ifndef MANAGED_LIST_OBJECT_TYPE_VALUE
0x3
#endif

namespace Cu {
namespace ManagedList {

	// You should really just use me. :)
void addFunctionsToEngine(Engine& engine, bool useShortNames);

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

class Iter : public Object {
	List* list;
	Node* node;

public:
	static const char* StaticTypeName() { return "mgditer"; }

	static ObjectType::Value
	StaticType() {
		return (ObjectType::Value)(MANAGED_LIST_OBJECT_TYPE_VALUE + 1);
	}

	explicit Iter(List* pList, Node* pNode);
	explicit Iter(List* pList, bool top);
	~Iter();

	virtual Object* copy();

	bool has_node();
	bool tied(List*);
	bool go_up();
	bool go_down();
	void get_elem(RefPtr<Object>& result);
	void set_result_to_elem(FFIServices& ffi);
	bool insert_above(Object* pData);
	bool insert_below(Object* pData);
	bool extract();
	bool replace(Object* pData);

	virtual const char* typeName() const {
		return StaticTypeName();
	}
};

class List : public Object {
protected:
	friend Iter;
	Node* bottom;
	Node* top;

public:
	static const char* StaticTypeName() { return "mgdlist"; }

	static ObjectType::Value
	StaticType() {
		return (ObjectType::Value)MANAGED_LIST_OBJECT_TYPE_VALUE;
	}

	List()
		: Object(	StaticType())
		, bottom(	REAL_NULL	)
		, top(		REAL_NULL	)
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
		return StaticTypeName();
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

inline bool getList(
	FFIServices& ffi,
	List*& list
);

inline bool getIter(
	FFIServices& ffi,
	Iter*& iter
);

struct AreList : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

struct Create : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

struct Build : public ForeignFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

void build_appendList( List* outList, List* src ); // used by bool Build::call()

// Abstract
struct OneListFunc : public ForeignFunc {
	virtual const char* getParameterName( unsigned int index ) {
		return List::StaticTypeName();
	}

	virtual unsigned int getParameterCount() {
		return 1;
	}
};

struct Length : public OneListFunc {
	virtual bool call( FFIServices& ffi );
};

struct HasNodes : public OneListFunc {
	virtual bool call( FFIServices& ffi );
};

struct PushTop : public OneListFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

struct PushBottom : public OneListFunc {
	virtual bool call( FFIServices& ffi );

	virtual bool isVariadic() {
		return true;
	}
};

struct PopTop : public OneListFunc {
	virtual bool call( FFIServices& ffi );
};

struct PopBottom : public OneListFunc {
	virtual bool call( FFIServices& ffi );
};

	// TODO - Implement
/*struct Foreach : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};*/

struct Iter_Top : public OneListFunc {
	virtual bool call( FFIServices& ffi );
};

struct Iter_Bottom : public OneListFunc {
	virtual bool call( FFIServices& ffi );
};

struct Iter_Tied : public OneListFunc {
	virtual bool call( FFIServices& ffi );
};


// Abstract
struct OneIterFunc : public ForeignFunc {
	virtual const char* getParameterName( unsigned int index ) {
		return Iter::StaticTypeName();
	}

	virtual unsigned int getParameterCount() {
		return 1;
	}
};

struct Iter_GoUp : public OneIterFunc {
	virtual bool call( FFIServices& ffi );
};

struct Iter_GoDown : public OneIterFunc {
	virtual bool call( FFIServices& ffi );
};

struct Iter_Elem : public OneIterFunc {
	virtual bool call( FFIServices& ffi );
};

struct Iter_InsertAbove : public OneIterFunc {
	virtual bool call( FFIServices& ffi );
};

struct Iter_InsertBelow : public OneIterFunc {
	virtual bool call( FFIServices& ffi );
};

struct Iter_Extract : public OneIterFunc {
	virtual bool call( FFIServices& ffi );
};

struct Iter_Replace : public OneIterFunc {
	virtual bool call( FFIServices& ffi );
};

/*

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
*/

}}

#endif
