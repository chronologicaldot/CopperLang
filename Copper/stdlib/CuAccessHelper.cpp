// (c) 2020 Nicolaus Anderson

#include "CuAccessHelper.h"

namespace Cu {

AccessHelper::AccessHelper()
	: scope(REAL_NULL)
{}

AccessHelper::AccessHelper( FunctionObject* parent )
	: scope(REAL_NULL)
{
	set(parent);
}

void
AccessHelper::set( FunctionObject* parent ) {
	if ( ! parent ) {
		scope = REAL_NULL;
		return;
	}

	Function* f = REAL_NULL;
	if ( parent->getFunction(f) ) {
		scope = &(f->getPersistentScope());
	}
}

FunctionObject*
AccessHelper::getMember( util::String name, bool createIfNotFound ) {
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

Object*
AccessHelper::getMemberData( util::String name ) {
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

Integer
AccessHelper::getMemberAsInteger( util::String name ) {
	Object* o = getMemberData(name);

	if ( !o ) return 0;

	if ( o->supportsInterface( ObjectType::Numeric ) ) {
		return ((NumericObject*)o)->getIntegerValue();
	}
	return 0;
}

Integer
AccessHelper::getMemberAsDecimalNum( util::String name ) {
	Object* o = getMemberData(name);

	if ( !o ) return 0;

	if ( o->supportsInterface( ObjectType::Numeric ) ) {
		return ((NumericObject*)o)->getDecimalValue();
	}
	return 0;
}

bool
AccessHelper::setMemberData( util::String name, Object* data, bool setNewResult /*doNotRefData*/ ) {
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

} // end namespace
