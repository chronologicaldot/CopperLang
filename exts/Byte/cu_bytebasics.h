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

	static const ObjectType::Value
	StaticByteType() {
		return static_cast<ObjectType::Value>( CU_BYTE_TYPE );
	}

		// Type specific methods

	void set( Integer index, bool on );
	bool get( Integer index );
	void flipbit( Integer index );
	void flipall();


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

// NOTE: Attempting to create bytes from integers turns out to be more complicated that worth doing.

ForeignFunc::Result
CreateFromString( FFIServices& );

ForeignFunc::Result
ToString( FFIServices& );

ForeignFunc::Result
SetBit( FFIServices& );

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
Clear( FFIServices& );

}}

#endif
