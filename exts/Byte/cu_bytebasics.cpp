// (c) 2020 Nicolaus Anderson

#include "cu_bytebasics.h"

namespace Cu {
namespace ByteLib {

namespace Basics {

	void
	addFunctionsToEngine( Engine&  engine ) {
		addForeignFuncInstance(engine, "byte", &CreateFromString);
		addForeignFuncInstance(engine, "byte_to_str", &ToString);
		addForeignFuncInstance(engine, "byte_set_bit", &SetBit);
		addForeignFuncInstance(engine, "byte_get_bit_as_int", &GetBitAsInteger);
		addForeignFuncInstance(engine, "byte_get_bit_as_str", &GetBitAsString);
		addForeignFuncInstance(engine, "byte_flip_bit", &FlipBit);
		addForeignFuncInstance(engine, "byte_flip", &FlipAllBits);
		addForeignFuncInstance(engine, "byte_clear", &Clear);
	}

} // end namespace Basics

//---------------------------------------------

ByteObject::ByteObject()
	: data(0)
{}

ByteObject::ByteObject( cu_byte value )
	: data(value)
{}

void
ByteObject::writeToString( String& out ) const {
	CharList c;
	Integer i = 0;
	for(; i < 8; ++i)
		c.push_front( '0' + (data & 1<<i) );
	out = c;
}

bool
ByteObject::supportsInterface( ObjectType::Value  typeValue ) const {
	return typeValue == ObjectType::Numeric
		|| typeValue == ByteObject::StaticByteType();
}

void
ByteObject::set( Integer index, bool on ) {
	if ( on )
		data ^= ~(1 << index);
	else
		data |= 1 << index;

	//cu_byte t = 1 << index;
	//data = !( !data | ( on? !t:t) );
}

bool
ByteObject::get( Integer index ) {
	return (data & 1<<index) > 0;
}

void
ByteObject::flipbit( Integer index ) {
	cu_byte mask = 1 << index;
	data &= ~mask;
}

void
ByteObject::flipall() {
	data = ~data;
}

void
ByteObject::setValue( const NumericObject&  other ) {
	if ( other.supportsInterface( ByteObject::StaticByteType() ) ) {
		data = ((ByteObject&)other).data;
	}
}

bool
ByteObject::isEqualTo(const NumericObject&  other ) {
	if ( other.supportsInterface( ByteObject::StaticByteType() ) ) {
		return data == ((ByteObject&)other).data;
	}
	return getIntegerValue() == other.getIntegerValue();
}

bool
ByteObject::isGreaterThan(const NumericObject&  other ) {
	if ( other.supportsInterface( ByteObject::StaticByteType() ) ) {
		return data > ((ByteObject&)other).data;
	}
	return getIntegerValue() > other.getIntegerValue();
}

bool
ByteObject::isGreaterOrEqual(const NumericObject&  other ) {
	if ( other.supportsInterface( ByteObject::StaticByteType() ) ) {
		return data >= ((ByteObject&)other).data;
	}
	return getIntegerValue() >= other.getIntegerValue();
}

NumericObject*
ByteObject::absValue() const {
	return (NumericObject*) new ByteObject(data);
}

NumericObject*
ByteObject::add(const NumericObject&  other ) {
	Integer total = (Integer)data + other.getIntegerValue();
	return (NumericObject*) new ByteObject( (cu_byte)( total & 0xFF ) );
}

NumericObject*
ByteObject::subtract(const NumericObject&  other ) {
	Integer total = (Integer)data - other.getIntegerValue();
	if ( total < 0 )
		return (NumericObject*) new ByteObject( 0 );
	else
		return (NumericObject*) new ByteObject( (cu_byte)( total & 0xFF ) );
}

NumericObject*
ByteObject::multiply(const NumericObject&  other ) {
	Integer total = (Integer)data * other.getIntegerValue();
	if ( total < 0 )
		return (NumericObject*) new ByteObject( 0 );
	else
		return (NumericObject*) new ByteObject( (cu_byte)( total & 0xFF ) );
}

NumericObject*
ByteObject::divide(const NumericObject&  other ) {
	Integer otherValue = other.getIntegerValue();
	if ( otherValue == 0 )
		return (NumericObject*) new ByteObject( 0xFF );

	Integer total = (Integer)data / otherValue;
	if ( total < 0 )
		return (NumericObject*) new ByteObject( 0 );
	else
		return (NumericObject*) new ByteObject( (cu_byte)( total & 0xFF ) );
}

NumericObject*
ByteObject::modulus(const NumericObject&  other ) {
	Integer otherValue = other.getIntegerValue();
	if ( otherValue == 0 )
		return (NumericObject*) new ByteObject( data );

	Integer total = (Integer)data / otherValue;
	return (NumericObject*) new ByteObject( (cu_byte)( total & 0xFF ) );
}


//---------------------------------------------

ForeignFunc::Result
CreateFromString( FFIServices& ffi ) {
	// When interpreting the string: Only zero is zero. Everything else is considered a 1.

	if ( ! ffi.demandArgType(0, ObjectType::String) )
		return ForeignFunc::NONFATAL;

	String value = ((StringObject&)ffi.arg(0)).getString();
	ByteObject::cu_byte b = 0;
	// Byte definitions with less than 8 values set the lower bits.
	Integer index = value.size();
	char c;
	for(; index > 0; --index) {
		c = value[index - 1] - '0';
		// Treat all values > 0 as 1
		c = c > 0? 1 : 0;
		b |= c << (value.size() - index); // (8 - index);
	}

	ffi.setNewResult( new ByteObject( b ) );
	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
ToString( FFIServices& ffi ) {
	if ( ! ffi.demandArgType(0, ByteObject::StaticByteType() ) )
		return ForeignFunc::NONFATAL;

	ByteObject byte = ((ByteObject&)ffi.arg(0));
	CharList sc;
	sc.push_back( byte.get(7) ? '1' : '0' );
	sc.push_back( byte.get(6) ? '1' : '0' );
	sc.push_back( byte.get(5) ? '1' : '0' );
	sc.push_back( byte.get(4) ? '1' : '0' );
	sc.push_back( byte.get(3) ? '1' : '0' );
	sc.push_back( byte.get(2) ? '1' : '0' );
	sc.push_back( byte.get(1) ? '1' : '0' );
	sc.push_back( byte.get(0) ? '1' : '0' );

	ffi.setNewResult( new StringObject( String(sc) ) );
	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
SetBit( FFIServices& ffi ) {
	if( ! ffi.demandArgCount(3)
		|| ! ffi.demandArgType(0, ByteObject::StaticByteType())
		|| ! ffi.demandArgType(1, ObjectType::Numeric)
		|| ! ffi.demandArgType(2, ObjectType::Bool)
	) {
		return ForeignFunc::NONFATAL;
	}

	((ByteObject&)ffi.arg(0)).set( // Byte to set
		((NumericObject&)ffi.arg(1)).getIntegerValue(), // Index of bit
		((BoolObject&)ffi.arg(0)).getValue() // New On/Off value of bit
	);

	return ForeignFunc::FINISHED; // Implicit return of empty function
}

ForeignFunc::Result
GetBitAsInteger( FFIServices& ffi ) {
	if( ! ffi.demandArgCount(2)
		|| ! ffi.demandArgType(0, ByteObject::StaticByteType())
		|| ! ffi.demandArgType(1, ObjectType::Numeric)
	) {
		return ForeignFunc::NONFATAL;
	}

	Integer index = ((NumericObject&)ffi.arg(1)).getIntegerValue();
	if ( ((ByteObject&)ffi.arg(0)).get(index) == 0 )
	{
		ffi.setNewResult( new IntegerObject(0) );
	} else {
		ffi.setNewResult( new IntegerObject(1) );
	}

	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
GetBitAsString( FFIServices& ffi ) {
	if( ! ffi.demandArgCount(2)
		|| ! ffi.demandArgType(0, ByteObject::StaticByteType())
		|| ! ffi.demandArgType(1, ObjectType::Numeric)
	) {
		return ForeignFunc::NONFATAL;
	}

	Integer index = ((NumericObject&)ffi.arg(1)).getIntegerValue();
	if ( ((ByteObject&)ffi.arg(0)).get(index) == 0 )
	{
		ffi.setNewResult( new StringObject("0") );
	} else {
		ffi.setNewResult( new StringObject("1") );
	}

	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
FlipBit( FFIServices& ffi ) {
	if( ! ffi.demandArgCount(2)
		|| ! ffi.demandArgType(0, ByteObject::StaticByteType() )
		|| ! ffi.demandArgType(1, ObjectType::Numeric)
	) {
		return ForeignFunc::NONFATAL;
	}

	Integer index = ((NumericObject&)ffi.arg(1)).getIntegerValue();
	((ByteObject&)ffi.arg(0)).flipbit(index);
	return ForeignFunc::FINISHED; // Implicit return of empty function
}

ForeignFunc::Result
FlipAllBits( FFIServices& ffi ) {
	if( ! ffi.demandArgCount(1)
		|| ! ffi.demandArgType(0, ByteObject::StaticByteType() )
	) {
		return ForeignFunc::NONFATAL;
	}

	((ByteObject&)ffi.arg(0)).flipall();
	return ForeignFunc::FINISHED; // Implicit return of empty function
}

ForeignFunc::Result
Clear( FFIServices& ffi ) {
	if( ! ffi.demandMinArgCount(1)
		|| ! ffi.demandAllArgsType( ByteObject::StaticByteType() )
	) {
		return ForeignFunc::NONFATAL;
	}

	Integer i = 0;
	for (; i < ffi.getArgCount(); ++i) {
		((ByteObject&)ffi.arg(i)).data = 0;
	}
	return ForeignFunc::FINISHED; // Implicit return of empty function
}


} // end namespace ByteLib
} // end namespace Cu
