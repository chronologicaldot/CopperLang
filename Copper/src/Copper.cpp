// Copyright 2016 Nicolaus Anderson

#include "Copper.h"

#ifdef ENABLE_COPPER_NUMERIC_BOUNDS_CHECKS
#include <limits>
#endif

namespace Cu {

// ******* Constants *******
static const char* CONSTANT_END_MAIN = "end_main";
static const char* CONSTANT_EXIT = "exit";
static const char* CONSTANT_FUNCTION_SELF = "this"; // maybe change to "me"
static const char* CONSTANT_FUNCTION_SUPER = "super";
static const char* CONSTANT_FUNCTION_RETURN = "ret"; // "return" is too long.
static const char* CONSTANT_TOKEN_IF = "if";
static const char* CONSTANT_TOKEN_ELIF = "elif";
static const char* CONSTANT_TOKEN_ELSE = "else";
static const char* CONSTANT_TOKEN_LOOP = "loop";
static const char* CONSTANT_TOKEN_ENDLOOP = "stop";
static const char* CONSTANT_TOKEN_LOOPSKIP = "skip";
static const char* CONSTANT_TOKEN_TRUE = "true";
static const char* CONSTANT_TOKEN_FALSE = "false";
static const char* CONSTANT_TOKEN_OWN = "own";
static const char* CONSTANT_TOKEN_IS_OWNER = "is_owner";
static const char* CONSTANT_TOKEN_IS_PTR = "is_ptr";
// Single character constants
static const char CONSTANT_COMMENT_TOKEN = '#';
static const char CONSTANT_STRING_TOKEN = '"';
static const char CONSTANT_ESCAPE_CHARACTER_TOKEN = '\\';

// *********** OPERATION PROCESSING BASE COMPONENTS **********

OpcodeContainer::OpcodeContainer( Opcode* pCode )
	: code(pCode)
{
	// Unfortunately, making opcodes independent would make the creation of loops messier.
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf(
		"[DEBUG: OpcodeContainer cstor(code): code = %p, type = %u, pCode= %p\n",
		(void*)code, (unsigned)(code->getType()), (void*)pCode);
#endif
	code->ref();
}

OpcodeContainer::OpcodeContainer( const OpcodeContainer& pOther ) // Used only for transmission in lists
	: code( pOther.code )
{
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: OpcodeContainer cstor copy: code = %p\n", (void*)code);
#endif
	code->ref();
}

OpcodeContainer::~OpcodeContainer() {
	code->deref();
}

//const Opcode*
Opcode*
OpcodeContainer::getOp() {
	return code;
}

/*
static OpcodeContainer
OpcodeContainer::indie( Opcode* pCode ) {
	OpcodeContainer out(pCode);
	pCode->deref();
	return out;
}
*/

//--------------

Opcode::Opcode( Opcode::Type pType )
	: type(pType)
	, dtype(ODT_Unset)
	, name()
	, address(REAL_NULL)
{
	switch(type)
	{
	case FuncBuild_execBody:
		dtype = ODT_Body;
		data.body = new Body(); // REQUIRED! (addToken depends on this)
		break;

	case Goto:
	case ConditionalGoto:
		dtype = ODT_CodeIndex;
		data.target = REAL_NULL; // REQUIRED! (copy constructer depends on this)
		break;

	// WARNING: It is assumed that the other values are initialized via setters
	default:
		break;
	}

#ifdef COPPER_DEBUG_ADDRESS
	if ( dtype == ODT_Address )
		address->print();
#endif
}

Opcode::Opcode( Opcode::Type pType, const String&  pStrValue, bool  onAddress )
	: type(pType)
	, dtype(ODT_Unset)
	, name()
	, address(REAL_NULL)
{
	if ( onAddress ) {
		dtype = ODT_Address;
		//address.push_back(pStrValue);
		address = new VarAddress();
		address->push_back(pStrValue);
	} else {
		dtype = ODT_Name;
		name = pStrValue;
	}

#ifdef COPPER_DEBUG_ADDRESS
	if ( dtype == ODT_Address )
		address->print();
#endif
}

//Opcode::Opcode( Opcode::Type pType, const VarAddress&  pAddress )
Opcode::Opcode( Opcode::Type pType, VarAddress*  pAddress )
	: type(pType)
	, dtype(ODT_Address)
	, name()
	, address(pAddress)
{
	address->ref();

#ifdef COPPER_DEBUG_ADDRESS
	if ( dtype == ODT_Address )
		address->print();
#endif
}

Opcode::Opcode( const Opcode& pOther )
	: type( pOther.type )
	, dtype(pOther.dtype)
	, name()
	, address(REAL_NULL)
{
	switch( pOther.dtype )
	{
	case ODT_Name:
		name = pOther.name;
		break;

	case ODT_Address:
		address = pOther.address;
		address->ref();
#ifdef COPPER_DEBUG_ADDRESS
		address->print();
#endif
		break;

	case ODT_Integer:
		data.integer = pOther.data.integer;
		break;

	case ODT_Decimal:
		data.decimal = pOther.data.decimal;
		break;

	case ODT_Body:
		// Copy for bodies not allowed
		throw CopyBodyOpcodeException();

	case ODT_CodeIndex:
		data.target = REAL_NULL;
		if ( data.target )
			data.target = new OpStrandIter(*(pOther.data.target));
		break;

	default:
		throw InvalidOpcodeInit();
	}
}

Opcode::~Opcode() {
	if ( dtype == ODT_Body ) // type == FuncBuild_execBody
		data.body->deref();

	else if ( dtype == ODT_Address )
		address->deref();
}

Opcode::Type
Opcode::getType() const {
	return type;
}

void
Opcode::setType( Opcode::Type pType ) {
	type = pType;
}

void
Opcode::appendAddressData( const String&  pString ) {
	dtype = ODT_Address;
	//address.push_back(pString);
	if ( !address ) {
		address = new VarAddress();
	}
	address->push_back(pString);
#ifdef COPPER_DEBUG_ADDRESS
		address->print();
#endif
}

/*
const VarAddress*
Opcode::getAddressData() const {
	return &address;
}*/

VarAddress*
Opcode::getAddressData() {
	return address;
}

// Used for function parameters and so forth
String
Opcode::getNameData() const {
	return name;
}

void
Opcode::setIntegerData( Integer value ) {
	dtype = ODT_Integer;
	data.integer = value;
}

Integer
Opcode::getIntegerData() const {
	return data.integer;
}

void
Opcode::setDecimalData( Decimal value ) {
	dtype = ODT_Decimal;
	data.decimal = value;
}

Decimal
Opcode::getDecimalData() const {
	return data.decimal;
}

void
Opcode::addToken( const Token& pToken ) {
	//if ( type != FuncBuild_execBody )
	if ( dtype != ODT_Body )
		throw InvalidBodyOpcodeAccess();

	data.body->addToken(pToken);
}

Body*
Opcode::getBody() const {
	if ( dtype != ODT_Body )
		throw InvalidBodyOpcodeAccess();

	return data.body;
}

void
Opcode::setTarget( const OpStrandIter pTarget ) {
	dtype = ODT_CodeIndex;
	data.target = new OpStrandIter(pTarget);
}

OpStrandIter&
Opcode::getOpStrandIter() {
	if ( isNull(data.target) )
		throw NullGotoOpcodeException();
	return *(data.target);
}

//--------------

Body::Body()
	: state(Raw)
	, tokens()
	, codes(new OpStrand())
{}

Body::~Body() {
	codes->deref();
}

void
Body::addToken(const Token& pToken) {
	if ( state == Raw )
		tokens.push_back(pToken);
	else
		throw TokenPushToFinishedBodyException();
}

Body::State
Body::getState() {
	return state;
}

// Attempts to convert code. Returns true if succeeded.
bool
Body::compile(Engine* engine) {
	switch( state ) {
	case Raw:
		return compile_internal(engine);
	case HasErrors:
		return false;
	case Ready:
		return true;
	// to get -Wall to stop griping:
	default:
		return false;
	}
}

OpStrand*
Body::getOpcodeStrand() {
	return codes;
}

bool
Body::isEmpty() const {
	return (! tokens.has()) && ( notNull(codes) ? codes->size() == 0 : true );
}

bool
Body::compile_internal(Engine* engine) {
	engine->printTokens(tokens);

	ParserContext context;
	context.setCodeStrand(codes);
	context.setTokenSource(tokens);
	if ( isNull(engine) )
		return false; //throw 0;
	if ( isEmpty() ) {
		return false;
	}
	tokens.push_back(Token(TT_end_segment)); // Avoid parser complaining about end of stream
	switch( engine->parse(context, true) ) {
	case ParseResult::More: // Wat?
		// Should throw
		tokens.pop();
		return false;
	case ParseResult::Error:
		state = HasErrors;
		tokens.pop();
		return false;
	case ParseResult::Done:
		tokens.clear();
		state = Ready;
		return true;
	// to get -Wall to stop griping:
	default:
		return false;
	}
}

// ******* Function definitions *******

Function::Function()
	: constantReturn(false)
	, body()
	, params()
	, result()
	, persistentScope(new Scope())
{
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Function constructor 1 [%p]\n", (void*)this);
#endif
	body.setWithoutRef(new Body());
}

Function::Function(const Function& pOther)
	: constantReturn( pOther.constantReturn )
	, body(pOther.body)
	, params(pOther.params)
	, result(pOther.result)
	, persistentScope(pOther.persistentScope)
{
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Function constructor 2 (const Function&) [%p]\n", (void*)this);
#endif
	persistentScope->ref();
}

Function::~Function() {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Function::~Function [%p]\n", (void*)this);
#endif
	persistentScope->deref();
}

Function& Function::operator=(const Function& pOther) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Function::operator= [%p]\n", (void*)this);
#endif
	constantReturn = pOther.constantReturn;
	body = pOther.body;
	params = pOther.params;
	result = pOther.result;
	*persistentScope = *(pOther.persistentScope);
}

Scope&
Function::getPersistentScope() {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Function::getPersistentScope [%p]\n", (void*)this);
#endif
	return *persistentScope;
}

void
Function::set( Function& other ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Function::set [%p]\n", (void*)this);
#endif
	constantReturn = other.constantReturn;
	body = other.body;
	params = other.params; // If params is changed to a pointer, this has to be changed to a copy
	*persistentScope = *(other.persistentScope);
	Object* rs;
	if ( other.result.obtain(rs) ) {
		result.setWithoutRef( rs->copy() );
	}
}

void
Function::addParam( const String pName ) {
	params.push_back(pName);
}

//--------------------------------------

FunctionObject::FunctionObject(Function* pFunction, UInteger id)
	: Object(FunctionObject::object_type)
	, funcBox()
	, owner(REAL_NULL)
	, ID(id)
{
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionObject constructor (Function*) [%p]\n", (void*)this);
#endif
	type = ObjectType::Function;
	funcBox.set(pFunction);
}

FunctionObject::FunctionObject()
	: Object(FunctionObject::object_type)
	, funcBox()
	, owner(REAL_NULL)
	, ID(0)
{
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionObject constructor 2 [%p]\n", (void*)this);
#endif
	type = ObjectType::Function;
	funcBox.setWithoutRef(new Function());
}

FunctionObject::FunctionObject(const FunctionObject& pOther)
	: Object(FunctionObject::object_type)
	, funcBox()
	, owner(REAL_NULL)
	, ID(pOther.ID)
{
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionObject constructor 3 (const FunctionObject&) [%p]\n", (void*)this);
#endif
	funcBox.set(pOther.funcBox.raw());
	owner = pOther.owner;
}

FunctionObject::~FunctionObject()
{
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionObject::~FunctionObject [%p]\n", (void*)this);
#endif
}

void FunctionObject::own( Owner* pGrabber ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionObject::own [%p]\n", (void*)this);
#endif
	// Finders keepers
	if ( isNull(owner) )
		owner = pGrabber;
	//ref(); // Called by RefPtr already, so I'm using FuncBoxRefPtr for variables instead.
}

void FunctionObject::disown( Owner* pDropper ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionObject::disown [%p]\n", (void*)this);
#endif
	// Language spec says abandoned functions must be destroyed
	if ( notNull(owner) && owner == pDropper ) {
		funcBox.set(REAL_NULL);
		owner = REAL_NULL;
	}
	//deref(); // Called by RefPtr already, so I'm using FuncBoxRefPtr for variables instead.
}

bool FunctionObject::isOwned() const {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionObject::isOwned [%p]\n", (void*)this);
#endif
	return notNull(owner);
}

bool FunctionObject::isOwner( const Owner* pOwner ) const {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionObject::isOwner [%p]\n", (void*)this);
#endif
	return owner == pOwner;
}

void FunctionObject::changeOwnerTo( Owner* pNewOwner ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionObject::changeOwnerTo [%p]\n", (void*)this);
#endif
	// NOTE: This performs no ref() or deref(). Those should have been performed elsewhere automatically.

	if ( pNewOwner->owns(this) ) {
		owner = pNewOwner;
	} else {
#ifdef COPPER_VAR_LEVEL_MESSAGES
		std::printf("[ERROR: Attempting to change function owner to an owner that doesn't point to this container.\n");
#endif
		throw BadFunctionObjectOwnerException();
	}
}

bool FunctionObject::getFunction( Function*& storage ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionObject::getFunction [%p]\n", (void*)this);
#endif
	return funcBox.obtain(storage);
}

void FunctionObject::setFrom( FunctionObject& pOther ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionObject::setFrom [%p]\n", (void*)this);
#endif
	Function* f;
	Function* of;
	if ( funcBox.obtain(f) ) {
		if ( pOther.funcBox.obtain(of) ) {
			f->set( *of );
		}
	} else {
		throw SettingNullFunctionInContainerException();
	}
}

Object* FunctionObject::copy() {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionObject::copy [%p]\n", (void*)this);
#endif
	Function* f = new Function();
	Function* mine;
	if ( funcBox.obtain(mine) ) {
		f->set( *mine );
	}
	FunctionObject* c = new FunctionObject(f, ID);
	f->deref();
	return c;
}

//--------------------------------------


Variable::Variable()
	: box(new FunctionObject())
{
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Variable constructor [%p]\n", (void*)this);
#endif
	box->own(this);
}

Variable::~Variable() {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Variable::~Variable [%p]\n", (void*)this);
#endif
	box->disown(this);
	box->deref();
}

void
Variable::reset() {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Variable::reset [%p]\n", (void*)this);
#endif
#ifdef COPPER_VAR_STRICT_CHECKS
	if ( isNull(box) )
		throw NullVariableException();
#endif
	box->disown(this);
	box->deref();
	box = new FunctionObject();
	box->own(this);
}

void
Variable::set( Variable* pOther, bool pReuseStorage ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Variable::set [%p]\n", (void*)this);
#endif
	if ( notNull(pOther) )
		setFunc( pOther->box, pReuseStorage );
	else
		throw NullVariableException();
}

void
Variable::setFunc( FunctionObject* pContainer, bool pReuseStorage ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Variable::setFunc [%p]\n", (void*)this);
#endif
	if ( isNull(pContainer) ) {
		throw BadParameterException<Variable>();
	}
#ifdef COPPER_VAR_STRICT_CHECKS
	if ( isNull(box) )
		throw NullVariableException();
#endif

	FunctionObject* fc = REAL_NULL;

	if ( pReuseStorage ) {
		pContainer->ref();
		box->disown(this);
		box->deref();
		box = pContainer;
		box->own(this);
	} else {
		// Data is either not owned or this is supposed to make a copy anyways.
		// Delinks from pointers (which is desired).
		// Copy occurs here in case of assigning box's contents to itself.
		// (See changelog.txt: 2017/1/24 v 0.12)
		fc = (FunctionObject*)(pContainer->copy());
		box->disown(this);
		box->deref();
		box = fc;
		box->own(this);
	}
}

// Used only for data
void
Variable::setFuncReturn( Object* pData, bool pPerformCopy ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Variable::setFuncReturn [%p]\n", (void*)this);
#endif
#ifdef COPPER_VAR_STRICT_CHECKS
	if ( isNull(box) )
		throw NullVariableException();
#endif
	box->disown(this);
	box->deref();

	if ( pPerformCopy ) {
		box = FunctionObject::createInitialized(pData);
	} else {
		box = FunctionObject::createInitializedNoCopy(pData);
	}
	box->own(this);
}

// Used for FuncFound_processRunVariable
Function*
Variable::getFunction(Logger* logger) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Variable::getFunction [%p]\n", (void*)this);
#endif
	Function* f = REAL_NULL;

#ifdef COPPER_VAR_STRICT_CHECKS
	if ( isNull(box) )
		throw NullVariableException();
#endif

	if ( box->getFunction(f) ) {
		return f;
	}
	// else:
	// I need to print the variable name (which is only in the scope now)
	// and it would be good to have all the stack info (which can be obtained otherwise).
	if ( notNull(logger) ) {
		logger->print(LogLevel::warning, EngineMessage::NoFunctionInContainer);
	}
	// Conveniently reset this variable if the function no longer exists.
	// This happens when this variable is a pointer and the variable-it-points-to changes.
	box->disown(this);
	box->deref();
	box = new FunctionObject();
	box->own(this);
	box->getFunction(f);
	return f;
}

bool
Variable::isPointer() const {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Variable::isPointer [%p]\n", (void*)this);
#endif
#ifdef COPPER_VAR_STRICT_CHECKS
	if ( isNull(box) )
		throw NullVariableException();
#endif
	return ! box->isOwner(this);
}

bool
Variable::owns( FunctionObject*  container ) const {
	return notNull(box) && box == container;
}

FunctionObject*
Variable::getRawContainer() {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Variable::getRawContainer [%p]\n", (void*)this);
#endif
#ifdef COPPER_VAR_STRICT_CHECKS
	if ( isNull(box) )
		throw NullVariableException();
#endif
	return box;
}

Variable*
Variable::getCopy() {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Variable::getCopy [%p]\n", (void*)this);
#endif
	Variable* var = new Variable();
	if ( isPointer() ) {
		box->ref();
		var->box = box;
		var->box->own(var);
		return var;
	} // else
	FunctionObject* fc = (FunctionObject*)(box->copy());
	var->box = fc;
	var->box->own(var);
	return var;
}

//--------------------------------------
// Defined here for the sake of the linker

NumericObject::NumericObject()
	: Object(NumericObject::object_type)
{}

NumericObject::~NumericObject()
{}

//--------------------------------------

void
IntegerObject::setValue( const NumericObject&  other ) {
	value = other.getIntegerValue();
}

bool
IntegerObject::isEqualTo( const NumericObject&  other ) {
	return value == other.getIntegerValue();
}

bool
IntegerObject::isGreaterThan( const NumericObject&  other ) {
	return value > other.getIntegerValue();
}

bool
IntegerObject::isGreaterOrEqual( const NumericObject&  other ) {
	return value >= other.getIntegerValue();
}

NumericObject*
IntegerObject::absValue() const {
	return new IntegerObject( value >= 0 ? value : -value );
}

// Thanks to pmg for the bounds tests for integers.
// https://stackoverflow.com/questions/199333/how-to-detect-integer-overflow-in-c-c
// and thanks to Franz D. for observing more detection is needed for multiplication.

NumericObject*
IntegerObject::add( const NumericObject&  other ) {
#ifdef ENABLE_COPPER_NUMERIC_BOUNDS_CHECKS
	Integer oi = other.getIntegerValue();
	// Detect overflow
	if ( oi > 0 && value > std::numeric_limits<Integer>::max() - oi ) {
		return new IntegerObject(std::numeric_limits<Integer>::max());
	}
	// Detect underflow
	if ( oi < 0 && value < std::numeric_limits<Integer>::min() - oi ) {
		return new IntegerObject(std::numeric_limits<Integer>::min());
	}
#endif
	return new IntegerObject(value + other.getIntegerValue());
}

NumericObject*
IntegerObject::subtract( const NumericObject&  other ) {
#ifdef ENABLE_COPPER_NUMERIC_BOUNDS_CHECKS
	Integer oi = other).getIntegerValue();
	// detect overflow
	if ( oi < 0 && value > std::numeric_limits<Integer>::max() + oi ) {
		return new IntegerObject(std::numeric_limits<Integer>::max());
	}
	// detect underflow
	if ( oi > 0 && value < std::numeric_limits<Integer>::min() + oi ) {
		return new IntegerObject(std::numeric_limits<Integer>::min());
	}
#endif
	return new IntegerObject(value - other.getIntegerValue());
}

NumericObject*
IntegerObject::multiply( const NumericObject&  other ) {
#ifdef ENABLE_COPPER_NUMERIC_BOUNDS_CHECKS
	Integer oi = other.getIntegerValue();
	if ( oi < 0 )
		oi = -oi; // Get absolute value
	if ( oi != 0 ) {
		// detect overflow
		if ( value > std::numeric_limits<Integer>::max() / oi ) {
			return new IntegerObject(std::numeric_limits<Integer>::max());
		}
		if ( value < std::numeric_limits<Integer>::min() / oi ) {
			return new IntegerObject(std::numeric_limits<Integer>::min());
		}
	}
#endif
	return new IntegerObject(value * other.getIntegerValue());
}

NumericObject*
IntegerObject::divide( const NumericObject&  other ) {
#ifdef ENABLE_COPPER_NUMERIC_BOUNDS_CHECKS
	Integer oi = other.getIntegerValue();
	if ( oi == 0 ) {
		if ( value > 0 ) {
			return new IntegerObject( std::numeric_limits<Integer>::max() );
		}
		if ( value == 0 ) {
			return new IntegerObject( 0 );
		}
		// else
		return new IntegerObject( std::numeric_limits<Integer>::min() );
	}
#endif
	return new IntegerObject(value / other.getIntegerValue());
}

NumericObject*
IntegerObject::modulus(const NumericObject&  other ) {
	return new IntegerObject(value % other.getIntegerValue());
}

//--------------------------------------

bool
iszero(
	Decimal p
) {
	// Check based on rounding error
	return p < DECIMAL_ROUNDING_ERROR && p > - DECIMAL_ROUNDING_ERROR;
}

void
DecimalNumObject::setValue( const NumericObject&  other ) {
	value = other.getDecimalValue();
}

bool
DecimalNumObject::isEqualTo( const NumericObject&  other ) {
	return value == other.getDecimalValue();
}

bool
DecimalNumObject::isGreaterThan( const NumericObject&  other ) {
	return value > other.getDecimalValue();
}

bool
DecimalNumObject::isGreaterOrEqual( const NumericObject&  other ) {
	return value >= other.getDecimalValue();
}

NumericObject*
DecimalNumObject::absValue() const {
	return new DecimalNumObject( value >= 0 ? value : -value );
}

NumericObject*
DecimalNumObject::add( const NumericObject&  other ) {
	return new DecimalNumObject(value + other.getDecimalValue());
}

NumericObject*
DecimalNumObject::subtract( const NumericObject&  other ) {
	return new DecimalNumObject(value - other.getDecimalValue());
}

NumericObject*
DecimalNumObject::multiply( const NumericObject&  other ) {
	return new DecimalNumObject(value * other.getDecimalValue());
}

NumericObject*
DecimalNumObject::divide( const NumericObject&  other ) {
#ifdef ENABLE_COPPER_NUMERIC_BOUNDS_CHECKS
	Decimal od = other.getDecimalValue();
	if ( iszero(od) ) {
		if ( value > 0 ) {
			return DecimalNumObject( std::numeric_limits<Decimal>::infinity() );
		}
		if ( iszero(value) ) {
			return DecimalNumObject( 0 );
		} else {
			return DecimalNumObject( - std::numeric_limits<Decimal>::infinity() );
		}
	}
#endif
	return new DecimalNumObject(value / other.getDecimalValue());
}

NumericObject*
DecimalNumObject::modulus(const NumericObject&  other ) {
	return new IntegerObject(Integer(value) % other.getIntegerValue());
}

//--------------------------------------

ListObject::ListObject()
	: Object( ListObject::object_type )
	, nodeCount(0)
	, selectorIndex(0)
	, selector()
	, head()
	, tail()
{}

/*
ListObject::ListObject( const ListObject&  pOther )
	: Object( ObjectType::List )
	, nodeCount(0)
	, selectorIndex(0)
	, selector()
	, head()
	, tail()
{	
}
*/

ListObject::~ListObject() {
	clear();
}

Object*
ListObject::copy() {
	ListObject*  outList = new ListObject();
	Node* n = head.node;
	Object* item;
	while( n ) {
		item = n->item->copy();
		outList->push_back( item );
		item->deref(); // After copy, refs==1. After push_back, refs==2. Only 1 is needed.
		n = n->post;
	}
	return outList;
}

Integer
ListObject::size() {
	return nodeCount;
}

bool
ListObject::gotoIndex( Integer  index ) {
	if ( index >= nodeCount )
		return false;
	while ( index < 0 ) {
		index += nodeCount;
	}
	if ( selectorIndex - index > index ) {
		selectorIndex = 0;
		selector.node = head.node;
	}
	else if ( nodeCount - index < index - selectorIndex ) {
		selectorIndex = nodeCount - 1;
		selector.node = tail.node;
	}
	// NOTE: The selectorIndex and the selector.node should not be out of sync
	// but this double-check is necessary to prevent segfaults.
	while ( selectorIndex > index && selector.node->prior ) {
		selector.node = selector.node->prior;
		--selectorIndex;
	}
	while ( selectorIndex < index && selector.node->post ) {
		selector.node = selector.node->post;
		++selectorIndex;
	}
	return true;
}

void
ListObject::clear() {
	if ( nodeCount == 0 )
		return;

	Node* n;

	head.node = REAL_NULL;
	selector.node = REAL_NULL;
	while ( tail.node ){
		n = tail.node;
		tail.node = tail.node->prior;
		n->destroy();
	}
	selectorIndex = 0;
	nodeCount = 0;
}

void
ListObject::append( Object*  pItem ) {
	push_back(pItem);
}

void
ListObject::push_back( Object*  pItem ) {
	if ( isNull(pItem) )
		return;
	if ( nodeCount == 0 ) {
		head.node = new Node(pItem);
		tail.node = head.node;
		selector.node = head.node;
		selectorIndex = 0;
	} else {
		tail.append(pItem);
	}
	++nodeCount;
}

