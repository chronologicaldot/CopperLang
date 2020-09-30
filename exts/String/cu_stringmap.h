// Copyright 2018 Nicolaus Anderson
#ifndef CU_STRING_MAP_H
#define CU_STRING_MAP_H
#include "../../Copper/src/Strings.h"
#include "../../Copper/src/Copper.h"

namespace Cu {
namespace StringLib {

/*
	To add to your copper engine, simply call Map::addToEngine( engine )
	Creates the following function in the Copper engine:

string_map( [string], [function] )
	Calls the given function (map function) for each char/byte in the string.
	The function is passed the char index and the char (as a StringObject/string).
	The parameter charKeepPolicy determines if characters of the original string are deleted based on the true/false return of the map function.
*/

class Map : public ForeignFunc, public Owner {
	Engine&  engine;
	FunctionObject*  mapFunction;
	bool defaultCharKeepPolicy;

	// Private constructor to prevent using a different engine
	Map( Engine&, bool );

	void clearMapFunction();

public:
	virtual Result call( FFIServices& );

	virtual bool owns( FunctionObject* ) const;

	static void addToEngine( Engine&  engine, bool  charKeepPolicy=true ) {
		Map* m = new Map(engine, charKeepPolicy);
		engine.addForeignFunction("string_map", m);
		m->deref();
	}
};

}}

#endif
