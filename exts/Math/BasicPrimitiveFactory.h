// Copyright 2017 Nicolaus Anderson
#ifndef CU_STD_NUMBER_FACTORY_H
#define CU_STD_NUMBER_FACTORY_H
#include "../../Copper/src/Strings.h"
#include "../../Copper/src/Copper.h"

namespace Cu {
namespace Numeric {

using util::String;

struct BasicPrimitiveFactory : public NumberObjectFactory {
	virtual Number* createNumber(const String& pValue);
};

}}

#endif
