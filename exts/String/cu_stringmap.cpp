// Copyright 2018 Nicolaus Anderson
#include "cu_stringmap.h"

namespace Cu {
namespace StringLib {

Map::Map( Engine&  hostEngine )
	: engine(hostEngine)
	, mapFunction(REAL_NULL)
{}

/*void Map::clearArgsList( ArgsList&  argsList ) {
	ArgsIter  ai = argsList.start();
	if ( ai.has() ) do {
		ai.getItem()->deref();
	} while ( ai.next() );
	argsList.clear();
}*/

void Map::clearMapFunction() {
	mapFunction->disown(this);
	mapFunction->deref();
	mapFunction = REAL_NULL;
}

bool Map::call( FFIServices&  ffi ) {
	if (!ffi.demandArgCount(2)
		|| !ffi.demandArgType(0, ObjectType::String)
		|| !ffi.demandArgType(1, ObjectType::Function)
	) {
		return false;
	}

	util::String  srcString = ((StringObject&)ffi.arg(0)).getString();
	mapFunction = (FunctionObject*)&(ffi.arg(1));
	mapFunction->ref();
	mapFunction->own(this);

	util::CharList  rebuild;
	ArgsList  args; // util::List<Object*>
	IntegerObject  indexObject(0);
	StringObject  charObject("");
	args.push_back(indexObject);
	args.push_back(charObject);
	Object*  result;
	bool keep;
	uint  index = 0;

	String  typestr; // for debug

	for (; index < srcString.size(); ++index) {
		indexObject.setValue(index);
		charObject.getString() = srcString[index]; // Should implicitly call const String(char)
		switch ( engine.runFunctionObject(mapFunction, &args) ) {
		case EngineResult::Ok:
			result = engine.getLastObject();
			if ( isBoolObject(*result) ) {
				keep = ((BoolObject*)result)->getValue();
			}
			else {
				// Default to true but warn
				//keep = true;
				keep = false; // for debug - seems to make no difference
				ffi.printWarning("Given map function did not return boolean value.");
				result->writeToString(typestr);
				ffi.printWarning( typestr.c_str() );
			}
			break;

		case EngineResult::Error:
			clearMapFunction();
			ffi.printError("Error during call to given map function.");
			return false;

		default: break;
		}

		if ( keep )
			rebuild.push_back( srcString[index] );
	}
	clearMapFunction();
	String  finalString( rebuild );
	ffi.setNewResult(new StringObject(finalString));
	return true;
}

bool Map::owns( FunctionObject*  obj ) const {
	return notNull(mapFunction) && mapFunction == obj;
}

}}