void
ListObject::push_front( Object* pItem ) {
	if ( isNull(pItem) )
		return;
	if ( nodeCount == 0 ) {
		head.node = new Node(pItem);
		tail.node = head.node;
		selector.node = head.node;
		selectorIndex = 0;
	} else {
		head.prepend(pItem);
		++selectorIndex;
	}
	++nodeCount;
}

bool
ListObject::remove( Integer  index ) {
	if ( ! gotoIndex(index) )
		return false;

	Node* n = selector.node;
	if ( nodeCount == 1 ) {
		head.node = REAL_NULL;
		tail.node = REAL_NULL;
		selector.node = REAL_NULL;
	} else {
		if ( head.node == n )
			head.node = head.node->post; // May become null
		if ( tail.node == n )
			tail.node = tail.node->prior; // May become null
		selectorIndex += selector.moveOff();
	}
	n->destroy();
	--nodeCount;
	return true;
}

bool
ListObject::insert( Integer  index, Object*  pItem ) {
	if ( isNull(pItem) ) {
		return false;
	}
	if ( index >= nodeCount ) {
		tail.append(pItem);
		++nodeCount;
		return true;
	}
	if ( index <= 0 ) {
		head.prepend(pItem);
		++selectorIndex;
		++nodeCount;
		return true;
	}
	gotoIndex(index);
	selector.insert(pItem);
	++nodeCount;
	return true;
}

bool
ListObject::swap( Integer  index1, Integer  index2 ) {
	Node* n;
	if ( gotoIndex(index1) ) {
		n = selector.node;
		if ( gotoIndex(index2) ) {
			selector.node->swapItem(*n);
			return true;
		}
	}
	return false;
}

bool
ListObject::replace( Integer  index, Object*  pNewItem ) {
	if ( isNull(pNewItem) ) {
		return false;
	}
	if ( gotoIndex(index) ) {
		selector.node->replace(pNewItem);
		return true;
	}
	return false;
}

Object*
ListObject::getItem( Integer  index ) {
	if ( gotoIndex(index) ) {
		return selector.node->item;
	}
	return REAL_NULL;
}


//--------------------------------------

RefVariableStorage::RefVariableStorage()
	: variable( new Variable() )
{
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: RefVariableStorage cstor 1 [%p]\n", (void*)this);
#endif
}

RefVariableStorage::RefVariableStorage(Variable* refedVariable)
	: variable(REAL_NULL)
{
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: RefVariableStorage cstor 2 (Variable&) [%p]\n", (void*)this);
#endif
	// Copy required in order to keep scopes independent.
	variable = refedVariable->getCopy();
}

RefVariableStorage::RefVariableStorage(RefVariableStorage& pOther)
	: variable(REAL_NULL)
{
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: RefVariableStorage cstor 3 (RefVariableStorage&) [%p]\n", (void*)this);
	std::printf("[DEBUG: arg = [%p]\n", (void*)&pOther);
#endif
	// Share to preserve variable data between copies
	variable = pOther.variable;
	variable->ref();
}

RefVariableStorage::RefVariableStorage(const RefVariableStorage& pOther)
	: variable(REAL_NULL)
{
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: RefVariableStorage cstor 4 (const RefVariableStorage&) [%p]\n", (void*)this);
#endif
	// Used for true copying
	// Copy required in order to keep scopes independent.
	variable = pOther.variable->getCopy();
}

RefVariableStorage::~RefVariableStorage() {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: RefVariableStorage::~RefVariableStorage [%p]\n", (void*)this);
#endif
	variable->deref();
	//variable = 0;
}

RefVariableStorage&  RefVariableStorage::operator= (RefVariableStorage& pOther) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: RefVariableStorage operator= (RefVariableStorage&) [%p]\n", (void*)this);
	std::printf("[DEBUG: arg = [%p]\n", (void*)&pOther);
#endif
	variable = pOther.variable;
	variable->ref();
}

Variable&
RefVariableStorage::getVariable() {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: RefVariableStorage::getVariable [%p]\n", (void*)this);
#endif
	return *variable;
}

RefVariableStorage
RefVariableStorage::copy() {
	return RefVariableStorage(variable);
}

//--------------------------------------

void Scope::copyAsgnFromHashTable( RobinHoodHash<RefVariableStorage>& pTable ) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::copyAsgnFromHashTable\n");
#endif
	CHECK_SCOPE_HASH_NULL(robinHoodTable)

	delete robinHoodTable;
	robinHoodTable = new RobinHoodHash<RefVariableStorage>(pTable.getSize());
	RobinHoodHash<RefVariableStorage>::Bucket*  rawBuckets = pTable.getTablePointer();
	uint i=0;
	// Manually copy to ensure copy of variables, not merely sharing when storage is "copied"
	for (; i < pTable.getSize(); ++i) {
		if ( rawBuckets[i].data ) {
			robinHoodTable->insert(
				rawBuckets[i].data->name,
				((RefVariableStorage&)(rawBuckets[i].data->item)).copy()
			);
		}
	}
}

Scope::Scope( UInteger  pTableSize )
	: robinHoodTable(REAL_NULL)
{
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope constructor 1 (Uinteger) [%p]\n", (void*)this);
#endif
	robinHoodTable = new RobinHoodHash<RefVariableStorage>(pTableSize);
}

Scope::Scope( Scope&  scope )
	: robinHoodTable(REAL_NULL)
{
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope copy constructor (Scope&) [%p]\n", (void*)this);
#endif
	// Maintain connection
	robinHoodTable = new RobinHoodHash<RefVariableStorage>(*(scope.robinHoodTable));
}

Scope::~Scope() {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::~Scope [%p]\n", (void*)this);
#endif
	CHECK_SCOPE_HASH_NULL(robinHoodTable)

	delete robinHoodTable;
}

Scope& Scope::operator = (Scope& pOther) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::operator = [%p]\n", (void*)this);
#endif
	CHECK_SCOPE_HASH_NULL(pOther.robinHoodTable)

	if ( &pOther != this )
		copyAsgnFromHashTable(*(pOther.robinHoodTable));

	return *this;
}

bool Scope::variableExists(const String& pName) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::variableExists , name=%s\n", pName.c_str());
#endif
	// TODO? Streamline?
	// For now...
	Variable* v;
	return findVariable(pName, v);
}

// Adds an empty variable and returns a pointer to it
Variable* Scope::addVariable(const String& pName) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::addVariable , name=%s\n", pName.c_str());
#endif
	CHECK_SCOPE_HASH_NULL(robinHoodTable)

	// Unfortunately, we can't use a local variable for the RefVariableStorage because it dies.
	return &( robinHoodTable->insert(pName)->getVariable() );
}

// Gets the variable, creating it if it does not exist
void Scope::getVariable(const String& pName, Variable*& pVariable) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::getVariable , name=%s\n", pName.c_str());
#endif
	CHECK_SCOPE_HASH_NULL(robinHoodTable)

	RobinHoodHash<RefVariableStorage>::BucketData* data = robinHoodTable->getBucketData(pName);
	if ( !data ) {
		// Create the slot
		// Unfortunately, we can't use a local variable for the RefVariableStorage because it dies.
		pVariable = &( robinHoodTable->insert(pName)->getVariable() );
	} else {
		pVariable = &( data->item.getVariable() );
	}
}

// Looks for the variable. If the function exists, it saves it to storage.
// Returns "true" if the variable was found, false otherwise.
// Shouldn't the string here be const?
bool Scope::findVariable(const String& pName, Variable*& pStorage) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::findVariable , name=%s\n", pName.c_str());
#endif
	CHECK_SCOPE_HASH_NULL(robinHoodTable)

	RobinHoodHash<RefVariableStorage>::BucketData* bucketData;

	bucketData = robinHoodTable->getBucketData(pName);
	if ( bucketData != 0 ) { // Hash table uses 0 instead of null
		pStorage = &( bucketData->item.getVariable() );
		return true;
	}
	pStorage = REAL_NULL;
	return false;
}

// Sets the variable, creating the variable if it does not exist
// The pMakePointer option may be ignored if the FunctionObject is not owned
void Scope::setVariable(const String& pName, Variable* pSourceVariable, bool pReuseStorage) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::setVariable , name=%s\n", pName.c_str());
#endif
	// Obtain variable...
	Variable* var;
	getVariable(pName, var);

	if ( notNull(pSourceVariable) ) {
		var->set(pSourceVariable, pReuseStorage);
	} else {
		// Default
		var->reset();
	}
}

// Sets the variable, creating the variable if it does not exist
void Scope::setVariableFrom(const String& pName, Object* pObject, bool pReuseStorage) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::setVariableFrom , name=%s\n", pName.c_str());
#endif
	// Obtain variable
	Variable* var;
	getVariable(pName, var);

	if ( notNull(pObject) ) {
		switch(pObject->getType()) {
		case ObjectType::Function:
			// Save directly
			var->setFunc( (FunctionObject*)pObject, pReuseStorage );
			break;
		//case ObjectType::Data:
		default:
			// Save as the return of a function
			var->setFuncReturn(pObject);
			break;
		}
	} else {
		var->reset();
	}
}

void Scope::appendNamesByInterface(AppendObjectInterface* aoi) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::appendNamesByInterface\n");
#endif
	CHECK_SCOPE_HASH_NULL(robinHoodTable)
	// This implementation is slow due to the need to copy strings.
	// Saving string addresses is possible but not safe.
	RobinHoodHash<RefVariableStorage>::Bucket* bucket;
	Object* obj;
	uint i=0;
	if ( notNull(robinHoodTable) ) {
		for(; i < robinHoodTable->getSize(); ++i) {
			bucket = robinHoodTable->get(i);
			if ( bucket->data != 0 ) {
				obj = new StringObject(bucket->data->name);
				aoi->append(obj);
				obj->deref();
			}
		}
	}
}

void Scope::copyMembersFrom(Scope& pOther) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::copyMembersFrom\n");
#endif
	// Creating a new scope is required for proper copy construction to delink pointers
	Scope newScope;
	newScope = pOther;
	robinHoodTable->appendCopyOf(*(newScope.robinHoodTable));
}

UInteger Scope::occupancy() {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::occupancy\n");
#endif
	CHECK_SCOPE_HASH_NULL(robinHoodTable)

	return robinHoodTable->getOccupancy();
}

//--------------------------------------

StackFrame::StackFrame( VarAddress* pAddress )
	: parent(REAL_NULL)
	, scope(new Scope(CU_STACK_FRAME_SCOPE_SIZE))
	, address(pAddress)
{
	if ( address )
		address->ref();
}

StackFrame::StackFrame( const StackFrame& pOther )
	: parent(REAL_NULL)
	, scope(REAL_NULL)
	, address(REAL_NULL)
{
	// Copy, don't share
	*scope = *(pOther.scope);
	address = pOther.address;
	if ( address )
		address->ref();
}

StackFrame::~StackFrame() {
	scope->deref();
	if ( address )
		address->deref();
}

void
StackFrame::replaceScope() {
	scope->deref();
	scope = new Scope();
}

Scope&
StackFrame::getScope() {
	return *scope;
}

VarAddress*
StackFrame::getAddress() {
	return address;
}

Stack::Stack()
	: size(1)
	, bottom(REAL_NULL)
	, top(REAL_NULL)
	, globalName(new VarAddress())
{
	globalName->push_back("[global scope]");
	bottom = new StackFrame(globalName);
	top = bottom;
}
/*
Stack::Stack(const Stack& pOther)
	: size(0)
	, bottom(REAL_NULL)
	, top(REAL_NULL)
{
	throw UnimplementedFunction<Stack>();
}
*/
Stack::~Stack() {
	clear();
	globalName->deref();
}

void
Stack::clear() {
	StackFrame* parent;
	while( notNull(top) ) {
		parent = top->parent;
		top->deref();
		top = parent;
		--size;
	}
}

void
Stack::clearNonGlobal() {
	StackFrame* parent;
	while ( top != bottom ) {
		parent = top->parent;
		top->deref();
		top = parent;
		--size;
	}
}

void
Stack::clearGlobal() {
	if ( notNull(bottom) )
		bottom->replaceScope();
}

StackFrame&
Stack::getBottom() { // Global namespace
	if ( size == 0 || isNull(bottom) )
		throw UninitializedStackException();
	return *bottom;
}

StackFrame&
Stack::getTop() {
	if ( size == 0 || isNull(top) )
		throw UninitializedStackException();
	return *top;
}

UInteger
Stack::getCurrLevel() {
	return size;
}

StackPopReturn
Stack::pop() {
	if ( size == 0 )
		return STACK_POP_done;
	StackFrame* parent; // initialized below
	if ( top == bottom ) {
		top->deref();
		bottom = REAL_NULL;
		top = REAL_NULL;
	} else {
		parent = top->parent;
		top->deref();
		top = parent;
	}
	if ( size > 0 ) {
		--size;
	}
	if ( size == 0 )
		return STACK_POP_done;
	// else
	return STACK_POP_more;
}

void
Stack::push( VarAddress*  pAddress ) {
	if ( isNull(bottom) ) {
		bottom = new StackFrame(globalName);
		top = bottom;
		++size;
		return;
	}
	StackFrame* parent = top;
	top = new StackFrame(pAddress);
	top->parent = parent;
	++size;
}

void
Stack::push( StackFrame*  pContext ) {
	if ( pContext == 0 )
		throw UninitializedStackContextException();
	pContext->ref();
	if ( isNull(bottom) ) {
		bottom = pContext;
		top = bottom;
		++size;
		return;
	}
	StackFrame* parent = top;
	top = pContext;
	top->parent = parent;
	++size;
}

void
Stack::print( Logger*  pLogger ) {
	if ( isNull(pLogger) )
		return;

	StackFrame* sf = top;
	UInteger i=size;
	VarAddress* address;
	while ( notNull(sf) ) {
		address = sf->getAddress();
		if ( address ) {
			pLogger->printStackTrace( addressToString(*address), i);
		} else {
			pLogger->printStackTrace( "[Nameless frame]", i);
		}
		sf = sf->parent;
		--i;
	}
}

// ************ PARSE TASKS **********

ParserContext::ParserContext()
	: tokenSource(REAL_NULL)
	, currToken(REAL_NULL)
	, lastUsedToken(REAL_NULL)
	, outputStrand(new OpStrand())
	, taskStack()
{
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: ParserContext constructor, outputStrand = %p\n", (void*)outputStrand);
#endif
}

ParserContext::~ParserContext() {
	if ( notNull(currToken) ) {
		delete currToken;
	}
	if ( notNull(lastUsedToken) ) {
		delete lastUsedToken;
	}
	outputStrand->deref();
}

void
ParserContext::setTokenSource(
	TokenQueue& source
) {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: ParserContext::setTokenSource\n");
#endif
	// Avoid resets
	if ( tokenSource == &source )
		return;
	tokenSource = &source;
	if ( notNull(currToken) ) {
		delete currToken;
	}
	if ( notNull(lastUsedToken) ) {
		delete lastUsedToken;
	}
	lastUsedToken = REAL_NULL;
	if ( source.has() )
		currToken = new TokenQueueIter(source);
}

void
ParserContext::setCodeStrand( OpStrand* strand ) {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: ParserContext::setCodeStrand\n");
#endif
	// Should probably throw if the strand is null
	outputStrand->deref();
	outputStrand = strand;
	outputStrand->ref();
}

bool
ParserContext::isFinished() {
	if ( notNull(lastUsedToken) ) {
		return lastUsedToken->atEnd();
	}
	return (isNull(tokenSource)) ? true : (! tokenSource->has());
}

void
ParserContext::onError() {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: ParserContext::onError\n");
#endif
	// Current activity: "Chew up" the bad tokens.
	if ( notNull(lastUsedToken) ) {
		delete lastUsedToken;
		lastUsedToken = new TokenQueueIter(*currToken);
	}
	// I may experiment with different actions here.
}

// Returns "true" if the move could be made
bool
ParserContext::moveToFirstUnusedToken() {
	if ( notNull(lastUsedToken) ) {
		if ( notNull(currToken) ) {
			delete currToken;
		}
		currToken = new TokenQueueIter(*lastUsedToken);
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
		if ( currToken->next() ) {
			std::printf("[DEBUG: ParserContext::moveToFirstUnusedToken fulfilled\n");
			return true;
		} else {
			return false;
		}
#else
		return currToken->next();
#endif
	}
	// Start from the beginning
	if ( isNull(tokenSource) )
		throw ParserContextEmptySourceException();

	// Apparently, none have been used yet
	if ( isNull( currToken ) ) {
		currToken = new TokenQueueIter(*tokenSource);
	} else {
		currToken->reset();
	}
	return currToken->has();
}

Token
ParserContext::peekAtToken() {
	if ( isNull(currToken) || ! currToken->has() )
		throw ParserContextEmptySourceException();
	return **currToken;
}

bool
ParserContext::moveToNextToken() {
	if ( isNull(currToken) )
		throw ParserContextEmptySourceException();
	if ( ! currToken->has() )
		return false;
	return currToken->next();
}

bool
ParserContext::moveToPreviousToken() {
	if ( isNull(currToken) )
		throw ParserContextEmptySourceException();
	if ( ! currToken->has() )
		return false;
	return currToken->prev();
}

void
ParserContext::commitTokenUsage() {
	if ( notNull(lastUsedToken) ) {
		delete lastUsedToken;
		lastUsedToken = REAL_NULL;
	}
	// Should this be if(notNull(currToken)) ?
	// Seems like it would mess up for clearUsedTokens().
	if ( currToken->has() ) {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
		std::printf("[DEBUG: ParserContext::commitTokenUsage fulfilled\n");
#endif
		lastUsedToken = new TokenQueueIter(*currToken);
	}
}

void
ParserContext::clearUsedTokens() {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: ParserContext::clearUsedTokens\n");
#endif
	if ( notNull(lastUsedToken) ) {
		if ( notNull(tokenSource) ) {
			tokenSource->removeUpTo(*lastUsedToken);
			// Note: The lastUsedToken is NOT removed so that complicated re-init
			// for the context is not needed
		}
	}
}

void
ParserContext::addNewOperation( Opcode* newOp ) {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: ParserContext::addNewOperation, type = %u\n", (newOp? newOp->getType() : 0));
#endif
	if ( isNull(outputStrand) )
		throw EmptyOpstrandException();
	if ( isNull(newOp) )
		throw NullOpcodeException();
	outputStrand->push_back(OpcodeContainer(newOp));
	newOp->deref();
}

void
ParserContext::addOperation( Opcode* op ) {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: ParserContext::addOperation, type = %u\n", (op? op->getType() : 0));
#endif
	if ( isNull(outputStrand) )
		throw EmptyOpstrandException();
	if ( isNull(op) )
		throw NullOpcodeException();
	outputStrand->push_back(OpcodeContainer(op));
}

void
addNewParseTask(
	ParseTaskList&	taskStack,
	ParseTask*		newTask
) {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: addNewParseTask\n");
#endif
	taskStack.push_back(ParseTaskContainer(newTask));
	newTask->deref();
}

/*
FuncFoundTask::FuncFoundTask(
	const VarAddress& pVarAddress
)
	: Task(TaskType::FuncFound)
	, varAddress(pVarAddress)
	, args()
{}
*/

FuncFoundTask::FuncFoundTask(
	VarAddress* pVarAddress
)
	: Task(TaskType::FuncFound)
	, varAddress(pVarAddress)
	, args()
{
	varAddress->ref();
}

/*
FuncFoundTask::FuncFoundTask(
	const FuncFoundTask& pOther
)
	: Task(TaskType::FuncFound)
	, varAddress(pOther.varAddress)
	, args(pOther.args)
{}
*/

FuncFoundTask::FuncFoundTask(
	FuncFoundTask& pOther
)
	: Task(TaskType::FuncFound)
	, varAddress(pOther.varAddress)
	, args(pOther.args)
{
	varAddress->ref();
}

FuncFoundTask::~FuncFoundTask() {
	ArgsIter ai = args.start();
	if ( ai.has() )
	do {
		(*ai)->deref();
	} while ( ai.next() );
	// args.clear(); // implicit
	varAddress->deref();
}

void
FuncFoundTask::addArg(
	Object* a
) {
	if ( notNull(a) ) {
		a->ref();
		args.push_back(a);
	}
}


IfStructureParseTask::IfStructureParseTask(
	OpStrand* strand
)
	: ParseTask(ParseTask::If)
	, firstIter(*strand)
	, openBodies(1) // The first parameter body token is always skipped, so it is counted here
	, conditionalGoto(REAL_NULL)
	, finalGotos()
	, atElse(false)
	, state(Start)
{
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: IfStructureParseTask constructor\n");
#endif
	// This task is created AFTER the terminal opcode has been added, which is the last opcode
	firstIter.makeLast();
}

// Queues a conditional goto meant to be set to the next terminal when added
void
IfStructureParseTask::queueNewConditionalJump(
	Opcode* jump
) {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: IfStructureParseTask::queueNewConditionalJump\n");
#endif
	// Fortunately, only one of these ever shows up at a time
	conditionalGoto = jump;
}

// Queues a new goto meant to be set to the last terminal when added
void
IfStructureParseTask::queueNewFinalJumpCode(
	Opcode* jump
) {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: IfStructureParseTask::queueNewFinalJumpCode\n");
#endif
	finalGotos.push_back(jump);
}

void
IfStructureParseTask::finalizeConditionalGoto(
	ParserContext& context
) {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: IfStructureParseTask::finalizeConditionalGoto\n");
#endif
	context.addNewOperation( new Opcode(Opcode::Terminal) );
	if ( notNull( conditionalGoto ) ) {
		conditionalGoto->setTarget( context.outputStrand->end() );
		conditionalGoto = REAL_NULL;
	} else {
		throw NullIfStructureConditionException();
	}
}

void
IfStructureParseTask::finalizeGotos(
	ParserContext& context
) {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: IfStructureParseTask::finalizeGotos\n");
#endif
	context.addNewOperation( new Opcode(Opcode::Terminal) );
	OpStrandIter lastIter = context.outputStrand->end();
	List<Opcode*>::Iter fg = finalGotos.start();
	if ( fg.has() )
	do {
		(*fg)->setTarget( lastIter );
	} while ( fg.next() );
	finalGotos.clear();
}

LoopStructureParseTask::LoopStructureParseTask(
	OpStrand* strand
)
	: ParseTask( ParseTask::Loop )
	, firstIter(*strand)
	, finalGotos()
	, openBodies(1)
	, state(Start)
{
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: LoopStructureParseTask constructor\n");
#endif
	// This task is created AFTER the terminal opcode has been added, which is the last opcode
	firstIter.makeLast();
}

void
LoopStructureParseTask::setGotoOpcodeToLoopStart(
	Opcode* code
) {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: LoopStructureParseTask::setGotoOpcodeToLoopStart\n");
#endif
	code->setTarget( firstIter );
}

void
LoopStructureParseTask::queueFinalGoto(
	Opcode* code
) {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: LoopStructureParseTask::queueFinalGoto\n");
#endif
	finalGotos.push_back(code);
}

void
LoopStructureParseTask::finalizeGotos(
	OpStrand* strand
) {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: LoopStructureParseTask::finalizeGotos\n");
#endif
	OpStrandIter lastIter = strand->end();
	List<Opcode*>::Iter fg = finalGotos.start();
	if ( fg.has() )
	do {
		(*fg)->setTarget( lastIter );
	} while ( fg.next() );
	finalGotos.clear();
}

// *********** FOREIGN FUNCTION INTERFACE ************

FFIServices::FFIServices(
	Engine&			enginePtr,
	ArgsList&		args,
	String			foreignFuncName
)
	: engine(enginePtr)
	, argsArray(REAL_NULL)
	, numArgs(0)
	, who()
{
	who.steal(foreignFuncName); // Cheat the copy
	ArgsIter  argsIter = args.start();
	numArgs = args.size();
	argsArray = new RefPtr<Object>[numArgs];
	UInteger i = 0;
	for(; i < numArgs; ++i) {
		argsArray[i].set( *argsIter );
		argsIter.next();
	}
}

FFIServices::~FFIServices() {
	if ( argsArray ) {
		delete[] argsArray;
		numArgs = 0;
	}
}

UInteger
FFIServices::getArgCount() const {
	return numArgs;
}

bool
FFIServices::demandArgCount( UInteger  count, bool  imperative ) {
	if ( numArgs == count )
		return true;

	engine.print( LogMessage::create( imperative? LogLevel::error : LogLevel::warning )
		.FunctionName(who)
		.Message( EngineMessage::WrongArgCount )
		.ArgIndex(numArgs)
		.ArgCount(count)
	);

	return false;
}

bool
FFIServices::demandArgCountRange( UInteger  min, UInteger  max, bool  imperative ) {
	if ( numArgs >= min && numArgs <= max )
		return true;

	engine.print( LogMessage::create( imperative? LogLevel::error : LogLevel::warning )
		.FunctionName(who)
		.Message( EngineMessage::WrongArgCount )
		.ArgIndex(numArgs)
		.ArgCount(min)
	);

	return false;
}

bool
FFIServices::demandMinArgCount( UInteger  min, bool  imperative ) {
	if ( numArgs >= min )
		return true;

	engine.print( LogMessage::create( imperative? LogLevel::error : LogLevel::warning )
		.FunctionName(who)
		.Message( EngineMessage::WrongArgCount )
		.ArgIndex(numArgs)
		.ArgCount(min)
	);

	return false;
}

bool
FFIServices::demandArgType( UInteger  index, ObjectType::Value  type ) {
	if ( index >= numArgs ) {
		engine.print( LogMessage::create( LogLevel::error )
			.FunctionName(who)
			.Message( EngineMessage::BadArgIndexInForeignFunc )
			.ArgIndex(index)
			.ArgCount(numArgs)
			.ExpectedArgType(type)
		);
		return false;
	}

	ObjectType::Value  givenType = argsArray[index].raw()->getType();

	if ( givenType == type )
		return true;

	engine.print( LogMessage::create( LogLevel::error )
		.FunctionName(who)
		.Message( EngineMessage::WrongArgType )
		.ArgIndex(index)
		.ArgCount(numArgs)
		.GivenArgType(givenType)
		.ExpectedArgType(type)
	);

	return false;
}

