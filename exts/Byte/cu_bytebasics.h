// (c) 2020 Nicolaus Anderson
#ifndef CU_BYTE_BASICS_H
#define CU_BYTE_BASICS_H

#include "../../Copper/src/Copper.h"

#ifndef CU_BYTE_TYPE
#define CU_BYTE_TYPE 11
#endif

namespace Cu {
namespace ByteLib {

namespace Basics {
/*
	Creates the following Copper functions in the given Copper Engine:

byte( [string], [int index] )
	Returns a byte constructed from the character at the given index.
	Index defaults to 0.

byte_from_str
	Creates a ByteObject (byte) from a string representing the byte as 1s and 0s.

byte_to_str( [byte] )
	Returns a string with 1s and 0s representing the bits of the given byte.

byte_to_char( [byte] )
	Returns a string of a single character constructed from this byte.

byte_set_bit( [byte], [int n], [bool setting] )
	Sets the given byte's n'th bit to the given setting. true = 1, false = 0.

byte_get_bit_as_int( [byte], [int n] )
	Returns the n'th bit of the given byte as an integer 1 or 0.

byte_get_bit_as_str( [byte], [int n] )
	Returns the n'th bit of the given byte as a string "1" or "0".

byte_flip_bit( [byte], [int n] )
	Flips the n'th bit of the given byte.

byte_flip( [byte] )
	Flips all the bits of the given byte.

byte_and( [byte], [byte], ... )
	Bitwise and's all the given bytes.

byte_or( [byte], [byte], ... )
	Bitwise or's all the given bytes.

byte_xor( [byte], [byte], ... )
	Bitwise xor's all the given bytes.

byte_clear( [byte] )
	Sets all the byte's bits to zero. (Faster than creating a new byte of all zeros.)
*/
	void addFunctionsToEngine( Engine& );
}

struct ByteObject : public NumericObject {

#ifdef MSC_VER
	typedef unsigned __int8  cu_byte;
#else
	typedef unsigned char  cu_byte;
#endif
	cu_byte  data;

	ByteObject();
	ByteObject( cu_byte );

	virtual Object*
	copy() {
		return new ByteObject( data );
	}

	virtual void
	writeToString( String& out ) const;

	static const char*
	StaticTypeName() {
		return "byte";
	}

	virtual const char*
	typeName() const {
		return StaticTypeName();
	}

	virtual bool
	supportsInterface( ObjectType::Value  typeValue ) const;

	static ObjectType::Value
	StaticByteType() {
		return static_cast<ObjectType::Value>( CU_BYTE_TYPE );
	}

		// Type specific methods

	String charValue() const;
	void set( Integer index, bool on );
	bool get( Integer index );
	void flipbit( Integer index );
	void flipall();
	cu_byte operateAnd( ByteObject& );	// &
	cu_byte operateOr( ByteObject& );	// |
	cu_byte operateXor( ByteObject& );	// ^


		// NumericObject methods

	virtual void
	setValue( const NumericObject& );

	virtual Integer
	getIntegerValue() const {
		return (Integer) data;
	}

	virtual Decimal
	getDecimalValue() const {
		return (Decimal) data;
	}

	virtual bool
	isEqualTo(const NumericObject&  other );

	virtual bool
	isGreaterThan(const NumericObject&  other );

	virtual bool
	isGreaterOrEqual(const NumericObject&  other );

	virtual NumericObject*
	absValue() const;

	virtual NumericObject*
	add(const NumericObject&  other );

	virtual NumericObject*
	subtract(const NumericObject&  other );

	virtual NumericObject*
	multiply(const NumericObject&  other );

	virtual NumericObject*
	divide(const NumericObject&  other );

	virtual NumericObject*
	modulus(const NumericObject&  other );

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char*
	getDebugName() const {
		return "Byte";
	}
#endif
};

// NOTE: Attempting to create bytes from integers turns out to be more complicated than worth doing.

ForeignFunc::Result
CreateFromString( FFIServices& );

ForeignFunc::Result
ToString( FFIServices& );

ForeignFunc::Result
SetBit( FFIServices& );

ForeignFunc::Result
ToCharValue( FFIServices& );

ForeignFunc::Result
FromCharValue( FFIServices& );

// Extra: Could add a function that sets a range of bits.
// See: https://www.geeksforgeeks.org/set-bits-given-range-number/

ForeignFunc::Result
GetBitAsInteger( FFIServices& );

ForeignFunc::Result
GetBitAsString( FFIServices& );

ForeignFunc::Result
FlipBit( FFIServices& );

ForeignFunc::Result
FlipAllBits( FFIServices& );

ForeignFunc::Result
OperateAnd( FFIServices& );

ForeignFunc::Result
OperateOr( FFIServices& );

ForeignFunc::Result
OperateXor( FFIServices& );

ForeignFunc::Result
Clear( FFIServices& );

}}

#endif
