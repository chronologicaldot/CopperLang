// Copyright 2017 Nicolaus Anderson
#ifndef CU_EXT_OBJECT_TYPE_VALUES
#define CU_EXT_OBJECT_TYPE_VALUES
/*
	Extensions in Copper must have an associated type value to make them distinguishable.
	Failure to have separate type values will result in casting errors.
	Extensions can have any type number greater than 4 and less than an int max of 0x7fffffff (about 2 billion).
*/

// For basic primitives, the following value is reserved
#define BASIC_PRIMITIVE_OBJECT_TYPE_VALUE 10

// For managed lists, this value and the next integral value after it are reserved
#define MANAGED_LIST_OBJECT_TYPE_VALUE 11

// For millisecond time, the following value is reserved
#define MSTIME_OBJECT_TYPE_VALUE 13

#endif
