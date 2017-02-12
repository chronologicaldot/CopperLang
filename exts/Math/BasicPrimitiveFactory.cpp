// Copyright 2017 Nicolaus Anderson
#include "BasicPrimitiveFactory.h"
#include "intmath.h"
#include "doublemath.h"

namespace Cu {
namespace Numeric {

Number* BasicPrimitiveFactory::createNumber(const String& pValue) {
	if ( pValue.contains('.') ) {
		return new Double::Double(pValue);
	} else {
		return new Int::Int(pValue);
	}
}

}}
