// (c) 2020 Nicolaus Anderson

#ifndef CU_ACCESS_HELPER_H
#define CU_ACCESS_HELPER_H

#include <Copper.h>

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
	AccessHelper()
		: scope(REAL_NULL)
	{}

	//! cstor
	AccessHelper( FunctionObject* parent )
		: scope(REAL_NULL)
	{
		set(parent);
	}

	//!
	void set( FunctionObject* parent ) {
		if ( ! parent ) {
			scope = REAL_NULL;
			return;
		}

		Function* f = REAL_NULL;
		if ( parent->getFunction(f) ) {
			scope = &(f->getPersistentScope());
		}
	}

	//!
	FunctionObject* getMember( util::String name, bool createIfNotFound = false ) {
		if ( ! scope ) return REAL_NULL;

		Variable* var;
		if ( createIfNotFound ) {
			if ( scope->getVariable(name, var) ) {
				return var->getRawContainer();
			}
		} else {
			if ( scope->findVariable(name, var) ) {
				return var->getRawContainer();
			}
		}

		return REAL_NULL;
	}

	//!
	Object* getMemberData( util::String name ) {
		FunctionObject* fo = getMember(name);
		Function* f = REAL_NULl;

		if ( fo ) {
			fo->getFunction(f);
			if ( f ) {
				if ( f->constantReturn )
					return f->result.raw();
			}
		}

		return REAL_NULL;
	}

	//!
	Integer getMemberAsInteger( util::String name ) {
		Object* o = getMemberData(name);

		if ( !o ) return 0;

		if ( o->supportsInterface( ObjectType::Numeric ) ) {
			return ((NumericObject*)o)->getIntegerValue();
		}
		return 0;
	}

	//!
	Integer getMemberAsDecimalNum( util::String name ) {
		Object* o = getMemberData(name);

		if ( !o ) return 0;

		if ( o->supportsInterface( ObjectType::Numeric ) ) {
			return ((NumericObject*)o)->getDecimalValue();
		}
		return 0;
	}

	//!
	bool setMemberData( util::String name, Object* data, bool setNewResult=false /*doNotRefData*/ ) {
		FunctionObject* member = getMember(name, true);
		Function* f;

		if ( member->getFunction(f) ) {
			f->constantReturn = true;
			if ( setNewResult ) {
				f->result.setWithoutRef( data );
			} else {
				f->result.set( data );
			}
		}

		return false;
	}

};

} // end namespace

#endif // CUIRR3D_ATTR_HELPER_H
