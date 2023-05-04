// (C) 2023 Nicolaus Anderson
#include "cu_stringcasts.h"

namespace Cu {
namespace StringLib {

namespace Casts {
	void addFunctionsToEngine( Engine& engine )
	{
		addForeignFuncInstance(engine, "cast_str_to_int", &ByteStrToInt_convert);
		addForeignFuncInstance(engine, "cast_str_to_dcml", &ByteStrToDcml_convert);
	}
}

Integer convert_bytestr_to_int4( const String& s )
{
	return (Integer)(
		(unsigned char)s[0] << 24
		| (unsigned char)s[1] << 16
		| (unsigned char)s[2] << 8
		| (unsigned char)s[3]
	);
}

ForeignFunc::Result
ByteStrToInt_convert( FFIServices& ffi )
{
	if ( !ffi.demandArgType(0, ObjectType::String) )
	{
		return ForeignFunc::NONFATAL;
	}
	
	const String& str = ((StringObject&)ffi.arg(0)).getString();
	ffi.setNewResult( new IntegerObject( convert_bytestr_to_int4(str)) );
	
	return ForeignFunc::FINISHED;
}

Decimal
convert_bytestr_to_dcml( const String& s )
{
	Decimal dest;
	char* ptr = (char*)&dest;
	Integer bytecount = sizeof(Decimal);
	Integer i=0;
	for (i = 0; i < bytecount && i < s.size(); ++i)
	{
		ptr[i] = s[i];
	}
	return dest;
};

ForeignFunc::Result
ByteStrToDcml_convert( FFIServices& ffi )
{
	if ( !ffi.demandArgType(0, ObjectType::String) )
	{
		return ForeignFunc::NONFATAL;
	}
	
	const String& str = ((StringObject&)ffi.arg(0)).getString();
	ffi.setNewResult( new DecimalNumObject( convert_bytestr_to_dcml(str)) );
	
	return ForeignFunc::FINISHED;
}

}
}