bool
FFIServices::demandAllArgsType( ObjectType::Value  type, bool  imperative ) {
	ObjectType::Value  givenType;
	UInteger index = 0;

	for (; index < numArgs; ++index) {
		givenType = argsArray[index].raw()->getType();
		if ( givenType != type ) {
			engine.print( LogMessage::create( imperative? LogLevel::error : LogLevel::warning )
				.FunctionName(who)
				.Message( EngineMessage::WrongArgType )
				.ArgIndex(index)
				.ArgCount(numArgs)
				.GivenArgType(givenType)
				.ExpectedArgType(type)
			);
			return false;
		}
	}
	return true;
}

Object&
FFIServices::arg( UInteger  index ) {
	if ( index >= numArgs )
		throw FFIMisuseException();

	return *( argsArray[index].raw() );
}

void
FFIServices::printInfo(
	const char* message
) {
	engine.print(LogLevel::info, message);
}

void
FFIServices::printWarning(
	const char* message
) {
	engine.print(LogLevel::warning, message);
}

void
FFIServices::printError(
	const char* message
) {
	engine.print(LogLevel::error, message);
}

void
FFIServices::printCustomInfoCode( UInteger  code ) {
	engine.print( LogMessage::create(LogLevel::info)
		.Message(EngineMessage::CustomMessage)
		.CustomCode(code)
	);
}

void
FFIServices::printCustomWarningCode( UInteger  code ) {
	engine.print( LogMessage::create(LogLevel::warning)
		.Message(EngineMessage::CustomMessage)
		.CustomCode(code)
	);
}

void
FFIServices::printCustomErrorCode( UInteger  code ) {
	engine.print( LogMessage::create(LogLevel::error)
		.Message(EngineMessage::CustomMessage)
		.CustomCode(code)
	);
}

void
FFIServices::setResult(
	Object* obj
) {
	engine.lastObject.set(obj);
}

void
FFIServices::setNewResult(
	Object* obj
) {
	engine.lastObject.set(obj);
	obj->deref();
}


// *********** HELPER FUNCTIONS ********

// Accepts an address in string format and converts it to an address
// Can be used to reverse the effect of addressToString()
VarAddress
createAddress(
	const char* textAddress
) {
#ifdef COPPER_DEBUG_ADDRESS_MESSAGES
	std::printf("[DEBUG: Engine::createAddress\n");
#endif
	CharList asCL(textAddress);
	CharList::Iter asCLIter = asCL.start();
	CharList part;
	VarAddress finalAddress;
	if ( asCLIter.has() )
	do {
		if ( *asCLIter == '.' ) {
			if ( part.has() ) {
				finalAddress.push_back(part);
				part.clear();
			}
		} else {
			part.push_back(*asCLIter);
		}
	} while ( asCLIter.next() );
	if ( part.size() > 0 )
		finalAddress.push_back(part);
	return finalAddress;
}

// Accepts an address and returns a string
// Can be used to reverse the effect of createAddress()
String
addressToString(
	const VarAddress& address
) {
#ifdef COPPER_DEBUG_ADDRESS_MESSAGES
	std::printf("[DEBUG: Engine::addressToString\n");
#endif
	// This is EXTREMELY SLOW
	CharList builder;
	VarAddress::Iterator i = address.iterator();
	while( ! i.atEnd() ) {
		builder.append( i.get() );
		if ( ! i.atLast() ) { // If not at the last part
			builder.push_back('.');
		}
		i.next();
	}
	return String(builder);
}


//--------------------------------------

Engine::Engine()
	: logger(REAL_NULL)
	, stack()
	, taskStack()
	, lastObject()
	, bufferedTokens()
	, globalParserContext()
	, opcodeStrandStack()
	, endMainCallback(REAL_NULL)
	, builtinFunctions(50)
	, foreignFunctions(100)
	, ignoreBadForeignFunctionCalls(false)
	, ownershipChangingEnabled(false)
	, stackTracePrintingEnabled(false)
	, printTokensWhenParsing(false)
	, nameFilter(REAL_NULL)
{
	setupSystemFunctions();

	// Ensures global/starting OpStrandContainer gets a valid iterator
	// Add a do-nothing "operation"
	globalParserContext.addNewOperation( new Opcode(Opcode::Terminal) );

	// Tie the base strand used by the parser to the base stack used by execute()
	opcodeStrandStack.push_back(
		OpStrandContainer(globalParserContext.outputStrand)
	);
}

void Engine::setLogger( Logger* pLogger ) {
	logger = pLogger;
}

void Engine::print(
	const LogLevel::Value  logLevel,
	const char*  msg
) const {
	if ( notNull(logger) ) {
		logger->print(logLevel, msg);
	}
#ifdef USE_CSTDIO
	else {
		switch( logLevel ) {
		case LogLevel::warning:
			std::fprintf(stderr, "WARNING: %s\n", msg);
			break;
		case LogLevel::error:
			std::fprintf(stderr, "ERROR: %s\n", msg);
			break;
		default:
			std::fprintf(stderr, "%s\n", msg);
			break;
		}
	}
#endif
}

void Engine::print(
	const LogLevel::Value  logLevel,
	const EngineMessage::Value  msg
) const {
	if ( notNull(logger) ) {
		logger->print(logLevel, msg);
	}
#ifdef USE_CSTDIO
	else {
		switch( logLevel ) {
		case LogLevel::warning:
			std::fprintf(stderr, "WARNING THROWN!\n");
			break;
		case LogLevel::error:
			std::fprintf(stderr, "ERROR OCCURED!\n");
			break;
		default:
			//std::fprintf(stderr, "MESSAGE LOGGED.\n");
			break;
		}
	}
#endif
}

