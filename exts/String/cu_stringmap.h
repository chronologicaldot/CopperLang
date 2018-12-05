// Copyright 2018 Nicolaus Anderson
#ifndef CU_STRING_MAP_H
#define CU_STRING_MAP_H
#include "../../Copper/src/Strings.h"
#include "../../Copper/src/Copper.h"

namespace Cu {
namespace StringLib {

class Map : public ForeignFunc, public Owner {
	Engine&  engine;
	FunctionObject*  mapFunction;

	// Private constructor to prevent using a different engine
	Map( Engine& );

	//void clearArgsList( ArgsList& );
	void clearMapFunction();

public:
	bool call( FFIServices& );

	virtual bool owns( FunctionObject* ) const;

	static void addToEngine( Engine&  engine ) {
		Map* m = new Map(engine);
		engine.addForeignFunction("string_map", m);
		m->deref();
	}
};

}}

#endif
