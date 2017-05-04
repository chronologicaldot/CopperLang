// Copyright 2016 Nicolaus Anderson

#include "Copper.h"

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
// Single character constants
static const char CONSTANT_COMMENT_TOKEN = '#';
static const char CONSTANT_STRING_TOKEN = '"';
static const char CONSTANT_ESCAPE_CHARACTER_TOKEN = '\\';

// *********** OPERATION PROCESSING BASE COMPONENTS **********

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
	ParserContext context;
	context.setCodeStrand(codes);
	context.setTokenSource(tokens);
	if ( isNull(engine) )
		return false; //throw 0;
	if ( isEmpty() )
		return false;
	switch( engine->parse(context, true) ) {
	case ParseResult::More: // Wat?
		// Should throw
		return false;
	case ParseResult::Error:
		state = HasErrors;
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
	body.set( other.body.raw() );
	params = other.params; // If params is changed to a pointer, this has to be changed to a copy
	*persistentScope = *(other.persistentScope);
	result.set( other.result.raw() ); // Should this be copied?
}

void
Function::addParam( const String pName ) {
	params.push_back(pName);
}

//--------------------------------------

FunctionContainer::FunctionContainer(Function* pFunction, unsigned int id)
	: Object(ObjectType::Function)
	, funcBox()
	, owner(REAL_NULL)
	, ID(id)
{
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionContainer constructor (Function*) [%p]\n", (void*)this);
#endif
	type = ObjectType::Function;
	funcBox.set(pFunction);
}

FunctionContainer::FunctionContainer()
	: Object(ObjectType::Function)
	, funcBox()
	, owner(REAL_NULL)
	, ID(0)
{
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionContainer constructor 2 [%p]\n", (void*)this);
#endif
	type = ObjectType::Function;
	funcBox.setWithoutRef(new Function());
}

FunctionContainer::FunctionContainer(const FunctionContainer& pOther)
	: Object(ObjectType::Function)
	, funcBox()
	, owner(REAL_NULL)
	, ID(pOther.ID)
{
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionContainer constructor 3 (const FunctionContainer&) [%p]\n", (void*)this);
#endif
	funcBox.set(pOther.funcBox.raw());
	owner = pOther.owner;
}

FunctionContainer::~FunctionContainer()
{
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionContainer::~FunctionContainer [%p]\n", (void*)this);
#endif
}

void FunctionContainer::own(Variable* pGrabber) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionContainer::own [%p]\n", (void*)this);
#endif
	// Finders keepers
	if ( isNull(owner) )
		owner = pGrabber;
	//ref(); // Called by RefPtr already, so I'm using FuncBoxRefPtr for variables instead.
}

void FunctionContainer::disown(Variable* pDropper) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionContainer::disown [%p]\n", (void*)this);
#endif
	// Language spec says abandoned functions must be destroyed
	if ( notNull(owner) && owner == pDropper ) {
		funcBox.set(REAL_NULL);
		owner = REAL_NULL;
	}
	//deref(); // Called by RefPtr already, so I'm using FuncBoxRefPtr for variables instead.
}

bool FunctionContainer::isOwned() const {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionContainer::isOwned [%p]\n", (void*)this);
#endif
	return notNull(owner);
}

bool FunctionContainer::isOwner( const Variable* pVariable ) const {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionContainer::isOwner [%p]\n", (void*)this);
#endif
	return owner == pVariable;
}

void FunctionContainer::changeOwnerTo(Variable* pNewOwner) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionContainer::changeOwnerTo [%p]\n", (void*)this);
#endif
	// NOTE: This performs no ref() or deref(). Those should have been performed elsewhere automatically.

	if ( pNewOwner->getRawContainer() == this ) {
		owner = pNewOwner;
	} else {
#ifdef COPPER_VAR_LEVEL_MESSAGES
		std::printf("[ERROR: Attempting to change owner to variable that doesn't own the container.\n");
#endif
		throw BadFunctionContainerOwnerException();
	}
}

bool FunctionContainer::getFunction(Function*& storage) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionContainer::getFunction [%p]\n", (void*)this);
#endif
	return funcBox.obtain(storage);
}

void FunctionContainer::setFrom( FunctionContainer& pOther ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionContainer::setFrom [%p]\n", (void*)this);
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

Object* FunctionContainer::copy() {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionContainer::copy [%p]\n", (void*)this);
#endif
	Function* f = new Function();
	Function* mine;
	if ( funcBox.obtain(mine) ) {
		f->set( *mine );
	}
	FunctionContainer* c = new FunctionContainer(f, ID);
	f->deref();
	return c;
}

//--------------------------------------


Variable::Variable()
	: box(new FunctionContainer())
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
	if ( isNull(box) )
		throw NullVariableException();

	box->disown(this);
	box->deref();
	box = new FunctionContainer();
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
Variable::setFunc( FunctionContainer* pContainer, bool pReuseStorage ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Variable::setFunc [%p]\n", (void*)this);
#endif
	if ( isNull(pContainer) ) {
		throw BadParameterException<Variable>();
	}
	if ( isNull(box) )
		throw NullVariableException();

	FunctionContainer* fc = REAL_NULL;

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
		fc = (FunctionContainer*)(pContainer->copy());
		box->disown(this);
		box->deref();
		box = fc;
		box->own(this);
	}
}

// Used only for data
void
Variable::setFuncReturn( Object* pData ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Variable::setFuncReturn [%p]\n", (void*)this);
#endif
	if ( isNull(box) )
		throw NullVariableException();

	Function* f = REAL_NULL;
	if ( box->getFunction(f) ) {
//std::printf("[DEBUG: Variable::setFuncReturn: using current function\n");
		f->result.set(pData->copy());
		f->constantReturn = true;
	} else {
//std::printf("[DEBUG: Variable::setFuncReturn: creating new container\n");
		box->disown(this);
		box->deref();
		box = FunctionContainer::createInitialized(pData);
		box->own(this);
	}
}

// Used for FuncFound_processRunVariable
Function*
Variable::getFunction(Logger* logger) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Variable::getFunction [%p]\n", (void*)this);
#endif
	Function* f = REAL_NULL;

	if ( isNull(box) )
		throw NullVariableException();

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
	box = new FunctionContainer();
	box->own(this);
	box->getFunction(f);
	return f;
}

bool
Variable::isPointer() const {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Variable::isPointer [%p]\n", (void*)this);
#endif
	if ( isNull(box) )
		throw NullVariableException();
	return ! box->isOwner(this);
}

FunctionContainer*
Variable::getRawContainer() {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Variable::getRawContainer [%p]\n", (void*)this);
#endif
	if ( isNull(box) )
		throw NullVariableException();
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
	FunctionContainer* fc = (FunctionContainer*)(box->copy());
	var->box = fc;
	var->box->own(var);
	return var;
}

//--------------------------------------

RefVariableStorage::RefVariableStorage()
	: variable( new Variable() )
{
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: RefVariableStorage cstor 1 [%p]\n", (void*)this);
#endif
}

RefVariableStorage::RefVariableStorage(Variable& refedVariable)
	: variable(REAL_NULL)
{
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: RefVariableStorage cstor 2 (Variable&) [%p]\n", (void*)this);
#endif
	// Note: Remove this line if there are copy problems.
	// Copying should be the default, but it is expensive.
	variable = refedVariable.getCopy();
}

RefVariableStorage::RefVariableStorage(const RefVariableStorage& pOther)
	: variable(REAL_NULL)
{
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: RefVariableStorage cstor 3 (const RefVariableStorage&) [%p]\n", (void*)this);
#endif
	// Note: Remove this line if there are copy problems.
	// Copying should be the default, but it is expensive.
	variable = pOther.variable->getCopy();
}

RefVariableStorage::~RefVariableStorage() {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: RefVariableStorage::~RefVariableStorage [%p]\n", (void*)this);
#endif
	variable->deref();
}

Variable&
RefVariableStorage::getVariable() {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: RefVariableStorage::getVariable [%p]\n", (void*)this);
#endif
	return *variable;
}

//--------------------------------------

void Scope::copyAsgnFromHashTable( RobinHoodHash<RefVariableStorage>& pTable ) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::copyAsgnFromHashTable\n");
#endif
	CHECK_SCOPE_HASH_NULL(robinHoodTable)

	delete robinHoodTable;
	robinHoodTable = new RobinHoodHash<RefVariableStorage>(pTable);
}