void Engine::print(LogMessage  logMsg) const {
	if ( notNull(logger) ) {
		logger->print(logMsg);
	}
#ifdef USE_CSTDIO
	else {
		switch( logMsg.level ) {
		case LogLevel::warning:
			std::fprintf(stderr, "WARNING THROWN!\n");
			break;
		case LogLevel::error:
			std::fprintf(stderr, "ERROR OCCURED!\n");
			break;
		default:
			//std::fprintf(stderr, "MESSAGE LOGGED.\n");
			break;
	}
#endif
}

void
Engine::addForeignFunction(
	const String&	pName,
	ForeignFunc*	pFunction
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::addForeignFunctionInstance");
#endif
	if ( isNull(pFunction) )
		throw NullForeignFunctionException();
	foreignFunctions.insert(pName, ForeignFuncContainer(pFunction));
}

EngineResult::Value
Engine::run(
	ByteStream& stream
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::run");
#endif
	EngineResult::Value  engineResult;

	switch( lexAndParse(stream, false) ) {
	case ParseResult::Done:
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
		printGlobalStrand();
#endif
		engineResult = execute();
		if ( engineResult == EngineResult::Ok ) {
			if ( stream.atEOS() )
				return EngineResult::Done;
		}
		return engineResult;

	case ParseResult::Error:
		print(LogLevel::error, "Parsing error.");
		return EngineResult::Error;

	case ParseResult::More:
		if ( stream.atEOS() ) {
			print(LogLevel::error, "Stream ended before parsing.");
			return EngineResult::Error;
		}
		return EngineResult::Ok;

	// to get -Wall to stop griping:
	default:
		print(LogLevel::error, "Falloff error in run.");
		return EngineResult::Error;
	}
}

#ifdef COPPER_DEBUG_ENGINE_MESSAGES
void
Engine::printGlobalStrand() {
	print(LogLevel::debug, "[DEBUG: Engine::printGlobalStrand()");
	OpStrandIter opIter = globalParserContext.outputStrand->start();
	if ( opIter.has() ) {
		print(LogLevel::debug, "[DEBUG: Global strand:");
		do {
			std::printf("[ Opcode = %p, value = %u\n", (void*)(opIter->getOp()), (UInteger)(opIter->getOp()->getType()) );
		} while ( opIter.next() );
	} else {
		print(LogLevel::debug, "[DEBUG: Global strand is empty.");
	}
}
#endif

void
Engine::printTokens( TokenQueue& tokenQueue ) {
	TokenQueueIter  tokenIter = tokenQueue.start();
	if ( printTokensWhenParsing && notNull(logger) && tokenIter.has() ) {
		do {
			logger->printToken(tokenIter.getItem());
		} while ( tokenIter.next() );
	}
}

ParseResult::Value
Engine::lexAndParse(
	ByteStream& stream,
	bool srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::lexAndParse");
#endif
	char c;
	CharList tokenValue; // Since I have to build with it, it's an easy-to-append-to list
	//List<Token> bufferedTokens; // Within the engine, so it's state is saved
	TokenType tokenType;

	while ( ! stream.atEOS() ) {
		c = stream.getNextByte();

		if ( c == '\0' ) {
			print(LogLevel::error, "Random null byte in stream");
			return ParseResult::Error;
		}

		if ( isWhitespace(c) ) {
			if ( tokenValue.size() == 0 ) {
				continue;
			}
			switch( tokenize(tokenValue, bufferedTokens) ) {
			case Result::Error:
				return ParseResult::Error;
			default: break;
			}
		}
		else if ( isSpecialCharacter(c, tokenType) ) {
			// If another token was started, finish it.
			if ( tokenValue.size() > 0 ) {
				// To allow for numbers to be tokenized with decimal places.
				if ( c == '.' && tokenValue[0] >= '0' && tokenValue[0] <= '9' ) {
					// Should check here for already-existing decimal and throw an error if found.
					tokenValue.push_back(c);
					continue;
				} else {
					switch( tokenize(tokenValue, bufferedTokens) ) {
					case Result::Error:
						return ParseResult::Error;
					default: break;
					}
				}
				tokenValue.clear();
			}
//#if COPPER_DEBUG_ENGINE_MESSAGES
			tokenValue.push_back(c);
//#endif
			// Comments, strings, and special chars are special cases that need to be handled immediately
			switch( handleCommentsStringsAndSpecials(tokenType, tokenValue, bufferedTokens, stream) ) {
			case Result::Error:
				return ParseResult::Error;
			default: break;
			}
		}
		else {
			if ( tokenValue.size() >= 255 ) { // Don't accept tokens longer than this
				//print(LogLevel::error, "ERROR: Token identifier is too long." );
				print(LogLevel::error, EngineMessage::LongToken);
				return ParseResult::Error;
			}
			tokenValue.push_back(c);
		}
	}

	if ( tokenValue.size() > 0 ) {
		// Push the last token
		switch( tokenize(tokenValue, bufferedTokens) ) {
		case Result::Error:
			return ParseResult::Error;
		default: break;
		}
	}

	if ( ! bufferedTokens.has() ) {
		return ParseResult::More;
	}

	ParserContext& context = getGlobalParserContext();
	context.setTokenSource( bufferedTokens );

	switch( parse( context, srcDone ) ) {
	case ParseResult::More:
		return ParseResult::More;
	case ParseResult::Error:
		return ParseResult::Error;
	case ParseResult::Done:
		context.clearUsedTokens();
		return ParseResult::Done;
	// to get -Wall to stop griping
	default:
		return ParseResult::Error;
	}
}


void Engine::signalEndofProcessing() {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::signalEndofProcessing");
#endif
	if ( notNull(endMainCallback) ) {
		endMainCallback->CuEngineDoneProcessing();
	}
}

void Engine::printTaskTrace() {
	if ( isNull(logger) )
		return;
	List<TaskContainer>::Iter  ti = taskStack.start();
	if ( ! ti.has() )
		return;
	UInteger  t_index = 1;
	do {
		if ( ti->getTask().name == TaskType::FuncBuild ) {
			logger->printTaskTrace(TaskType::FuncBuild, "Body construction", t_index);
		} else if ( ti->getTask().name == TaskType::FuncFound ) {
			logger->printTaskTrace(
				TaskType::FuncFound,
				addressToString(
					((FuncFoundTask&)(ti->getTask())).getAddress()
				),
				t_index
			);
		}
		++t_index;
	} while ( ti.next() );
}

void Engine::printStackTrace() {
	stack.print(logger);
}

void Engine::clearGlobals() {
	stack.clearGlobal();
}

void Engine::clearStacks() {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::clearStacks");
#endif
	taskStack.clear();
	stack.clearNonGlobal();
	//stack.clear(); // now using clearNonGlobal
	//globalParserContext.clear();
	//globalParserContext.moveToFirstUnusedToken(); // may conflict with ParserContext.onError() ?

	// Reinitialized the stack
	// This is useful if the program should continue
	//stack.push(); // now using clearNonGlobal
}

Scope&
Engine::getGlobalScope() {
	return stack.getBottom().getScope();
}

Scope&
Engine::getCurrentTopScope() {
	return stack.getTop().getScope();
}

ParserContext&
Engine::getGlobalParserContext() {
	return globalParserContext;
}

void
Engine::setVariableByAddress(
	const VarAddress&	address,
	Object*				obj,
	bool				reuseStorage
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::setVariableByAddress");
#endif
	Variable* var = resolveVariableAddress(address);
	if ( notNull(var) ) {
		if ( isNull(obj) ) {
			throw BadParameterException<Engine>();
		}

#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	std::printf("[DEBUG: Engine::setVariableByAddress: object addr=%p\n\t\tvar raw container address = %p\n", obj, var->getRawContainer());
#endif

		switch( obj->getType() ) {
		case ObjectType::Function:
			var->setFunc((FunctionObject*)obj, reuseStorage);
			lastObject.set(var->getRawContainer());
			break;
		//case ObjectType::Data:
		default:
			// Use pPerformCopy (default) if this object is held by anything other than the lastObject.
			var->setFuncReturn( obj, (obj->getRefCount() != 1) );
			lastObject.set(var->getRawContainer());
			break;
		}
	} else {
		lastObject.setWithoutRef(new FunctionObject());
	}
}

TokenType
Engine::resolveTokenType( const String& pName ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::resolveTokenType");
#endif
	if ( pName.size() == 0 ) {
		print(LogLevel::error, EngineMessage::ZeroSizeToken);
		return TT_malformed;
	}

	//TokenType tokenType;
	// Repeat duty
	//if ( isSpecialCharacter(pName[0], tokenType) ) {
	//	if ( pName.size() > 1 )
	//		return TT_malformed;
	//	return tokenType;
	//}

	if ( pName.equals(CONSTANT_END_MAIN) )
		return TT_end_main;
	if ( pName.equals(CONSTANT_EXIT) )
		return TT_exit;
	if ( pName.equals(CONSTANT_TOKEN_IF) )
		return TT_if;
	if ( pName.equals(CONSTANT_TOKEN_ELIF) )
		return TT_elif;
	if ( pName.equals(CONSTANT_TOKEN_ELSE) )
		return TT_else;
	if ( pName.equals(CONSTANT_TOKEN_LOOP) )
		return TT_loop;
	if ( pName.equals(CONSTANT_TOKEN_ENDLOOP) )
		return TT_endloop;
	if ( pName.equals(CONSTANT_TOKEN_LOOPSKIP) )
		return TT_skip;
	if ( pName.equals(CONSTANT_TOKEN_TRUE) )
		return TT_boolean_true;
	if ( pName.equals(CONSTANT_TOKEN_FALSE) )
		return TT_boolean_false;

	if ( pName[0] >= '0' && pName[0] <= '9' ) {
		switch ( pName.numberType() ) {
		case 1:
			return TT_num_integer;
		case 2:
			return TT_num_decimal;
		default:
			print(LogLevel::warning, EngineMessage::MalformedNumber);
			return TT_string;
		}
	}

	// Excluding lower system characters, other characters are valid (Unicode acceptable)
	// Lower system characters are unknown, not malformed, so check here for lower bound
	if ( isValidNameCharacter(pName[0]) ) {
		if ( isValidName(pName) ) {
			return TT_name;
		} else {
			print(LogLevel::error, EngineMessage::InvalidName);
			return TT_malformed;
		}
	}

	return TT_unknown;
}

Result::Value
Engine::tokenize( CharList& tokenValue, List<Token>& tokens ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::tokenize");
#endif
	const String tokenName(tokenValue);
	TokenType tokenType = resolveTokenType( tokenName );

	//std::printf("[DEBUG: token id =%u\n", (UInteger)tokenType);

	if ( isValidToken(tokenType) ) {
		tokens.push_back( Token(tokenType, tokenName) );
	} else {
		return Result::Error;
	}
	tokenValue.clear();
	return Result::Ok;
}

Result::Value
Engine::handleCommentsStringsAndSpecials(
	const TokenType&	tokenType,
	CharList&			tokenValue,
	List<Token>&		tokens,
	ByteStream&			stream
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::handleCommentsStringsAndSpecials");
#endif
	CharList collectedValue;

	const String tokenName(tokenValue);

	switch( tokenType ) {
	// Handle comments
	case TT_comment:
		switch( scanComment(stream) ) {
		case Result::Ok:
			tokenValue.clear();
			return Result::Ok;
		case Result::Error:
			return Result::Error;
		}
		break;

	case TT_string:
		switch( collectString(stream, collectedValue) ) {
		case Result::Ok:
			tokens.push_back( Token(TT_string, String(collectedValue)) );
			tokenValue.clear();
			return Result::Ok;
		case Result::Error:
			return Result::Error;
		}
		break;

	case TT_escape_character:
		//print(LogLevel::error, "ERROR: Escape-character outside of string.");
		print(LogLevel::error, EngineMessage::EscapeCharMisplaced);
			tokenValue.clear();
		return Result::Error;

	default:
		if ( isValidToken(tokenType) ) {
			tokens.push_back( Token(tokenType, String(tokenValue)) );
			tokenValue.clear();
		} else {
			return Result::Error;
		}
	}
	return Result::Ok;
}

bool
Engine::isWhitespace( const char c ) const {
	// I forgot the other types of whitespace, but I need them, such as the no-separator character
	// which could be the source of huge problems (hidden security bugs in open-source code).
	return (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\f' || c == '\v' || c == '\0');
	// Whether or not I should test c == '\0' or not is an interesting consideration.
	// I probably shouldn't, and that way unicode chunks given as int via getc() work fine.
}

bool
Engine::isSpecialCharacter(
	const char c,
	TokenType& pTokenType
) {
	switch( c ) {
	case ',': pTokenType = TT_end_segment;				return true;
	case '=': pTokenType = TT_assignment;				return true;
	case '~': pTokenType = TT_pointer_assignment;		return true;
	case '.': pTokenType = TT_member_link;				return true;
	case '(': pTokenType = TT_parambody_open;			return true;
	case ')': pTokenType = TT_parambody_close;			return true;
	case '{': pTokenType = TT_execbody_open;			return true;
	case '}': pTokenType = TT_execbody_close;			return true;
	case '[': pTokenType = TT_objectbody_open;			return true;
	case ']': pTokenType = TT_objectbody_close;			return true;
	case ':': pTokenType = TT_immediate_run;			return true;
	case CONSTANT_COMMENT_TOKEN: 
				pTokenType = TT_comment;				return true;
	// TT_string1 would allow TT_string2, but it's also a mess
	case CONSTANT_STRING_TOKEN:
				pTokenType = TT_string;					return true;
	case CONSTANT_ESCAPE_CHARACTER_TOKEN:
				pTokenType = TT_escape_character;		return true;

	default: break;
	}
	return false;
}

bool
Engine::isEscapeCharacter( const char c ) const {
	return c == CONSTANT_ESCAPE_CHARACTER_TOKEN;
}

bool
Engine::isValidToken( const TokenType& token ) const {
	switch( token ) {
	case TT_unknown:
		//print(LogLevel::error, "ERROR: Unresolvable token found.\n");
		print( LogLevel::error, EngineMessage::UnresolvedToken );
		return false;
	case TT_malformed:
		//print(LogLevel::error, "ERROR: Malformed token found.\n");
		print( LogLevel::error, EngineMessage::MalformedToken );
		return false;
	default:
		return true;
	}
}

bool
Engine::isValidNameCharacter( const char c ) const {
	switch(c) {
	case '_':
		return true;
#ifdef COPPER_ENABLE_EXTENDED_NAME_SET
	case '+':
		return true;
	case '-':
		return true;
	case '*':
		return true;
	case '/':
		return true;
	case '%':
		return true;
	case '!':
		return true;
	case '?':
		return true;
#endif
	default:
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
#ifdef COPPER_ENABLE_NUMERIC_NAMES
				|| (c >= '0' && c <= '9')
#endif
				;
	}
}

bool
Engine::isValidName( const String& pName ) const {
	//if ( pName.size() > 256 )
	//	return false;
		// For checking Unicode if desired
	if ( nameFilter ) {
		return nameFilter(pName);
	}
	UInteger i=0;
	for (; i < pName.size(); ++i ) {
		if ( isValidNameCharacter(pName[i]) )
			continue;
		// TODO: I need to block invisible and undefined/nonprintable characters.
		return false; // Otherwise, invalid character
	}
	return true;
}

bool
Engine::isCommentToken( const char c ) const {
	return c == CONSTANT_COMMENT_TOKEN;
}

bool
Engine::isStringToken( const char c ) const {
	return c == CONSTANT_STRING_TOKEN;
}

Result::Value
Engine::scanComment(
	ByteStream& stream
) {
	char c = '`';
	while ( ! stream.atEOS() ) {
		c = stream.getNextByte();
		if ( isEscapeCharacter(c) ) {
			if ( !stream.atEOS() ) {
				c = stream.getNextByte();
				continue;
			}
		} else {
			if ( isCommentToken(c) ) {
				return Result::Ok;
			}
		}
	}
	//print(LogLevel::error, "ERROR: Stream halted before comment ended.");
	print( LogLevel::error, EngineMessage::StreamHaltedInComment );
	return Result::Error;
}

Result::Value
Engine::collectString(
	ByteStream& stream,
	CharList& collectedValue
) {
	char c = '`';
	while ( ! stream.atEOS() ) {
		c = stream.getNextByte();
		if ( isEscapeCharacter(c) ) {
			if ( ! stream.atEOS() ) {
				c = stream.getNextByte();
				switch(c) {
				case 'n':
					collectedValue.push_back( '\n' );
					break;
				case 'r':
					collectedValue.push_back( '\r' );
					break;
				case 't':
					collectedValue.push_back( '\t' );
					break;
				default:
					collectedValue.push_back( c );
					break;
				}
			}
			continue;
		}
		if ( isStringToken(c) ) {
			return Result::Ok;
		}
		collectedValue.push_back(c);
	}
	//print(LogLevel::error, "ERROR: Stream halted before string ended.");
	print( LogLevel::error, EngineMessage::StreamHaltedInString );
	return Result::Error;
}


//--------------------------------------
// ******** SYSTEM FUNCTION SETUP ******

void
Engine::setupSystemFunctions() {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::setupSystemFunctions");
#endif
	builtinFunctions.insert(String(CONSTANT_FUNCTION_RETURN), SystemFunction::_return);
	//builtinFunctions.insert(String("own"), SystemFunction::_own); // handled as SRPS
	builtinFunctions.insert(String("not"), SystemFunction::_not);
	builtinFunctions.insert(String("all"), SystemFunction::_all);
	builtinFunctions.insert(String("any"), SystemFunction::_any);
	builtinFunctions.insert(String("nall"), SystemFunction::_nall);
	builtinFunctions.insert(String("none"), SystemFunction::_none);
	builtinFunctions.insert(String("are_fn"), SystemFunction::_are_fn);
	//builtinFunctions.insert(String("is_ptr"), SystemFunction::_is_ptr); // handled as SRPS
	//builtinFunctions.insert(String("is_owner"), SystemFunction::_is_owner); // handled as SRPS
	builtinFunctions.insert(String("are_empty"), SystemFunction::_are_empty);
	builtinFunctions.insert(String("are_same"), SystemFunction::_are_same);
	builtinFunctions.insert(String("member"), SystemFunction::_member);
	builtinFunctions.insert(String("member_count"), SystemFunction::_member_count);
	builtinFunctions.insert(String("is_member"), SystemFunction::_is_member);
	builtinFunctions.insert(String("set_member"), SystemFunction::_set_member);
	builtinFunctions.insert(String("member_list"), SystemFunction::_member_list);
	builtinFunctions.insert(String("union"), SystemFunction::_union);
	builtinFunctions.insert(String("type_of"), SystemFunction::_type);
	builtinFunctions.insert(String("are_same_type"), SystemFunction::_are_same_type);
	builtinFunctions.insert(String("are_bool"), SystemFunction::_are_bool);
	builtinFunctions.insert(String("are_string"), SystemFunction::_are_string);
	builtinFunctions.insert(String("are_list"), SystemFunction::_are_list);
	builtinFunctions.insert(String("are_number"), SystemFunction::_are_number);
	builtinFunctions.insert(String("are_int"), SystemFunction::_are_integer);
	builtinFunctions.insert(String("are_dcml"), SystemFunction::_are_decimal);
	builtinFunctions.insert(String("assert"), SystemFunction::_assert);

	builtinFunctions.insert(String("copy_of"), SystemFunction::_copy);
	builtinFunctions.insert(String("xwsv"), SystemFunction::_execute_with_alt_super);

	builtinFunctions.insert(String("list"), SystemFunction::_make_list);
	builtinFunctions.insert(String("length"), SystemFunction::_list_size);
	builtinFunctions.insert(String("append"), SystemFunction::_list_append);
	builtinFunctions.insert(String("prepend"), SystemFunction::_list_prepend);
	builtinFunctions.insert(String("insert"), SystemFunction::_list_insert);
	builtinFunctions.insert(String("item_at"), SystemFunction::_list_get_item);
	builtinFunctions.insert(String("erase"), SystemFunction::_list_remove);
	builtinFunctions.insert(String("dump"), SystemFunction::_list_clear);
	builtinFunctions.insert(String("swap"), SystemFunction::_list_swap);
	builtinFunctions.insert(String("replace"), SystemFunction::_list_replace);
	builtinFunctions.insert(String("sublist"), SystemFunction::_list_sublist);

	builtinFunctions.insert(String("matching"), SystemFunction::_string_match);
	builtinFunctions.insert(String("concat"), SystemFunction::_string_concat);

	builtinFunctions.insert(String("equal"), SystemFunction::_num_equal);
	builtinFunctions.insert(String("gt"), SystemFunction::_num_greater_than);
	builtinFunctions.insert(String("gte"), SystemFunction::_num_greater_or_equal);
	builtinFunctions.insert(String("lt"), SystemFunction::_num_less_than);
	builtinFunctions.insert(String("lte"), SystemFunction::_num_less_or_equal);
	builtinFunctions.insert(String("abs"), SystemFunction::_num_abs);
#ifdef COPPER_ENABLE_EXTENDED_NAME_SET
	builtinFunctions.insert(String("+"), SystemFunction::_num_add);
	builtinFunctions.insert(String("-"), SystemFunction::_num_subtract);
	builtinFunctions.insert(String("*"), SystemFunction::_num_multiply);
	builtinFunctions.insert(String("/"), SystemFunction::_num_divide);
	builtinFunctions.insert(String("%"), SystemFunction::_num_modulus);
	builtinFunctions.insert(String("++"), SystemFunction::_num_incr);
	builtinFunctions.insert(String("--"), SystemFunction::_num_decr);
#else
	// Alternative names
	builtinFunctions.insert(String("add"), SystemFunction::_num_add);
	builtinFunctions.insert(String("sbtr"), SystemFunction::_num_subtract);
	builtinFunctions.insert(String("mult"), SystemFunction::_num_multiply);
	builtinFunctions.insert(String("divd"), SystemFunction::_num_divide);
	builtinFunctions.insert(String("mod"), SystemFunction::_num_modulus);
	builtinFunctions.insert(String("incr"), SystemFunction::_num_incr);
	builtinFunctions.insert(String("decr"), SystemFunction::_num_decr);
#endif
}

//-------------------------------------
//********** PARSING SYSTEM ***********

// The ParserContext must be passed in by whatever called this function.
ParseResult::Value
Engine::parse(
	ParserContext&	context,
	bool			srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::parse");
#endif
	// IMPORTANT NOTE: You cannot short-circuit this function by merely checking for if the context
	// is finished. There may be unfinished tasks that prevent valid processing.

	ParseTaskIter parseIter = context.taskStack.start();
	ParseTask::Result::Value result;

	do {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::parse cycle");
#endif
		parseIter.makeLast();
		do {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::parse task cycle");
#endif
			if ( parseIter.has() ) {
				// IMPORTANT NOTE: You cannot move to the first unused token here because
				// it prevents assignment to variables from working correctly.

				result = processParseTask( parseIter->getTask(), context, srcDone );
				// Do tasks first, checking for valid tokens.
				// Tasks are expected to check for ALL the tokens they need and err if they are not available.
				switch( result )
				{
				case ParseTask::Result::need_more:
					return ParseResult::More;

				case ParseTask::Result::task_done:
					context.taskStack.pop();
					parseIter.makeLast();
					break;

				case ParseTask::Result::interpret_token:
					// Tasks requiring another token that won't exist will gum up the works.
					if ( context.isFinished() ) {
						if ( srcDone ) {
							print(LogLevel::error, EngineMessage::StreamHaltedParsingNotDone);
							return ParseResult::Error;
						} else {
							return ParseResult::More;
						}
					}
					break;

				case ParseTask::Result::syntax_error:
				default:
					context.onError();
					return ParseResult::Error;
				}
			} else {
				break;
			}
		} while ( result == ParseTask::Result::task_done );

		switch( interpretToken( context ) )
		{
		case ParseResult::More:
			continue; // Skip to perform processing cycle (task has been added)
		case ParseResult::Error:
			context.onError();
			return ParseResult::Error;
		case ParseResult::Done:
		default:
			// All up-front processing for this token has been done, so continue to next token.
			break;
		}

	} while ( ! context.isFinished() || parseIter.has() );

	// Note: tasks themselves also throw an error if they don't have enough tokens,
	// so there should be no need to check here except for debugging.

#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	context.outputStrand->validate();
	print(LogLevel::debug, "[DEBUG: Engine::parse validated output strand.");
#endif

	//outStrand.append(context.buildStrand); // Should be saved in the context anyways
	return ParseResult::Done;
}

ParseTask::Result::Value
Engine::moveToFirstUnusedToken(
	ParserContext&		context,
	bool				srcDone
) {
	if ( ! context.moveToFirstUnusedToken() ) {
		if ( srcDone ) {
			print(LogLevel::error, EngineMessage::StreamHaltedParsingNotDone);
			return ParseTask::Result::syntax_error;
		} else {
			return ParseTask::Result::need_more;
		}
	}
	return ParseTask::Result::task_done;
}

ParseResult::Value
Engine::interpretToken(
	ParserContext&	context
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::intepretToken");
#endif
	// ATTENTION!! Before calling, be sure to commit token usage.
	// This function call (of moveToFirstUnusedToken) ensures that interpretToken()
	// never acts on the same token twice.
	if ( ! context.moveToFirstUnusedToken() )
		return ParseResult::Done;

	Token  currToken = context.peekAtToken();

	Opcode*  code = REAL_NULL;

	switch( currToken.type ) {

	case TT_malformed:
		print(LogLevel::error, EngineMessage::MalformedToken);
		return ParseResult::Error;

	case TT_unknown:
		print(LogLevel::error, EngineMessage::UnknownToken);
		return ParseResult::Error;

	//---------------

	//case TT_comment:
		// This shouldn't be in the parser
		//throw (UInteger)TT_comment;

	//---------------
	case TT_exit:
		// Add this token
		context.addNewOperation( new Opcode(Opcode::Exit) );
		break;

	// TODO: Also setup the end_main token.

	//---------------
	case TT_end_segment:
		// Skip doing anything with this token, but causes errors when in the wrong place
		break;

	//---------------
	case TT_parambody_open:
		// Should be absorbed by a task, so generate error message if not.
		print( LogLevel::error, EngineMessage::OrphanParamBodyOpener );
		return ParseResult::Error;

	case TT_parambody_close:
		// Should be absorbed by a task, so generate error message if not.
		print( LogLevel::error, EngineMessage::OrphanParamBodyCloser );
		return ParseResult::Error;

	//---------------
		// Note: The system could be modified to make "fn" optional again.

	case TT_execbody_open:
		// Let the parse task begin by expecting the current token is the open-exec-body-token
		addNewParseTask(
			context.taskStack,
			new FuncBuildParseTask( FuncBuildParseTask::State::FromExecBody )
		);
		context.addNewOperation( new Opcode(Opcode::FuncBuild_start) );
		return ParseResult::More;

	case TT_execbody_close:
		print( LogLevel::error, EngineMessage::OrphanBodyCloser );
		return ParseResult::Error;

	//---------------
	case TT_objectbody_open:
		// Let the parse task begin by expecting the current token is the open-object-body token
		addNewParseTask(
			context.taskStack,
			new FuncBuildParseTask( FuncBuildParseTask::State::FromObjectBody )
		);
		context.addNewOperation( new Opcode(Opcode::FuncBuild_start) );
		return ParseResult::More;

	case TT_objectbody_close:
		print( LogLevel::error, EngineMessage::OrphanObjectBodyCloser );
		return ParseResult::Error;

	//---------------

	case TT_if:
		// If-statements always begin with a terminal
		context.addNewOperation( new Opcode(Opcode::Terminal) );
		addNewParseTask(
			context.taskStack,
			new IfStructureParseTask( context.outputStrand )
		);
		context.commitTokenUsage(); // Keep the "if"
		return ParseResult::More;

	case TT_elif:
		// Should have been absorbed
		print( LogLevel::error, EngineMessage::OrphanElif );
		return ParseResult::Error;

	case TT_else:
		// Should have been absorbed
		print( LogLevel::error, EngineMessage::OrphanElse );
		return ParseResult::Error;

	//---------------

	case TT_loop:
		context.addNewOperation( new Opcode(Opcode::Terminal) ); // Start of the loop
		addNewParseTask(
			context.taskStack,
			new LoopStructureParseTask( context.outputStrand )
		);
		return ParseResult::More;

	case TT_endloop:
		// Must tie in with the loop's starting terminal by searching back through the tasks
		// to find the loop's initial one.
		// Then this becomes a goto loop end.
		return ParseLoop_AddEndLoop(context);

	case TT_skip:
		// Must tie in with the loop's starting terminal by searching back through the tasks
		// to find the loop's initial one.
		// Then this becomes a goto loop start.
		return ParseLoop_AddLoopSkip(context);

	//---------------
	case TT_name:
		// Certain names can't be treated like regular functions because they require accessing
		// the variable directly. Furthermore, their calls can be optimized into a single opcode
		// by parsing them as unique tasks.

		// Special names
		if ( context.peekAtToken().name.equals(CONSTANT_TOKEN_OWN) ) {
			addNewParseTask(
				context.taskStack,
				new SRPSParseTask(Opcode::Own)
			);
			context.commitTokenUsage();
			return ParseResult::More;
		}
		// else
		if ( context.peekAtToken().name.equals(CONSTANT_TOKEN_IS_OWNER) ) {
			addNewParseTask(
				context.taskStack,
				new SRPSParseTask(Opcode::Is_owner)
			);
			context.commitTokenUsage();
			return ParseResult::More;
		}
		// else
		if ( context.peekAtToken().name.equals(CONSTANT_TOKEN_IS_PTR) ) {
			addNewParseTask(
				context.taskStack,
				new SRPSParseTask(Opcode::Is_pointer)
			);
			context.commitTokenUsage();
			return ParseResult::More;
		}
		// else
		// Regular names. Request more processing.
		addNewParseTask(
			context.taskStack,
			new FuncFoundParseTask( currToken.name )
		);
		context.commitTokenUsage();
		return ParseResult::More;

	//---------------
	// Handling data

	case TT_boolean_true:
		context.addNewOperation(
			new Opcode(Opcode::CreateBoolTrue)
		);
		break;

	case TT_boolean_false:
		context.addNewOperation(
			new Opcode(Opcode::CreateBoolFalse)
		);
		break;

	//---------------

	case TT_string:
		code = new Opcode(Opcode::CreateString, currToken.name, false);
		context.addNewOperation(code);
		break;

	//---------------

	case TT_num_integer:
		code = new Opcode(Opcode::CreateInteger);
		code->setIntegerData(currToken.name.toInt());
		context.addNewOperation(code);
		break;

	//---------------

	case TT_num_decimal:
		code = new Opcode(Opcode::CreateDecimal);
		code->setDecimalData(currToken.name.toDouble());
		context.addNewOperation(code);
		break;

	//---------------
	default:
		// Unhandled token
		print(LogLevel::error, EngineMessage::TokenNotHandled);
		return ParseResult::Error;
	}

	// Indicate the tokens were used
	context.commitTokenUsage();

	return ParseResult::Done;
}

ParseTask::Result::Value
Engine::processParseTask(
	ParseTask*		task,
	ParserContext&	context,
	bool			srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::processParseTask");
#endif
	// Perform actions based on the current task
	switch( task->type ) {
	case ParseTask::FuncBuild:
		return parseFuncBuildTask( (FuncBuildParseTask*)task, context, srcDone );

	case ParseTask::FuncFound:
		return parseFuncFoundTask( (FuncFoundParseTask*)task, context, srcDone );

	case ParseTask::If:
		return parseIfStructure( (IfStructureParseTask*)task, context, srcDone );

	case ParseTask::Loop:
		return parseLoopStructure( (LoopStructureParseTask*)task, context, srcDone );

	case ParseTask::SRPS: // Single Raw Parameter Structure "own", "is_owner", "is_ptr"
		return parseSRPS( (SRPSParseTask*)task, context, srcDone );

	default:
		throw BadParserStateException();
	}
}

ParseTask::Result::Value
Engine::parseFuncBuildTask(
	FuncBuildParseTask*	task,
	ParserContext&		context,
	bool				srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::parseFuncBuildTask");
#endif

	// All the tasks here require the next token to be unused
	ParseTask::Result::Value r = moveToFirstUnusedToken(context, srcDone);
	if ( r != ParseTask::Result::task_done )
		return r;

	// if ( task->state == FuncBuildParseTask::State::FromFn ) {
	//	// Look for brackets. If none found, add function-end opcode and return task_done
	//	// If object body is found, call ParseFunctionBuild_FromObjectBody().
	//	// If execution body is found, call ParseFunctionBuild_FromExecBody().
	// }

	switch( task->state )
	{
	case FuncBuildParseTask::State::FromObjectBody:
		return ParseFunctionBuild_FromObjectBody(task, context, srcDone);

	case FuncBuildParseTask::State::CollectParams:
		return ParseFunctionBuild_CollectParameters(task, context, srcDone);

	case FuncBuildParseTask::State::AwaitAssignment:
		context.addNewOperation(
			new Opcode( Opcode::FuncBuild_assignToVar, task->paramName, false )
		);
		task->state = FuncBuildParseTask::State::CollectParams;
		return ParseFunctionBuild_CollectParameters(task, context, srcDone);

	case FuncBuildParseTask::State::AwaitPointerAssignment:
		context.addNewOperation(
			new Opcode( Opcode::FuncBuild_pointerAssignToVar, task->paramName, false )
		);
		task->state = FuncBuildParseTask::State::CollectParams;
		return ParseFunctionBuild_CollectParameters(task, context, srcDone);

	case FuncBuildParseTask::State::FromExecBody:
		return ParseFunctionBuild_FromExecBody(context, srcDone);

	default:
		throw BadParserStateException();
	}

	// REMEMBER TO EAT TOKENS!

	// When waiting on data, this needs to check to make sure that the last object satisfies
	// the requirements.
	// Remember, pointer assignment requires a function-found operation to be in the opcodes next.
}

ParseTask::Result::Value
Engine::ParseFunctionBuild_FromObjectBody(
	FuncBuildParseTask*	task,
	ParserContext&		context,
	bool				srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseFunctionBuild_FromObjectBody");
#endif
	// The current token should be an object body opener. We can check to verify.
#ifdef COPPER_STRICT_CHECKS
	if ( context.peekAtToken().type != TT_objectbody_open )
		throw ParserTokenException( context.peekAtToken() );
#endif

	if ( ! context.moveToNextToken() ) {
		if ( srcDone ) {
			//print(LogLevel::error, "Unfinished object body.");
			print(LogLevel::error, EngineMessage::OrphanObjectBodyOpener);
			return ParseTask::Result::syntax_error;
		} else {
			return ParseTask::Result::need_more;
		}
	}

	// Check for the other object-body close token.
	// If the tokens run out before it is found, request more.
	// Otherwise, error.
	UInteger openBrackets = 1;

	do {
		switch( context.peekAtToken().type ) {

		case TT_objectbody_open:
			++openBrackets;
			break;

		case TT_objectbody_close:
			--openBrackets;
			if ( openBrackets == 0 ) {
				// All done
				context.moveToFirstUnusedToken();
#ifdef COPPER_STRICT_CHECKS
				if ( context.peekAtToken().type != TT_objectbody_open )
					throw ParserTokenException( context.peekAtToken() );
#endif
				context.commitTokenUsage(); // Keep the opening token
				context.moveToNextToken();
				task->state = FuncBuildParseTask::State::CollectParams;
				// As this function only verifies the last object body closing token is present,
				// it ignores the tokens in between.
				return ParseFunctionBuild_CollectParameters(task, context, srcDone);
			}
			break;

		default:
			break;
		}
	} while ( context.moveToNextToken() );

	if ( srcDone ) {
		//print(LogLevel::error, "Unfinished object body.");
		print(LogLevel::error, EngineMessage::OrphanObjectBodyOpener);
		return ParseTask::Result::syntax_error;
	} else {
		return ParseTask::Result::need_more;
	}
}

ParseTask::Result::Value
Engine::ParseFunctionBuild_CollectParameters(
	FuncBuildParseTask*	task,
	ParserContext&		context,
	bool				srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseFunctionBuild_CollectParameters");
#endif
	//FuncFoundParseTask* fft;
	// Only names are valid parameters within function parameter bodies / object bodies.
	// Object bodies and function bodies are scanned over when encountered.
	// The object body close token should be the only stray token encountered.

	// Current parsing state: After the open-object-body token.

	// Ignore end-segment tokens
	if ( context.peekAtToken().type == TT_end_segment ) {
		while( context.peekAtToken().type == TT_end_segment ) {
			context.commitTokenUsage();
			if ( !context.moveToNextToken() ) // Unnecessary if-check, but moveToNextToken is necessary.
				return ParseTask::Result::need_more;
		}
	}

	while( context.peekAtToken().type == TT_name ) {
		task->paramName = context.peekAtToken().name;
		// Check for valid tokens
		context.commitTokenUsage();
		context.moveToNextToken();

		switch ( context.peekAtToken().type ) {
		case TT_assignment:
			// Start with the token after the assignment token
			context.commitTokenUsage();
			context.moveToNextToken();

			// Valid tokens: (Basically, anything starting an expression and resulting in an object)
			switch( context.peekAtToken().type ) {
			case TT_objectbody_open:
			case TT_execbody_open:
			case TT_name:
			case TT_boolean_true:
			case TT_boolean_false:
			case TT_string:
			case TT_num_integer:
			case TT_num_decimal:
				task->state = FuncBuildParseTask::State::AwaitAssignment;
				return ParseTask::Result::interpret_token;

			case TT_objectbody_close:
				//print(LogLevel::error, "Unfinished parameter assignment in object body.");
				print(LogLevel::error, EngineMessage::ObjectBodyInvalidParamAsgn);
				return ParseTask::Result::syntax_error;
			default:
				//print(LogLevel::error, "Stray token encountered while collecting names in object body.");
				print(LogLevel::error, EngineMessage::ObjectBodyInvalidParamAsgn);
				return ParseTask::Result::syntax_error;
			}

		case TT_pointer_assignment:
			// Start with the token after the assignment token
			context.commitTokenUsage();
			context.moveToNextToken();

			// Valid tokens: names, object bodies, and function bodies
			switch( context.peekAtToken().type ) {
			case TT_name:
				task->state = FuncBuildParseTask::State::AwaitPointerAssignment;
				return ParseTask::Result::interpret_token;

				// Collect function-found data here rather than waiting.
//					fft = new FuncFoundParseTask();
//					switch( parseFuncFound(fft, context, srcDone) ) {
//					case ParseTask::Result::task_done:
//						createPointerAssignmentOpcode(
//							context,
//							paramName,
//							*(fft->getAddressData())
//						);
//						break;
//					default:
//						// All the tokens should be available, everything else is considered syntax error
//						return ParseTask::Result::error;
//					}

			case TT_objectbody_open:
			case TT_execbody_open:
				// a~[], a~[]{}, a~{} not implemented.
				// They could be resolved correctly if I implemented such parsing,
				// so they should be permitted but discouraged.
				//print(LogLevel::error, "Invalid token following pointer assignment in object body.");
				print(LogLevel::error, EngineMessage::ObjectBodyInvalidParamPtrAsgn);
				return ParseTask::Result::syntax_error;

			case TT_objectbody_close:
				//print(LogLevel::error, "Unfinished parameter pointer assignment in object body.");
				print(LogLevel::error, EngineMessage::ObjectBodyInvalidParamPtrAsgn);
				return ParseTask::Result::syntax_error;

			default:
				print(LogLevel::error, EngineMessage::ObjectBodyInvalidParamPtrAsgn);
				return ParseTask::Result::syntax_error;
			}
			break;

		case TT_name:
		case TT_objectbody_close:
		case TT_end_segment:
			// Regular parameters
			context.addNewOperation(
				new Opcode( Opcode::FuncBuild_createRegularParam, task->paramName, false )
			);
			break;

		default:
			print(LogLevel::error, EngineMessage::ObjectBodyInvalid);
			return ParseTask::Result::syntax_error;
		}

		// Ignore end-of-segment tokens
		if ( context.peekAtToken().type == TT_end_segment ) {
			while ( context.peekAtToken().type == TT_end_segment ) {
				context.commitTokenUsage();
				context.moveToNextToken();
			}
		}
	}

	if ( context.peekAtToken().type == TT_objectbody_close ) {
		// Check if there is another token, which may be the execution body
		if ( context.moveToNextToken() ) {

			if ( context.peekAtToken().type == TT_execbody_open ) {
				// State is []{
				context.moveToPreviousToken(); // Go back to object-body-close token
				context.commitTokenUsage(); // Keep object-body-close token
				task->state = FuncBuildParseTask::State::FromExecBody;
				context.moveToNextToken(); // Move back to execbody open token
				return ParseFunctionBuild_FromExecBody(context, srcDone);
			} else {
				// No execution body, so end here
				context.moveToPreviousToken(); // Go back to object-body-close-token
				context.addNewOperation( new Opcode( Opcode::FuncBuild_end ) );
				context.commitTokenUsage(); // Keep the object-body close
				return ParseTask::Result::task_done;
			}

		} else {
			// No execution body, so end here
			context.addNewOperation( new Opcode( Opcode::FuncBuild_end ) );
			context.commitTokenUsage(); // Keep the object-body close
			return ParseTask::Result::task_done;
		}
	}

	// else
	// Error
	print(LogLevel::error, EngineMessage::ObjectBodyInvalid);
	return ParseTask::Result::syntax_error;
}

ParseTask::Result::Value
Engine::ParseFunctionBuild_FromExecBody(
	//FuncBuildParseTask*	task,
	ParserContext&		context,
	bool				srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseFunctionBuild_FromExecBody");
#endif
	// The current token should be an execution body opener. We can check to verify.
#ifdef COPPER_STRICT_CHECKS
	if ( context.peekAtToken().type != TT_execbody_open )
		throw ParserTokenException( context.peekAtToken() );
#endif
	// Check for the body close token.
	// If the tokens run out before it is found, request more and restart.
	// Otherwise, error.

	// For counting brackets:
	UInteger openBrackets = 1;

	do {
		// Start inside the execbody open token
		if ( ! context.moveToNextToken() ) {
			if ( srcDone ) {
				print(LogLevel::error, EngineMessage::ExecBodyUnfinished);
				return ParseTask::Result::syntax_error;
			}
			return ParseTask::Result::need_more;
		}

		switch( context.peekAtToken().type )
		{
		case TT_execbody_open:
			++openBrackets;
			if ( openBrackets == PARSER_OPENBODY_MAX_COUNT ) {
				print(LogLevel::error, EngineMessage::ExceededBodyCountLimit);
				return ParseTask::Result::syntax_error;
			}
			break;

		case TT_execbody_close:
			--openBrackets;
			break;

		default: break;
		}

#ifdef COPPER_PARSER_LEVEL_MESSAGES
		std::printf("[DEBUG: Engine::ParseFunctionBuild_FromExecBody - openBrackets = %u\n", openBrackets);
#endif

	} while ( openBrackets > 0 );	

	// Everything worked out
	Opcode* bodyOpcode = new Opcode(Opcode::FuncBuild_execBody);
	context.moveToFirstUnusedToken();
#ifdef COPPER_STRICT_CHECKS
	if ( context.peekAtToken().type != TT_execbody_open )
		throw ParserTokenException( context.peekAtToken() );
#endif
	openBrackets = 1;
	do {
		context.moveToNextToken(); // Start inside the body
		switch( context.peekAtToken().type )
		{
		case TT_execbody_open:
			++openBrackets;
			break;

		case TT_execbody_close:
			--openBrackets;
			break;

		default: break;
		}

		if ( openBrackets > 0 ) {
			// Add token to body
			bodyOpcode->addToken(context.peekAtToken());
		}

	} while ( openBrackets > 0 );	

	context.commitTokenUsage(); // Eat all these tokens
	context.addNewOperation( bodyOpcode );
	context.addNewOperation( new Opcode( Opcode::FuncBuild_end ) );
	return ParseTask::Result::task_done;
}



//Note to self:
//	FunctionFound should error if it does not have enough tokens (srcDone == true).
//	Note: This is the same with assignment, which is REQUIRED to check for the value being
//	assigned to it, including the entirety of the object-function.
//	Object functions that end as just objects are acceptable, as well as function bodies without
//	object/parameter heads.

ParseTask::Result::Value
Engine::parseFuncFoundTask(
	FuncFoundParseTask*	task,
	ParserContext&		context,
	bool				srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::parseFuncFoundTask");
#endif
	ParseTask::Result::Value r;

	switch( task->state ) {
	case FuncFoundParseTask::Start:
		// Get next token, which is the first one after the name
		r = moveToFirstUnusedToken(context, srcDone);
		if ( r != ParseTask::Result::task_done )
			return r;

		// It is possible that this is a chain of member names, connected by the member link.
		while( context.peekAtToken().type == TT_member_link ) {
			// Get the next name and restart
			if ( context.moveToNextToken() ) {
				if ( context.peekAtToken().type == TT_name ) {
					task->code->appendAddressData( context.peekAtToken().name );
					context.commitTokenUsage();
					// Next token should be member-link, assignment, pointer-assignment, or parambody-open
					if ( context.moveToNextToken() ) {
						continue;
					} else {
						// However, this may be a function call or assignment. There is no way of knowing what
						// comes next, so it is imperative that we find out.
						return ParseTask::Result::need_more;
					}
				} else {
					print(LogLevel::error, EngineMessage::InvalidTokenAfterMemberAccessor);
					return ParseTask::Result::syntax_error;
				}
			} else {
				if ( srcDone ) {
					//print(LogLevel::error, "Member access incomplete.");
					print(LogLevel::error, EngineMessage::InvalidTokenAfterMemberAccessor);
					return ParseTask::Result::syntax_error;
				} else {
					return ParseTask::Result::need_more;
				}
			}
		}

		switch( context.peekAtToken().type ) {
		case TT_assignment:
			task->state = FuncFoundParseTask::ValidateAssignment;
			context.commitTokenUsage(); // Keep the assignment symbol
			if ( context.moveToNextToken() ) {
				return ParseFuncFound_ValidateAssignment(task, context);
			} else {
				if ( srcDone ) {
					//print(LogLevel::error, "Variable assignment incomplete.");
					print(LogLevel::error, EngineMessage::InvalidAsgn);
					return ParseTask::Result::syntax_error;
				} else {
					return ParseTask::Result::need_more;
				}
			}

		case TT_pointer_assignment:
			task->state = FuncFoundParseTask::ValidatePointerAssignment;
			context.commitTokenUsage(); // Keep the assignment symbol
			if ( context.moveToNextToken() ) {
				return ParseFuncFound_ValidatePointerAssignment(task, context);
			} else {
				if ( srcDone ) {
					//print(LogLevel::error, "Variable pointer assignment incomplete.");
					print(LogLevel::error, EngineMessage::InvalidPtrAsgn);
					return ParseTask::Result::syntax_error;
				} else {
					return ParseTask::Result::need_more;
				}
			}

		//case TT_member_link: // Handled above

		case TT_parambody_open:
			context.commitTokenUsage(); // Keep the parameter body open token
			task->state = FuncFoundParseTask::VerifyParams;
			task->code->setType( Opcode::FuncFound_call );
			context.addOperation( task->code );
			if ( context.moveToNextToken() ) {
				return ParseFuncFound_VerifyParams(task, context, srcDone);
			} else {
				return ParseTask::Result::need_more;
			}

		case TT_immediate_run:
			context.commitTokenUsage();
			task->code->setType( Opcode::FuncFound_call );
			context.addOperation( task->code );
			context.addNewOperation( new Opcode( Opcode::FuncFound_finishCall ) );
			context.addNewOperation( new Opcode( Opcode::Terminal ) ); // For incrementing to after the function call
			return ParseTask::Result::task_done;

		// case TT_name:
		// NOTE: We can't handle other names starting new FuncFound tasks here because
		// function-building might be using them for parameter names.

		default:
			// At the very least, we have a member-access call
			context.addOperation( task->code );
			// Don't eat this token
			context.moveToPreviousToken();
			context.commitTokenUsage();
			return ParseTask::Result::task_done;
		}

	case FuncFoundParseTask::ValidateAssignment:
		r = moveToFirstUnusedToken(context, srcDone);
		if ( r != ParseTask::Result::task_done )
			return r;
		return ParseFuncFound_ValidateAssignment(task, context);

	case FuncFoundParseTask::ValidatePointerAssignment:
		r = moveToFirstUnusedToken(context, srcDone);
		if ( r != ParseTask::Result::task_done )
			return r;
		return ParseFuncFound_ValidatePointerAssignment(task, context);

	case FuncFoundParseTask::CompleteAssignment:
		task->code->setType( Opcode::FuncFound_assignment );
		context.addOperation( task->code );
		return ParseTask::Result::task_done;

	case FuncFoundParseTask::CompletePointerAssignment:
		task->code->setType( Opcode::FuncFound_pointerAssignment );
		context.addOperation( task->code );
		return ParseTask::Result::task_done;

	case FuncFoundParseTask::VerifyParams:
		r = moveToFirstUnusedToken(context, srcDone);
		if ( r != ParseTask::Result::task_done )
			return r;
		//task->code->setType( Opcode::FuncFound_call );
		return ParseFuncFound_VerifyParams(task, context, srcDone);

	case FuncFoundParseTask::CollectParams:
		r = moveToFirstUnusedToken(context, srcDone);
		if ( r != ParseTask::Result::task_done )
			return r;
		// Need to collect parameters until the parameter body closing is found.
		// This means keeping track of brackets.
		return ParseFuncFound_CollectParams(task, context);

	default:
		throw BadParserStateException();
	}
}

ParseTask::Result::Value
Engine::ParseFuncFound_ValidateAssignment(
	FuncFoundParseTask*	task,
	ParserContext&		context
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseFuncFound_ValidateAssignment");
#endif
	// This task really needs to cause the VM to wait until the last object is set before it can proceed.

	// Check tokens for valid assignable objects.
	switch( context.peekAtToken().type ) {
	case TT_objectbody_open:
	case TT_execbody_open:
		// ^ TT_objectbody_open and TT_execbody_open don't trigger completeness verification because
		// the function building task itself will return an error if there are not enough tokens.
	case TT_name:
	case TT_boolean_true:
	case TT_boolean_false:
	case TT_string:
	case TT_num_integer:
	case TT_num_decimal:
		task->state = FuncFoundParseTask::CompleteAssignment;
		return ParseTask::Result::interpret_token;

	default:
		//print(LogLevel::error, "Invalid token following assignment.");
		print(LogLevel::error, EngineMessage::InvalidAsgn);
		return ParseTask::Result::syntax_error;
	}
}

ParseTask::Result::Value
Engine::ParseFuncFound_ValidatePointerAssignment(
	FuncFoundParseTask*	task,
	ParserContext&		context
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseFuncFound_ValidatePointerAssignment");
#endif
	// This task really needs to cause the VM to wait until the last object is set before it can proceed.

	switch( context.peekAtToken().type ) {
	//case TT_objectbody_open:
	//case TT_execbody_open:
		// ^ Should be handled but aren't allowed yet for this VM. (See also function-build task.)
	case TT_name:
		task->state = FuncFoundParseTask::CompletePointerAssignment;
		return ParseTask::Result::interpret_token;

	default:
		//print(LogLevel::error, "Invalid token following pointer assignment.");
		print(LogLevel::error, EngineMessage::InvalidPtrAsgn);
		return ParseTask::Result::syntax_error;
	}
}

ParseTask::Result::Value
Engine::ParseFuncFound_VerifyParams(
	FuncFoundParseTask*	task,
	ParserContext&		context,
	bool				srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseFuncFound_VerifyParams");
	//std::printf("[ currToken.name = %s\n", context.peekAtToken().name.c_str());
#endif
	// Only for scanning
	UInteger openBodies = 1;

	do {
		switch( context.peekAtToken().type ) {
		case TT_parambody_open:
			if ( openBodies == PARSER_OPENBODY_MAX_COUNT ) {
				print( LogLevel::error, EngineMessage::ExceededParamBodyCountLimit );
				return ParseTask::Result::syntax_error;
			}
			++openBodies;
			break;
		case TT_parambody_close:
			openBodies--;
			if ( openBodies == 0 ) {
				task->state = FuncFoundParseTask::CollectParams;
				task->openBodies = 1;
				context.moveToFirstUnusedToken(); // Start from the beginning
				return ParseFuncFound_CollectParams(task, context);
			}
			break;
		default: // Ignore
			break;
		}
	} while ( context.moveToNextToken() );

	//( openBodies > 0 )
	if ( srcDone ) {
		//print(LogLevel::error, "Function call argument collection incomplete.");
		print(LogLevel::error, EngineMessage::ArgBodyIncomplete);
		return ParseTask::Result::syntax_error;
	} else {
		return ParseTask::Result::need_more;
	}
}

ParseTask::Result::Value
Engine::ParseFuncFound_CollectParams(
	FuncFoundParseTask*	task,
	ParserContext&		context
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseFuncFound_CollectParams");
	//std::printf("[ currToken.name = %s\n", context.peekAtToken().name.c_str());
#endif
	// Current parsing state: After the first parameter-body-opener token.

	if ( task->waitingOnAssignment ) {
		context.addNewOperation(
			new Opcode( Opcode::FuncFound_setParam )
		);
		task->waitingOnAssignment = false;
	}

	// Ignore end-segment tokens
	while ( context.peekAtToken().type == TT_end_segment ) {
		context.commitTokenUsage();
		context.moveToNextToken();
	}

	switch( context.peekAtToken().type ) {
	case TT_parambody_open:
		task->openBodies++;
		return ParseTask::Result::interpret_token;

	case TT_parambody_close:
		task->openBodies--;
		if ( task->openBodies == 0 ) {
			context.commitTokenUsage();
			context.addNewOperation( new Opcode( Opcode::FuncFound_finishCall ) );
			context.addNewOperation( new Opcode( Opcode::Terminal ) ); // for incrementing to after the call
			return ParseTask::Result::task_done;
		}
		return ParseTask::Result::interpret_token;

	case TT_objectbody_open:
	case TT_execbody_open:
	case TT_name:
	case TT_boolean_true:
	case TT_boolean_false:
	case TT_string:
	case TT_num_integer:
	case TT_num_decimal:
		task->waitingOnAssignment = true;
		return ParseTask::Result::interpret_token;

	default:
		//print(LogLevel::error, "Invalid token in function call parameter parsing.");
		print(LogLevel::error, EngineMessage::InvalidFunctionArguments);
		return ParseTask::Result::syntax_error;
	}
}

/*
Basic If-structure:
- Terminal
- _ // condition
- ConditionalGoto [2nd terminal]
- _ // body
- Goto [last terminal]
- Terminal

Basic If-structure with else:
- Terminal
- _ // condition
- ConditionalGoto [2nd terminal]
- _ // body
- Goto [last terminal] // end of if-statement
- Terminal // else
- _ // body
- Terminal

Basic If-structure with elif:
- Terminal
- _ // condition
- ConditionalGoto [2nd terminal]
- _ // body
- Goto [last terminal] // end of if-statement
- Terminal // elif
- _ // condition
- ConditionalGoto [3rd terminal]
- _ // body
- Goto [last terminal] // end of elif
- Terminal // else
- _ // body
- Terminal
*/

ParseTask::Result::Value
Engine::parseIfStructure(
	IfStructureParseTask*	task,
	ParserContext&			context,
	bool					srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::parseIfStructure");
	//std::printf("[DEBUG: Engine::parseIfStructure state = %u\n", (unsigned)(task->state));
#endif

	// If the stream ends, the if-statement can quit.

	ParseTask::Result::Value r = moveToFirstUnusedToken(context, srcDone);
	if ( r != ParseTask::Result::task_done ) {
		return r;
	}

	switch( task->state ) {

	case IfStructureParseTask::Start:
		return ParseIfStructure_InitScan(task, context, srcDone);

	case IfStructureParseTask::CreateCondition:
		return ParseIfStructure_CreateCondition(task, context, srcDone);

	case IfStructureParseTask::CreateBody:
		return ParseIfStructure_CreateBody(task, context, srcDone);

	case IfStructureParseTask::PostBody:
		// This section performs appending of last opcodes to strand
		return ParseIfStructure_PostBody(task, context, srcDone);

	default:
		throw BadParserStateException();
	}

	// Before popping, this task must add a final terminal that all the remaining goto's can point to.
	// You will need to set all of these codes before returning task_done.
}

// Checks to make sure the condition body is enclosed by parambody tokens
// and the execution body is enclosed by execbody tokens
ParseTask::Result::Value
Engine::ParseIfStructure_InitScan(
	IfStructureParseTask*	task,
	ParserContext&			context,
	bool					srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseIfStructure_InitScan");
#endif
	// Current parser state: "if" (or "elif") has been found, but nothing else that should follow has been yet.
	// Search for the condition body AND the execution body

	// Get the opening of the condition body, which should be the current token
	if ( context.peekAtToken().type != TT_parambody_open ) {
		//print(LogLevel::error, "If-structure must have parameter body.");
		print(LogLevel::error, EngineMessage::InvalidTokenAtIfStart);
		return ParseTask::Result::syntax_error;
	}

	UInteger openBodies = 1;
	bool firstParamFound = false;

	while ( context.moveToNextToken() ) {
		if ( !firstParamFound ) {
			// First token MUST be something that will set the condition.
			// Otherwise, we have a leak where an empty if-statement will use data
			// outside the conditional region as its condition.
			switch( context.peekAtToken().type ) {
			case TT_boolean_true:
			case TT_boolean_false:
			case TT_name:
				break;
			default:
				//print(LogLevel::error, "If-structure conditional must start with boolean value or name.");
				print(LogLevel::error, EngineMessage::IfConditionContaminated);
				return ParseTask::Result::syntax_error;
			}
			firstParamFound = true;
		}

		switch( context.peekAtToken().type ) {
		case TT_parambody_open:
			if ( openBodies == PARSER_OPENBODY_MAX_COUNT ) {
				print( LogLevel::error, EngineMessage::ExceededParamBodyCountLimit );
				return ParseTask::Result::syntax_error;
			}
			++openBodies;
			break;

		case TT_parambody_close:
			--openBodies;
			if ( openBodies == 0 ) {
				// All done with condition
				return ParseIfStructure_ScanExecBody(task, context, srcDone);
			}
			break;

		default:
			break;
		}
	}
	if ( srcDone ) {
		//print(LogLevel::error, "Incomplete if-structure condition.");
		print(LogLevel::error, EngineMessage::StreamHaltedParsingNotDone);
		return ParseTask::Result::syntax_error;
	} else {
		return ParseTask::Result::need_more;
	}
}

// Checks to make sure the execution body is enclosed by execbody tokens
ParseTask::Result::Value
Engine::ParseIfStructure_ScanExecBody(
	IfStructureParseTask*	task,
	ParserContext&			context,
	bool					srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseIfStructure_ScanExecBody");
#endif
	// Current parser state: Condition body has been verified as complete (unless this is an "else"),
	// and now the execution body completeness needs to be verified
	// and no tokens within either body (condition or execution) have been processed.
	// NOTE: TOKENS USAGE HAS NOT BEEN COMMITTED
	UInteger openBodies = 1;

	// Move from ")"/"else" to "{"
	if ( !context.moveToNextToken() ) {
		if ( srcDone ) {
			//print(LogLevel::error, "Incomplete if-structure body.");
			print(LogLevel::error, EngineMessage::StreamHaltedParsingNotDone);
			return ParseTask::Result::syntax_error;
		} else {
			return ParseTask::Result::need_more;
		}
	}
	if ( context.peekAtToken().type != TT_execbody_open ) {
		//print(LogLevel::error, "Illegal token following if-structure condition or start of else-section.");
		print(LogLevel::error, EngineMessage::StrayTokenInLoopHead);
		return ParseTask::Result::syntax_error;
	}

	while ( context.moveToNextToken() ) {
		switch( context.peekAtToken().type ) {
		case TT_execbody_open:
			if ( openBodies == PARSER_OPENBODY_MAX_COUNT ) {
				print( LogLevel::error, EngineMessage::ExceededBodyCountLimit );
				return ParseTask::Result::syntax_error;
			}
			++openBodies;
			break;

		case TT_execbody_close:
			--openBodies;
			if ( openBodies == 0 ) {
				// All done with body

				// Else takes a different route
				if ( task->atElse )
					return ParseTask::Result::interpret_token;

				// Finally, the state can be changed and the first parameter body token can be viewed
				// Skip the if-structure "if" and the open-parameter-body token (both known to be unused)
				context.moveToFirstUnusedToken();	// "(" following the "if"
				context.commitTokenUsage();
				context.moveToNextToken();			// Token after "("
				task->openBodies = 1;
				task->state = IfStructureParseTask::CreateCondition;
				return ParseIfStructure_CreateCondition(task, context, srcDone);
			}
			break;

		default:
			break;
		}
	}
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	Token t = context.peekAtToken();
	std::printf("[DEBUG: Engine::ParseIfStructure_ScanExecBody: Incomplete body, ending in token: %s, %u\n", t.name.c_str(), t.type);
#endif
	if ( srcDone ) {
		//print(LogLevel::error, "Incomplete if-structure body.");
		print(LogLevel::error, EngineMessage::StreamHaltedParsingNotDone);
		return ParseTask::Result::syntax_error;
	} else {
		return ParseTask::Result::need_more;
	}
}

// Lets a token generate an object and readies a conditional jump in case of false
ParseTask::Result::Value
Engine::ParseIfStructure_CreateCondition(
	IfStructureParseTask*	task,
	ParserContext&			context,
	bool					srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseIfStructure_CreateCondition");
#endif
	// State: Both conditional and execution bodies have been verified as complete.
	// The first parambody_open token has been skipped.
	// The first unused token has been moved to.
	Opcode* code;

	// Keep an eye out for parameter body openers and closers, tracking them in the task
	switch( context.peekAtToken().type ) {
	case TT_parambody_open: // Funny, this is actually an error, but interpretToken() handles it.
		task->openBodies++;
		break;

	case TT_parambody_close:
		task->openBodies--;
		if ( task->openBodies == 0 ) {
			code = new Opcode(Opcode::ConditionalGoto);
			context.addNewOperation( code );
			// Since the code's jump has not been set yet, add it to the queue for setting
			task->queueNewConditionalJump(code); // Jumps to the next section of the if-structure
			// Capture this and the next token
			context.moveToNextToken(); // Should be the execution body opener
#ifdef COPPER_STRICT_CHECKS
			if ( context.peekAtToken().type != TT_execbody_open )
				throw ParserTokenException( context.peekAtToken() );
#endif
			// Keep both of these
			context.commitTokenUsage();
			context.moveToFirstUnusedToken();
			task->openBodies = 1;
			// Change state (needed for when parsing must pause to wait for more tokens)
			task->state = IfStructureParseTask::CreateBody;
			return ParseIfStructure_CreateBody(task, context, srcDone);
		}
		break;

	default:
		break;
	}
	return ParseTask::Result::interpret_token;
}

ParseTask::Result::Value
Engine::ParseIfStructure_CreateBody(
	IfStructureParseTask*	task,
	ParserContext&			context,
	bool					srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseIfStructure_CreateBody");
#endif
	// State: One execution token has been grabbed and the existence of its match/pair has been verified
	Opcode* code;

	switch( context.peekAtToken().type ) {
	case TT_execbody_open:
		task->openBodies++;
		break;

	case TT_execbody_close:
		task->openBodies--;
		if ( task->openBodies == 0 ) {
			context.commitTokenUsage(); // Use this body-closing token
			// Create an opcode at the end of the body for jumping to the end of the if-structure
			code = new Opcode(Opcode::Goto);
			context.addNewOperation( code );
			task->queueNewFinalJumpCode(code);

			if ( task->atElse ) {
				task->finalizeGotos( context );
				return ParseTask::Result::task_done;
			}

			// Add terminal and set conditional-goto
			task->finalizeConditionalGoto( context );

			// Skip to the next token
			// If there is no next token, end here.
			if ( ! context.moveToNextToken() ) {
				if ( srcDone ) {
					// End of if-structure
					// Add final terminal and sets gotos
					task->finalizeGotos( context );
					return ParseTask::Result::task_done;
				} else {
					task->state = IfStructureParseTask::PostBody;
					return ParseTask::Result::need_more;
				}
			}
			task->state = IfStructureParseTask::PostBody;
			return ParseIfStructure_PostBody(task, context, srcDone);
		}
		break;

	default:
		break;
	}
	return ParseTask::Result::interpret_token;
}

ParseTask::Result::Value
Engine::ParseIfStructure_PostBody(
	IfStructureParseTask*	task,
	ParserContext&			context,
	bool					srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseIfStructure_PostBody");
#endif
	// State: Searching for "elif" and "else" after the execbody close

	switch( context.peekAtToken().type ) {
	case TT_elif:
		// Keep the "elif"
		context.commitTokenUsage();
		task->state = IfStructureParseTask::Start;
		if ( ! context.moveToNextToken() ) {
			if ( srcDone ) {
				//print(LogLevel::error, "Elif structure is incomplete.");
				print(LogLevel::error, EngineMessage::StreamHaltedParsingNotDone);
			} else {
				return ParseTask::Result::need_more;
			}
		}
		return ParseIfStructure_InitScan(task, context, srcDone);
		
	case TT_else:
		task->atElse = true;
		switch( ParseIfStructure_ScanExecBody(task, context, srcDone) ) {
		case ParseTask::Result::interpret_token:
			// Commit token collection
			context.moveToFirstUnusedToken();	// "else"
			context.moveToNextToken();			// "{"
			context.commitTokenUsage();
			task->openBodies = 1;

			//context.addNewOperation( new Opcode(Opcode::Terminal) );
			// Change the task to the correct one
			task->state = IfStructureParseTask::CreateBody;
			return ParseTask::Result::interpret_token;

		case ParseTask::Result::need_more:
			return ParseTask::Result::need_more;

		case ParseTask::Result::syntax_error:
		default:
			return ParseTask::Result::syntax_error;
		}

	default:
		task->finalizeGotos( context );
		return ParseTask::Result::task_done;
	}
}

ParseTask::Result::Value
Engine::parseLoopStructure(
	LoopStructureParseTask*	task,
	ParserContext&			context,
	bool					srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::parseLoopStructure");
#endif

	switch( task->state ) {

	case LoopStructureParseTask::Start:
		// First token is the loop
#ifdef COPPER_STRICT_CHECKS
		if ( context.peekAtToken().type != TT_loop )
			throw ParserTokenException( context.peekAtToken() );
#endif
		// All loops must begin with "loop" + execution body token opener and end with the body closer
		// Check for the starting body token:
		if ( !context.moveToNextToken() && srcDone ) {
			//print(LogLevel::error, "Loop-structure not complete before stream end.");
			print(LogLevel::error, EngineMessage::StreamHaltedParsingNotDone);
			return ParseTask::Result::syntax_error;
		}
		if ( context.peekAtToken().type != TT_execbody_open ) {
			//print(LogLevel::error, "Loop-structure encountered bad token before body.");
			print(LogLevel::error, EngineMessage::StrayTokenInLoopHead);
			return ParseTask::Result::syntax_error;
		}
		// Loop head found
		context.commitTokenUsage();
		// Note: Loop start terminal was added when the "loop" token was found.

		// Scan the body first to make sure that it's all there. Once that's confirmed, I can collect the tokens.
		task->state = LoopStructureParseTask::CollectBody;

	case LoopStructureParseTask::CollectBody:

		while ( context.moveToNextToken() ) {

			switch( context.peekAtToken().type ) {
			case TT_execbody_open:
				task->openBodies++;
				if ( task->openBodies == PARSER_OPENBODY_MAX_COUNT ) {
					print(LogLevel::error, EngineMessage::ExceededBodyCountLimit );
					return ParseTask::Result::syntax_error;
				}
				break;

			case TT_execbody_close:
				task->openBodies--;
				if ( task->openBodies == 0 ) {
					task->openBodies = 1; // Reset to allow AwaitFinish to use it.
					task->state = LoopStructureParseTask::AwaitFinish;
					return ParseLoop_AwaitFinish(task, context);
				}
				break;

			default:
				break;
			}
		}

		if ( srcDone ) {
			//print(LogLevel::error, "Stream ended before loop structure was completed.");
			print(LogLevel::error, EngineMessage::StreamHaltedParsingNotDone);
			return ParseTask::Result::syntax_error;
		} else {
			return ParseTask::Result::need_more;
		}

	case LoopStructureParseTask::AwaitFinish:
		return ParseLoop_AwaitFinish(task, context);

	default:
		throw BadParserStateException();
	}
}

ParseTask::Result::Value
Engine::ParseLoop_AwaitFinish(
	LoopStructureParseTask*	task,
	ParserContext&			context
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseLoop_AwaitFinish");
#endif
	Opcode* code = REAL_NULL;

	ParseTask::Result::Value r = moveToFirstUnusedToken(context, false);
	if ( r != ParseTask::Result::task_done )
		return r;

	switch( context.peekAtToken().type ) {
	case TT_execbody_open:
		task->openBodies++;
		break;

	case TT_execbody_close:
		task->openBodies--;
		if ( task->openBodies == 0 ) {
			// Take this last body token
			context.commitTokenUsage();
			// Add the re-loop goto
			code = new Opcode(Opcode::Goto);
			task->setGotoOpcodeToLoopStart(code);
			context.addNewOperation(code);

			// Add the loop completion terminal
			context.addNewOperation( new Opcode(Opcode::Terminal) );
			task->finalizeGotos( context.outputStrand );
			return ParseTask::Result::task_done;
		}
		break;

	default:
		break;
	}
	return ParseTask::Result::interpret_token;
}

ParseResult::Value
Engine::ParseLoop_AddEndLoop(
	ParserContext& context
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseLoop_AddEndLoop");
#endif
#ifdef COPPER_STRICT_CHECKS
	if ( context.peekAtToken().type != TT_endloop )
		throw ParserTokenException( context.peekAtToken() );
#endif
	if ( context.taskStack.size() == 0 ) {
		//print(LogLevel::error, "Loop-stop token found outside a loop.");
		print(LogLevel::error, EngineMessage::UselessLoopStopper);
		return ParseResult::Error;
	}

	context.commitTokenUsage(); // Use this token
	// Must search for the top-most loop task in the parser task stack
	Opcode* code = REAL_NULL;
	ParseTaskIter taskIter = context.taskStack.end();
	ParseTask* task;
	do {
		task = taskIter->getTask();
		if ( task->type == ParseTask::Loop ) {
			code = new Opcode(Opcode::Goto);
			((LoopStructureParseTask*)task)->queueFinalGoto(code);
			context.addNewOperation(code);
			// Unfortunately, adding the code as a new operation would delink it from the
			// final goto queue if Opcodes are copied in the main constructor of OpcodeContainer.
			// Hence, OpcodeContainer must use references.
			return ParseResult::Done;
		}
	} while ( taskIter.prev() );
	//print(LogLevel::error, "Loop-stop token found outside a loop.");
	print(LogLevel::error, EngineMessage::UselessLoopStopper);
	return ParseResult::Error;
}

ParseResult::Value
Engine::ParseLoop_AddLoopSkip(
	ParserContext&			context
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseLoop_AddLoopSkip");
#endif
#ifdef COPPER_STRICT_CHECKS
	if ( context.peekAtToken().type != TT_skip )
		throw ParserTokenException( context.peekAtToken() );
#endif
	if ( context.taskStack.size() == 0 ) {
		//print(LogLevel::error, "Loop-stop token found outside a loop.");
		print(LogLevel::error, EngineMessage::UselessLoopSkipper);
		return ParseResult::Error;
	}

	context.commitTokenUsage(); // Use this token
	// Must search for the top-most loop task in the parser task stack
	Opcode* code;
	ParseTaskIter taskIter = context.taskStack.end();
	ParseTask* task;
	do {
		task = taskIter->getTask();
		if ( task->type == ParseTask::Loop ) {
			code = new Opcode(Opcode::Goto);
			((LoopStructureParseTask*)task)->setGotoOpcodeToLoopStart(code);
			context.addNewOperation(code);
			return ParseResult::Done;
		}
	} while ( taskIter.prev() );
	//print(LogLevel::error, "Loop-skip token found outside a loop.");
	print(LogLevel::error, EngineMessage::UselessLoopSkipper);
	return ParseResult::Error;
}

ParseTask::Result::Value
Engine::parseSRPS(
	SRPSParseTask*			task,
	ParserContext&			context,
	bool					srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::parseSRPS");
#endif
	// Current parsing state: starting (name) token found, but nothing else

	// Scan for parameter body first
	if ( !context.moveToFirstUnusedToken() ) {
		if ( srcDone ) {
			//print(LogLevel::error, "SRPS (own/is_owner/is_ptr) is missing parameter body.");
				print(LogLevel::error, EngineMessage::StreamHaltedParsingNotDone);
			return ParseTask::Result::syntax_error;
		} else {
			return ParseTask::Result::need_more;
		}
	}

	if ( context.peekAtToken().type != TT_parambody_open ) {
		//print(LogLevel::error, "SRPS (own/is_owner/is_ptr) is missing parameter body.");
		print(LogLevel::error, EngineMessage::InvalidTokenBeforePtrHandlerParamBody);
		return ParseTask::Result::syntax_error;
	}

	// Next: scan for closing of parameter body
	UInteger openBodies = 1;
	while ( openBodies != 0 ) {
		if ( !context.moveToNextToken()) {
			if ( srcDone ) {
				//print(LogLevel::error, "SRPS (own/is_owner/is_ptr) is missing parameter body.");
				print(LogLevel::error, EngineMessage::StreamHaltedParsingNotDone);
				return ParseTask::Result::syntax_error;
			} else {
				return ParseTask::Result::need_more;
			}
		}
		switch( context.peekAtToken().type ) {
		case TT_parambody_open:
			if ( openBodies == PARSER_OPENBODY_MAX_COUNT ) {
				print(LogLevel::error, EngineMessage::ExceededBodyCountLimit );
				return ParseTask::Result::syntax_error;
			}
			openBodies++;
			break;

		case TT_parambody_close:
			openBodies--;
			break;

		default:
			break;
		}
	}
	// Now that the parameter body has been verified, iteration through it can proceed without pause
	context.moveToFirstUnusedToken();
	// Skip the parambody_open token
	context.moveToNextToken();
	// The next token should be name for the variable address
	if ( context.peekAtToken().type != TT_name ) {
		//print(LogLevel::error, "SRPS (own/is_owner/is_ptr) contains invalid token for parameter.");
		print(LogLevel::error, EngineMessage::NonNameFoundInPtrHandlerParamBody);
		return ParseTask::Result::syntax_error;
	}

	// It'd be faster if I saved directly to the opcode instead
	//VarAddress varAddress;
	Opcode* addressCode = new Opcode( task->codeType );

	bool loop = true;
	while ( context.peekAtToken().type == TT_name && loop ) {
		addressCode->appendAddressData( context.peekAtToken().name );
		context.moveToNextToken();
		switch( context.peekAtToken().type ) {
		case TT_parambody_close:
			loop = false;
			break;

		case TT_member_link:
			context.moveToNextToken();
			continue;

		// If more than one variable is allowed, use the following.
		// Note: You will also need to change Engine::run_Own(), Engine::run_Is_owner(), Engine::run_Is_pointer(), etc.
		//case TT_name:
		//	context.moveToNextToken();
		//	continue;

		default:
			//print(LogLevel::error, "SRPS (own/is_owner/is_ptr) contains invalid token for extra parameter.");
			print(LogLevel::error, EngineMessage::InvalidTokenForEndingPtrHandlerParamBody);
			delete addressCode;
			return ParseTask::Result::syntax_error;
		}
	}

	context.commitTokenUsage();
	context.addNewOperation( addressCode );
	return ParseTask::Result::task_done;
}


//============ Engine: operation execution code ==========

void
Engine::addNewTaskToStack(
	Task* t
) {
	//if ( isNull(t) )
	//	throw NullTaskException();
	taskStack.push_back(TaskContainer(t));
	// TaskContainer does NOT reference count in its default constructor.
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	std::printf("[DEBUG TaskStack.size = %lu\n", taskStack.size());
#endif
}

Task*
Engine::getLastTask() {
	if ( ! taskStack.has() )
		throw EmptyTaskStackException();
	return &(taskStack.getLast().getTask());
}

void
Engine::popLastTask() {
	taskStack.pop();
}

void
Engine::addOpStrandToStack(
	OpStrand*	strand
) {
	if ( isNull(strand) )
		throw EmptyOpstrandException();
	opcodeStrandStack.push_back( OpStrandContainer(strand) );
}



EngineResult::Value
Engine::execute() {
	return execute(opcodeStrandStack);	
}

EngineResult::Value
Engine::execute( OpStrandStack&  srcOpcodeStrandStack ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::execute");
#endif

#ifdef COPPER_SPEED_PROFILE
	std::printf("PROFILE Engine::execute() start\n");
	fullTime = 0;
	timespec startTime, endTime;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime);
#endif


	// Note: When adding a list of operations, also set currOp.
	OpStrandStackIter opcodeStrandStackIter = srcOpcodeStrandStack.end();
	if ( ! opcodeStrandStackIter.has() )
		return EngineResult::Ok;

	//opcodeStrandStack.start()->getCurrStrand()->validate();

	// This could cause problems as the same opcode might be run twice if the execution is paused
	// or stopped due to lack of tokens:
	OpStrandIter* currOp;
	// To counter this, a Terminal opcode is appended to the end of the global strand.

	bool hasNextToken;
	do {
		currOp = &(opcodeStrandStackIter->getCurrOp());

		if ( currOp->has() ) {
			// Otherwise, process
			do {
				hasNextToken = true;

				switch( operate( opcodeStrandStackIter, *currOp ) ) {
				case ExecutionResult::Ok:
					break;

				case ExecutionResult::Error:
					return EngineResult::Error;

				case ExecutionResult::Done:
					clearStacks();
					signalEndofProcessing();

#ifdef COPPER_SPEED_PROFILE
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime);
	std::printf("done time sec(%ld) ns(%ld)\n", (endTime.tv_sec - startTime.tv_sec), (endTime.tv_nsec - startTime.tv_nsec) );
#endif

					return EngineResult::Done;

				case ExecutionResult::Reset:
					currOp = &(opcodeStrandStackIter->getCurrOp());
					currOp->reset();
					continue;

				default:
					// Should throw
					break;
				}

				hasNextToken = currOp->next();
			} while ( hasNextToken );
		}

		// A user function may have been called at the end of another function,
		// which results in the current operation iterator being at the end of the strand
		// when execute returns to it. This would cause the last opcode to execute again,
		// which is wrong. To avoid this, we continue to pop the stack until the global
		// strand level. The global strand must not be popped, so we just add a terminal
		// and go to that.
		do {
			if ( opcodeStrandStackIter.atStart() ) {
				// At the global level, so don't pop.
				if ( opcodeStrandStack.isSame( srcOpcodeStrandStack ) ) {
					// The engine's default strand stack is also the current one, so add a terminal.
					// (Otherwise, there would be a memory leak.)
					// This is also for preventing all other operations from being repeated.
					globalParserContext.addNewOperation( new Opcode(Opcode::Terminal) );
					currOp->next(); //... and go to it.
				}
				break;
			}

			// Not at the global opcodes strand, so it's safe to pop the strand
			// Functions should pop variable/scope stack contexts.
			stack.pop();
			srcOpcodeStrandStack.pop();
			opcodeStrandStackIter.makeLast();
			currOp = &(opcodeStrandStackIter->getCurrOp());

		} while ( opcodeStrandStackIter->getCurrOp().atEnd() );

	} while ( ! opcodeStrandStackIter.atStart() || ! currOp->atEnd() );

	//printGlobalStrand();

	// Clear all first-level opcodes that have been run up to this point
	srcOpcodeStrandStack.start()->removeAllUpToCurrentCode();

#ifdef COPPER_SPEED_PROFILE
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime);
	std::printf("\nPROFILE\n\tEngine::execute() final time sec(%ld) ns(%ld)\n\tsection full time = %f\n",
				(endTime.tv_sec - startTime.tv_sec), (endTime.tv_nsec - startTime.tv_nsec),
				fullTime);
