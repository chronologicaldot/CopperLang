// Copyright 2016-2017 Nicolaus Anderson
#ifndef COPPER_BASIC_PRIMITIVE_SIZES_H
#define COPPER_BASIC_PRIMITIVE_SIZES_H
#include "../../Copper/src/Copper.h"

namespace Cu {
namespace Numeric {
namespace Sizes {

void addFunctionsToEngine(Engine& engine);

struct GetSizeofInt : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

struct GetSizeofULong : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

struct GetSizeofFloat : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

struct GetSizeofDouble : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

struct GetMaxofInt : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

struct GetMaxofULong : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};
/*
struct GetMaxofFloat : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};

struct GetMaxofDouble : public ForeignFunc {
	virtual bool call( FFIServices& ffi );
};
*/
}}}

#endif