Scope::Scope()
	: robinHoodTable(REAL_NULL)
{
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope constructor 1 (bool) [%p]\n", (void*)this);
#endif
	robinHoodTable = new RobinHoodHash<RefVariableStorage>(100);
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

	return &( robinHoodTable->insert(pName, RefVariableStorage())->getVariable() );
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
		pVariable = &( robinHoodTable->insert(pName, RefVariableStorage())->getVariable() );
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
// The pMakePointer option may be ignored if the FunctionContainer is not owned
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
			var->setFunc( (FunctionContainer*)pObject, pReuseStorage );
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
				obj = new ObjectString(bucket->data->name);
				aoi->append(obj);
				obj->deref();
			}
		}
		return;
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

	// ^ I don't think this will work, but it might.
/*
	// Creating a new scope is required for proper copy construction to delink pointers
	// Use a list version to make it easier to copy
	Scope newScope(false);
	newScope = pOther;
	List<ListVariableStorage>::Iter li = newScope.listTable->start();
	if ( li.has() )
	do {
		setVariable( (*li).name, &((*li).getVariable()), false );
	} while( li.next() );
*/
}

unsigned int Scope::occupancy() {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::occupancy\n");
#endif
	CHECK_SCOPE_HASH_NULL(robinHoodTable)

	return robinHoodTable->getOccupancy();
}

//--------------------------------------

Stack::Stack()
	: size(1)
	, bottom(REAL_NULL)
	, top(REAL_NULL)
{
	bottom = new StackFrame();
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
}