#endif

	return EngineResult::Ok;
}

ExecutionResult::Value
Engine::operate(
	OpStrandStackIter&	opStrandStackIter,
	OpStrandIter&		opIter // was using Opcode&
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::operate");
#endif
	//const Opcode* opcode = opIter->getOp();
	Opcode* opcode = opIter->getOp();
	Task* task;
	Variable* variable;

	switch( opcode->getType() ) {
	case Opcode::Exit:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode Exit");
#endif
		return ExecutionResult::Done;

	//------- Opcodes for building an object-function

	case Opcode::FuncBuild_start:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode FuncBuild_start");
#endif
		// Should create the function and scope
		addNewTaskToStack( new FuncBuildTask() );
		break;

	case Opcode::FuncBuild_createRegularParam:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode FuncBuild_createRegularParam");
#endif
		task = getLastTask();
		if ( task->name == TaskType::FuncBuild ) {
			((FuncBuildTask*)task)->function->addParam( opcode->getNameData() );
		} else {
			throw BadOpcodeException(Opcode::FuncBuild_createRegularParam);
		}
		break;

	case Opcode::FuncBuild_assignToVar:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode FuncBuild_assignToVar");
#endif
		task = getLastTask();
		if ( task->name == TaskType::FuncBuild ) {
			((FuncBuildTask*)task)->function->getPersistentScope().setVariableFrom(
				opcode->getNameData(),
				lastObject.raw(),
				false
			);
		} else {
			throw BadOpcodeException(Opcode::FuncBuild_assignToVar);
		}
		break;

	case Opcode::FuncBuild_pointerAssignToVar:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode FuncBuild_pointerAssignToVar");
#endif
		task = getLastTask();
		if ( task->name == TaskType::FuncBuild ) {
			((FuncBuildTask*)task)->function->getPersistentScope().setVariableFrom(
				opcode->getNameData(),
				lastObject.raw(),
				true
			);
		} else {
			throw BadOpcodeException(Opcode::FuncBuild_pointerAssignToVar);
		}
		break;

	case Opcode::FuncBuild_execBody:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode FuncBuild_execBody");
#endif
		task = getLastTask();
		if ( task->name == TaskType::FuncBuild ) {
			// Should check for function existence, but if there is no function, there is an internal failure
			((FuncBuildTask*)task)->function->body.set( opcode->getBody() );
		} else {
			throw BadOpcodeException(Opcode::FuncBuild_execBody);
		}
		break;

	case Opcode::FuncBuild_end:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode FuncBuild_end");
#endif
		task = getLastTask();
		if ( task->name == TaskType::FuncBuild ) {
			lastObject.setWithoutRef(
				new FunctionObject( ((FuncBuildTask*)task)->function )
			);
			popLastTask();
		} else {
			throw BadOpcodeException(Opcode::FuncBuild_end);
		}
		break;

	//-------- Opcodes for when an address has been found

	case Opcode::FuncFound_access:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode FuncFound_access");
#endif
		variable = resolveVariableAddress( *(opcode->getAddressData()) );

		if ( notNull(variable) ) {
			lastObject.set( variable->getRawContainer() );
		} else {
			lastObject.setWithoutRef( new FunctionObject() );
		}
		break;

	case Opcode::FuncFound_assignment:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode FuncFound_assignment");
#endif
		setVariableByAddress( *(opcode->getAddressData()), lastObject.raw(), false );
		break;

	case Opcode::FuncFound_pointerAssignment:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode FuncFound_pointerAssignment");
#endif
		setVariableByAddress( *(opcode->getAddressData()), lastObject.raw(), true );
		break;

	case Opcode::FuncFound_call:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode FuncFound_call");
#endif
		//addNewTaskToStack( new FuncFoundTask( *(opcode->getAddressData()) ) );
		addNewTaskToStack( new FuncFoundTask( opcode->getAddressData() ) );
		break;

	case Opcode::FuncFound_setParam:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode FuncFound_setParam");
#endif
		task = getLastTask();
		if ( task->name == TaskType::FuncFound ) {
			((FuncFoundTask*)task)->addArg( lastObject.raw() );
		} else {
			throw BadOpcodeException(Opcode::FuncFound_setParam);
		}
		break;

	case Opcode::FuncFound_finishCall:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode FuncFound_finishCall");
#endif
		task = getLastTask();
		if ( task->name == TaskType::FuncFound ) {
			opIter.next(); // Increment to the Terminal
			switch( setupFunctionExecution(*((FuncFoundTask*)task), opStrandStackIter) ) {

			case FuncExecReturn::Ran:
				popLastTask();
				return ExecutionResult::Ok;

			case FuncExecReturn::Reset:
				popLastTask();
				return ExecutionResult::Reset; // Restart the iterator

			case FuncExecReturn::ErrorOnRun:
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
				print(LogLevel::debug, "[DEBUG: Function call exited with error.");
#endif
				if ( stackTracePrintingEnabled ) {
					printTaskTrace();
					printStackTrace();
				}
				return ExecutionResult::Error;

			case FuncExecReturn::ExitCalled:
				return ExecutionResult::Done;

			case FuncExecReturn::Return:
				popLastTask();
				// Pop opcode stack and reset the iterator or...
				opIter.makeLast(); // have the engine pop the opcode iterator itself
				break;

			case FuncExecReturn::NoMatch:
			default:
				popLastTask();
				// Should never happen
				print(LogLevel::debug, "SYSTEM ERROR: Function failed to call in setupFunctionExecution.");
				//return EngineResult::Error;
				throw BadFuncFoundTaskException();
				break;
			}
		} else {
			throw BadOpcodeException(Opcode::FuncFound_finishCall);
		}
		break;

	//------ Opcodes for if-structures, loops, and goto operations

	case Opcode::Terminal:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode Terminal");
#endif
		break;

	case Opcode::Goto:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode Goto");
#endif
		// Here we cheat and cast to volatile because we don't have a const iterator for the list
		//opIter.set( ((const Opcode*)opcode)->getOpStrandIter() );
		opIter.set( opcode->getOpStrandIter() );
		break;

	case Opcode::ConditionalGoto:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode ConditionalGoto");
#endif
		Object* obj; // Needs to be declared at the beginning of this method
		if ( lastObject.obtain(obj) ) {
			if ( isBoolObject(*obj) ) {
				// Use the inverse of the value because if-statements request jumps when condition is false
				if ( ((BoolObject*)obj)->getValue() == false ) {
					// Here we cheat and cast to volatile because we don't have a const iterator for the list
					//opIter.set( ((const Opcode*)opcode)->getOpStrandIter() );
					opIter.set( opcode->getOpStrandIter() );
				}
			} else {
				//print(LogLevel::warning, "Condition for goto operation is not boolean. Default is false.");
				print(LogLevel::warning, EngineMessage::ConditionlessIf);
				return ExecutionResult::Error;
			}
		} else {
			//print(LogLevel::error, "Missing condition for goto operation.");
			print(LogLevel::error, EngineMessage::ConditionlessIf);
			return ExecutionResult::Error;
		}
		break;

	//------ Opcodes for handling structures that accept a single raw address

	case Opcode::Own:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode Own");
#endif
		return run_Own( *(opcode->getAddressData()) );

	case Opcode::Is_owner:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode Is_owner");
#endif
		return run_Is_owner( *(opcode->getAddressData()) );

	case Opcode::Is_pointer:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode Is_pointer");
#endif
		return run_Is_ptr( *(opcode->getAddressData()) );

	//------ Opcodes for creating basic types

	case Opcode::CreateBoolTrue:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode CreateBoolTrue");
#endif
		lastObject.setWithoutRef(new BoolObject(true));
		break;

	case Opcode::CreateBoolFalse:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode CreateBoolFalse");
#endif
		lastObject.setWithoutRef(new BoolObject(false));
		break;

	case Opcode::CreateString:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode CreateString");
#endif
		// Should be creating strings from a user-set factory
		lastObject.setWithoutRef(
			new StringObject( opcode->getNameData() )
		);
		break;

	case Opcode::CreateInteger:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode CreateInteger");
#endif
		lastObject.setWithoutRef(
			new IntegerObject( opcode->getIntegerData() )
		);
		break;

	case Opcode::CreateDecimal:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode CreateInteger");
#endif
		lastObject.setWithoutRef(
			new DecimalNumObject( opcode->getDecimalData() )
		);
		break;

	default:
		print(LogLevel::error, "Invalid opcode encountered.");
		return ExecutionResult::Error;
	}

	return ExecutionResult::Ok;
}

