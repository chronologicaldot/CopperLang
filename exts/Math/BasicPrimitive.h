// Copyright 2016-2017 Nicolaus Anderson
#ifndef COPPER_BASIC_PRIMITIVES_TYPE_H
#define COPPER_BASIC_PRIMITIVES_TYPE_H
#include "../ExtTypeValues.h"
#include "../../Copper/src/Copper.h"

/*
	To do:
	This class itself should have a "are_primitive()" function.
	Notably, I still want the types in case I really need it for some reason. Leave it alone.
*/

namespace Cu {
namespace Numeric {

struct BasicPrimitive : public Object {
	struct Type {
		enum Value {
			_char,
			_int,
			//_uint, // unimplemented
			_ulong,
			_long,
			_float,
			_double
		};
	};

	static const char*
	StaticTypeName() { return "bp#"; }

	static ObjectType::Value
	StaticType() {
		return (ObjectType::Value)BASIC_PRIMITIVE_OBJECT_TYPE_VALUE;
	}

protected:
	Type::Value primitiveType;

public:
	BasicPrimitive(
		Type::Value			pPrimitiveTypeValue
	)
		: Object(			StaticType()	)
		, primitiveType(	pPrimitiveTypeValue	)
	{}

	virtual ~BasicPrimitive() {}

	bool
	isPrimitiveType(
		const Type::Value t
	) const {
		return primitiveType == t;
	}

	Type::Value
	getPrimitiveType() const {
		return primitiveType;
	}

	virtual const char*
	typeName() const {
		return StaticTypeName();
	}

	virtual char
	getAsChar() const {
		return 0;
	}

	virtual int
	getAsInt() const {
		return 0;
	}

	virtual unsigned int
	getAsUnsignedInt() const {
		return 0;
	}

	virtual unsigned long
	getAsUnsignedLong() const {
		return 0;
	}

	virtual float
	getAsFloat() const {
		return 0;
	}

	virtual double
	getAsDouble() const {
		return 0;
	}
};

inline bool
isBasicPrimitive(
	const Object& pObject
) {
	return pObject.getType() == BasicPrimitive::StaticType();
}

// Meant to be the base for any function with two basic primitive arguments
struct TwoArgBase : public ForeignFunc {

	virtual ObjectType::Value
	getParameterType(
		unsigned int index
	) {
		switch( index ) {
		case 0:
		case 1:
			return BasicPrimitive::StaticType();

		default:
			return ObjectType::Function;
		}
	}

	virtual unsigned int
	getParameterCount() {
		return 2;
	}
};

}}

#endif