void
Stack::clear() {
	StackFrame* parent;
	while( notNull(top) ) {
		parent = top->parent;
		top->deref();
		top = parent;
	}
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

unsigned int
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
Stack::push() {
	if ( isNull(bottom) ) {
		bottom = new StackFrame();
		top = bottom;
		++size;
		return;
	}
	StackFrame* parent = top;
	top = new StackFrame();
	top->parent = parent;
	++size;
}

void
Stack::push(StackFrame* pContext) {
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

StringOpcode::StringOpcode(
	Opcode::Value	pType,
	const String&	pName
)
	: Opcode( pType )
	, name( pName )
{}

StringOpcode::StringOpcode(
	const StringOpcode& pOther
)
	: Opcode( pOther.type )
	, name( pOther.name )
{}

String
StringOpcode::getNameData() const {
	return name;
}

Opcode*
StringOpcode::getCopy() const {
	return new StringOpcode(type, name);
}

BodyOpcode::BodyOpcode()
	: Opcode( Opcode::FuncBuild_execBody )
	, body()
{
	body.setWithoutRef(new Body());
}

void
BodyOpcode::addToken(
	const Token& pToken
) {
	body.raw()->addToken(pToken);
}

Body*
BodyOpcode::getBody() const {
	return body.raw();
}

Opcode*
BodyOpcode::getCopy() const {
	BodyOpcode* out = new BodyOpcode();
	out->body.set(body.raw());
	return out;
}

GotoOpcode::GotoOpcode(
	const Opcode::Value		pType
)
	: Opcode(pType)
	, target(REAL_NULL)
{}

GotoOpcode::GotoOpcode(
	const GotoOpcode& pOther
)
	: Opcode(pOther.type)
	, target(REAL_NULL)
{
	if ( pOther.target )
		target = new OpStrandIter(*(pOther.target));
}

GotoOpcode::~GotoOpcode() {
	if ( notNull(target) )
		delete target;
}

void
GotoOpcode::setTarget(
	const OpStrandIter pTarget
) {
	target = new OpStrandIter(pTarget);
}

OpStrandIter&
GotoOpcode::getOpStrandIter() {
	if ( isNull(target) )
		throw NullGotoOpcodeException();
	return *target;
}

Opcode*
GotoOpcode::getCopy() const {
	return new GotoOpcode(*this);
}

FuncFoundTask::FuncFoundTask(
	const VarAddress& pVarAddress
)
	: Task(TaskName::FuncFound)
	, varAddress(pVarAddress)
	, args()
{}

FuncFoundTask::FuncFoundTask(
	const FuncFoundTask& pOther
)
	: Task(TaskName::FuncFound)
	, varAddress(pOther.varAddress)
	, args(pOther.args)
{}

FuncFoundTask::~FuncFoundTask() {
	ArgsIter ai = args.start();
	if ( ai.has() )
	do {
		(*ai)->deref();
	} while ( ai.next() );
	// args.clear(); // implicit
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

AddressOpcode::AddressOpcode(
	const Opcode::Value value
)
	: Opcode(value)
	, varAddress()
{}

AddressOpcode::AddressOpcode(
	const Opcode::Value		value,
	const VarAddress&		pAddress
)
	: Opcode(value)
	, varAddress(pAddress)
{}

const VarAddress*
AddressOpcode::getAddressData() const {
	return &varAddress;
}

Opcode*
AddressOpcode::getCopy() const {
	return new AddressOpcode(type, varAddress);
}

FuncFoundOpcode::FuncFoundOpcode(
	const String& pStartName
)
	: AddressOpcode(Opcode::FuncFound_access)
{
	varAddress.push_back(pStartName);
}

FuncFoundOpcode::FuncFoundOpcode(
	const FuncFoundOpcode& pOther
)
	: AddressOpcode(pOther.type)
{
	varAddress = pOther.varAddress;
}

Task*
FuncFoundOpcode::getTask() const {
	return new FuncFoundTask(varAddress);
}

Opcode*
FuncFoundOpcode::getCopy() const {
	return new FuncFoundOpcode(*this);
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
	GotoOpcode* jump
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
	GotoOpcode* jump
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
	List<GotoOpcode*>::Iter fg = finalGotos.start();
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
	GotoOpcode* code
) {
#ifdef COPPER_PARSER_LEVEL_MESSAGES
	std::printf("[DEBUG: LoopStructureParseTask::setGotoOpcodeToLoopStart\n");
#endif
	code->setTarget( firstIter );
}

void
LoopStructureParseTask::queueFinalGoto(
	GotoOpcode* code
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
	List<GotoOpcode*>::Iter fg = finalGotos.start();
	if ( fg.has() )
	do {
		(*fg)->setTarget( lastIter );
	} while ( fg.next() );
	finalGotos.clear();
}

// *********** FOREIGN FUNCTION INTERFACE ************

FFIServices::FFIServices(
	Engine&			enginePtr,
	ArgsIter		paramsStart
)
	: engine(enginePtr)
	, paramsIter(paramsStart)
	, done( ! paramsIter.has() )
{}

// returns each successive parameter sent to the function
Object*
FFIServices::getNextArg() {
	Object* out;
	if ( done )
		throw FFIMisuseException(); // Prevents accidental misuse

	out = *paramsIter;
	done = ! paramsIter.next();
	return out;
}

bool
FFIServices::hasMoreArgs() {
	return !done;
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
FFIServices::setResult(
	Object* obj
) {
	engine.lastObject.set(obj);
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
	VarAddressConstIter addrIter = address.constStart();
	CharList builder;
	if ( addrIter.has() )
	do {
		builder.append( *addrIter );
		if ( addrIter.atEnd() == false ) {
			builder.push_back('.');
		}
	} while ( addrIter.next() );
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
	//, stackTracePrintingEnabled(false)
	//, numberObjectFactoryPtr()
	, nameFilter(REAL_NULL)
{
	setupSystemFunctions();

	// TODO:
	// Initialize the number object factory to the default.
	//numberObjectFactoryPtr.setWithoutRef(new NumberObjectFactory());

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

void Engine::print(const LogLevel::Value& logLevel, const char* msg) const {
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

void Engine::print(const LogLevel::Value& logLevel, const EngineMessage::Value& msg) const {
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

void
Engine::addForeignFunctionInstance(
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

/*
void
Engine::addForeignFunction(
	const String&	pName,
	bool			(*pFunction)( FFIServices& ),
	bool			pIsVariadic
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::addForeignFunction");
#endif
	ForeignFunctionWrapper* ffw = new ForeignFunctionWrapper(pFunction, pIsVariadic );
	foreignFunctions.insert(pName, ForeignFuncContainer(ffw));
	ffw->deref();
}
*/

EngineResult::Value
Engine::run(
	ByteStream& stream
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::run");
#endif
	switch( lexAndParse(stream, false) ) {
	case ParseResult::Done:
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
		printGlobalStrand();
#endif
		return execute();
	case ParseResult::Error:
		return EngineResult::Error;
	case ParseResult::More:
		return EngineResult::Ok;
	// to get -Wall to stop griping:
	default:
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
			std::printf("[ Opcode = %p, value = %u\n", (void*)(opIter->getOp()), (unsigned int)(opIter->getOp()->getType()) );
		} while ( opIter.next() );
	} else {
		print(LogLevel::debug, "[DEBUG: Global strand is empty.");
	}
}
#endif

ParseResult::Value
Engine::lexAndParse(
	ByteStream& stream,
	bool srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::lexAndParse");
#endif
#ifdef COPPER_SPEED_PROFILE
	std::printf("PROFILE Engine::lexAndParse() start\n");
	time_t endTime;
	time_t startTime = clock();
#endif
	char c;
	CharList tokenValue; // Since I have to build with it, it's an easy-to-append-to list
	//List<Token> bufferedTokens; // Within the engine, so it's state is saved
	TokenType tokenType;

	while ( ! stream.atEOS() ) {
		c = stream.getNextByte();

		if ( c == '\0' )
			throw RandomNullByteInStream();

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

#ifdef COPPER_SPEED_PROFILE
	endTime = clock();
	std::printf("PROFILE Engine::lexAndParse() time = %f\n", static_cast<double>(endTime - startTime) * 1000.0 / (double)CLOCKS_PER_SEC);
#endif

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

void Engine::clearStacks() {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::clearStacks");
#endif
	taskStack.clear();
	stack.clear();
	//globalParserContext.clear();
	//globalParserContext.moveToFirstUnusedToken(); // may conflict with ParserContext.onError() ?

	// Reinitialized the stack
	// This is useful if the program should continue
	stack.push();
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
		switch( obj->getType() ) {
		case ObjectType::Function:
			var->setFunc((FunctionContainer*)obj, reuseStorage);
			lastObject.set(var->getRawContainer());
			break;
		//case ObjectType::Data:
		default:
			var->setFuncReturn( obj );
			lastObject.set(var->getRawContainer());
			break;
		}
	} else {
		lastObject.setWithoutRef(new FunctionContainer());
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
		if ( pName.isLiteralNumber() ) {
			return TT_number;
		} else {
			print(LogLevel::error, EngineMessage::MalformedNumber);
			return TT_malformed;
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

//std::printf("[DEBUG: token id =%u\n", (unsigned int)tokenType);

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
	//return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool
Engine::isValidName( const String& pName ) const {
	//if ( pName.size() > 256 )
	//	return false;
		// For checking Unicode if desired
	if ( nameFilter ) {
		return nameFilter(pName);
	}
	unsigned int i=0;
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
	print( LogLevel::error, EngineMessage::StreamInteruptedInComment );
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
	print( LogLevel::error, EngineMessage::StreamInteruptedInString );
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
	builtinFunctions.insert(String("union"), SystemFunction::_union);
	builtinFunctions.insert(String("type_of"), SystemFunction::_type);
	builtinFunctions.insert(String("are_same_type"), SystemFunction::_are_same_type);
	builtinFunctions.insert(String("are_bool"), SystemFunction::_are_bool);
	builtinFunctions.insert(String("are_string"), SystemFunction::_are_string);
	builtinFunctions.insert(String("are_number"), SystemFunction::_are_number);
	builtinFunctions.insert(String("assert"), SystemFunction::_assert);
}

//-------------------------------------
//********** PARSING SYSTEM ***********

// The ParserContext must be passed in by whatever called this function. If none is provided, a new
// context is made.
ParseResult::Value
Engine::parse(
	ParserContext&	context,
	bool			srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::parse");
#endif
#ifdef COPPER_SPEED_PROFILE
	std::printf("PROFILE Engine::parse() start\n");
	time_t endTime;
	time_t startTime = clock();
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
#ifdef COPPER_SPEED_PROFILE
	endTime = clock();
	std::printf("PROFILE Engine::parse()=More time = %f\n", static_cast<double>(endTime - startTime) * 1000.0 / (double)CLOCKS_PER_SEC);
#endif
					return ParseResult::More;

				case ParseTask::Result::task_done:
					context.taskStack.pop();
					parseIter.makeLast();
					break;

				case ParseTask::Result::interpret_token:
					// Tasks requiring another token that won't exist will gum up the works.
					if ( context.isFinished() ) {
						if ( srcDone ) {
							print(LogLevel::error, "Token source finished before parsing.");
							return ParseResult::Error;
						} else {
#ifdef COPPER_SPEED_PROFILE
	endTime = clock();
	std::printf("PROFILE Engine::parse()=More time = %f\n", static_cast<double>(endTime - startTime) * 1000.0 / (double)CLOCKS_PER_SEC);
#endif
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
#ifdef COPPER_SPEED_PROFILE
	endTime = clock();
	std::printf("PROFILE Engine::parse() time = %f\n", static_cast<double>(endTime - startTime) * 1000.0 / (double)CLOCKS_PER_SEC);
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
			print(LogLevel::error, "Unfinished parsing.");
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

	Token currToken = context.peekAtToken();

	switch( currToken.type ) {

	case TT_malformed:
		print(LogLevel::error, EngineMessage::MalformedToken);
		return ParseResult::Error;

	case TT_unknown:
		print(LogLevel::error, EngineMessage::UnknownToken);
		return ParseResult::Error;

	//---------------

	case TT_comment:
		// This shouldn't be in the parser
		throw (unsigned int)TT_comment;

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
		context.addNewOperation( new FuncBuildOpcode() );
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
		context.addNewOperation( new FuncBuildOpcode() );
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
		if ( context.peekAtToken().name.equals("own") ) {
			addNewParseTask(
				context.taskStack,
				new SRPSParseTask(Opcode::Own)
			);
			return ParseResult::More;
		}
		// else
		if ( context.peekAtToken().name.equals("is_owner") ) {
			addNewParseTask(
				context.taskStack,
				new SRPSParseTask(Opcode::Is_owner)
			);
			return ParseResult::More;
		}
		// else
		if ( context.peekAtToken().name.equals("is_ptr") ) {
			addNewParseTask(
				context.taskStack,
				new SRPSParseTask(Opcode::Is_pointer)
			);
			return ParseResult::More;
		}
		// else
		// Regular names. Request more processing.
		addNewParseTask(
			context.taskStack,
			new FuncFoundParseTask( currToken.name )
		);
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

	case TT_number:
		context.addNewOperation(
			new StringOpcode(Opcode::CreateNumber, currToken.name)
		);
		break;

	//---------------

	case TT_string:
/*
		// Originally, there was a purging mechanism, which should also be applied here.
#ifdef COPPER_PURGE_NON_PRINTABLE_ASCII_INPUT_STRINGS
		ObjectString* objStr = new ObjectString(value);
		objStr->purgeNonPrintableASCII();
		lastObject.setWithoutRef( objStr );
#else
		lastObject.setWithoutRef( new ObjectString(value) );
#endif
*/
		context.addNewOperation(
			new StringOpcode(Opcode::CreateString, currToken.name)
		);
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
			new StringOpcode( Opcode::FuncBuild_assignToVar, task->paramName )
		);
		task->state = FuncBuildParseTask::State::CollectParams;
		return ParseFunctionBuild_CollectParameters(task, context, srcDone);

	case FuncBuildParseTask::State::AwaitPointerAssignment:
		context.addNewOperation(
			new StringOpcode( Opcode::FuncBuild_pointerAssignToVar, task->paramName )
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
			print(LogLevel::error, "Unfinished object body.");
			return ParseTask::Result::syntax_error;
		} else {
			return ParseTask::Result::need_more;
		}
	}

	// Check for the other object-body close token.
	// If the tokens run out before it is found, request more.
	// Otherwise, error.
	unsigned int openBrackets = 1;

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
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
std::printf("[DEBUG: Engine::ParseFunctionBuild_FromObjectBody -> CollectParams, token.type == %u\n", (unsigned int)context.peekAtToken().type);
#endif
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
		print(LogLevel::error, "Unfinished object body.");
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
			case TT_number:
			case TT_string:
				task->state = FuncBuildParseTask::State::AwaitAssignment;
				return ParseTask::Result::interpret_token;

			case TT_objectbody_close:
				print(LogLevel::error, "Unfinished parameter assignment in object body.");
				return ParseTask::Result::syntax_error;
			default:
				print(LogLevel::error, "Stray token encountered while collecting names.");
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
				print(LogLevel::error, "Stray token encountered while collecting names.");
				return ParseTask::Result::syntax_error;

			case TT_objectbody_close:
				print(LogLevel::error, "Unfinished parameter pointer assignment in object body.");
				return ParseTask::Result::syntax_error;
			default:
				print(LogLevel::error, "Stray token encountered while collecting names.");
				return ParseTask::Result::syntax_error;
			}
			break;

		case TT_name:
		case TT_objectbody_close:
		case TT_end_segment:
			// Regular parameters
			context.addNewOperation(
				new StringOpcode( Opcode::FuncBuild_createRegularParam, task->paramName )
			);
			break;

		default:
			print(LogLevel::error, "Stray token encountered while collecting names.");
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
		if ( context.moveToNextToken() && context.peekAtToken().type == TT_execbody_open ) {
			context.moveToFirstUnusedToken();
#ifdef COPPER_STRICT_CHECKS
std::printf("[DEBUG: Engine::ParseFunctionBuild_CollectParams -> FromExecBody, token.type == %u\n", (unsigned int)context.peekAtToken().type);

			if ( context.peekAtToken().type != TT_objectbody_close )
				throw ParserTokenException( context.peekAtToken() );
#endif
			context.commitTokenUsage(); // Keep object-body-close token
			task->state = FuncBuildParseTask::State::FromExecBody;
			context.moveToNextToken(); // Move back to execbody open token
			return ParseFunctionBuild_FromExecBody(context, srcDone);
		} else {
			// No execution body, so end here
			context.addNewOperation( new Opcode( Opcode::FuncBuild_end ) );
			context.commitTokenUsage(); // Keep the object-body close
			return ParseTask::Result::task_done;
		}
	}

	// else
	// Error
	print(LogLevel::error, "Invalid token found! Object bodies may have only named parameters.");
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
	unsigned int openBrackets = 1;
	//BodyOpcode* bodyOpcode = new BodyOpcode();

	do {
		// Start inside the execbody open token
		if ( ! context.moveToNextToken() ) {
			if ( srcDone ) {
				print(LogLevel::error, "Function execution body not finished before token stream ended.");
				return ParseTask::Result::syntax_error;
			}
			return ParseTask::Result::need_more;
		}

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

#ifdef COPPER_PARSER_LEVEL_MESSAGES
		std::printf("[DEBUG: Engine::ParseFunctionBuild_FromExecBody - openBrackets = %u\n", openBrackets);
#endif

	} while ( openBrackets > 0 );	

	// Everything worked out
	BodyOpcode* bodyOpcode = new BodyOpcode();
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
#ifdef COPPER_PARSER_LEVEL_MESSAGES
			std::printf("[DEBUG: Engine::ParseFunctionBuild_FromExecBody - added body token(%s, %u)\n",
				context.peekAtToken().name.c_str(), (unsigned int)(context.peekAtToken().type) );
#endif
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
		// First token is a name, the start of a variable's address
		r = moveToFirstUnusedToken(context, srcDone);
		if ( r != ParseTask::Result::task_done )
			return r;

		// Skip the name and get the next token.
		// We know the name is the current token because we immediately come here after interpretToken()
		if ( ! context.moveToNextToken() ) {
			if ( srcDone ) {
				task->code->varAddress.push_back( context.peekAtToken().name );
				context.addOperation( task->code );
				return ParseTask::Result::task_done;
			} else {
				return ParseTask::Result::need_more;
			}
		}

		// It is possible that this is a chain of member names, connected by the member link.
		while( context.peekAtToken().type == TT_member_link ) {
			// Get the next name and restart
			if ( context.moveToNextToken() ) {
				if ( context.peekAtToken().type == TT_name ) {
					task->code->varAddress.push_back(context.peekAtToken().name);
					// Next token should be member-link, assignment, pointer-assignment, or parambody-open
					if ( context.moveToNextToken() ) {
						continue;
					} else {
						// However, this may be a function call or assignment. There is no way of knowing what
						// comes next, so it is imperative that we find out.
						return ParseTask::Result::need_more;
					}
				} else {
					print(LogLevel::error, "Member access followed by invalid token.");
					return ParseTask::Result::syntax_error;
				}
			} else {
				if ( srcDone ) {
					print(LogLevel::error, "Member access incomplete.");
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
					print(LogLevel::error, "Variable assignment incomplete.");
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
					print(LogLevel::error, "Variable pointer assignment incomplete.");
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
		return ParseFuncFound_CollectParams(task, context, srcDone);

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
	case TT_number:
	case TT_string:
		task->state = FuncFoundParseTask::CompleteAssignment;
		return ParseTask::Result::interpret_token;

	default:
		print(LogLevel::error, "Invalid token following assignment.");
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
		print(LogLevel::error, "Invalid token following pointer assignment.");
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
	std::printf("[ currToken.name = %s\n", context.peekAtToken().name.c_str());
#endif
	// Only for scanning
	unsigned int openBodies = 1;

	do {
		switch( context.peekAtToken().type ) {
		case TT_parambody_open:
			if ( openBodies == PARSER_OPENBODY_MAX_COUNT ) {
				print( LogLevel::error, EngineMessage::ExceededParamBodyCountLimit );
				return ParseTask::Result::syntax_error;
			}
			openBodies++;
			break;
		case TT_parambody_close:
			openBodies--;
			if ( openBodies == 0 ) {
				task->state = FuncFoundParseTask::CollectParams;
				task->openBodies = 1;
				context.moveToFirstUnusedToken(); // Start from the beginning
				return ParseFuncFound_CollectParams(task, context, srcDone);
			}
			break;
		default: // Ignore
			break;
		}
	} while ( context.moveToNextToken() );

	//( openBodies > 0 )
	if ( srcDone ) {
		print(LogLevel::error, "Function call parameter collection incomplete.");
		return ParseTask::Result::syntax_error;
	} else {
		return ParseTask::Result::need_more;
	}
}

ParseTask::Result::Value
Engine::ParseFuncFound_CollectParams(
	FuncFoundParseTask*	task,
	ParserContext&		context,
	bool				srcDone
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseFuncFound_CollectParams");
	std::printf("[ currToken.name = %s\n", context.peekAtToken().name.c_str());
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
		if ( ! context.moveToNextToken() ) {
			if (srcDone) {
				print(LogLevel::error, "Function call parameter parsing incomplete.");
				return ParseTask::Result::syntax_error;
			} else {
				return ParseTask::Result::need_more;
			}
		}
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
	case TT_name:
	case TT_boolean_true:
	case TT_boolean_false:
	case TT_number:
	case TT_string:
		task->waitingOnAssignment = true;
		return ParseTask::Result::interpret_token;

	default:
		print(LogLevel::error, "Invalid token in function call parameter parsing.");
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
	std::printf("[DEBUG: Engine::parseIfStructure state = %u\n", (unsigned)(task->state));
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
		print(LogLevel::error, "If-structure must have parameter body.");
		return ParseTask::Result::syntax_error;
	}

	unsigned int openBodies = 1;
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
				print(LogLevel::error, "If-structure conditional must start with boolean value or name.");
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
		print(LogLevel::error, "Incomplete if-structure condition.");
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
	unsigned int openBodies = 1;

	// Move from ")"/"else" to "{"
	if ( !context.moveToNextToken() ) {
		if ( srcDone ) {
			print(LogLevel::error, "Incomplete if-structure body.");
			return ParseTask::Result::syntax_error;
		} else {
			return ParseTask::Result::need_more;
		}
	}
	if ( context.peekAtToken().type != TT_execbody_open ) {
		print(LogLevel::error, "Illegal token following if-structure condition or start of else-section.");
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
		print(LogLevel::error, "Incomplete if-structure body.");
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
	GotoOpcode* code;

	// Keep an eye out for parameter body openers and closers, tracking them in the task
	switch( context.peekAtToken().type ) {
	case TT_parambody_open: // Funny, this is actually an error, but interpretToken() handles it.
		task->openBodies++;
		break;

	case TT_parambody_close:
		task->openBodies--;
		if ( task->openBodies == 0 ) {
			code = new GotoOpcode(Opcode::ConditionalGoto);
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
	GotoOpcode* code;

	switch( context.peekAtToken().type ) {
	case TT_execbody_open:
		task->openBodies++;
		break;

	case TT_execbody_close:
		task->openBodies--;
		if ( task->openBodies == 0 ) {
			context.commitTokenUsage(); // Use this body-closing token
			// Create an opcode at the end of the body for jumping to the end of the if-structure
			code = new GotoOpcode(Opcode::Goto);
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
				print(LogLevel::error, "Elif structure is incomplete.");
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
			print(LogLevel::error, "Loop-structure not complete before stream end.");
			return ParseTask::Result::syntax_error;
		}
		if ( context.peekAtToken().type != TT_execbody_open ) {
			print(LogLevel::error, "Loop-structure encountered bad token before body.");
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
			print(LogLevel::error, "Stream ended before loop structure was completed.");
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
	GotoOpcode* code = REAL_NULL;

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
			code = new GotoOpcode(Opcode::Goto);
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
	ParserContext&			context
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ParseLoop_AddEndLoop");
#endif
#ifdef COPPER_STRICT_CHECKS
	if ( context.peekAtToken().type != TT_endloop )
		throw ParserTokenException( context.peekAtToken() );
#endif
	context.commitTokenUsage(); // Use this token
	// Must search for the top-most loop task in the parser task stack
	GotoOpcode* code = REAL_NULL;
	ParseTaskIter taskIter = context.taskStack.end();
	ParseTask* task;
	do {
		task = taskIter->getTask();
		if ( task->type == ParseTask::Loop ) {
			code = new GotoOpcode(Opcode::Goto);
			((LoopStructureParseTask*)task)->queueFinalGoto(code);
			context.addNewOperation(code);
			// Unfortunately, adding the code as a new operation would delink it from the
			// final goto queue if Opcodes are copied in the main constructor of OpcodeContainer.
			// Hence, OpcodeContainer must use references.
			return ParseResult::Done;
		}
	} while ( taskIter.prev() );
	print(LogLevel::error, "Loop-stop token found outside a loop.");
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
	context.commitTokenUsage(); // Use this token
	// Must search for the top-most loop task in the parser task stack
	GotoOpcode* code;
	ParseTaskIter taskIter = context.taskStack.end();
	ParseTask* task;
	do {
		task = taskIter->getTask();
		if ( task->type == ParseTask::Loop ) {
			code = new GotoOpcode(Opcode::Goto);
			((LoopStructureParseTask*)task)->setGotoOpcodeToLoopStart(code);
			context.addNewOperation(code);
			return ParseResult::Done;
		}
	} while ( taskIter.prev() );
	print(LogLevel::error, "Loop-stop token found outside a loop.");
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
	if ( !context.moveToNextToken() ) {
		if ( srcDone ) {
			print(LogLevel::error, "SRPS (own/is_owner/is_ptr) is missing parameter body.");
			return ParseTask::Result::syntax_error;
		} else {
			return ParseTask::Result::need_more;
		}
	}
	// Next: scan for closing of parameter body
	unsigned int openBodies = 1;
	while ( openBodies != 0 ) {
		if ( !context.moveToNextToken()) {
			if ( srcDone ) {
				print(LogLevel::error, "SRPS (own/is_owner/is_ptr) is missing parameter body.");
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
		print(LogLevel::error, "SRPS (own/is_owner/is_ptr) contains invalid token for parameter.");
		return ParseTask::Result::syntax_error;
	}

	// It'd be faster if I saved directly to the opcode instead
	//VarAddress varAddress;
	AddressOpcode* addressCode = new AddressOpcode( task->codeType );

	while ( context.peekAtToken().type == TT_name ) {
		addressCode->varAddress.push_back( context.peekAtToken().name );
		context.moveToNextToken();
		switch( context.peekAtToken().type ) {
		case TT_parambody_close:
			break;

		case TT_member_link:
			context.moveToNextToken();
			continue;

		default:
			print(LogLevel::error, "SRPS (own/is_owner/is_ptr) contains invalid token for parameter.");
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
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::execute");
#endif
#ifdef COPPER_SPEED_PROFILE
	std::printf("PROFILE Engine::execute() start\n");
	fullTime = 0;
	double elapsedTime = 0;
	rusage startRusage, endRusage;
	getrusage(RUSAGE_SELF, &startRusage);
#endif

	// Note: When adding a list of operations, also set currOp.
	OpStrandStackIter opcodeStrandStackIter = opcodeStrandStack.end();
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
				// Should check for the end_main opcode, which ends everything

				switch( operate( opcodeStrandStackIter, *currOp ) ) {
				case ExecutionResult::Ok:
					break;

				case ExecutionResult::Error:
					return EngineResult::Error;

				case ExecutionResult::Done:
					clearStacks();
					signalEndofProcessing();
#ifdef COPPER_SPEED_PROFILE
	getrusage(RUSAGE_SELF, &endRusage);
	elapsedTime = (endRusage.ru_utime.tv_sec - startRusage.ru_utime.tv_sec) * 1000.0
					+ (endRusage.ru_utime.tv_usec - startRusage.ru_utime.tv_usec) / 1000.0;
	std::printf("PROFILE Engine::execute() Done time = %f\n", elapsedTime);
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
				// At the global opcodes level, so don't pop. Instead, just add a terminal.
				// This is also for preventing all other operations from being repeated.
				globalParserContext.addNewOperation( new Opcode(Opcode::Terminal) );
				currOp->next(); //... and go to it.
				break;
			}

			// Not at the global opcodes strand, so it's safe to pop the strand
			// Functions should pop variable/scope stack contexts.
			stack.pop();
			opcodeStrandStack.pop();
			opcodeStrandStackIter.makeLast();
			currOp = &(opcodeStrandStackIter->getCurrOp());

		} while ( opcodeStrandStackIter->getCurrOp().atEnd() );

	} while ( ! opcodeStrandStackIter.atStart() || ! currOp->atEnd() );

	//std::printf("[Debug: Engine::execute final currOp index = %lu, type = %u\n",
	//	opcodeStrandStackIter->getCurrStrand()->indexOf(*currOp),
	//	(unsigned int)((*currOp)->getOp()->getType())
	//);

	//printGlobalStrand();

	// Clear all first-level opcodes that have been run up to this point
	opcodeStrandStack.start()->removeAllUpToCurrentCode();

#ifdef COPPER_SPEED_PROFILE
	getrusage(RUSAGE_SELF, &endRusage);
	elapsedTime = (endRusage.ru_utime.tv_sec - startRusage.ru_utime.tv_sec) * 1000.0
					+ (endRusage.ru_utime.tv_usec - startRusage.ru_utime.tv_usec) / 1000.0;

	std::printf("PROFILE Engine::execute() Normal time = %f\n", elapsedTime);
	std::printf("PROFILE Engine::execute() Section full time = %f\n", fullTime);
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
	const Opcode* opcode = opIter->getOp();
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
		addNewTaskToStack( opcode->getTask() );
		break;

	case Opcode::FuncBuild_createRegularParam:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode FuncBuild_createRegularParam");
#endif
		task = getLastTask();
		if ( task->name == TaskName::FuncBuild ) {
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
		if ( task->name == TaskName::FuncBuild ) {
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
		if ( task->name == TaskName::FuncBuild ) {
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
		if ( task->name == TaskName::FuncBuild ) {
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
		if ( task->name == TaskName::FuncBuild ) {
			lastObject.setWithoutRef(
				new FunctionContainer( ((FuncBuildTask*)task)->function )
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
			lastObject.setWithoutRef( new FunctionContainer() );
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
		addNewTaskToStack( opcode->getTask() );
		break;

	case Opcode::FuncFound_setParam:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode FuncFound_setParam");
#endif
		task = getLastTask();
		if ( task->name == TaskName::FuncFound ) {
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
		if ( task->name == TaskName::FuncFound ) {
			opIter.next(); // Increment to the Terminal
			switch( setupFunctionExecution(*((FuncFoundTask*)task), opStrandStackIter) ) {

			case FuncExecReturn::Ran:
				popLastTask();
				return ExecutionResult::Ok;

			case FuncExecReturn::Reset:
				popLastTask();
				return ExecutionResult::Reset; // Restart the iterator

			case FuncExecReturn::ErrorOnRun:
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
				print(LogLevel::debug, "Function failed to call in setupFunctionExecution.");
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
		opIter.set( ((GotoOpcode*)opcode)->getOpStrandIter() );
		break;

	case Opcode::ConditionalGoto:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode ConditionalGoto");
#endif

		Object* obj; // Needs to be declared at the beginning of this method
		if ( lastObject.obtain(obj) ) {
			if ( isObjectBool(*obj) ) {
				// Use the inverse of the value because if-statements request jumps when condition is false
				if ( ((ObjectBool*)obj)->getValue() == false ) {
					opIter.set( ((GotoOpcode*)opcode)->getOpStrandIter() );
				}
			} else {
				print(LogLevel::warning, "Condition for goto operation is not boolean. Default is false.");
				// Perform false operation
				opIter.set( ((GotoOpcode*)opcode)->getOpStrandIter() );
			}
		} else {
			print(LogLevel::error, "Missing condition for goto operation.");
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
		lastObject.setWithoutRef(new ObjectBool(true));
		break;

	case Opcode::CreateBoolFalse:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode CreateBoolFalse");
#endif
		lastObject.setWithoutRef(new ObjectBool(false));
		break;

	case Opcode::CreateNumber:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode CreateNumber");
#endif
		// Should be creating numbers from a user-set factory
/*
		NumberObjectFactory* factory;
		if ( numberObjectFactoryPtr.obtain(factory) ) {
			lastObject.setWithoutRef( factory->createNumber(value) );
		} else {
			print(LogLevel::warning, "Missing number factory.");
			lastObject.setWithoutRef( new ObjectNumber(value) );
		}
*/
		lastObject.setWithoutRef(
			new ObjectNumber( opcode->getNameData() )
		);
		break;

	case Opcode::CreateString:
#ifdef COPPER_OPCODE_DEBUGGING
		print(LogLevel::debug, "[DEBUG: Execute opcode CreateString");
#endif
		// Should be creating strings from a user-set factory
		lastObject.setWithoutRef(
			new ObjectString( opcode->getNameData() )
		);
		break;

	default:
		print(LogLevel::error, "Invalid opcode encountered.");
		return ExecutionResult::Error;
	}

	return ExecutionResult::Ok;
}

//EngineResult::Value
FuncExecReturn::Value
Engine::setupFunctionExecution(
	FuncFoundTask& task,
	OpStrandStackIter&	opStrandStackIter
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::setupFunctionExecution");
#endif
	// Protect from polution and allow for functions to have a default return.
	lastObject.setWithoutRef(new FunctionContainer());
	FuncExecReturn::Value result;

	if ( ! task.varAddress.has() )
		throw BadVarAddressException();

	result = setupBuiltinFunctionExecution(task);
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
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::setupBuiltinFunctionExecution");
#endif

	// Which one is faster?
	// a) Getting a key and using a switch
	// b) Using a virtual function
	// Based on time tests, the first one. Using a switch is basically a manual lookup-table.

	//VarAddressConstIter addrIter = task.varAddress.constStart();
	//RobinHoodHash<SystemFunction::Value>::BucketData* bucketData
	//	= builtinFunctions.getBucketData(*addrIter);

	RobinHoodHash<SystemFunction::Value>::BucketData* bucketData
		= builtinFunctions.getBucketData(task.varAddress.getConstFirst());

	// No matching function found
	if ( ! bucketData )
		return FuncExecReturn::NoMatch;
	// Else, bucketData found...

	// Built-in function names should only have one name
	if ( task.varAddress.size() > 1 ) {
		print(LogLevel::error, "Attempt to call member of built-in function.");
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

	case SystemFunction::_union:
		process_sys_union(task);
		break;

	case SystemFunction::_type:
		process_sys_type(task);
		break;

	case SystemFunction::_are_same_type:
		process_sys_are_same_type(task);
		break;

	case SystemFunction::_are_bool:
		process_sys_are_bool(task);
		break;

	case SystemFunction::_are_string:
		process_sys_are_string(task);
		break;

	case SystemFunction::_are_number:
		process_sys_are_number(task);
		break;

	case SystemFunction::_assert:
		return process_sys_assert(task);

	default:
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
		= foreignFunctions.getBucketData(task.varAddress.getConstFirst());

	if ( ! bucketData ) {
		return FuncExecReturn::NoMatch;
	}

	ForeignFunc* foreignFunc = bucketData->item.getForeignFunction();

	if ( (foreignFunc->getParameterCount() != task.args.size()) && ! foreignFunc->isVariadic() ) {
		// Language specification says this should optionally be a warning
		print(LogLevel::error, "Parameter count does not match foreign function header.");
		if ( ignoreBadForeignFunctionCalls ) {
			return FuncExecReturn::Ran; // Leaves the return as empty function
		} else {
			return FuncExecReturn::ErrorOnRun;
		}
	}

	unsigned int ffhIndex = 0;
	ArgsIter taskArgsIter = task.args.start();
	// This is for non-variadic functions. Variadic functions should only return a parameter count
	// when they have a specific number of parameters that MUST be a certain type for any call.
	// For example, a list function might be Top(List, Iter, Iter...) so it's constant parameter is
	// the first one: Top(List...) meaning it should return a parameter count of 1 and have
	// getParameterName() return the typename for the List.
	while ( ffhIndex < foreignFunc->getParameterCount() ) {
		if ( ! taskArgsIter.has() ) {
			break; // For variadic functions
		}
		if ( ! foreignFunc->getParameterType(ffhIndex) == (*taskArgsIter)->getType() )
		{
			// Language specification says this should optionally be a warning
			print(LogLevel::error, "Argument types do not match foreign function header.");
			if ( ignoreBadForeignFunctionCalls ) {
				return FuncExecReturn::Ran; // Leaves the return as empty function
			} else {
				return FuncExecReturn::ErrorOnRun;
			}
		}
		++ffhIndex;
		if ( ! taskArgsIter.next() ) {
			break;
		}
	}

	FFIServices ffi(*this, task.args.start());
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
	VarAddressConstIter addrIter = task.varAddress.constStart();
	do {
		super = callVariable;
		scope->getVariable(*addrIter, callVariable);
		func = callVariable->getFunction(logger);
		scope = &(func->getPersistentScope());
	} while ( addrIter.next() );
*/

//----------

#ifdef COPPER_SPEED_PROFILE
	//rusage startTime, endTime;
	//double elapsedTime = 0;
	//getrusage(RUSAGE_SELF, &startTime);
#endif

	// Search local, then global, then create in local if not found.
	// Code modified from resolveVariableAddress().

	Variable* callVariable = REAL_NULL;
	Variable* super = REAL_NULL;
	Scope* scope = &getCurrentTopScope();
	Function* func = REAL_NULL;
	bool foundVar = false;
	VarAddressConstIter addrIter = task.varAddress.constStart();
	addrIter.reset(); // Restart
	do {
		super = callVariable;
		if ( scope->findVariable(*addrIter, callVariable) ) {
			func = callVariable->getFunction(logger);
			scope = &(func->getPersistentScope());
			foundVar = true;
		} else {
			break;
		}
	} while ( addrIter.next() );

	if ( !foundVar ) {
		addrIter.reset(); // Restart
		scope = &getGlobalScope();
		do {
			super = callVariable;
			if ( scope->findVariable(*addrIter, callVariable) ) {
				func = callVariable->getFunction(logger);
				scope = &(func->getPersistentScope());
				foundVar = true;
			} else {
				break;
			}
		} while ( addrIter.next() );
	}

	if ( !foundVar ) {
		addrIter.reset(); // Restart
		scope = &getCurrentTopScope();
		do {
			super = callVariable;
			// Create the variable if it doesn't exist
			scope->getVariable(*addrIter, callVariable);
			func = callVariable->getFunction(logger);
			scope = &(func->getPersistentScope());
		} while ( addrIter.next() );
	}

#ifdef COPPER_SPEED_PROFILE
	//getrusage(RUSAGE_SELF, &endTime);
	//elapsedTime = (endTime.ru_utime.tv_sec - startTime.ru_utime.tv_sec) * 1000.0 + (endTime.ru_utime.tv_usec - startTime.ru_utime.tv_usec) / 1000.0;
	//fullTime += elapsedTime;
	//static int cycle = 1;
	//cycle++;
	//std::printf("section time = %f, %s", elapsedTime, (cycle % 6 == 0 ? "\n":"") );
#endif

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
		print(LogLevel::error, "Function body is missing.");
		return FuncExecReturn::ErrorOnRun;
	}

	if ( body->isEmpty() ) {
		return FuncExecReturn::Ran;
	}

	// Attempt to compile. Automatically returns true if compiled.
	if ( ! body->compile(this) ) {
		print(LogLevel::error, "Function body contains errors.");
		return FuncExecReturn::ErrorOnRun;
	}

	// "super" and "this" are added to new stack context if the body can be run.
	StackFrame* stackFrame = new StackFrame();

	if ( notNull(super) ) {
		// Add "super" pointer
		stackFrame->getScope().setVariable(CONSTANT_FUNCTION_SUPER, super, true);
	}

	// Add "this" pointer
	stackFrame->getScope().setVariable(CONSTANT_FUNCTION_SELF, callVariable, true);

	// For each parameter that the function requires, take from the passed parameters and
	// assign it by pointer to a parameter name within the newly added scope.
	ArgsIter givenParamsIter = task.args.start();
	List<String>::Iter funcParamsIter = func->params.start();
	bool done = false;
	if ( funcParamsIter.has() ) {
		if ( givenParamsIter.has() )
		do {
			// Match parameters with parameter list items
			stackFrame->getScope().setVariableFrom( *funcParamsIter, *givenParamsIter, true );
			if ( ! funcParamsIter.next() ) {
				done = true;
				break;
			}
		} while ( givenParamsIter.next() );
		if ( !done )
		do {
			print( LogLevel::warning, EngineMessage::MissingFunctionCallParam );
			// Default remaining parameters to empty function.
			stackFrame->getScope().addVariable( *funcParamsIter );
		} while ( funcParamsIter.next() );
	}
	// TODO: Leftover task parameters may eventually be stored in a splat variable or included as part
	// of an all-inclusive "arguments" array.

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

//! Runs a given function with the passed parameters.
// This is useful for callback functions.
// It must NOT be run alongside Engine::run() or Engine::execute() but
// must be run after them.
/*
	Currently, it is difficult to make this work.
	It may be possible to simply append a set of function run opcodes to the end of the current operations
	strand and then call Engine::execute().
	Other than that, I can't think of a way that wouldn't overcomplicate the system.
	Unfortunately, this wouldn't allow for functions like Foreach().
*/
/*
bool
Engine::runFunction( FunctionContainer* function, ArgsList* parameters ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::runFunction");
#endif

}
*/

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
		throw BadVarAddressException();

	// Should return REAL_NULL if the address belongs to a non-variable, such as a
	// built-in function or foreign function.

	VarAddressConstIter ai = address.constStart();

	RobinHoodHash<SystemFunction::Value>::BucketData* sfBucket
		= builtinFunctions.getBucketData(*ai);
	if ( sfBucket != 0 ) { // Error, but handling is determined by the method that calls this one
		print(LogLevel::warning, "Attempt to use standard access on a built-in function.");
		return REAL_NULL;
	}

	// The varAddress is guaranteed to be of length > 0 because of parsing
	String addr = addressToString(address);
	RobinHoodHash<ForeignFuncContainer>::BucketData* ffBucket
		= foreignFunctions.getBucketData(addr);
	if ( ffBucket != 0 ) { // Error, but handling is determined by the method that calls this one
		print(LogLevel::warning, "Attempt to use standard access on a foreign function.");
		return REAL_NULL;
	}

	Scope* scope = &getCurrentTopScope();
	Variable* var;
	Function* func;
	ai.reset(); // Restart
	do {
		if ( scope->findVariable(*ai, var) ) {
			func = var->getFunction(logger);
			scope = &(func->getPersistentScope());
		} else {
			goto resolve_addr_search_globals;
		}
	} while ( ai.next() );
	return var;

resolve_addr_search_globals:
	ai.reset(); // Restart
	scope = &getGlobalScope();

	do {
		if ( scope->findVariable(*ai, var) ) {
			func = var->getFunction(logger);
			scope = &(func->getPersistentScope());
		} else {
			goto resolve_addr_search_locals;
		}
	} while ( ai.next() );
	return var;

resolve_addr_search_locals:
	ai.reset();
	scope = &getCurrentTopScope();

	do {
		scope->getVariable(*ai, var);
		func = var->getFunction(logger);
		scope = &(func->getPersistentScope());
	} while ( ai.next() );
	return var;
}

ExecutionResult::Value
Engine::run_Own(
	const VarAddress& address
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::run_Own");
#endif
	lastObject.setWithoutRef(new FunctionContainer());
	if ( !ownershipChangingEnabled ) {
		print(LogLevel::warning, "Ownership changing is disabled.");
		return ExecutionResult::Ok;
	}
	Variable* var = resolveVariableAddress(address);
	if ( isNull(var) ) {
		print(LogLevel::warning, "Cannot own given function. Built-in or foreign function.");
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
	print(LogLevel::debug, "[DEBUG: Engine::run_Is_owner");
#endif
	Variable* var = resolveVariableAddress(address);
	if ( isNull(var) ) {
		print(LogLevel::info, "Given function has system as owner.");
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
	lastObject.setWithoutRef(new ObjectBool(
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
	lastObject.setWithoutRef(new ObjectBool(
		is_var_pointer( address )
	));
	return ExecutionResult::Ok;
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
	ArgsIter pi = task.args.start();
	if ( pi.has() ) {
		result = ! getBoolValue(**pi);
	}
	lastObject.setWithoutRef(new ObjectBool(result));
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
	ArgsIter pi = task.args.start();
	if ( pi.has() )
	do {
		result = getBoolValue(**pi);
		if ( !result )
			break;
	} while ( pi.next() );
	lastObject.setWithoutRef(new ObjectBool(result));
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
	ArgsIter pi = task.args.start();
	if ( pi.has() )
	do {
		result = getBoolValue(**pi);
		if ( result )
			break;
	} while ( pi.next() );
	lastObject.setWithoutRef(new ObjectBool(result));
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
	ArgsIter pi = task.args.start();
	if ( pi.has() )
	do {
		result = ! getBoolValue(**pi);
		if ( !result )
			break;
	} while ( pi.next() );
	lastObject.setWithoutRef(new ObjectBool(result));
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
	ArgsIter pi = task.args.start();
	if ( pi.has() )
	do {
		result = ! getBoolValue(**pi);
		if ( !result )
			break;
	} while ( pi.next() );
	lastObject.setWithoutRef(new ObjectBool(result));
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
	ArgsIter pi = task.args.start();
	if ( pi.has() )
	do {
		result = isObjectFunction(**pi);
		if ( !result )
			break;
	} while ( pi.next() );
	lastObject.setWithoutRef(new ObjectBool(result));
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
	ArgsIter pi = task.args.start();
	if ( pi.has() )
	do {
		result = isObjectEmptyFunction(**pi);
		if ( !result )
			break;
	} while ( pi.next() );
	lastObject.setWithoutRef(new ObjectBool(result));
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
	ArgsIter pi = task.args.start();
	if ( pi.has() ) {
		firstPtr = *pi;
		while( pi.next() && result ) {
			result = ( firstPtr == *pi );
		}
	}
	lastObject.setWithoutRef(new ObjectBool(result));
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
	ArgsIter paramsIter = task.args.start();
	if ( task.args.size() != 2 ) {
		print(LogLevel::error, EngineMessage::MemberWrongParamCount);
		return FuncExecReturn::ErrorOnRun;
	}
	// First parameter is the parent function of the member sought
	if ( ! isObjectFunction(**paramsIter) ) {
		print(LogLevel::error, EngineMessage::MemberParam1NotFunction);
		return FuncExecReturn::ErrorOnRun;
	}
	FunctionContainer* parentFc = (FunctionContainer*)*paramsIter;
	Function* parentFunc;
		// Attempt to extract the function
		// Failure occurs for "pointer variables" that have not reset
	if ( ! parentFc->getFunction(parentFunc) ) {
		print(LogLevel::error, EngineMessage::DestroyedFuncAsMemberParam);
		return FuncExecReturn::ErrorOnRun;
	}
	// Second parameter is the name of the member
	paramsIter.next();
	if ( !isObjectString(**paramsIter) ) {
		print(LogLevel::error, EngineMessage::MemberParam2NotString);
		return FuncExecReturn::ErrorOnRun;
	}
	ObjectString* objStr = (ObjectString*)(*paramsIter);
	String& rawStr = objStr->getString();
	if ( ! isValidName( rawStr ) ) {
		print(LogLevel::error, EngineMessage::MemberFunctionInvalidNameParam);
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
	ArgsIter paramsIter = task.args.start();
	if ( task.args.size() == 0 ) {
		print(LogLevel::warning, EngineMessage::MemberCountWrongParamCount);
		// Seems inefficient, but it's faster that ObjectNumber(0) because the class uses string representation
		lastObject.setWithoutRef(new ObjectNumber(String("0")));
		return FuncExecReturn::ErrorOnRun;
	}
	// The only parameter is the parent function of the members
	if ( ! isObjectFunction(**paramsIter) ) {
		print(LogLevel::error, EngineMessage::MemberCountParam1NotFunction);
		return FuncExecReturn::ErrorOnRun;
	}
	Function* parentFunc;
	FunctionContainer* fc = (FunctionContainer*)(*paramsIter);
	if ( ! fc->getFunction(parentFunc) ) {
		print(LogLevel::error, EngineMessage::DestroyedFuncAsMemberCountParam);
		return FuncExecReturn::ErrorOnRun;
	}
	unsigned long size = parentFunc->getPersistentScope().occupancy();
	// The bummer here is that it's cramming an unsigned int into an int.
	// On the bright side, no one should be creating 2.7 billion variables.
	lastObject.setWithoutRef(new ObjectNumber(size));
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
	ArgsIter paramsIter = task.args.start();
	if ( task.args.size() != 2 ) {
		print(LogLevel::error, EngineMessage::IsMemberWrongParamCount);
		lastObject.setWithoutRef(new ObjectBool(false));
		return FuncExecReturn::ErrorOnRun;
	}
	// First parameter is the parent function of the members
	if ( ! isObjectFunction(**paramsIter) ) {
		print(LogLevel::error, EngineMessage::IsMemberParam1NotFunction);
		return FuncExecReturn::ErrorOnRun;
	}
	Function* parentFunc;
	FunctionContainer* fc = (FunctionContainer*)(*paramsIter);
	if ( ! fc->getFunction(parentFunc) ) {
		print(LogLevel::error, EngineMessage::DestroyedFuncAsIsMemberParam);
		return FuncExecReturn::ErrorOnRun;
	}
	// Second parameter should be a string
	paramsIter.next();
	bool result = false;
	if ( ! isObjectString(**paramsIter) ) {
		print(LogLevel::error, EngineMessage::IsMemberParam2NotString);
		return FuncExecReturn::ErrorOnRun;
	}
	const String& memberName = ((ObjectString*)*paramsIter)->getString();
	result = parentFunc->getPersistentScope().variableExists( memberName );
	lastObject.setWithoutRef(new ObjectBool(result));
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
	ArgsIter paramsIter = task.args.start();
	if ( task.args.size() != 3 ) {
		print(LogLevel::error, EngineMessage::SetMemberWrongParamCount);
		lastObject.setWithoutRef(new FunctionContainer());
		return FuncExecReturn::ErrorOnRun;
	}
	// First parameter should be the parent function of the members
	if ( ! isObjectFunction(**paramsIter) ) {
		print(LogLevel::error, EngineMessage::SetMemberParam1NotFunction);
		return FuncExecReturn::ErrorOnRun;
	}
	Function* parentFunc;
	FunctionContainer* fc = (FunctionContainer*)(*paramsIter);
	if ( ! fc->getFunction(parentFunc) ) {
		print(LogLevel::error, EngineMessage::DestroyedFuncAsSetMemberParam);
		return FuncExecReturn::ErrorOnRun;
	}
	// Second parameter should be a string
	paramsIter.next();
	if ( ! isObjectString(**paramsIter) ) {
		print(LogLevel::error, EngineMessage::SetMemberParam2NotString);
		return FuncExecReturn::ErrorOnRun;
	}
	String& memberName = ((ObjectString*)*paramsIter)->getString();
	if ( !isValidName(memberName) ) {
		print(LogLevel::error, EngineMessage::SystemFunctionBadParam);
		return FuncExecReturn::ErrorOnRun;
	}
	// Third parameter can be anything
	paramsIter.next();
	parentFunc->getPersistentScope().setVariableFrom( memberName, *paramsIter, false );
	lastObject.setWithoutRef(new FunctionContainer());
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_union(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_union");
#endif
	ArgsIter paramsIter = task.args.start();
	if ( !paramsIter.has() ) {
		print(LogLevel::info, "Union function called without parameters. Default return is empty function.");
		lastObject.setWithoutRef(new FunctionContainer());
		return FuncExecReturn::Ran;
	}
	FunctionContainer* finalFC = new FunctionContainer();
	Function* finalFunc;
	finalFC->getFunction(finalFunc); // Guaranteed
	FunctionContainer* usableFC;
	Function* usableFunc;
	do {
		if ( isObjectFunction(**paramsIter) ) {
			usableFC = (FunctionContainer*)(*paramsIter);
			if ( usableFC->getFunction(usableFunc) ) {
				finalFunc->getPersistentScope().copyMembersFrom( usableFunc->getPersistentScope() );
			} else {
				//print(LogLevel::warning, "Destroyed function passed to \"union\" function.");
				print(LogLevel::warning, EngineMessage::DestroyedFuncAsUnionParam);
			}
		} else {
			print(LogLevel::warning, EngineMessage::NonFunctionAsUnionParam);
		}
	} while( paramsIter.next() );
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
	ArgsIter paramsIter = task.args.start();
	if ( !paramsIter.has() ) {
		lastObject.setWithoutRef(new ObjectString());
		return FuncExecReturn::Ran;
	}
	// Get only the first parameter
	lastObject.setWithoutRef(new ObjectString( (*paramsIter)->typeName() ));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_are_same_type(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_same_type");
#endif
	ArgsIter paramsIter = task.args.start();
	if ( !paramsIter.has() ) {
		lastObject.setWithoutRef(new FunctionContainer());
		return FuncExecReturn::Ran;
	}
	// First parameter, to which all other parameters are compared
	ObjectType::Value firstType = (*paramsIter)->getType();
	String first( (*paramsIter)->typeName() );
	bool same = true;
	do {
		same = firstType == (*paramsIter)->getType();
		if ( same ) {
			same = first.equals( (*paramsIter)->typeName() );
		}
		if ( !same )
			break;
	} while( paramsIter.next() );
	// Get only the first parameter
	lastObject.setWithoutRef(new ObjectBool(same));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_are_bool(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_bool");
#endif
	ArgsIter paramsIter = task.args.start();
	if ( !paramsIter.has() ) {
		lastObject.setWithoutRef(new ObjectBool(false));
		return FuncExecReturn::Ran;
	}
	// Check all parameters
	bool out = true;
	do {
		out = isObjectBool(**paramsIter);
		if ( !out) break;
	} while ( paramsIter.next() );
	lastObject.setWithoutRef(new ObjectBool(out));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_are_string(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_string");
#endif
	ArgsIter paramsIter = task.args.start();
	if ( !paramsIter.has() ) {
		lastObject.setWithoutRef(new ObjectBool(false));
		return FuncExecReturn::Ran;
	}
	// Check all parameters
	bool out = true;
	do {
		out = isObjectString(**paramsIter);
		if ( !out)
			break;
	} while ( paramsIter.next() );
	lastObject.setWithoutRef(new ObjectBool(out));
	return FuncExecReturn::Ran;
}

FuncExecReturn::Value
Engine::process_sys_are_number(
	FuncFoundTask& task
) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_number");
#endif
	ArgsIter paramsIter = task.args.start();
	if ( !paramsIter.has() ) {
		lastObject.setWithoutRef(new ObjectBool(false));
		return FuncExecReturn::Ran;
	}
	// Check all parameters
	bool out = true;
	do {
		out = isObjectNumber(**paramsIter);
		if ( !out)
			break;
	} while ( paramsIter.next() );
	lastObject.setWithoutRef(new ObjectBool(out));
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
	ArgsIter paramsIter = task.args.start();
	if ( !paramsIter.has() ) {
		return FuncExecReturn::Ran;
	}
	do {
		if ( ! getBoolValue(**paramsIter) ) {
			print(LogLevel::error, EngineMessage::AssertionFailed);
			return FuncExecReturn::ErrorOnRun;
		}
	} while ( paramsIter.next() );
	lastObject.setWithoutRef(new ObjectBool(true));
	return FuncExecReturn::Ran;
}


} // end namespace Cu