//! Runs a given function with the passed parameters.
// This is useful for callback functions.
// Originally, it could not be safely run alongside Engine::run() or Engine::execute(), only after them.
// Now it should be able to.
EngineResult::Value
Engine::runFunctionObject(
	FunctionObject*  functionObject,
	util::List<Object*>*  args
) {
	// Slightly-varied version of setupUserFunctionExecution.

	Function*  function;
	if ( ! functionObject->getFunction(function) ) {
		print(LogLevel::error, "(Error during Engine::runFunctionObject): Function object is empty.");
		return EngineResult::Error;
	}

	// Short-circuit for constant-return functions
	if ( function->constantReturn ) {
		lastObject.set( function->result.raw() ); // Copy is created when saved to variable
		return EngineResult::Ok;
	}

	// Set default result value (required especially for empty functions)
	lastObject.setWithoutRef(new FunctionObject());

	// Parse function body if not done yet.
	// If function body contained errors, return error.
	// If function body has been / is parsed, add its opcodes to the stack.
	Body* body;
	if ( ! function->body.obtain(body) ) {
		print(LogLevel::error, "(Error during Engine::runFunctionObject): Function body is missing.");
		return EngineResult::Error;
	}

	if ( body->isEmpty() ) {
		print(LogLevel::debug, "Engine::runFunctionObject: Function body is empty.");
		return EngineResult::Ok;
	}

	// Attempt to compile. Automatically returns true if compiled.
	if ( ! body->compile(this) ) {
		//print(LogLevel::error, "Function body contains errors.");
		print(LogLevel::error, EngineMessage::UserFunctionBodyError);
		return EngineResult::Error;
	}

	// TODO: Should be passed in as a name so that the user can identify the name
	VarAddress* foreignAddr = new VarAddress();
	foreignAddr->push_back("[FOREIGN FUNCTION]");

	// "this" pointer is added to new stack context if the body can be run.
	StackFrame* stackFrame = new StackFrame(foreignAddr);
	foreignAddr->deref();

	// Add "this" pointer
	Variable* callVariable;
	stackFrame->getScope().getVariable(CONSTANT_FUNCTION_SELF, callVariable);
	callVariable->setFunc( functionObject, true );

	// Argument-passing
	if ( notNull(args) ) {
		addForeignFunctionArgsToStackFrame(*stackFrame, function->params, *args);
	}

	// Add stack frame
	stack.push(stackFrame);
	stackFrame->deref();

	// Create temporary strand stack
	OpStrandStack  contextStrandStack;
	contextStrandStack.push_back( OpStrandContainer(body->getOpcodeStrand(), true) );

	// Run the function's body of opcodes in the engine
	// NOTE: To prevent memory leaks of adding a Terminal to the global strand stack,
	// the currently running strand stack has been checked in execute().
	EngineResult::Value  result = execute( contextStrandStack );

	// Remove the frame just added
	stack.pop();

	if ( result == EngineResult::Error ) {
		return result;
	}

	return EngineResult::Ok;
}

void
Engine::addForeignFunctionArgsToStackFrame(
	StackFrame&  stackFrame,
	const List<String>&  functionParams,
	List<Object*>&  arguments
) {
	List<String>::ConstIter  funcParamsIter = functionParams.constStart();
	List<Object*>::Iter  givenArgsIter = arguments.start();
	bool done = false;

	if ( funcParamsIter.has() && givenArgsIter.has() ) {
		do {
			// Match parameters with parameter list items
			stackFrame.getScope().setVariableFrom( *funcParamsIter, *givenArgsIter, true );
			if ( ! funcParamsIter.next() ) {
				done = true;
				break;
			}
		} while ( givenArgsIter.next() );
		if ( !done )
		do {
			print( LogLevel::warning, EngineMessage::MissingFunctionCallArg );
		} while ( funcParamsIter.next() );
	}
}

Object*
Engine::
getLastObject() const {
	return lastObject.raw();
}

FuncExecReturn::Value
Engine::setupFunctionExecution(
	FuncFoundTask& task,
	OpStrandStackIter&	opStrandStackIter
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::setupFunctionExecution");
#endif
	// Protect from polution and allow for functions to have a default return.
	lastObject.setWithoutRef(new FunctionObject());
	FuncExecReturn::Value result;

	if ( ! task.getAddress().has() )
		throw VarAddressException( VarAddressException::is_empty );

	result = setupBuiltinFunctionExecution(task, opStrandStackIter);
	if ( result != FuncExecReturn::NoMatch ) {
		return result;
	}

	result = setupForeignFunctionExecution(task);
	if ( result != FuncExecReturn::NoMatch ) {
		return result;
	}

	return setupUserFunctionExecution(task, opStrandStackIter);
}

