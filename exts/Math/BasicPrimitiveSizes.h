// Copyright 2016-2017 Nicolaus Anderson
#ifndef COPPER_BASIC_PRIMITIVE_SIZES_H
#define COPPER_BASIC_PRIMITIVE_SIZES_H
#include "../../Copper/src/Copper.h"

namespace Cu {
namespace Numeric {
namespace Sizes {

void addFunctionsToEngine(Engine& engine, Logger& logger, bool useShortNames);

bool getSizeofInt( const util::List<Object*>& params, RefPtr<Object>& result );
bool getSizeofULong( const util::List<Object*>& params, RefPtr<Object>& result );
bool getSizeofFloat( const util::List<Object*>& params, RefPtr<Object>& result );
bool getSizeofDouble( const util::List<Object*>& params, RefPtr<Object>& result );

bool getMaxofInt( const util::List<Object*>& params, RefPtr<Object>& result );
bool getMaxofULong( const util::List<Object*>& params, RefPtr<Object>& result );
bool getMaxofFloat( const util::List<Object*>& params, RefPtr<Object>& result );
bool getMaxofDouble( const util::List<Object*>& params, RefPtr<Object>& result );

}}}

#endif
