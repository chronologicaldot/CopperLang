// Copyright 2016-2017 Nicolaus Anderson
#ifndef COPPER_BASIC_PRIMITIVES_TYPE_H
#define COPPER_BASIC_PRIMITIVES_TYPE_H
#include "../../Copper/src/Copper.h"

/*
	To do:
	This class itself should have a "are_primitive()" function.
	Notably, I still want the types in case I really need it for some reason. Leave it alone.
*/

namespace Cu {
namespace Numeric {

static const char* BASIC_PRIMITIVE_TYPENAME = "bp#";

struct BasicPrimitive : public Number {
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

protected:
	Type::Value type;

public:
	BasicPrimitive(Type::Value t)
		: type(t)
	{}

	virtual ~BasicPrimitive() {}

	bool isPrimitiveType(const Type::Value t) const {
		return type == t;
	}

	Type::Value getPrimitiveType() const {
		return type;
	}

	virtual const char* typeName() const {
		return BASIC_PRIMITIVE_TYPENAME;
	}

	static const char* realName() {
		return BASIC_PRIMITIVE_TYPENAME;
	}

	virtual char getAsChar() const {
		return 0;
	}

	virtual int getAsInt() const {
		return 0;
	}

	virtual unsigned int getAsUnsignedInt() const {
		return 0;
	}

	virtual unsigned long getAsUnsignedLong() const {
		return 0;
	}

	virtual float getAsFloat() const {
		return 0;
	}

	virtual double getAsDouble() const {
		return 0;
	}
};

}}

#endif