FuncExecReturn::Value
Engine::setupBuiltinFunctionExecution(
	FuncFoundTask& task,
	OpStrandStackIter&	opStrandStackIter
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::setupBuiltinFunctionExecution");
#endif

	// Which one is faster?
	// a) Getting a key and using a switch
	// b) Using a virtual function
	// Based on time tests, the first one. Using a switch is basically a manual lookup-table.

	//VarAddressConstIter addrIter = task.getAddress().constStart();
	//RobinHoodHash<SystemFunction::Value>::BucketData* bucketData
	//	= builtinFunctions.getBucketData(*addrIter);

	RobinHoodHash<SystemFunction::Value>::BucketData* bucketData
		= builtinFunctions.getBucketData(task.getAddress().first());

	// No matching function found
	if ( ! bucketData ) {
		return FuncExecReturn::NoMatch;
	}
	// Else, bucketData found...

	// Built-in function names should only have one name
	if ( ! task.getAddress().hasOne() ) {
		//print(LogLevel::error, "Attempt to call member of built-in function.");
		print(LogLevel::error, EngineMessage::SystemFuncInvalidAccess);
		return FuncExecReturn::ErrorOnRun;
	}

	SystemFunction::Value funcName = bucketData->item;

	// Built-in function buckets only contain the name of the function to be run.
	// While they could contain a "header" for indicating what they accept, the reality is that
	// many of them can accept any object and/or any number of parameters.
	// (e.g. are_same_type())

	switch( funcName ) {
	case SystemFunction::_unset:
		print(LogLevel::error, "SYSTEM ERROR: Attempting to run system function without setting it.");
		return FuncExecReturn::ErrorOnRun;

	case SystemFunction::_return:
		return process_sys_return(task);

	case SystemFunction::_not:
		process_sys_not(task);
		break;

	case SystemFunction::_all:
		process_sys_all(task);
		break;

	case SystemFunction::_any:
		process_sys_any(task);
		break;

	case SystemFunction::_nall:
		process_sys_nall(task);
		break;

	case SystemFunction::_none:
		process_sys_none(task);
		break;

	case SystemFunction::_are_fn:
		process_sys_are_fn(task);
		break;

	case SystemFunction::_are_empty:
		process_sys_are_empty(task);
		break;

	case SystemFunction::_are_same:
		process_sys_are_same(task);
		break;

	case SystemFunction::_member:
		return process_sys_member(task);

	case SystemFunction::_member_count:
		return process_sys_member_count(task);

	case SystemFunction::_is_member:
		return process_sys_is_member(task);

	case SystemFunction::_set_member:
		return process_sys_set_member(task);

	case SystemFunction::_member_list:
		return process_sys_member_list(task);

	case SystemFunction::_union:
		return process_sys_union(task);

	case SystemFunction::_type:
		return process_sys_type(task);

	case SystemFunction::_are_same_type:
		return process_sys_are_same_type(task);

	case SystemFunction::_are_bool:
		return process_sys_are_bool(task);

	case SystemFunction::_are_string:
		return process_sys_are_string(task);

	case SystemFunction::_are_list:
		return process_sys_are_list(task);

	case SystemFunction::_are_number:
		return process_sys_are_number(task);

	case SystemFunction::_are_integer:
		return process_sys_are_integer(task);

	case SystemFunction::_are_decimal:
		return process_sys_are_decimal(task);

	case SystemFunction::_assert:
		return process_sys_assert(task);

	case SystemFunction::_copy:
		return process_sys_copy(task);

	case SystemFunction::_execute_with_alt_super:
		return process_sys_execute_with_alt_super(task, opStrandStackIter);

	case SystemFunction::_make_list:
		return process_sys_make_list(task);

	case SystemFunction::_list_size:
		return process_sys_list_size(task);

	case SystemFunction::_list_append:
		return process_sys_list_append(task);

	case SystemFunction::_list_prepend:
		return process_sys_list_prepend(task);

	case SystemFunction::_list_insert:
		return process_sys_list_insert(task);

	case SystemFunction::_list_get_item:
		return process_sys_list_get_item(task);

	case SystemFunction::_list_remove:
		return process_sys_list_remove(task);

	case SystemFunction::_list_clear:
		return process_sys_list_clear(task);

	case SystemFunction::_list_swap:
		return process_sys_list_swap(task);

	case SystemFunction::_list_replace:
		return process_sys_list_replace(task);

	case SystemFunction::_list_sublist:
		return process_sys_list_sublist(task);

	case SystemFunction::_string_match:
		return process_sys_string_match(task);

	case SystemFunction::_string_concat:
		return process_sys_string_concat(task);

	case SystemFunction::_num_equal:
		return process_sys_num_equal(task);

	case SystemFunction::_num_greater_than:
		return process_sys_num_greater_than(task, false);

	case SystemFunction::_num_greater_or_equal:
		return process_sys_num_greater_or_equal(task, false);

	case SystemFunction::_num_less_than:
		return process_sys_num_greater_than(task, true);

	case SystemFunction::_num_less_or_equal:
		return process_sys_num_greater_or_equal(task, true);

	case SystemFunction::_num_abs:
		return process_sys_num_abs(task);

	case SystemFunction::_num_add:
		return process_sys_num_chain_num(task, process_sys_num_add, SystemFunction::_num_add);

	case SystemFunction::_num_subtract:
		return process_sys_num_chain_num(task, process_sys_num_subtract, SystemFunction::_num_subtract);

	case SystemFunction::_num_multiply:
		return process_sys_num_chain_num(task, process_sys_num_multiply, SystemFunction::_num_multiply);

	case SystemFunction::_num_divide:
		return process_sys_num_chain_num(task, process_sys_num_divide, SystemFunction::_num_divide);

	case SystemFunction::_num_modulus:
		return process_sys_num_chain_num(task, process_sys_num_modulus, SystemFunction::_num_modulus);

	case SystemFunction::_num_incr:
		if ( task.args.size() > 1 ) {
			return process_sys_num_chain_num(task, process_sys_num_incr2, SystemFunction::_num_incr_2);
		} else {
			return process_sys_solo_num(task, process_sys_num_incr, SystemFunction::_num_incr);
		}

	case SystemFunction::_num_decr:
		if ( task.args.size() > 1 ) {
			return process_sys_num_chain_num(task, process_sys_num_decr2, SystemFunction::_num_decr_2);
		} else {
			return process_sys_solo_num(task, process_sys_num_decr, SystemFunction::_num_decr);
		}

	default:
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::setupBuiltinFunctionExecution could not match function");
#endif
		return FuncExecReturn::NoMatch;
	}
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::setupForeignFunctionExecution(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::setupForeignFunctionExecution");
#endif

	RobinHoodHash<ForeignFuncContainer>::BucketData* bucketData
		= foreignFunctions.getBucketData(task.getAddress().first());

	if ( ! bucketData ) {
		return FuncExecReturn::NoMatch;
	}

	ForeignFunc* foreignFunc = bucketData->item.getForeignFunction();

	FFIServices ffi(*this, task.args, task.getAddress().first());
	bool result = foreignFunc->call( ffi );

	// lastObject is set by setResult() or setNewResult() of the FFI.
	return result ?
		FuncExecReturn::Ran :
		FuncExecReturn::ErrorOnRun;
}

FuncExecReturn::Value
Engine::setupUserFunctionExecution(
	FuncFoundTask& task,
	OpStrandStackIter&	opStrandStackIter
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::setupUserFunctionExecution");
#endif

	// Remember, "super" is set by what is on the address chain, NOT by the outer function.
	// The "this" pointer (for function building) is set by the current scope,
	// but that should already have been set.

	// To get "super":
	// If the address contains 2 or more names, the second from last is the "super".
	// If there is a super, resolve it.
	// Furthermore, you need only open its scope to get the variable whose function is called.
	// If there is NO super, you must resolve the entire address for just the function being called.
/*
	// Original method, which only works for local scope. KEEP!
	// You may later require access to globals via a "global" keyword.

	Variable* callVariable = REAL_NULL;
	Variable* super = REAL_NULL;
	Scope* scope = &getCurrentTopScope();
	Function* func = REAL_NULL;
	// TODO: Change to index
	VarAddressConstIter addrIter = task.getAddress().constStart();
	do {
		super = callVariable;
		scope->getVariable(*addrIter, callVariable);
		func = callVariable->getFunction(logger);
		scope = &(func->getPersistentScope());
	} while ( addrIter.next() );
*/

//----------

	// Search local, then global, then create in local if not found.
	// Code modified from resolveVariableAddress().

	Variable* callVariable = REAL_NULL;
	Variable* super = REAL_NULL;
	Scope* scope = &getCurrentTopScope();
	Function* func = REAL_NULL;
	bool foundVar = false;

	VarAddress::Iterator ai = task.getAddress().iterator();
	if ( scope->findVariable(ai.get(), callVariable) ) {
		foundVar = true;
		while ( ai.next() ) {
			super = callVariable;
			func = callVariable->getFunction(logger);
			scope = &(func->getPersistentScope());
			scope->getVariable(ai.get(), callVariable);
		}
		func = callVariable->getFunction(logger);
	}

	if ( !foundVar ) {
		ai.reset();
		scope = &getGlobalScope();
		if ( scope->findVariable(ai.get(), callVariable) ) {
			foundVar = true;
			while( ai.next() ) {
				super = callVariable;
				func = callVariable->getFunction(logger);
				scope = &(func->getPersistentScope());
				scope->getVariable(ai.get(), callVariable);
			}
			func = callVariable->getFunction(logger);
		}
	}

	if ( !foundVar ) {
		ai.reset();
		scope = &getCurrentTopScope();
		scope->getVariable(ai.get(), callVariable); // Create the base variable since it does not exist
		while ( ai.next() ) {
			super = callVariable;
			func = callVariable->getFunction(logger);
			scope = &(func->getPersistentScope());
			scope->getVariable(ai.get(), callVariable);
		}
		func = callVariable->getFunction(logger);
	}

	// Short form (but re-checks globals and foreigns)
	//callVariable = resolveVariableAddress( task.getAddress() );
	//func = callVariable->getFunction(logger);
	//scope = &(func->getPersistentScope());

//----------

	// Short-circuit for constant-return functions
	if ( func->constantReturn ) {
		lastObject.set( func->result.raw() ); // Copy is created when saved to variable
		return FuncExecReturn::Ran;
	}

	// Parse function body if not done yet.
	// If function body contained errors, return error.
	// If function body has been / is parsed, add its opcodes to the stack.
	Body* body;
	if ( ! func->body.obtain(body) ) {
		print(LogLevel::error, "SYSTEM ERROR: Function body is missing.");
		return FuncExecReturn::ErrorOnRun;
	}

	if ( body->isEmpty() ) {
		return FuncExecReturn::Ran;
	}

	// Attempt to compile. Automatically returns true if compiled.
	if ( ! body->compile(this) ) {
		//print(LogLevel::error, "Function body contains errors.");
		print(LogLevel::error, EngineMessage::UserFunctionBodyError);
		return FuncExecReturn::ErrorOnRun;
	}

	// "super" and "this" are added to new stack context if the body can be run.
	StackFrame* stackFrame = new StackFrame(task.varAddress);

	if ( notNull(super) ) {
		// Add "super" pointer
		stackFrame->getScope().setVariable(CONSTANT_FUNCTION_SUPER, super, true);
	}

	// Add "this" pointer
	stackFrame->getScope().setVariable(CONSTANT_FUNCTION_SELF, callVariable, true);

	// For each parameter that the function requires, take from the passed parameters and
	// assign it by pointer to a parameter name within the newly added scope.
	ArgsIter givenArgsIter = task.args.start();
	List<String>::Iter funcParamsIter = func->params.start();
	bool done = false;
	if ( funcParamsIter.has() ) {
		if ( givenArgsIter.has() )
		do {
			// Match parameters with parameter list items
			stackFrame->getScope().setVariableFrom( *funcParamsIter, *givenArgsIter, true );
			if ( ! funcParamsIter.next() ) {
				done = true;
				break;
			}
		} while ( givenArgsIter.next() );
		if ( !done )
		do {
			print( LogLevel::warning, EngineMessage::MissingFunctionCallArg );
			// Default remaining parameters to empty function.
			stackFrame->getScope().addVariable( *funcParamsIter );
		} while ( funcParamsIter.next() );
	}
	// TODO: Leftover task arguments may eventually be stored in a splat variable or included as part
	// of an all-inclusive "arguments" list.

	stack.push(stackFrame);
	stackFrame->deref();

	// Unfortunately, incrementing up a stack frame leaves the current operator at a used token.
	// We don't want this, so handle it in execute().

	// Finally, add the body to be processed
	addOpStrandToStack(body->getOpcodeStrand());

	opStrandStackIter.next();
	//opStrandStackIter.makeLast();
	// Since the iterator in the OpStrand container is separately defined from the list, there is no
	// need to reset it and getCurrOp() should return the first opcode every time the function is called.

	return FuncExecReturn::Reset;
}

Variable*
Engine::resolveVariableAddress(
	const VarAddress& address
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::resolveVariableAddress");
#endif
	// First, check if the variable exists in the local scope.
	// Then, check the global scope.
	// Then, if it's not in either, create it in the local scope.

	if ( ! address.has() )
		throw VarAddressException( VarAddressException::is_empty );

#ifdef COPPER_DEBUG_ADDRESS
	address.print();
#endif

	// Should return REAL_NULL if the address belongs to a non-variable, such as a
	// built-in function or foreign function.

	RobinHoodHash<SystemFunction::Value>::BucketData* sfBucket
		= builtinFunctions.getBucketData( address.first() );
	if ( sfBucket != 0 ) { // Error, but handling is determined by the method that calls this one
		//print(LogLevel::warning, "Attempt to use standard access on a built-in function.");
		print(LogLevel::warning, EngineMessage::SystemFuncInvalidAccess);
		return REAL_NULL;
	}

	RobinHoodHash<ForeignFuncContainer>::BucketData* ffBucket
		= foreignFunctions.getBucketData( addressToString(address) );
	if ( ffBucket != 0 ) { // Error, but handling is determined by the method that calls this one
		//print(LogLevel::warning, "Attempt to use standard access on a foreign function.");
		print(LogLevel::warning, EngineMessage::ForeignFuncInvalidAccess);
		return REAL_NULL;
	}

	Scope* scope = &getCurrentTopScope();
	Variable* var = REAL_NULL;
	Function* func = REAL_NULL;
	VarAddress::Iterator ai = address.iterator();

	// Search locals
	if ( scope->findVariable(ai.get(), var) ) {
		while( ai.next() ) {
			func = var->getFunction(logger);
			scope = &(func->getPersistentScope());		
			scope->getVariable(ai.get(), var);	
		}
		return var;
	}

	// Search globals
	scope = &getGlobalScope();
	ai.reset();
	if ( scope->findVariable(ai.get(), var) ) {
		while( ai.next() ) {
			func = var->getFunction(logger);
			scope = &(func->getPersistentScope());		
			scope->getVariable(ai.get(), var);	
		}
		return var;
	}

	// No base variable found, so just create it as a local
	scope = &getCurrentTopScope();
	ai.reset();
	while(true) {
		scope->getVariable(ai.get(), var);
		if ( !ai.next() ) {
			break;
		}
		func = var->getFunction(logger);
		scope = &(func->getPersistentScope());
	}
	return var;
}

ExecutionResult::Value
Engine::run_Own(
	const VarAddress& address
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::run_Own");
#endif
	lastObject.setWithoutRef(new FunctionObject());
	if ( !ownershipChangingEnabled ) {
		//print(LogLevel::warning, "Ownership changing is disabled.");
		print(LogLevel::warning, EngineMessage::PointerNewOwnershipDisabled);
		return ExecutionResult::Ok;
	}
	Variable* var = resolveVariableAddress(address);
	if ( isNull(var) ) {
		//print(LogLevel::warning, "Cannot own given function. Built-in or foreign function.");
		print(LogLevel::warning, EngineMessage::NonVariablePassedToPtrHandler);
		return ExecutionResult::Ok; // Could be set to EngineResult::Error
	}
	var->getRawContainer()->changeOwnerTo(var);
	return ExecutionResult::Ok;
}

bool
Engine::is_var_pointer(
	const VarAddress& address
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::is_var_pointer");
#endif
	Variable* var = resolveVariableAddress(address);
	if ( isNull(var) ) {
		//print(LogLevel::info, "Given function has system as owner.");
		print(LogLevel::warning, EngineMessage::NonVariablePassedToPtrHandler);
		return false;
	}
	return var->isPointer();
}

ExecutionResult::Value
Engine::run_Is_owner(
	const VarAddress& address
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::run_Is_owner");
#endif
	lastObject.setWithoutRef(new BoolObject(
		! is_var_pointer( address )
	));
	return ExecutionResult::Ok;
}

ExecutionResult::Value
Engine::run_Is_ptr(
	const VarAddress& address
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::run_Is_ptr");
#endif
	lastObject.setWithoutRef(new BoolObject(
		is_var_pointer( address )
	));
	return ExecutionResult::Ok;
}

void
Engine::printSystemFunctionWrongArgCount(
	SystemFunction::Value  function,
	UInteger  argsGiven,
	UInteger  argCount
) {
	print( LogMessage::create(LogLevel::error)
		.SystemFunctionId( function )
		.Message(EngineMessage::WrongArgCount)
		.ArgIndex(argsGiven)
		.ArgCount(argCount)
	);
}

void
Engine::printSystemFunctionWrongArg(
	SystemFunction::Value  function,
	UInteger  argIndex,
	UInteger  argCount,
	ObjectType::Value  given,
	ObjectType::Value  expected,
	bool isError
) {
	print( LogMessage::create( isError? LogLevel::error : LogLevel::warning )
		.SystemFunctionId( function )
		.Message(EngineMessage::WrongArgType)
		.ArgIndex(argIndex)
		.ArgCount(argCount)
		.GivenArgType(given)
		.ExpectedArgType(expected)
	);
}

FuncExecReturn::Value
Engine::process_sys_return(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_return");
#endif
	if ( task.args.has() ) {
		lastObject.set( task.args.getFirst() );
	}
	return FuncExecReturn::Return;
}

FuncExecReturn::Value
Engine::process_sys_not(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_not");
#endif
	bool result = true;
	ArgsIter ai = task.args.start();
	if ( ai.has() ) {
		result = ! getBoolValue(**ai);
	}
	lastObject.setWithoutRef(new BoolObject(result));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_all(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_all");
#endif
	bool result = false;
	ArgsIter ai = task.args.start();
	if ( ai.has() )
	do {
		result = getBoolValue(**ai);
	} while ( ai.next() && result );
	lastObject.setWithoutRef(new BoolObject(result));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_any(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_any");
#endif
	bool result = false;
	ArgsIter ai = task.args.start();
	if ( ai.has() )
	do {
		result = getBoolValue(**ai);
	} while ( ai.next() && ! result );
	lastObject.setWithoutRef(new BoolObject(result));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_nall(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_nall");
#endif
	bool result = true;
	ArgsIter ai = task.args.start();
	if ( ai.has() )
	do {
		result = ! getBoolValue(**ai);
	} while ( ai.next() && !result );
	lastObject.setWithoutRef(new BoolObject(result));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_none(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_none");
#endif
	bool result = true;
	ArgsIter ai = task.args.start();
	if ( ai.has() )
	do {
		result = ! getBoolValue(**ai);
	} while ( ai.next() && !result );
	lastObject.setWithoutRef(new BoolObject(result));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_are_fn(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_fn");
#endif
	bool result = false;
	ArgsIter ai = task.args.start();
	if ( ai.has() )
	do {
		result = isFunctionObject(**ai);
	} while ( ai.next() && !result );
	lastObject.setWithoutRef(new BoolObject(result));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_are_empty(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_empty");
#endif
	bool result = false;
	ArgsIter ai = task.args.start();
	if ( ai.has() )
	do {
		result = isEmptyFunctionObject(**ai);
	} while ( ai.next() && result );
	lastObject.setWithoutRef(new BoolObject(result));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_are_same(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_same");
#endif
	// Technically, this function should probably only compare equality of pointers
	// to the same function, but as data is copied by default anyways, it won't matter.
	bool result = true;
	Object* firstPtr = REAL_NULL;
	ArgsIter ai = task.args.start();
	if ( ai.has() ) {
		firstPtr = *ai;
		while( ai.next() && result ) {
			result = ( firstPtr == *ai );
		}
	}
	lastObject.setWithoutRef(new BoolObject(result));
	return FuncExecReturn::Ran;
}

// special return
FuncExecReturn::Value
Engine::process_sys_member(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_member");
#endif
	ArgsIter argsIter = task.args.start();
	if ( task.args.size() != 2 ) {
		printSystemFunctionWrongArgCount( SystemFunction::_member, task.args.size(), 2 );
		return FuncExecReturn::ErrorOnRun;
	}
	// First parameter is the parent function of the member sought
	if ( ! isFunctionObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_member, 1, 2, (*argsIter)->getType(), ObjectType::Function );
		return FuncExecReturn::ErrorOnRun;
	}
	FunctionObject* parentFc = (FunctionObject*)*argsIter;
	Function* parentFunc;
		// Attempt to extract the function
		// Failure occurs for "pointer variables" that have not reset
	if ( ! parentFc->getFunction(parentFunc) ) {
		print( LogMessage::create(LogLevel::error)
			.SystemFunctionId( SystemFunction::_member )
			.Message(EngineMessage::DestroyedFuncAsArg)
			.ArgIndex(1).ArgCount(2)
		);
		return FuncExecReturn::ErrorOnRun;
	}
	// Second parameter is the name of the member
	argsIter.next();
	if ( !isStringObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_member, 2, 2, (*argsIter)->getType(), ObjectType::String );
		return FuncExecReturn::ErrorOnRun;
	}
	StringObject* objStr = (StringObject*)(*argsIter);
	String& rawStr = objStr->getString();
	if ( ! isValidName( rawStr ) ) {
		print( LogMessage::create(LogLevel::error)
			.SystemFunctionId( SystemFunction::_member )
			.Message(EngineMessage::SystemFunctionBadArg)
			.ArgIndex(1).ArgCount(2)
			.GivenArgType((*argsIter)->getType())
			.ExpectedArgType(ObjectType::String)
			.ExpectedArgTypeName("[name]")
		);
		return FuncExecReturn::ErrorOnRun;
	}
	Variable* var;
	parentFunc->getPersistentScope().getVariable( rawStr, var );
	lastObject.set( var->getRawContainer() );
	return FuncExecReturn::Ran;
}

// special return
FuncExecReturn::Value
Engine::process_sys_member_count(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_member_count");
#endif
	ArgsIter argsIter = task.args.start();
	if ( task.args.size() != 1 ) {
		printSystemFunctionWrongArgCount( SystemFunction::_member_count, task.args.size(), 1 );
		lastObject.setWithoutRef(new IntegerObject(0));
		return FuncExecReturn::ErrorOnRun;
	}
	// The only parameter is the parent function of the members
	if ( ! isFunctionObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_member_count, 1, 1,
			(*argsIter)->getType(), FunctionObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	// Obtain the actual function (which is the parent of the member of interest)
	Function* parentFunc;
	FunctionObject* fc = (FunctionObject*)(*argsIter);
	if ( ! fc->getFunction(parentFunc) ) {
		print( LogMessage::create(LogLevel::error)
			.SystemFunctionId( SystemFunction::_member_count )
			.Message(EngineMessage::DestroyedFuncAsArg)
			.ArgIndex(1)
			.ArgCount(1)
		);
		return FuncExecReturn::ErrorOnRun;
	}
	unsigned long size = parentFunc->getPersistentScope().occupancy();
	lastObject.setWithoutRef(new IntegerObject(Integer(size)));

	return FuncExecReturn::Ran;
}

// special return
FuncExecReturn::Value
Engine::process_sys_is_member(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_is_member");
#endif
	ArgsIter argsIter = task.args.start();
	if ( task.args.size() != 2 ) {
		printSystemFunctionWrongArgCount( SystemFunction::_is_member, task.args.size(), 2 );
		lastObject.setWithoutRef(new BoolObject(false));
		return FuncExecReturn::ErrorOnRun;
	}
	// First parameter is the parent function of the members
	if ( ! isFunctionObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_is_member, 1, 2,
			(*argsIter)->getType(), FunctionObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	// Obtain the actual function (which is the parent of the member of interest)
	Function* parentFunc;
	FunctionObject* fc = (FunctionObject*)(*argsIter);
	if ( ! fc->getFunction(parentFunc) ) {
		print( LogMessage::create(LogLevel::error)
			.SystemFunctionId( SystemFunction::_is_member )
			.Message(EngineMessage::DestroyedFuncAsArg)
			.ArgIndex(1)
			.ArgCount(2)
		);
		return FuncExecReturn::ErrorOnRun;
	}
	// Second parameter should be a string
	argsIter.next();
	bool result = false;
	if ( ! isStringObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_is_member, 2, 2,
			(*argsIter)->getType(), StringObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	const String& memberName = ((StringObject*)*argsIter)->getString();
	result = parentFunc->getPersistentScope().variableExists( memberName );
	lastObject.setWithoutRef(new BoolObject(result));
	return FuncExecReturn::Ran;
}

// special return
FuncExecReturn::Value
Engine::process_sys_set_member(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_set_member");
#endif
	ArgsIter argsIter = task.args.start();
	if ( task.args.size() != 3 ) {
		printSystemFunctionWrongArgCount( SystemFunction::_set_member, task.args.size(), 3 );

		lastObject.setWithoutRef(new FunctionObject());
		return FuncExecReturn::ErrorOnRun;
	}
	// First parameter should be the parent function of the members
	if ( ! isFunctionObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_set_member, 1, 3,
			(*argsIter)->getType(), FunctionObject::object_type);

		return FuncExecReturn::ErrorOnRun;
	}
	// Obtain the actual function (which is the parent of the member of interest)
	Function* parentFunc;
	FunctionObject* fc = (FunctionObject*)(*argsIter);
	if ( ! fc->getFunction(parentFunc) ) {
		print( LogMessage::create(LogLevel::error)
			.SystemFunctionId( SystemFunction::_set_member )
			.Message(EngineMessage::DestroyedFuncAsArg)
			.ArgIndex(1)
			.ArgCount(3)
		);
		return FuncExecReturn::ErrorOnRun;
	}
	// Second parameter should be a string
	argsIter.next();
	if ( ! isStringObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_set_member, 2, 3,
			(*argsIter)->getType(), StringObject::object_type);

		return FuncExecReturn::ErrorOnRun;
	}
	String& memberName = ((StringObject*)*argsIter)->getString();
	if ( !isValidName(memberName) ) {
		print( LogMessage::create(LogLevel::error)
			.SystemFunctionId( SystemFunction::_set_member )
			.Message(EngineMessage::SystemFunctionBadArg)
			.ArgIndex(2)
			.ArgCount(3)
			.GivenArgType((*argsIter)->getType())
			.ExpectedArgType(ObjectType::String)
		);
		return FuncExecReturn::ErrorOnRun;
	}
	// Third parameter can be anything
	argsIter.next();
	parentFunc->getPersistentScope().setVariableFrom( memberName, *argsIter, false );
	lastObject.setWithoutRef(new FunctionObject());
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_member_list(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_member_list");
#endif
	ArgsIter  argsIter = task.args.start();
	if ( !argsIter.has() ) {
		// Nothing to do here
		lastObject.setWithoutRef(new ListObject());
		return FuncExecReturn::Ran;
	}
	// This function can take any number of functions; it will simply add them to the same list.
	ListObject* outList = new ListObject();
	FunctionObject* usableFC;
	Function* usableFunc;
	UInteger argIndex = 1; // For printing purposes
	do {
		if ( isFunctionObject(**argsIter) ) {
			usableFC = (FunctionObject*)(*argsIter);
			if ( usableFC->getFunction(usableFunc) ) {
				usableFunc->getPersistentScope().appendNamesByInterface(outList);
			} else {
				print( LogMessage::create(LogLevel::warning)
					.SystemFunctionId( SystemFunction::_member_list )
					.Message(EngineMessage::DestroyedFuncAsArg)
					.ArgIndex(argIndex)
				);
			}
		} else {
			printSystemFunctionWrongArg( SystemFunction::_member_list, argIndex, 0,
				(*argsIter)->getType(), FunctionObject::object_type, false );
		}
		++argIndex;
	} while ( argsIter.next() );
	lastObject.setWithoutRef(outList);
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_union(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_union");
#endif
	ArgsIter  argsIter = task.args.start();
	if ( !argsIter.has() ) {
		//print(LogLevel::info, "union function called without parameters. Default return is empty function.");
		lastObject.setWithoutRef(new FunctionObject());
		return FuncExecReturn::Ran;
	}
	FunctionObject*  finalFC = new FunctionObject();
	Function*  finalFunc;
	finalFC->getFunction(finalFunc); // Guaranteed
	FunctionObject*  usableFC;
	Function*  usableFunc;
	UInteger  argIndex = 1; // For printing
	do {
		if ( isFunctionObject(**argsIter) ) {
			usableFC = (FunctionObject*)(*argsIter);
			if ( usableFC->getFunction(usableFunc) ) {
				finalFunc->getPersistentScope().copyMembersFrom( usableFunc->getPersistentScope() );
			} else {
				print( LogMessage::create(LogLevel::warning)
					.SystemFunctionId( SystemFunction::_union )
					.Message(EngineMessage::DestroyedFuncAsArg)
					.ArgIndex(argIndex)
				);
			}
		} else {
			printSystemFunctionWrongArg( SystemFunction::_union, argIndex, 0,
				(*argsIter)->getType(), FunctionObject::object_type, false );
		}
		++argIndex;
	} while( argsIter.next() );
	lastObject.set(finalFC);
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_type(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_type");
#endif
	ArgsIter argsIter = task.args.start();
	if ( !argsIter.has() ) {
		lastObject.setWithoutRef(new StringObject());
		return FuncExecReturn::Ran;
	}
	// Get only the first parameter
	lastObject.setWithoutRef(new StringObject( (*argsIter)->typeName() ));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_are_same_type(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_same_type");
#endif
	ArgsIter argsIter = task.args.start();
	if ( !argsIter.has() ) {
		lastObject.setWithoutRef(new FunctionObject());
		return FuncExecReturn::Ran;
	}
	// First parameter, to which all other parameters are compared
	ObjectType::Value firstType = (*argsIter)->getType();
	String first( (*argsIter)->typeName() );

	bool same = true;
	do {
		same = firstType == (*argsIter)->getType();
		if ( !same )
			break;

		same = first.equals( (*argsIter)->typeName() );
		if ( !same )
			break;

	} while( argsIter.next() );
	// Get only the first parameter
	lastObject.setWithoutRef(new BoolObject(same));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_are_bool(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_bool");
#endif
	ArgsIter argsIter = task.args.start();
	if ( !argsIter.has() ) {
		lastObject.setWithoutRef(new BoolObject(false));
		return FuncExecReturn::Ran;
	}
	// Check all parameters
	bool out = true;
	do {
		out = isBoolObject(**argsIter);
		if ( !out) break;
	} while ( argsIter.next() );
	lastObject.setWithoutRef(new BoolObject(out));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_are_string(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_string");
#endif
	ArgsIter argsIter = task.args.start();
	if ( !argsIter.has() ) {
		lastObject.setWithoutRef(new BoolObject(false));
		return FuncExecReturn::Ran;
	}
	// Check all parameters
	bool out = true;
	do {
		out = isStringObject(**argsIter);
		if ( !out)
			break;
	} while ( argsIter.next() );
	lastObject.setWithoutRef(new BoolObject(out));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_are_list(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_list");
#endif
	ArgsIter argsIter = task.args.start();
	if ( !argsIter.has() ) {
		lastObject.setWithoutRef(new BoolObject(false));
		return FuncExecReturn::Ran;
	}
	// Check all parameters
	bool out = true;
	do {
		out = isListObject(**argsIter);
		if ( !out)
			break;
	} while ( argsIter.next() );
	lastObject.setWithoutRef(new BoolObject(out));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_are_number(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_number");
#endif
	ArgsIter argsIter = task.args.start();
	if ( !argsIter.has() ) {
		lastObject.setWithoutRef(new BoolObject(false));
		return FuncExecReturn::Ran;
	}
	// Check all parameters
	bool out = true;
	do {
		out = isNumericObject(**argsIter);
		if ( !out)
			break;
	} while ( argsIter.next() );
	lastObject.setWithoutRef(new BoolObject(out));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_are_integer(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_integer");
#endif
	ArgsIter argsIter = task.args.start();
	if ( !argsIter.has() ) {
		lastObject.setWithoutRef(new BoolObject(false));
		return FuncExecReturn::Ran;
	}
	// Check all parameters
	bool out = true;
	do {
		out = isIntegerObject(**argsIter);
		if ( !out)
			break;
	} while ( argsIter.next() );
	lastObject.setWithoutRef(new BoolObject(out));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_are_decimal(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_decimal");
#endif
	ArgsIter argsIter = task.args.start();
	if ( !argsIter.has() ) {
		lastObject.setWithoutRef(new BoolObject(false));
		return FuncExecReturn::Ran;
	}
	// Check all parameters
	bool out = true;
	do {
		out = isDecimalNumObject(**argsIter);
		if ( !out)
			break;
	} while ( argsIter.next() );
	lastObject.setWithoutRef(new BoolObject(out));
	return FuncExecReturn::Ran;
}

// special return
FuncExecReturn::Value
Engine::process_sys_assert(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_assert");
#endif
	ArgsIter argsIter = task.args.start();
	if ( !argsIter.has() ) {
		return FuncExecReturn::Ran;
	}
	do {
		if ( ! getBoolValue(**argsIter) ) {
			print(LogLevel::error, EngineMessage::AssertionFailed);
			return FuncExecReturn::ErrorOnRun;
		}
	} while ( argsIter.next() );
	lastObject.setWithoutRef(new BoolObject(true));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_copy(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_copy");
#endif
	ArgsIter argsIter = task.args.start();
	if ( argsIter.has() ) {
		lastObject.setWithoutRef( (*argsIter)->copy() );
	}
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_execute_with_alt_super(
	FuncFoundTask& task,
	OpStrandStackIter&	opStrandStackIter
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_execute_with_alt_super");
#endif
	if ( task.args.size() < 2 ) {
		printSystemFunctionWrongArgCount( SystemFunction::_execute_with_alt_super, task.args.size(), 2 );
		return FuncExecReturn::ErrorOnRun;
	}

	ArgsIter argsIter = task.args.start();
	Object* superObject = *argsIter;
	argsIter.next();
	Object* callObject = *argsIter;
	argsIter.next();

	if ( ! isFunctionObject(*superObject) ) {
		printSystemFunctionWrongArg( SystemFunction::_execute_with_alt_super, 1, 2,
			(*argsIter)->getType(), FunctionObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	if ( ! isFunctionObject(*callObject) ) {
		printSystemFunctionWrongArg( SystemFunction::_execute_with_alt_super, 2, 2,
			(*argsIter)->getType(), FunctionObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}

	//Function* superFunc;
	Function* func;
	/*if ( ! ((FunctionObject*)superObject)->getFunction(superFunc) ) {
		print(LogLevel::debug, "[DEBUG: Empty function container passed to execute-with-alt-super.");
		return FuncExecReturn::ErrorOnRun;
	}*/
	if ( ! ((FunctionObject*)callObject)->getFunction(func) ) {
		print(LogLevel::debug, "[DEBUG: Empty function container passed to execute-with-alt-super.");
		//print(LogLevel::error, "ERROR: Empty function passed to execute-with-alternate-super.");
		return FuncExecReturn::ErrorOnRun;
	}

	// Modified code from setupUserFunctionExecution(). See there for more comments.
	// Since we aren't using variables, the setting of the "this" and "super" pointers is different.

	// Short-circuit for constant-return functions
	if ( func->constantReturn ) {
		lastObject.set( func->result.raw() ); // Copy is created when saved to variable
		return FuncExecReturn::Ran;
	}

	Body* body;
	if ( ! func->body.obtain(body) ) {
		print(LogLevel::error, "SYSTEM ERROR: Function body is missing.");
		return FuncExecReturn::ErrorOnRun;
	}

	if ( body->isEmpty() ) {
		return FuncExecReturn::Ran;
	}

	if ( ! body->compile(this) ) {
		print(LogLevel::error, EngineMessage::UserFunctionBodyError);
		return FuncExecReturn::ErrorOnRun;
	}

	VarAddress* callAddress = new VarAddress();
	callAddress->push_back("xwsv");
	StackFrame* stackFrame = new StackFrame(callAddress);
	callAddress->deref();

	// Add "super" pointer
	stackFrame->getScope().setVariableFrom(CONSTANT_FUNCTION_SUPER, superObject, true);

	// Add "this" pointer
	stackFrame->getScope().setVariableFrom(CONSTANT_FUNCTION_SELF, callObject, true);

	// Assign arguments to each function parameter name.
	// The first two arguments were used, so we use the remainder.
	List<String>::Iter funcParamsIter = func->params.start();
	bool done = false;
	if ( funcParamsIter.has() ) {
		if ( argsIter.has() )
		do {
			// Match parameters with parameter list items
			stackFrame->getScope().setVariableFrom( *funcParamsIter, *argsIter, true );
			if ( ! funcParamsIter.next() ) {
				done = true;
				break;
			}
		} while ( argsIter.next() );
		if ( !done )
		do {
			print( LogLevel::warning, EngineMessage::MissingFunctionCallArg );
			// Default remaining parameters to empty function.
			stackFrame->getScope().addVariable( *funcParamsIter );
		} while ( funcParamsIter.next() );
	}
	// TODO: Leftover task arguments may eventually be stored in a splat variable or included as part
	// of an all-inclusive "arguments" list.

	stack.push(stackFrame);
	stackFrame->deref();

	addOpStrandToStack(body->getOpcodeStrand());
	opStrandStackIter.next();

	return FuncExecReturn::Reset;
}

FuncExecReturn::Value
Engine::process_sys_make_list(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_make_list");
#endif
	ArgsIter argsIter = task.args.start();
	ListObject* out = new ListObject();
	lastObject.setWithoutRef(out);
	if ( argsIter.has() ) {
		do {
			out->push_back(*argsIter);
		} while ( argsIter.next() );
	}
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_list_size(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_list_size");
#endif
	if ( task.args.size() != 1 ) {
		printSystemFunctionWrongArgCount( SystemFunction::_list_size, task.args.size(), 1 );
		return FuncExecReturn::ErrorOnRun;
	}

	ArgsIter argsIter = task.args.start();

	if ( ! isListObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_size, 1, 1,
			(*argsIter)->getType(), ListObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	ListObject* listPtr = (ListObject*)*argsIter;

	lastObject.setWithoutRef(
		new IntegerObject( listPtr->size() )
	);
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_list_append(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_list_append");
#endif
	if ( task.args.size() < 2 ) {
		printSystemFunctionWrongArgCount( SystemFunction::_list_append, task.args.size(), 2 );
		return FuncExecReturn::ErrorOnRun;
	}

	ArgsIter argsIter = task.args.start();

	if ( ! isListObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_append, 1, 2,
			(*argsIter)->getType(), ListObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	ListObject* listPtr = (ListObject*)*argsIter;

	while ( argsIter.next() ) {
		listPtr->push_back(*argsIter);
	}
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_list_prepend(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_list_prepend");
#endif
	if ( task.args.size() < 2 ) {
		printSystemFunctionWrongArgCount( SystemFunction::_list_prepend, task.args.size(), 2 );
		return FuncExecReturn::ErrorOnRun;
	}

	ArgsIter argsIter = task.args.start();

	if ( ! isListObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_prepend, 1, 2,
			(*argsIter)->getType(), ListObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	ListObject* listPtr = (ListObject*)*argsIter;

	while ( argsIter.next() ) {
		listPtr->push_front(*argsIter);
	}
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_list_insert(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_list_insert");
#endif
	if ( task.args.size() != 3 ) {
		printSystemFunctionWrongArgCount( SystemFunction::_list_insert, task.args.size(), 3 );
		return FuncExecReturn::ErrorOnRun;
	}

	ArgsIter argsIter = task.args.start();

	if ( ! isListObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_insert, 1, 3,
			(*argsIter)->getType(), ListObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	ListObject* listPtr = (ListObject*)*argsIter;

	argsIter.next();

	if ( ! isNumericObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_insert, 2, 3,
			(*argsIter)->getType(), NumericObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}

	Integer index = ((NumericObject*)(*argsIter))->getIntegerValue();

	while ( argsIter.next() ) {
		listPtr->insert(index, *argsIter);
	}
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_list_get_item(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_list_get_item");
#endif
	if ( task.args.size() != 2 ) {
		printSystemFunctionWrongArgCount( SystemFunction::_list_get_item, task.args.size(), 2 );
		return FuncExecReturn::ErrorOnRun;
	}

	ArgsIter argsIter = task.args.start();

	if ( ! isListObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_get_item, 1, 2,
			(*argsIter)->getType(), ListObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	ListObject* listPtr = (ListObject*)*argsIter;

	argsIter.next();

	if ( ! isNumericObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_get_item, 2, 2,
			(*argsIter)->getType(), NumericObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}

	Integer index = ((NumericObject*)(*argsIter))->getIntegerValue();

	Object* out = listPtr->getItem(index);
	if ( notNull(out) )
		lastObject.set(out);
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_list_remove(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_list_remove");
#endif
	if ( task.args.size() != 2 ) {
		printSystemFunctionWrongArgCount( SystemFunction::_list_remove, task.args.size(), 2 );
		return FuncExecReturn::ErrorOnRun;
	}

	ArgsIter argsIter = task.args.start();

	if ( ! isListObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_remove, 1, 2,
			(*argsIter)->getType(), ListObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	ListObject* listPtr = (ListObject*)*argsIter;

	argsIter.next();

	if ( ! isNumericObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_remove, 2, 2,
			(*argsIter)->getType(), NumericObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}

	Integer index = ((NumericObject*)(*argsIter))->getIntegerValue();
	listPtr->remove(index);
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_list_clear(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_list_clear");
#endif
	if ( task.args.size() != 1 ) {
		printSystemFunctionWrongArgCount( SystemFunction::_list_clear, task.args.size(), 1 );
		return FuncExecReturn::ErrorOnRun;
	}

	ArgsIter argsIter = task.args.start();

	if ( ! isListObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_clear, 1, 1,
			(*argsIter)->getType(), ListObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	((ListObject*)*argsIter)->clear();
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_list_swap(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_list_swap");
#endif
	if ( task.args.size() != 3 ) {
		printSystemFunctionWrongArgCount( SystemFunction::_list_swap, task.args.size(), 3 );
		return FuncExecReturn::ErrorOnRun;
	}

	ArgsIter argsIter = task.args.start();

	if ( ! isListObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_swap, 1, 3,
			(*argsIter)->getType(), ListObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	ListObject* listPtr = (ListObject*)*argsIter;

	argsIter.next();

	if ( ! isNumericObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_swap, 2, 3,
			(*argsIter)->getType(), NumericObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}

	Integer index1 = ((NumericObject*)(*argsIter))->getIntegerValue();

	argsIter.next();

	if ( ! isNumericObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_swap, 3, 3,
			(*argsIter)->getType(), NumericObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}

	Integer index2 = ((NumericObject*)(*argsIter))->getIntegerValue();
	listPtr->swap(index1, index2);
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_list_replace(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_list_replace");
#endif
	if ( task.args.size() != 3 ) {
		printSystemFunctionWrongArgCount( SystemFunction::_list_replace, task.args.size(), 3 );
		return FuncExecReturn::ErrorOnRun;
	}

	ArgsIter argsIter = task.args.start();

	if ( ! isListObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_replace, 1, 3,
			(*argsIter)->getType(), ListObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	ListObject* listPtr = (ListObject*)*argsIter;

	argsIter.next();

	if ( ! isNumericObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_replace, 2, 3,
			(*argsIter)->getType(), NumericObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}

	Integer index = ((NumericObject*)(*argsIter))->getIntegerValue();

	argsIter.next();

	listPtr->replace(index, *argsIter);
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_list_sublist(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_list_sublist");
#endif
	if ( task.args.size() != 3 ) {
		printSystemFunctionWrongArgCount( SystemFunction::_list_sublist, task.args.size(), 3 );
		return FuncExecReturn::ErrorOnRun;
	}

	ArgsIter argsIter = task.args.start();

	if ( ! isListObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_sublist, 1, 3,
			(*argsIter)->getType(), ListObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	ListObject* listPtr = (ListObject*)*argsIter;
	Integer startIndex = 0;
	Integer endIndex = listPtr->size();
	ListObject* newList = new ListObject();

	argsIter.next();

	if ( ! isNumericObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_sublist, 2, 3,
			(*argsIter)->getType(), NumericObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	startIndex = ((NumericObject*)(*argsIter))->getIntegerValue();

	argsIter.next();

	if ( ! isNumericObject(**argsIter) ) {
		printSystemFunctionWrongArg( SystemFunction::_list_sublist, 3, 3,
			(*argsIter)->getType(), NumericObject::object_type );

		return FuncExecReturn::ErrorOnRun;
	}
	endIndex = ((NumericObject*)(*argsIter))->getIntegerValue();

	if ( startIndex > listPtr->size() ) {
		print( LogMessage::create(LogLevel::warning)
				.SystemFunctionId( SystemFunction::_list_sublist )
				.Message( EngineMessage::IndexOutOfBounds )
				.ArgIndex(2).ArgCount(3)
		);
		startIndex = listPtr->size();
	}
	if ( endIndex > listPtr->size() ) {
		print( LogMessage::create(LogLevel::warning)
				.SystemFunctionId( SystemFunction::_list_sublist )
				.Message( EngineMessage::IndexOutOfBounds )
				.ArgIndex(3).ArgCount(3)
		);
		endIndex = listPtr->size();
	}

	// Copy items from starting index to end of list
	for (; startIndex < endIndex && startIndex < listPtr->size(); ++startIndex) {
		newList->push_back( listPtr->getItem(startIndex) );
	}
	lastObject.setWithoutRef(newList);
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_string_match(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_string_match");
#endif
	ArgsIter  argsIter = task.args.start();
	bool  matches = true;
	String*  base_string = REAL_NULL;
	UInteger  argIndex = 1; // For printing

	if ( argsIter.has() ) {
		do {
			if ( isStringObject( **argsIter ) ) {
				if ( isNull(base_string) ) {
					base_string = &(((StringObject*)(*argsIter))->getString());
				} else {
					matches &= base_string->equals( ((StringObject*)(*argsIter))->getString() );
				}
			} else {
				// DO NOT CONVERT TO STRING USING writeToString.
				// _string_match is reserved for true string comparison only!
				printSystemFunctionWrongArg( SystemFunction::_string_match, argIndex, 0, (*argsIter)->getType(), StringObject::object_type, false );
			}
			++argIndex;
		} while ( argsIter.next() && matches );
	}
	lastObject.setWithoutRef( new BoolObject(matches) );
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_string_concat(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_string_concat");
#endif
	ArgsIter argsIter = task.args.start();
	CharList builder;
	String part_str;
	if ( argsIter.has() ) {
		do {
			if ( isStringObject( **argsIter ) ) {
				builder.append( ((StringObject*)(*argsIter))->getString() );
			} else {
				(*argsIter)->writeToString(part_str);
				builder.append(part_str);
				part_str = String();
			}
		} while ( argsIter.next() );
	}
	lastObject.setWithoutRef( new StringObject( String(builder) ) ); // implicit cast to const String here
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_num_chain_num(
	FuncFoundTask&  task,
	NumericObject*  (*operation)(NumericObject&, NumericObject&),
	SystemFunction::Value  functionId
) {
	NumericObject*  startNumber = REAL_NULL;
	NumericObject*  resultNumber = REAL_NULL;
	ArgsIter  argsIter = task.args.start();
	UInteger  argIndex = 1; // For printing

	if ( argsIter.has() ) {
		if ( isNumericObject(**argsIter) ) {
			startNumber = (NumericObject*)*argsIter;
			startNumber->ref(); // Treat as new
		} else {
			printSystemFunctionWrongArg( functionId, 1, 0, (*argsIter)->getType(), NumericObject::object_type, false );
			return FuncExecReturn::Ran;
		}
		while ( argsIter.next() ) {
			++argIndex;
			if ( isNumericObject(**argsIter) ) {
				resultNumber = operation(*startNumber, *((NumericObject*)*argsIter));
				startNumber->deref();
				startNumber = resultNumber;
			} else {
				printSystemFunctionWrongArg( functionId, argIndex, 0, (*argsIter)->getType(), NumericObject::object_type, false );
			}
		}
	}
	if ( resultNumber ) {
		lastObject.setWithoutRef(resultNumber);
	}
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_solo_num(
	FuncFoundTask& task,
	NumericObject* (*operation)(NumericObject&),
	SystemFunction::Value  functionId
) {
	NumericObject*  number = REAL_NULL;
	ArgsIter argsIter = task.args.start();
	if ( argsIter.has() ) {
		if ( isNumericObject(**argsIter) ) {
			number = (NumericObject*)*argsIter;
			number = operation( *number );
		} else {
			printSystemFunctionWrongArg( functionId, 1, 1, (*argsIter)->getType(), NumericObject::object_type, false );
			return FuncExecReturn::Ran;
		}
	}
	if ( number ) {
		lastObject.setWithoutRef(number);
	}
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_num_equal( FuncFoundTask& task ) {
	bool  result = true;
	NumericObject*  first;
	ArgsIter  argsIter = task.args.start();
	UInteger  argIndex = 0; // For printing

	if ( argsIter.has() ) {
		if ( isNumericObject(**argsIter) ) {
			first = (NumericObject*)*argsIter;
		} else {
			printSystemFunctionWrongArg( SystemFunction::_num_equal, 1, 0,
				(*argsIter)->getType(), NumericObject::object_type, false );

			return FuncExecReturn::Ran;
		}
		while ( argsIter.next() ) {
			++argIndex;
			if ( isNumericObject(**argsIter) ) {
				if ( ! first->isEqualTo(*((NumericObject*)*argsIter)) ) {
					result = false;
					break;
				}
			} else {
				printSystemFunctionWrongArg( SystemFunction::_num_equal, argIndex, 0,
					(*argsIter)->getType(), NumericObject::object_type, false );
			}
		}
	}
	lastObject.setWithoutRef( new BoolObject(result) );
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_num_greater_than( FuncFoundTask& task, bool flip ) {
	bool result = true;
	NumericObject*  first;
	ArgsIter argsIter = task.args.start();
	UInteger  argIndex = 0; // For printing

	if ( argsIter.has() ) {
		if ( isNumericObject(**argsIter) ) {
			first = (NumericObject*)*argsIter;
		} else {
			print( LogMessage::create(LogLevel::warning)
				.SystemFunctionId( flip ? SystemFunction::_num_less_or_equal : SystemFunction::_num_greater_than )
				.Message(EngineMessage::WrongArgType)
				.ArgIndex(1)
				.GivenArgType((*argsIter)->getType())
				.ExpectedArgType(NumericObject::object_type)
			);
			return FuncExecReturn::Ran;
		}
		while ( argsIter.next() ) {
			++argIndex;
			if ( isNumericObject(**argsIter) ) {
				if ( flip ) {
					// Less than or equal
					if ( first->isGreaterOrEqual(*((NumericObject*)*argsIter)) ) {
						result = false;
						break;
					}
				} else {
					// Greater than
					if ( ! first->isGreaterThan(*((NumericObject*)*argsIter)) ) {
						result = false;
						break;
					}
				}
			} else {
				print( LogMessage::create(LogLevel::warning)
					.SystemFunctionId( flip ? SystemFunction::_num_less_or_equal : SystemFunction::_num_greater_than )
					.Message(EngineMessage::WrongArgType)
					.ArgIndex(argIndex)
					.GivenArgType((*argsIter)->getType())
					.ExpectedArgType(NumericObject::object_type)
				);
			}
		}
	}
	lastObject.setWithoutRef( new BoolObject(result) );
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_num_greater_or_equal( FuncFoundTask& task, bool flip ) {
	bool  result = true;
	NumericObject*  first;
	ArgsIter  argsIter = task.args.start();
	UInteger  argIndex = 0; // For printing

	if ( argsIter.has() ) {
		if ( isNumericObject(**argsIter) ) {
			first = (NumericObject*)*argsIter;
		} else {
			print( LogMessage::create(LogLevel::warning)
				.SystemFunctionId( flip ? SystemFunction::_num_less_than : SystemFunction::_num_greater_or_equal )
				.Message(EngineMessage::WrongArgType)
				.ArgIndex(1)
				.GivenArgType((*argsIter)->getType())
				.ExpectedArgType(NumericObject::object_type)
			);
			return FuncExecReturn::Ran;
		}
		while ( argsIter.next() ) {
			++argIndex;
			if ( isNumericObject(**argsIter) ) {
				if ( flip ) {
					// Less than
					if ( first->isGreaterThan(*((NumericObject*)*argsIter)) ) {
						result = false;
						break;
					}
				} else {
					// Greater than or equal
					if ( ! first->isGreaterOrEqual(*((NumericObject*)*argsIter)) ) {
						result = false;
						break;
					}
				}
			} else {
				print( LogMessage::create(LogLevel::warning)
					.SystemFunctionId( flip ? SystemFunction::_num_less_than : SystemFunction::_num_greater_or_equal )
					.Message(EngineMessage::WrongArgType)
					.ArgIndex(argIndex)
					.GivenArgType((*argsIter)->getType())
					.ExpectedArgType(NumericObject::object_type)
				);
			}
		}
	}
	lastObject.setWithoutRef( new BoolObject(result) );
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_num_abs( FuncFoundTask& task ) {
	ArgsIter argsIter = task.args.start();
	if ( argsIter.has() ) {
		if ( isNumericObject(**argsIter) ) {
			lastObject.setWithoutRef( ((NumericObject*)*argsIter)->absValue() );
		} else {
			printSystemFunctionWrongArg( SystemFunction::_num_abs, 1, 1,
				(*argsIter)->getType(), NumericObject::object_type, false );
		}
	}
	return FuncExecReturn::Ran;
}

NumericObject*
Engine::process_sys_num_add( NumericObject& first, NumericObject& second ) {
	return first.add(second);
}

NumericObject*
Engine::process_sys_num_subtract( NumericObject& first, NumericObject& second ) {
	return first.subtract(second);
}

NumericObject*
Engine::process_sys_num_multiply( NumericObject& first, NumericObject& second ) {
	return first.multiply(second);
}

NumericObject*
Engine::process_sys_num_divide( NumericObject& first, NumericObject& second ) {
	return first.divide(second);
}

NumericObject*
Engine::process_sys_num_modulus( NumericObject& first, NumericObject& second ) {
	return first.modulus(second);
}

NumericObject*
Engine::process_sys_num_incr( NumericObject& base ) {
	// TODO: Add an increment function to the objects themselves, which is much faster
	NumericObject* out = base.add(IntegerObject(1));
	base.setValue(*out);
	return out;
}

NumericObject*
Engine::process_sys_num_decr( NumericObject& base ) {
	// TODO: Add a decrement function to the objects themselves, which is much faster
	NumericObject* out = base.subtract(IntegerObject(1));
	base.setValue(*out);
	return out;
}

NumericObject*
Engine::process_sys_num_incr2( NumericObject& first, NumericObject& second ) {
	NumericObject* result = first.add(second);
	first.setValue(*result);
	result->deref();
	first.ref();
	return &first;
}

NumericObject*
Engine::process_sys_num_decr2( NumericObject& first, NumericObject& second ) {
	NumericObject* result = first.subtract(second);
	first.setValue(*result);
	result->deref();
	first.ref();
	return &first;
}

void
addNewForeignFunc(
	Engine& pEngine,
	const String& pName,
	ForeignFunc* pFunction
) {
	pEngine.addForeignFunction(pName, pFunction);
	pFunction->deref();
}

void addForeignFuncInstance(
	Engine&  pEngine,
	const String&  pName,
	bool (*pFunction)( FFIServices& )
) {
	ForeignFunc* ff = new ForeignFuncWrapper(pFunction);
	pEngine.addForeignFunction(pName, ff);
	ff->deref();
}

CallbackWrapper::CallbackWrapper(
	Engine&  pEngine,
	const String&  pName
)
	: engine( pEngine )
	, callback(REAL_NULL)
{
	engine.addForeignFunction(pName, this);
}

CallbackWrapper::~CallbackWrapper() {
	if ( callback ) {
		callback->disown(this);
		callback->deref();
	}
}

bool
CallbackWrapper::run() {
	if ( callback ) {
		return engine.runFunctionObject(callback) != EngineResult::Error;
	}
	return true;
}

bool
CallbackWrapper::owns(
	FunctionObject*  container
) const {
	return notNull(callback) && callback == container;
}

bool
CallbackWrapper::call(
	FFIServices&  ffi
) {
	if ( ffi.demandArgCount(1) && ffi.demandArgType(0, ObjectType::Function) ) {
		callback = (FunctionObject*)&(ffi.arg(0));
		callback->ref();
		callback->changeOwnerTo(this);
		return true;
	}
	return false;
}


} // end namespace Cu
