// (c) 2020 Nicolaus Anderson

#ifndef CU_ACCESS_HELPER_H
#define CU_ACCESS_HELPER_H

#include "../src/Copper.h"

namespace Cu {

//! Member-Accessor Helper
/*
	This class is useful in foreign functions that need to access the members of a FunctionObject.
	It takes the pain out of accessing members.
*/
class AccessHelper {
	Scope* scope;

public:

	//! cstor
	AccessHelper();

	//! cstor
	AccessHelper( FunctionObject* parent );

	//!
	void set( FunctionObject* parent );

	//!
	FunctionObject* getMember( util::String name, bool createIfNotFound = false );

	//!
	Object* getMemberData( util::String name );

	//!
	bool getMemberAsBool( util::String name );

	//!
	Integer getMemberAsInteger( util::String name );

	//!
	Integer getMemberAsDecimalNum( util::String name );

	//!
	bool setMemberData( util::String name, Object* data, bool setNewResult=false /*doNotRefData*/ );

};

} // end namespace

#endif // CU_ACCESS_HELPER_H
