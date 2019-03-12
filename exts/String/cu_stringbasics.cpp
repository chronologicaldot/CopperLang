// (c) 2019 Nicolaus Anderson

#include "cu_stringbasics.h"

namespace Cu {
namespace StringLib {

namespace Basics {

	void
	addFunctionsToEngine( Engine&  engine ) {
		addForeignFuncInstance(engine, "str_overwrite", &Overwrite);
		addForeignFuncInstance(engine, "str_byte_length", &Length);
		addForeignFuncInstance(engine, "str_byte_at", &ByteAt);
	}

}

ForeignFunc::Result
Overwrite( FFIServices&  ffi ) {
	if (
		! ffi.demandArgCount(3)
		|| ! ffi.demandArgType(0, ObjectType::String)
		|| ! ffi.demandArgType(1, ObjectType::Numeric)
		|| ! ffi.demandArgType(2, ObjectType::String)
	) {
		return ForeignFunc::NONFATAL;
	}

	String&  baseString = ((StringObject&) ffi.arg(0)).getString();
	Integer  baseIndex = ((NumericObject&) ffi.arg(1)).getIntegerValue();
	String&  topString = ((StringObject&) ffi.arg(2)).getString();

	// Location of the index puts the topString after the baseString's last character
	if ( baseIndex >= baseString.size() ) {
		return ForeignFunc::FINISHED;
	}

	Integer topIndex = 0;

	if ( baseIndex < 0 ) {
		// Location of index is so far back the topString never covers the bottom
		if ( 0 - baseIndex >= static_cast<Integer>(topString.size()) ) {
			return ForeignFunc::FINISHED;
		}

		baseIndex = 0 - baseIndex;
	}

	for (; topIndex < topString.size() && baseIndex < baseString.size(); ++baseIndex, ++topIndex) {
		baseString.set(baseIndex, topString[topIndex]);
	}

	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
Length( FFIServices&  ffi ) {
	if (
		! ffi.demandMinArgCount(1)
		|| ! ffi.demandAllArgsType( ObjectType::String )
	) {
		ffi.setNewResult(new IntegerObject(0));
		return ForeignFunc::NONFATAL;
	}

	Integer  total = 0;
	Integer  i = 0;
	for (; i < ffi.getArgCount(); ++i) {
		total += ((StringObject&)ffi.arg(i)).getString().size();
	}

	ffi.setNewResult(new IntegerObject(total));
	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
ByteAt( FFIServices&  ffi ) {
	if ( ! ffi.demandArgCount(2)
		|| ! ffi.demandArgType(0, ObjectType::String)
		|| ! ffi.demandArgType(1, ObjectType::Numeric)
	) {
		return ForeignFunc::NONFATAL;
	}

	String&  str = ((StringObject&) ffi.arg(0)).getString();
	Integer  idx = ((NumericObject&) ffi.arg(1)).getIntegerValue();

	const char  c = str[idx];
	ffi.setNewResult( new StringObject( util::String(c) ) );
	return ForeignFunc::FINISHED;
}

}}
