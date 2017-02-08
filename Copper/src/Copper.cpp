// Copyright 2016 Nicolaus Anderson

#include "Copper.h"

namespace Cu {

// ******* Constants *******
static const char* CONSTANT_END_MAIN = "end_main";
static const char* CONSTANT_EXIT = "exit";
static const char* CONSTANT_FUNCTION_DECLARE = "fn"; // "fn" is boring. CopperLang should have lots of fun.
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

Scope& Function::getPersistentScope() {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Function::getPersistentScope [%p]\n", (void*)this);
#endif
	return *persistentScope;
}

void Function::set( Function& other ) {
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: Function::set [%p]\n", (void*)this);
#endif
	constantReturn = other.constantReturn;
	body.set( other.body.raw() );
	params = other.params; // If params is changed to a pointer, this has to be changed to a copy
	*persistentScope = *(other.persistentScope);
	result.set( other.result.raw() ); // Should this be copied?
}

//--------------------------------------

FunctionContainer::FunctionContainer(Function* pFunction, unsigned int id)
	: funcBox()
	, owner(REAL_NULL)
	, ID(id)
{
	type = ObjectType::Function;
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionContainer constructor (Function*) [%p]\n", (void*)this);
#endif
	type = ObjectType::Function;
	funcBox.set(pFunction);
}

FunctionContainer::FunctionContainer()
	: funcBox()
	, owner(REAL_NULL)
	, ID(0)
{
	type = ObjectType::Function;
#ifdef COPPER_VAR_LEVEL_MESSAGES
	std::printf("[DEBUG: FunctionContainer constructor 2 [%p]\n", (void*)this);
#endif
	type = ObjectType::Function;
	funcBox.setWithoutRef(new Function());
}

FunctionContainer::FunctionContainer(const FunctionContainer& pOther)
	: funcBox()
	, owner(REAL_NULL)
	, ID(pOther.ID)
{
	type = ObjectType::Function;
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

void Scope::getVariableFromHashTable(const String& pName, Variable*& pVariable) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::getVariableFromHashTable\n");
#endif
	if ( isNull(robinHoodTable) ) throw NonExistentScopeTableException();

	RobinHoodHash<RefVariableStorage>::BucketData* data = robinHoodTable->getBucketData(pName);
	if ( !data ) {
		// Create the slot
		pVariable = &( robinHoodTable->insert(pName, RefVariableStorage())->getVariable() );
	} else {
		pVariable = &( data->item.getVariable() );
	}
}

void Scope::getVariableFromList(const String& pName, Variable*& pVariable) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::getVariableFromList\n");
#endif
	if ( isNull(listTable) ) throw NonExistentScopeTableException();

	List<ListVariableStorage>::Iter lti = listTable->start();
	if ( listTable->has() ) {
		lti.makeLast(); // Newest items are at the back
		do {
			if ( (*lti).name.equals(pName) ) {
				pVariable = &((*lti).getVariable());
				return;
			}
		} while(--lti);
	} // else
	listTable->push_back(ListVariableStorage(pName));
	pVariable = &(listTable->getLast().getVariable());
}

void Scope::copyAsgnFromHashTable( RobinHoodHash<RefVariableStorage>& pTable ) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::copyAsgnFromHashTable\n");
#endif
	if ( notNull(robinHoodTable) ) {
		delete robinHoodTable;
		robinHoodTable = new RobinHoodHash<RefVariableStorage>(pTable);
		return;
	} // else
	uint i=0;
	RobinHoodHash<RefVariableStorage>::Bucket* bucket;
	if ( notNull(listTable) ) {
		for(; i < pTable.getSize(); ++i) {
			bucket = pTable.get(i);
			if ( notNull(bucket->data) ) {
				listTable->push_back(
					ListVariableStorage(bucket->data->name, bucket->data->item.getVariable())
				);
			}
		}
	} else {
		throw NonExistentScopeTableException();
	}
}

void Scope::copyAsgnFromList( List<ListVariableStorage>& pList ) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::copyAsgnFromList\n");
#endif
	List<ListVariableStorage>::Iter li = pList.start();
	if ( li.has() ) {
		li.makeLast();
		if ( notNull(robinHoodTable) ) {
			delete robinHoodTable;
			robinHoodTable = new RobinHoodHash<RefVariableStorage>(100);
			do {
				robinHoodTable->insert( (*li).name, RefVariableStorage( ((*li).getVariable()) ) );
			} while( ++li );
		} else if ( notNull(listTable) ) {
			delete listTable;
			do {
				listTable = new List<ListVariableStorage>();
				listTable->push_back( ListVariableStorage( (*li).name, ((*li).getVariable()) ) );
			} while( ++li );
		} else {
			throw NonExistentScopeTableException();
		}
	}
}

Scope::Scope( bool fast )
	: robinHoodTable(REAL_NULL)
	, listTable(REAL_NULL)
{
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope constructor 1 (bool) [%p]\n", (void*)this);
#endif
	if ( fast ) {
		robinHoodTable = new RobinHoodHash<RefVariableStorage>(100);
	} else {
		listTable = new List<ListVariableStorage>();
	}
}

Scope::~Scope() {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::~Scope [%p]\n", (void*)this);
#endif
	if ( notNull(robinHoodTable) )
		delete robinHoodTable;
	if ( notNull(listTable) )
		delete listTable;
}

Scope& Scope::operator = (Scope& pOther) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::operator = [%p]\n", (void*)this);
#endif
	if ( notNull(pOther.robinHoodTable) ) {
		copyAsgnFromHashTable(*(pOther.robinHoodTable));
	}
	else if ( notNull(pOther.listTable) ) {
		copyAsgnFromList(*(pOther.listTable));
	}
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
	if ( notNull(robinHoodTable) ) {
		return &( robinHoodTable->insert(pName, RefVariableStorage())->getVariable() );
	}
	else if ( notNull(listTable) ) {
		listTable->push_back( ListVariableStorage(pName) );
		return &(listTable->getLast().getVariable());
	} // else
	throw NonExistentScopeTableException();
}

// Gets the variable, creating it if it does not exist
void Scope::getVariable(const String& pName, Variable*& pVariable) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::getVariable , name=%s\n", pName.c_str());
#endif

	if ( notNull( robinHoodTable ) ) {
		getVariableFromHashTable(pName, pVariable);
	}
	else if ( notNull( listTable ) ) {
		getVariableFromList(pName, pVariable);
	}
	else
		throw NonExistentScopeTableException();
}

// Looks for the variable. If the function exists, it saves it to storage.
// Returns "true" if the variable was found, false otherwise.
// Shouldn't the string here be const?
bool Scope::findVariable(const String& pName, Variable*& pStorage) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::findVariable , name=%s\n", pName.c_str());
#endif
	RobinHoodHash<RefVariableStorage>::BucketData* bucketData;
	if ( notNull(robinHoodTable) ) {
		bucketData = robinHoodTable->getBucketData(pName);
		if ( bucketData != 0 ) { // Hash table uses 0 instead of null
			pStorage = &( bucketData->item.getVariable() );
			return true;
		}
		pStorage = REAL_NULL;
		return false;
	}
	// Should never happen
	if ( isNull(listTable) ) {
		throw NonExistentScopeTableException();
	}
	List<ListVariableStorage>::Iter varIter = listTable->start();
	if ( varIter.has() ) {
		varIter.makeLast();
		do {
			if ( pName.equals( (*varIter).name ) ) {
				pStorage = &( (*varIter).getVariable() );
				return true;
			}
		} while( --varIter );
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
		case ObjectType::Data:
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
	std::printf("[DEBUG: Scope::appendNamesToList\n");
#endif
	// This implementation is slow due to the need to copy strings.
	// Saving string addresses is possible but not safe.
	RobinHoodHash<RefVariableStorage>::Bucket* bucket;
	Object* obj;
	uint i=0;
	if ( notNull(robinHoodTable) ) {
		for(; i < robinHoodTable->getSize(); ++i) {
			bucket = robinHoodTable->get(i);
			if ( notNull(bucket->data) ) {
				obj = new ObjectString(bucket->data->name);
				aoi->append(obj);
				obj->deref();
			}
		}
		return;
	}
	if ( isNull(listTable) ) {
		return;
	}
	List<ListVariableStorage>::Iter li = listTable->start();
	if ( li.has() )
	do {
		obj = new ObjectString( (*li).name );
		aoi->append(obj);
		obj->deref();
	} while( ++li );
}

void Scope::copyMembersFrom(Scope& pOther) {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::copyMembersFrom\n");
#endif
	// Creating a new scope is required for proper copy construction to delink pointers
	// Use a list version to make it easier to copy
	Scope newScope(false);
	newScope = pOther;
	List<ListVariableStorage>::Iter li = newScope.listTable->start();
	if ( li.has() )
	do {
		setVariable( (*li).name, &((*li).getVariable()), false );
	} while( ++li );
}

unsigned int Scope::occupancy() {
#ifdef COPPER_SCOPE_LEVEL_MESSAGES
	std::printf("[DEBUG: Scope::occupancy\n");
#endif
	if ( notNull(robinHoodTable) ) {
		return robinHoodTable->getOccupancy();
	}
	if ( notNull( listTable ) ) {
		return listTable->size();
	}
	return 0;
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

void Stack::clear() {
	StackFrame* parent;
	while( notNull(top) ) {
		parent = top->parent;
		top->deref();
		top = parent;
	}
}

StackFrame& Stack::getBottom() { // Global namespace
	if ( size == 0 || isNull(bottom) )
		throw UninitializedStackException();
	return *bottom;
}

StackFrame& Stack::getTop() {
	if ( size == 0 || isNull(top) )
		throw UninitializedStackException();
	return *top;
}

unsigned int Stack::getCurrLevel() {
	return size;
}

StackPopReturn Stack::pop() {
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

void Stack::push() {
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

void Stack::push(StackFrame* pContext) {
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

//--------------------------------------

bool isObjectFunction( const Object& pObject ) {
	return ( pObject.getType() == ObjectType::Function );
}

bool isObjectEmptyFunction( const Object& pObject ) {
	if ( pObject.getType() != ObjectType::Function ) {
		return false;
	}
	Function* function;
	if ( !((FunctionContainer&)pObject).getFunction(function) ) {
		return false;
	}
	if ( function->constantReturn ) {
		if ( notNull(function->result.raw()) )
			return false;
	}
	return (
		function->body.raw()->size() == 0
		&& function->params.size() == 0
		&& function->getPersistentScope().occupancy() == 0
	);
}

bool isObjectBool( const Object& pObject ) {
	return ( pObject.getType() == ObjectType::Data && util::equals( ((Data&)pObject).typeName(), "bool") );
}

bool getBoolValue( const Object& pObject ) {
	if ( isObjectBool( pObject ) ) {
		return ((ObjectBool&)pObject).getValue();
	}
	return false;
}

bool isObjectString( const Object& pObject ) {
	return ( pObject.getType() == ObjectType::Data && util::equals( ((Data&)pObject).typeName(), "string") );
}

bool isObjectNumber( const Object& pObject ) {
	return ( pObject.getType() == ObjectType::Data && util::equals( ((Data&)pObject).typeName(), "number") );
}

bool isObjectOfType( const Object& pObject, const char* pTypeName ) {
	return ( pObject.getType() == ObjectType::Data && util::equals( pObject.typeName(), pTypeName ) );
}


//--------------------------------------

Engine::Engine()
	: logger(REAL_NULL)
	, stack()
	, taskStack()
	, lastObject()
	, functionID(0)
	, functionReturnMailbox(false)
	, loopEndMailbox(false)
	, loopSkipMailbox(false)
	, systemExitTrigger(false)
	, endMainCallback(REAL_NULL)
	, foreignFunctions(100)
	, ownershipChangingEnabled(false)
	, stackTracePrintingEnabled(false)
	, numberObjectFactoryPtr()
{
	// Stack scope should be initialized to the slow (linked list) version for the 1st frame.
	stack.getBottom().replaceScope(false);

	// Initialize the number object factory to the default.
	numberObjectFactoryPtr.setWithoutRef(new NumberObjectFactory());
}

void Engine::signalEndofProcessing() {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::signalEndofProcessing");
#endif
	if ( notNull(endMainCallback) ) {
printf("has endMainCallback\n");
		endMainCallback->CuEngineDoneProcessing();
	}
}

void Engine::clearStacks() {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::clearStacks");
#endif
	taskStack.clear();
	stack.clear();
	// Some extra cleanup
	functionReturnMailbox = false;
	loopEndMailbox = false;
	loopSkipMailbox = false;
	functionID = 0;

	// Reinitialized the stack
	// This is useful if the program should continue
	stack.push();
	stack.getBottom().replaceScope(false);
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

void Engine::printFunctionError( unsigned int id, unsigned int tokenIdx, const Token& token ) const {
	if ( notNull(logger) ) {
		logger->printFunctionError(id, tokenIdx, token.type);
	}
#ifdef USE_CSTDIO
	else {
		std::fprintf(stderr, "Stack trace: fn( %u ) token( %u )::id(%u)", id, tokenIdx, (unsigned int)token.type);
	}
#endif
}

void Engine::addForeignFunction(const String& pName, ForeignFunc* pFunction) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::addForeignFunction");
#endif
	foreignFunctions.insert(pName, ForeignFuncContainer(pFunction));
}

TokenType Engine::resolveTokenType( const String& pName ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::resolveTokenType");
#endif
	if ( pName.size() == 0 ) {
		print(LogLevel::error, EngineMessage::ZeroSizeToken);
		return TT_malformed;
	}

	// I don't want this here. It forces repeat-duty.
	// Unfortunately, if the user decides to use it or tokenize() separate from run(), I have issues.
	// To resolve this, I could make a public version of resolveTokenType (which checks isSpecialCharacter)
	// and a private one, which is the rest of this. But then I would have to do the same for tokenize().
	// This isn't a big slow-down, so it's not a big deal.
	TokenType tokenType;
	if ( isSpecialCharacter(pName[0], tokenType) ) {
		if ( pName.size() > 1 )
			return TT_malformed;
		return tokenType;
	}

	if ( pName.equals(CONSTANT_END_MAIN) )
		return TT_end_main;
	if ( pName.equals(CONSTANT_EXIT) )
		return TT_exit;
	if ( pName.equals(CONSTANT_FUNCTION_DECLARE) )
		return TT_function_declare;
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
		// Put in a new function if this gets too complicated

		// Note: Since decimal places are read as the scope-opener, it can't be used
		// at the beginning of a number.

		// Indentify the kind of number this is.
		// if ( pName[-1] == 'b' ) // Binary
		// if ( pName.containsIgnoreCase("abcdef") || pName[-1] == 'h' ) // Hex (bad. hex vals could be pure #s)
		// if ( pName.contains('.', 1) ) // Decimal number
		// else Integer

		// Default return
		return TT_number; // Should really be a data type, not a token type
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

Result::Value Engine::tokenize( CharList& tokenValue, List<Token>& tokens ) {
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
			const TokenType& tokenType,
			CharList& tokenValue,
			List<Token>& tokens,
			ByteStream& stream )
{
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

EngineResult::Value Engine::run( ByteStream& stream ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::info, EngineMessage::Running);
#endif

	char c;
	CharList tokenValue; // Since I have to build with it, it's an easy-to-append-to list
	List<Token> tokens;
	TokenType tokenType;

	while ( ! stream.atEOS() ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
		print(LogLevel::info, EngineMessage::COUNT);
#endif
		c = stream.getNextByte();
		if ( c == '\0' )
			throw RandomNullByteInStream();
		// Quit adding to a token identifier when whitespace is found
		if ( isWhitespace(c) ) {
			if ( tokenValue.size() == 0 ) {
				continue;
			}
			switch( tokenize(tokenValue, tokens) ) {
			case Result::Error:
				return EngineResult::Error;
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
					switch( tokenize(tokenValue, tokens) ) {
					case Result::Error:
						return EngineResult::Error;
					default: break;
					}
				}
				tokenValue.clear();
			}
//#if COPPER_DEBUG_ENGINE_MESSAGES
			tokenValue.push_back(c);
//#endif
			// Comments, strings, and special chars are special cases that need to be handled immediately
			switch( handleCommentsStringsAndSpecials(tokenType, tokenValue, tokens, stream) ) {
			case Result::Error:
				return EngineResult::Error;
			default: break;
			}
		}
		else {
			if ( tokenValue.size() >= 255 ) { // Don't accept tokens longer than this
				//print(LogLevel::error, "ERROR: Token identifier is too long." );
				print(LogLevel::error, EngineMessage::LongToken);
				return EngineResult::Error;
			}
			tokenValue.push_back(c);
		}
	}
	if ( tokenValue.size() > 0 ) {
		// Push the last token
		switch( tokenize(tokenValue, tokens) ) {
		case Result::Error:
			return EngineResult::Error;
		default: break;
		}
	}
	List<Token>::Iter i = tokens.start();
	if ( i.has() )
	do {
		switch ( process(*i) ) {
		case EngineResult::Ok:
			break;
		case EngineResult::Error:
			return EngineResult::Error;
		case EngineResult::Done:
			return EngineResult::Done;
		}
	} while( ++i );
	return EngineResult::Ok;
}

EngineResult::Value Engine::process( const Token& pToken )
{
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process");
#endif
#ifdef COPPER_PRINT_ENGINE_PROCESS_TOKENS
	std::printf("[DEBUG VAR: pToken.name = %s\n", pToken.name.c_str());
	std::printf("[DEBUG VAR: pToken.type = %u\n", (unsigned)(pToken.type));
	std::printf("[DEBUG taskStack SIZE = %lu\n", taskStack.size());
	if ( taskStack.size() > 0 )
		std::printf("[DEBUG task id(%u)\n", (unsigned)(taskStack.getLast().getTask().name) );
#endif
	if ( taskStack.size() == 2000 ) {
		clearStacks();
		print(LogLevel::error, EngineMessage::StackOverflow);
		return EngineResult::Error;
	}

	// Immediate stoppage of processing.
	if ( pToken.type == TT_end_main || systemExitTrigger ) {
		clearStacks();
		signalEndofProcessing();
		return EngineResult::Done;
	}
	/*
		! RULES !
		Every token must result in either a task or setting the last object.
		Every task must, in its finality, set the lastObject.
		Furthermore, NO TASK may use the lastObject until after the interpret() call
		(in which the lastObject is set).
	*/
	bool looprun = true;
	bool skipInterpret = false;
	while ( taskStack.size() > 0 && looprun ) {
		switch( processTask(taskStack.getLast().getTask(), pToken) ) {
		case TaskResult::_none:
			looprun = false;
			break;
		case TaskResult::_pop_loop:
			// If a task eats a token, it needs to return cycle, not pop.
			// If a task cannot handle a token but can terminate at that point, it should pop.
//std::printf("[Task Result pop loop in first section] [stack=%u]\n", stack.getCurrLevel()); // Legacy debug
			taskStack.pop();
			continue;
		case TaskResult::_cycle:
			return EngineResult::Ok;
		case TaskResult::_skip:
			skipInterpret = true;
			looprun = false;
			break;
		case TaskResult::_error:
			clearStacks();
			return EngineResult::Error;
		case TaskResult::_done:
			clearStacks();
			signalEndofProcessing();
			return EngineResult::Done;
		}
	}

	if ( !skipInterpret ) {
		switch( interpret(pToken) ) {
		case TaskResult::_none:
			break;
		case TaskResult::_pop_loop:
			return EngineResult::Error;
		case TaskResult::_cycle:
			return EngineResult::Ok;
		case TaskResult::_skip:
			// There is no correct default for this situation, but it never happens.
			//print(LogLevel::error, "SYSTEM ERROR: interpret(token) returning interpret-skip.");
			return EngineResult::Error;
		case TaskResult::_error:
			clearStacks();
			return EngineResult::Error;
		case TaskResult::_done:
			clearStacks();
			signalEndofProcessing();
			return EngineResult::Done;
		}
	}

	switch( processTasksOnObjects() ) {
	case TaskResult::_none:
		if ( systemExitTrigger ) {
			return EngineResult::Done;
		}
		// A task is waiting on something. It may pop itself.
		return EngineResult::Ok;
	case TaskResult::_done:
		clearStacks();
		signalEndofProcessing();
		return EngineResult::Done;
	case TaskResult::_error:
	default:
		clearStacks();
		return EngineResult::Error;
	}
	// The last object is destroyed upon Engine deletion.
}

TaskResult::Value Engine::processTasksOnObjects() {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::processTasksOnObjects");
#endif
	while ( taskStack.size() > 0 ) {
		switch( processTaskOnLastObject(taskStack.getLast().getTask()) ) {
		case TaskResult::_none:
			// A task is waiting on something. It may pop itself.
			return TaskResult::_none;
		case TaskResult::_pop_loop:
			// The task needs to be popped and processing should continue
//print(LogLevel::info, "[Task Result pop loop in third section]\n"); Legacy debug
			taskStack.pop();
			break; // i.e. continue with loop
		case TaskResult::_cycle:
			return TaskResult::_none;
		case TaskResult::_skip:
			// There is no correct default for this situation, but it never happens.
			// The print() call here can be safely removed when the core VM code is finally frozen.
			print(LogLevel::error, "SYSTEM ERROR: processTaskOnLastObject() returning interpret-skip.");
			return TaskResult::_error;
		case TaskResult::_error:
			return TaskResult::_error;
		case TaskResult::_done:
			return TaskResult::_done;
		}
	}
	// All tasks are done
	return TaskResult::_none;
}

TaskResult::Value Engine::performObjProcessAndCycle(const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::performObjProcessAndCycle");
#endif
	// Exceptional case: "exit" called.
	if ( lastToken.type == TT_exit ) {
		systemExitTrigger = true;
		return TaskResult::_done;
	}

	switch( processTasksOnObjects() ) {
	case TaskResult::_none:
		switch( process(lastToken) ) {
		case EngineResult::Ok:
			return TaskResult::_cycle;
		case EngineResult::Error:
			return TaskResult::_error;
		case EngineResult::Done:
			return TaskResult::_done;
		}
	case TaskResult::_done:
		return TaskResult::_done;
	case TaskResult::_error:
	default:
		return TaskResult::_error;
	}
}

TaskResult::Value Engine::interpret( const Token& pToken ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::interpret");
#endif
	// Tokens in this switch that result in tasks should return TaskResult::_cycle.
	// Tokens in this switch that do not result in tasks should return TaskResult::_none and set lastObject.
	switch( pToken.type ) {
	case TT_malformed:
		//print(LogLevel::error, "ERROR: Malformed token.");
		print(LogLevel::error, EngineMessage::MalformedToken);
		return TaskResult::_error;

	case TT_unknown:
		//print(LogLevel::error, "ERROR: Unknown token.");
		print(LogLevel::error, EngineMessage::UnknownToken);
		return TaskResult::_error;

	//---------------

	case TT_comment:
		return TaskResult::_cycle;

	//---------------
	case TT_exit:
		systemExitTrigger = true;
		return TaskResult::_done;

	//---------------
	case TT_end_segment:
		// Handled inside the task process, so it shouldn't ever show up here, but it can.
		return TaskResult::_cycle;

	//---------------
	case TT_parambody_open:
		// Should be absorbed by a task, so generate error message if not.
		//print( LogLevel::error, "ERROR: Unused parameter body opening '('." );
		print( LogLevel::error, EngineMessage::OrphanParamBodyOpener );
		return TaskResult::_error;

	case TT_parambody_close:
		// Should be absorbed by a task, so generate error message if not.
		//print( LogLevel::error, "ERROR: Unmatched parameter body closing ')'." );
		print( LogLevel::error, EngineMessage::OrphanParamBodyCloser );
		return TaskResult::_error;

	//---------------
	case TT_body_open:
		processBodyOpen();
		return TaskResult::_cycle;

	case TT_body_close:
		// Should be absorbed by a task, so generate error message if not.
		//print( LogLevel::error, "ERROR: Unmatched body closing '}'." );
		print( LogLevel::error, EngineMessage::OrphanBodyCloser );
		clearStacks(); // Unwind stacks
		return TaskResult::_error;

	//---------------
	case TT_function_declare:
		constructFunctionFromFN();
		return TaskResult::_cycle;

		// Only encountered by tasks within a function whose executing is ending
	case TT_function_end_run:
		return TaskResult::_cycle;

	//---------------

	case TT_if:
		taskStack.push_back(TaskContainer(new TaskIfStructure()));
		return TaskResult::_cycle;

	case TT_elif:
		// Should be captured by if-structure
		print( LogLevel::error, EngineMessage::OrphanElif );
		return TaskResult::_error;

	case TT_else:
		// Should be captured by if-structure
		print( LogLevel::error, EngineMessage::OrphanElse );
		return TaskResult::_error;

	case TT_loop:
		taskStack.push_back(TaskContainer(new TaskLoopStructure()));
		return TaskResult::_cycle;

	case TT_endloop:
		loopEndMailbox = true;
		return TaskResult::_cycle;

	case TT_skip:
		loopSkipMailbox = true;
		return TaskResult::_cycle;

	//---------------
	case TT_name:
		if ( taskpushSystemFunction(pToken.name) ) {
			return TaskResult::_cycle;
		}
		if ( taskpushExternalFunction(pToken.name) ) {
			return TaskResult::_cycle;
		}
		taskpushUserFunction(pToken.name);
		return TaskResult::_cycle;

	//---------------
	// Handling boolean

	case TT_boolean_true:
		constructBoolean(true);
		return TaskResult::_none;

	case TT_boolean_false:
		constructBoolean(false);
		return TaskResult::_none;

	//---------------

	case TT_number:
		constructNumber(pToken.name);
		return TaskResult::_none;

	//---------------

	case TT_string:
		constructString(pToken.name);
		return TaskResult::_none;

	//---------------
	default: break;
	}

	// Unhandled token
	//print(LogLevel::error, "ERROR: Unhandled token.");
	print(LogLevel::error, EngineMessage::TokenNotHandled);
	return TaskResult::_error;
}

bool Engine::isWhitespace( const char c ) const {
	// I forgot the other types of whitespace, but I need them, such as the no-separator character
	// which could be the source of huge problems (hidden security bugs in open-source code).
	return (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\f' || c == '\v' || c == '\0');
	// Whether or not I should test c == '\0' or not is an interesting consideration.
	// I probably shouldn't, and that way unicode chunks given as int via getc() work fine.
}

bool Engine::isSpecialCharacter( const char c, TokenType& pTokenType ) {
	//unsigned int i=0;
	// The only special characters I really need are:
	// ,=.~(){}";
	// ... the slash being for comments.
	// The others can be inferred. If I wanted, maybe I could have a function named "a+b"
	//static const char* specialChars = ",=.~(){}\";"; // Should be \0 terminated.
	//for (; specialChars[i] != '\0'; ++i) {
	//	if ( specialChars[i] == c )
	//		return true;
	//}
	//return false;

	switch( c ) {
	case ',': pTokenType = TT_end_segment;				return true;
	case '=': pTokenType = TT_assignment;				return true;
	case '~': pTokenType = TT_pointer_assignment;		return true;
	case '.': pTokenType = TT_member_link;				return true;
	case '(': pTokenType = TT_parambody_open;			return true;
	case ')': pTokenType = TT_parambody_close;			return true;
	case '{': pTokenType = TT_body_open;				return true;
	case '}': pTokenType = TT_body_close;				return true;
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

bool Engine::isEscapeCharacter(const char c) const {
	return c == CONSTANT_ESCAPE_CHARACTER_TOKEN;
}

bool Engine::isValidToken( const TokenType& token ) const {
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

bool Engine::isValidNameCharacter( const char c ) const {
	switch(c) {
	case '_':
		return true;
	//case '+':
	//	return true;
	//case '-':
	//	return true;
	//case '*':
	//	return true;
	//case '/':
	//	return true;
	//case '%':
	//	return true;
	//case '!':
	//	return true;
	//case '?':
	//	return true;
	default:
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}
	//return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Engine::isValidName( const String& pName ) const {
	//if ( pName.size() > 256 )
	//	return false;

	unsigned int i=0;
	for (; i < pName.size(); ++i ) {
		if ( isValidNameCharacter(pName[i]) )
			continue;
		// TODO: Should check for valid Unicode, but that means bringing in a lib or making a custom one.
		// TODO: I need to block invisible and undefined/nonprintable characters.
		return false; // Otherwise, invalid character
	}
	return true;
}

bool Engine::isCommentToken(const char c) const {
	return c == CONSTANT_COMMENT_TOKEN;
}

bool Engine::isStringToken(const char c) const {
	return c == CONSTANT_STRING_TOKEN;
}

Result::Value Engine::scanComment( ByteStream& stream ) {
	while ( ! stream.atEOS() ) {
		if ( isCommentToken(stream.getNextByte()) ) {
			return Result::Ok;
		}
	}
	//print(LogLevel::error, "ERROR: Stream halted before comment ended.");
	print( LogLevel::error, EngineMessage::StreamInteruptedInComment );
	return Result::Error;
}

Result::Value Engine::collectString(ByteStream& stream, CharList& collectedValue) {

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

//------------------------------------
// &&&&&&&&& Task creation &&&&&&&&&&&

void Engine::processBodyOpen() {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::processBodyOpen");
#endif
	// If we get to this point, it means that the token wasn't gobbled up by "fn"
	taskStack.push_back( TaskContainer(new TaskFunctionConstruct(TASK_FCS_from_bodystart)) );
}

void Engine::constructFunctionFromFN() {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::constructFunctionFromFN");
#endif
	taskStack.push_back( TaskContainer(new TaskFunctionConstruct(TASK_FCS_from_fn)) );
}

void Engine::constructBoolean(bool value) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::constructBoolean");
#endif
	lastObject.setWithoutRef( new ObjectBool(value) );
}

void Engine::constructString(const String& value) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::constructString");
#endif
	// Should actually check for unicode-only text.
	// But this can also be done by purging the string.
/*
	ObjectString* objStr;
	switch( ObjectString::createUTF8String(value, objStr) ) {
	case Result::Ok:
		lastObjectsetWithoutRef(objStr);
		return Result::Ok;
	case Result::Error:
		print(LogLevel::Error, EngineMessage::InvalidUTF8String
		return Result::Error;
	}
*/
#ifdef COPPER_PURGE_NON_PRINTABLE_ASCII_INPUT_STRINGS
	ObjectString* objStr = new ObjectString(value);
	objStr->purgeNonPrintableASCII();
	lastObject.setWithoutRef( objStr );
#else
	lastObject.setWithoutRef( new ObjectString(value) );
#endif
/*
#ifdef COPPER_PURGE_NON_UTF8_INPUT_STRINGS
	ObjectString* objStr = new ObjectString(value);
	objStr->purgeNonUTF8();
	lastObject.setWithoutRef( objStr );
#else
	lastObject.setWithoutRef( new ObjectString(value) );
#endif
*/
}

void Engine::constructNumber(const String& value) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::constructNumber");
#endif
	NumberObjectFactory* factory;
	if ( numberObjectFactoryPtr.obtain(factory) ) {
		lastObject.setWithoutRef( factory->createNumber(value) );
	} else {
		print(LogLevel::warning, "Missing number factory.");
		lastObject.setWithoutRef( new ObjectNumber(value) );
	}
}

bool Engine::taskpushSystemFunction( const String& pName ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::taskpushSystemFunction");
#endif
	// Note: Boolean function are built-in because they are required for functionality
	if ( pName.equals(CONSTANT_FUNCTION_RETURN) ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_return)) );
		return true;
	}
	if ( pName.equals("own") ) {
		if ( ownershipChangingEnabled ) {
			print(LogLevel::warning, EngineMessage::PointerNewOwnershipDisabled);
			return false;
		}
		taskStack.push_back( TaskContainer(new TaskProcessNamed(TaskName::_own)) );
		return true;
	}
	if ( pName.equals("not") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_not)) );
		return true;
	}
	if ( pName.equals("all") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_all)) );
		return true;
	}
	if ( pName.equals("any") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_any)) );
		return true;
	}
	if ( pName.equals("nall") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_nall)) );
		return true;
	}
	if ( pName.equals("none") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_none)) );
		return true;
	}
	if ( pName.equals("are_fn") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_are_fn)) );
		return true;
	}
	if ( pName.equals("is_ptr") ) {
		taskStack.push_back( TaskContainer(new TaskProcessNamed(TaskName::_is_ptr)) );
		return true;
	}
	if ( pName.equals("is_owner") ) {
		taskStack.push_back( TaskContainer(new TaskProcessNamed(TaskName::_is_owner)) );
		return true;
	}
	if ( pName.equals("are_empty") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_are_empty)) );
		return true;
	}
	if ( pName.equals("are_same") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_are_same)) );
		return true;
	}
	if ( pName.equals("member") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_member)) );
		return true;
	}
	if ( pName.equals("member_count") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_member_count)) );
		return true;
	}
	if ( pName.equals("is_member") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_is_member)) );
		return true;
	}
	if ( pName.equals("set_member") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_set_member)) );
		return true;
	}
	if ( pName.equals("union") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_union)) );
		return true;
	}
	if ( pName.equals("type") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_type)) );
		return true;
	}
	if ( pName.equals("are_same_type") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_are_same_type)) );
		return true;
	}
	if ( pName.equals("are_bool") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_are_bool)) );
		return true;
	}
	if ( pName.equals("are_string") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_are_string)) );
		return true;
	}
	if ( pName.equals("are_number") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_are_number)) );
		return true;
	}
	if ( pName.equals("assert") ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(SystemFunction::_assert)) );
		return true;
	}
	return false;
}

bool Engine::taskpushExternalFunction( const String& pName ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::taskpushExternalFunction");
#endif

	RobinHoodHash<ForeignFuncContainer>::BucketData* bucketData = foreignFunctions.getBucketData(pName);
	if ( bucketData != 0 ) {
		taskStack.push_back( TaskContainer(new TaskFunctionFound(bucketData->item.getForeignFunction())) );
		return true;
	}
	return false;
}

void Engine::taskpushUserFunction( const String& pName ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::taskpushUserFunction");
#endif
	/*
		Order:
		1) Search the top scope.
		2) Search the global scope (if it's not the same scope).
		3) Add the variable to the top scope.
	*/
	Variable* var = REAL_NULL;
	if ( stack.getTop().getScope().findVariable(pName, var) ) {
		taskStack.push_back(TaskContainer(new TaskFunctionFound(var)));
	}
		// bottom stack fram != top stack frame
	else if ( stack.getCurrLevel() > 1 && stack.getBottom().getScope().findVariable(pName, var) ) {
		taskStack.push_back(TaskContainer(new TaskFunctionFound(var)));
	} else {
		// Using new(no throw) for creating variables is not helpful. It should throw and catch elsewhere.
		var = stack.getTop().getScope().addVariable(pName);
		if ( var != 0 ) {
			taskStack.push_back(TaskContainer(new TaskFunctionFound(var)));
		} else {
			print(LogLevel::error, EngineMessage::TaskVarIsNull);
		}
	}
}

//-------------------------------------
// &&&&&&&&& Task processing &&&&&&&&&&

TaskResult::Value Engine::processTask(Task& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::processTask");
#endif
	switch( task.name ) {
	case TaskName::_Function_Construct:
		return processFunctionConstruct((TaskFunctionConstruct&)task, lastToken);
	case TaskName::_Function_Found:
		return processFunctionFound((TaskFunctionFound&)task, lastToken);
	case TaskName::_if:
		return processIfStatement((TaskIfStructure&)task, lastToken);
	case TaskName::_loop:
		return processLoop((TaskLoopStructure&)task, lastToken);
	case TaskName::_own:
	case TaskName::_is_ptr:
	case TaskName::_is_owner:
		return processNamed((TaskProcessNamed&)task, lastToken);
	default:
		return TaskResult::_none;
	}
}

TaskResult::Value Engine::processTaskOnLastObject(Task& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::processTaskOnLastObject");
#endif
	switch( task.name ) {
	case TaskName::_Function_Construct:
		return processObjForFunctionConstruct((TaskFunctionConstruct&)task);
	case TaskName::_Function_Found:
		return processObjForFunctionFound((TaskFunctionFound&)task);
	case TaskName::_if:
		return processObjForIfStatement((TaskIfStructure&)task);
	default:
		return TaskResult::_none;
	}
}

TaskResult::Value Engine::processFunctionConstruct(TaskFunctionConstruct& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::processFunctionConstruct");
#endif
	switch(task.state ) {
	case TASK_FCS_from_fn:
		return FuncBuild_processFromFn(task, lastToken);

	case TASK_FCS_param_collect:
		return FuncBuild_processAtParamCollect(task, lastToken);

	case TASK_FCS_param_value:
		// Skip a token so that it can turn into an object, however
		// for assignment, the next token type must be one of the following:
		// a name, function start, body start
		switch( lastToken.type ) {
		case TT_function_declare:
		case TT_body_open:
		case TT_name:
		case TT_boolean_true:
		case TT_boolean_false:
		case TT_number:
		case TT_string:
			return TaskResult::_none;
		default:
			//print(LogLevel::error, "ERROR: Bad token found when obtaining parameter value.");
			print(LogLevel::error, EngineMessage::InvalidParamValueToken);
			return TaskResult::_error;
		}
		return TaskResult::_error;

	case TASK_FCS_param_ptr_value:
		return FuncBuild_processCollectPointerValue(lastToken);

	case TASK_FCS_end_param_collect:
		return FuncBuild_processEndParamCollect(task, lastToken);

	case TASK_FCS_from_bodystart:
		return FuncBuild_processFromBodyStart(task, lastToken);

	// Default should be an error as it should never be reached
	default:
		print(LogLevel::error, "SYSTEM ERROR: Default case reached in processFunctionConstruct");
		return TaskResult::_error; // defensive programming
	}
}

TaskResult::Value Engine::processObjForFunctionConstruct(TaskFunctionConstruct& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::processObjForFunctionConstruct");
#endif
	switch( task.state ) {
	case TASK_FCS_param_value:
		task.function->getPersistentScope().setVariableFrom(task.paramName, lastObject.raw(), false);
		task.state = TASK_FCS_param_collect;
		return TaskResult::_none;

	case TASK_FCS_param_ptr_value:
		task.function->getPersistentScope().setVariableFrom(task.paramName, lastObject.raw(), true);
		task.state = TASK_FCS_param_collect;
		return TaskResult::_none;

	// Should never reach this point
	default:
		// This could probably be removed once bugs are worked out.
		print(LogLevel::error, "SYSTEM ERROR: Default case reached in processObjForFunctionConstruct");
		return TaskResult::_error; // defensive programming
	}
}

TaskResult::Value Engine::FuncBuild_processFromFn(TaskFunctionConstruct& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FuncBuild_processFromFn");
#endif
	// From the beginning of the function declaration
	// "fn" has been found already, initiating this task.
	switch( lastToken.type ) {
	case TT_parambody_open:
		task.state = TASK_FCS_param_collect;
		return TaskResult::_cycle;
	case TT_body_open:
		// Parameter-less body
		task.openBodies = 1;
		task.state = TASK_FCS_from_bodystart;
		return TaskResult::_cycle;
	default:
#ifdef COPPER_SHORT_CIRCUIT_FN
		// Short circuit (unconventional)
		// Must pop this task before proceeding
		taskStack.pop();
		lastObject.setWithoutRef(new FunctionContainer());
		return performObjProcessAndCycle(lastToken);
#else
		//print(LogLevel::error, "ERROR: Bad token after function start.");
		print(LogLevel::error, EngineMessage::InvalidTokenAtFunctionStart);
		return TaskResult::_error;
#endif
	}
}

TaskResult::Value Engine::FuncBuild_processAtParamCollect(TaskFunctionConstruct& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FuncBuild_processAtParamCollect");
#endif
	// Parameter collection looks for just names, but also has to be aware when those
	// names are assigned values, which forces changing to another construction state.
	// Furthermore, parameter collection cannot occur when the body is a nested parameter body
	// (which is technically a processing body in such cases).
	switch( lastToken.type ) {
	case TT_end_segment:
		// It doesn't matter if a parameter name is not followed by an end-segment token
		// but adding one prevents ambiguity
		if ( task.hasOpenParam ) {
			// Apparently the last token was not followed by an end-segment or assignment token
			task.function->params.push_back( task.paramName );
		}
		task.hasOpenParam = false;
		return TaskResult::_cycle;
	case TT_assignment:
		if ( task.hasOpenParam ) {
			task.state = TASK_FCS_param_value;
			task.hasOpenParam = false;
		} else {
			//print(LogLevel::warning, "Warning: Unused assignment symbol among newly constructed function parameters. (Is it after an end-segment symbol?)");
			print(LogLevel::warning, EngineMessage::UnusedAsgnInFunctionBuildParams);
		}
		return TaskResult::_cycle;
	case TT_pointer_assignment:
		if ( task.hasOpenParam ) {
			task.state = TASK_FCS_param_ptr_value;
			task.hasOpenParam = false;
		} else {
			//print(LogLevel::warning, "Warning: Unused pointer creation symbol among newly constructed function parameters. (Is it after an end-segment symbol?)");
			print(LogLevel::warning, EngineMessage::UnusedPtrAsgnInFunctionBuildParams);
		}
		return TaskResult::_cycle;
	case TT_parambody_open: // '('
		//print(LogLevel::error, "ERROR: Found alternate processing body among newly constructed function parameter names.");
		print(LogLevel::error, EngineMessage::OrphanParamBodyOpener); // It may be best to give this its own unique error message.
		return TaskResult::_error;
	case TT_parambody_close: // ')'
		// Is this right?
		if ( task.hasOpenParam ) {
			task.function->params.push_back( task.paramName );
		}
		task.state = TASK_FCS_end_param_collect;
		return TaskResult::_cycle;
	case TT_name:
		if ( task.hasOpenParam ) {
			// Apparently the last token was not followed by an end-segment or assignment token
			task.function->params.push_back( task.paramName );
		}
		// New token
		task.paramName = lastToken.name;
		task.hasOpenParam = true;
		return TaskResult::_cycle;
	default: break;
	}

	//print(LogLevel::error, "ERROR: Unused token found among newly constructed function parameter names.");
	print(LogLevel::error, EngineMessage::UnusedTokenInFunctionBuildParams);
	return TaskResult::_error;
}

TaskResult::Value Engine::FuncBuild_processCollectPointerValue(const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FuncBuild_processCollectPointerValue");
#endif
	if ( lastToken.type == TT_name ) {
		// Note that it is only possible to save a pointer to user functions, for good reason.
		// I want "." to have higher precedence than "~", and since I only need the function container
		// anyways (which is already an object), I can simply complete the pointer operation after
		// the last object has been acquired.
		return TaskResult::_none;
	} else {
		//print(LogLevel::error, "ERROR: Cannot point to non-names.");
		print(LogLevel::error, EngineMessage::PtrAssignedNonName);
		return TaskResult::_error;
	}
}

TaskResult::Value Engine::FuncBuild_processEndParamCollect(TaskFunctionConstruct& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FuncBuild_processEndParamCollect");
#endif
	// Language rule: The very next token in this state MUST be the execution body opening
	if ( lastToken.type == TT_body_open ) {
		task.openBodies = 1;
		task.state = TASK_FCS_from_bodystart;
		return TaskResult::_cycle;
	}
	// Optionally, function creation can short-circuit here
	// a = fn()
	lastObject.setWithoutRef(new FunctionContainer(task.function));
	taskStack.pop();
	return performObjProcessAndCycle(lastToken);
}

TaskResult::Value Engine::FuncBuild_processFromBodyStart(TaskFunctionConstruct& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FuncBuild_processFromBodyStart");
	//std::printf("DEBUG VARS:\ntask =%u\ntask.body =%u\ntask.function =%u\n", (unsigned int)&task, (unsigned int)(task.body), (unsigned int)(task.function) );
	//std::printf("DEBUG VAR task.name id=%u\n", (unsigned int)(task.name));
#endif
	// Everything in a body must be collected and added to the body
	switch( lastToken.type ) {
	case TT_body_open:
		task.openBodies += 1;
		if ( task.openBodies == (unsigned int)-1 ) {
			//print( LogLevel::error, "ERROR: Too many nested bodies in given execution body." );
			print( LogLevel::error, EngineMessage::ExceededBodyCountLimit);
			return TaskResult::_error;
		}
		task.body->addToken(lastToken);
		return TaskResult::_cycle;
	case TT_body_close:
		if ( task.openBodies == 0 ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
			print(LogLevel::error, "SYSTEM ERROR: function construction task has no open bodies.");
#endif
			return TaskResult::_error;
		}
		task.openBodies -= 1;
		if ( task.openBodies == 0 ) {
			task.function->body.set(task.body);
			lastObject.setWithoutRef(new FunctionContainer(task.function, generateFunctionID()));
			taskStack.pop();
			return TaskResult::_skip;
		} // else
		task.body->addToken(lastToken);
		return TaskResult::_cycle;
	default:
		task.body->addToken(lastToken);
		return TaskResult::_cycle;
	}
}

unsigned int Engine::generateFunctionID() {
	return ++functionID;
}

TaskResult::Value Engine::processFunctionFound(TaskFunctionFound& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FuncFound_processFunctionFound");
#endif
	// Note that this plays tag-team with the other function

	// Remember that when attempting to use a pointer, if a function no longer exists, it must
	// be recreated, UNIQUE to the pointer (not simply saving the old function container).
	// Otherwise, you get stuff tied together.

	// Perform standard actions first, only resorting to differentiating between types of functions
	// on the last step
	switch( task.state ) {
	case TASK_FF_start:
		return FuncFound_processFromStart(task, lastToken);
	case TASK_FF_assignment:
		return FuncFound_processAssignment(lastToken);
	case TASK_FF_pointer_assignment:
		return FuncFound_processPointerAssignment(lastToken);
	case TASK_FF_find_member:
		return FuncFound_processFindMember(task, lastToken);
	case TASK_FF_collect_params:
		return FuncFound_processCollectParams(task, lastToken);
	default:
		print(LogLevel::error, "SYSTEM ERROR: Default case reached in FuncFound_processObjForFunctionFound");
		return TaskResult::_error;
	}
	//return TaskResult::_error; // All cases should be handled. If not, compiler should error.
}

TaskResult::Value Engine::processObjForFunctionFound(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FuncFound_processObjForFunctionFound");
#endif
	switch( task.state ) {
	case  TASK_FF_start:
		// Technically, this is probably an error.
		break;
	case TASK_FF_assignment:
		// Copy assignment
		return FunctionFound_processAssignmentObj(task);
	case TASK_FF_pointer_assignment:
		return FunctionFound_processPointerAssignmentObj(task);
	case TASK_FF_find_member:
		// Technically, this is probably an error.
		break;
	case TASK_FF_collect_params:
		task.addParam( lastObject.raw() );
		return TaskResult::_none;
	}
	return TaskResult::_error;
}

TaskResult::Value Engine::FuncFound_processFromStart(TaskFunctionFound& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FuncFound_processFromStart");
#endif
	switch(lastToken.type) {
	case TT_assignment:
		task.state = TASK_FF_assignment;
		return TaskResult::_cycle;
	case TT_pointer_assignment:
		task.state = TASK_FF_pointer_assignment;
		return TaskResult::_cycle;
	case TT_member_link:
		task.state = TASK_FF_find_member;
		return TaskResult::_cycle;
	case TT_parambody_open:
		task.state = TASK_FF_collect_params;
		task.openParamBodies = 1;
		return TaskResult::_cycle;
	case TT_immediate_run:
		task.state = TASK_FF_collect_params;
		return FuncFound_processRun(task);
	default:
		if ( task.type == TASK_FFTYPE_variable ) {
			lastObject.set( task.varPtr.raw()->getRawContainer() );
		} else {
			lastObject.setWithoutRef(new FunctionContainer());
		}
/*
	This is a tough spot. The system has a token that must be used, but if a variable is found that is
	not followed by parentheses, it is supposed to create an object to be processed (supposedly).
	For it to be processed as an object, the following should be performed:
	taskStack.pop()
	return TaskResult::_skip;

	For skipping this and letting processing continue as though nothing happened, call:
	return TaskResult::_pop_loop;

	Since both routes are required for language functionality, these are opposed.
	To compromise for the problem, we simulate the finishing and restarting of a cycle.
	This goes against the policy of limiting the stack, but there isn't much choice at the moment.
*/
		// Must pop this task before proceeding
		taskStack.pop();
		return performObjProcessAndCycle(lastToken);
	}
}

TaskResult::Value Engine::FuncFound_processAssignment(const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FuncFound_processAssignment");
#endif
	switch( lastToken.type ) {
	case TT_body_open:
	case TT_function_declare:
	case TT_name:
	case TT_boolean_true:
	case TT_boolean_false:
	case TT_number: // Other numbers may be needed
	case TT_string:
	case TT_binary:
		return TaskResult::_none;
	default: break;
	}
	//print( LogLevel::error, "ERROR: Invalid assignment.");
	print( LogLevel::error, EngineMessage::InvalidAsgn );
	//printf("[DEBUG: token id =%u\n", (unsigned int)lastToken.type);
	return TaskResult::_error;
}

TaskResult::Value Engine::FuncFound_processPointerAssignment(const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FuncFound_processPointerAssignment");
#endif
	switch( lastToken.type ) {
	case TT_name: return TaskResult::_none;
	default: break;
	}
	//print( LogLevel::error, "ERROR: Invalid pointer assignment.");
	print( LogLevel::error, EngineMessage::InvalidPtrAsgn );
	return TaskResult::_error;
}

TaskResult::Value Engine::FuncFound_processFindMember(TaskFunctionFound& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FuncFound_processFindMember");
#endif
	if ( lastToken.type != TT_name ) {
		//print(LogLevel::error, "ERROR: Expected name after member-accessor token. Found other.");
		print(LogLevel::error, EngineMessage::InvalidTokenAfterMemberAccessor);
		return TaskResult::_error;
	}
	// Search the persistent scope of the object, then call function found on it
	Variable* var;
	Variable* member;
	Function* f;

	// Oops: Needs to check for the type. CSide functions can't open a scope.
	switch(task.type)
	{
	case TASK_FFTYPE_variable:
		if ( task.varPtr.obtain(var) ) {
			f = var->getFunction(logger);
			f->getPersistentScope().getVariable(lastToken.name, member);
			// Rather than replacing the TaskFunctionFound object, reuse it
			task.state = TASK_FF_start;
			task.superPtr.set(var);
			task.varPtr.set(member);
			return TaskResult::_cycle;
		} else {
			//print(LogLevel::error, "ERROR: Something has destroyed the current task variable.");
			print(LogLevel::error, EngineMessage::TaskVarIsNull);
			lastObject.setWithoutRef(new FunctionContainer());
			return TaskResult::_error;
		}
	case TASK_FFTYPE_cside:
		print(LogLevel::error, EngineMessage::OpeningSystemFuncScope);
		return TaskResult::_error;

	case TASK_FFTYPE_ext_cside:
		// What could happen here is that the foreign function could have its own scope.
		// This could be obtained via bool getScope(Scope*)
		// This subscope could contain other functions.
		// At the moment, however, there isn't much point to this.
		print(LogLevel::error, EngineMessage::OpeningExtensionFuncScope);
		return TaskResult::_error;

	default: break;
	}
	return TaskResult::_error;
}

TaskResult::Value Engine::FuncFound_processCollectParams(TaskFunctionFound& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FuncFound_processCollectParams");
#endif
	switch( lastToken.type ) {
	case TT_parambody_open:
		task.openParamBodies += 1;
		if ( task.openParamBodies == (unsigned int)-1 ) {
			//print( LogLevel::error, "ERROR: Too many open parameter bodies within parameter collection." );
			print( LogLevel::error, EngineMessage::ExceededParamBodyCountLimit );
			return TaskResult::_error;
		}
		return TaskResult::_none;
	case TT_parambody_close:
		task.openParamBodies -= 1;
		if ( task.openParamBodies == 0 ) {
			return FuncFound_processRun(task);
		}
		return TaskResult::_none;
	default:
		return TaskResult::_none;
	}
	// Parameters are collected as a list in the object collection
}

TaskResult::Value Engine::FuncFound_processRun(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FuncFound_processRun");
#endif
	//task.state = TASK_FF_running; // Flag for the return function to seek.
	functionReturnMailbox = false;
	// Protect from polution and allow for functions to have a default return.
	lastObject.setWithoutRef(new FunctionContainer());

	TaskResult::Value result;
	switch(task.type) {
	case TASK_FFTYPE_variable:
		result = FuncFound_processRunVariable(task);
		taskStack.pop();
		return result;

	case TASK_FFTYPE_cside:
		result = FuncFound_processRunSysFunction(task);
		taskStack.pop();
		return result;

	case TASK_FFTYPE_ext_cside:
		if ( notNull(task.extFuncPtr) ) {
			if ( ! task.extFuncPtr->call(task.getParamsList(), lastObject) ) {
				lastObject.setWithoutRef(new FunctionContainer());
			}
			taskStack.pop();
			return TaskResult::_skip;
		} else {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
			print(LogLevel::debug, "[C++ ERROR: Missing external function in FuncFound_processRun.");
#endif
			return TaskResult::_error;
		}

	default: break;
	}
	return TaskResult::_error;
}

TaskResult::Value Engine::FuncFound_processRunVariable(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FuncFound_processRunVariable");
#endif
	Function* func = task.varPtr.raw()->getFunction(logger);
	if ( func->constantReturn ) {
		lastObject.set( func->result.raw() ); // Copy is created when saved to variable
		return TaskResult::_skip;
	}
	// Save the container ID so it isn't destroyed with the stack
	unsigned int funcID = task.varPtr.raw()->getRawContainer()->getID();

	Variable* super; // Parent scope pointer

	// Add temporary stack context
	stack.push();
	if ( task.superPtr.obtain(super) ) {
		// Add super (parent) pointer reference to the temporary scope
		stack.getTop().getScope().setVariable(CONSTANT_FUNCTION_SUPER, super, true);
	}
	// Add self-reference to the temporary scope
	stack.getTop().getScope().setVariable(CONSTANT_FUNCTION_SELF, task.varPtr.raw(), true);
	// Add parameters to temporary scope
	unsigned int pi=0;
	List<Object*>::Iter tpi = task.createParamsIterator();
	if ( task.getParamCount() > 0 ) {
		for (; pi < func->params.size(); ++pi, ++tpi ) {
			if ( pi >= task.getParamCount() ) {
				//print( LogLevel::warning, "Warning: Missing parameter on function call. Defaulting to {}" );
				print( LogLevel::warning, EngineMessage::MissingFunctionCallParam );
				// Default remaining parameters to empty function.
				stack.getTop().getScope().addVariable(func->params[pi]);
				continue;
			}
			// Match parameters with parameter list items
			stack.getTop().getScope().setVariableFrom(func->params[pi], *tpi, true);
		}
	}
	if ( func->body.raw() == REAL_NULL ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
		print(LogLevel::debug, "[C++ ERROR: Function body-to-be-run is null.");
#endif
		return TaskResult::_error;
	}
	// Run body
	RefPtr<Body> bodyGuard;
	bodyGuard.set(func->body.raw());
	List<Token>::ConstIter bodyIter = func->body.raw()->getIterator();
	unsigned int tokenIdx = 1; // for user-debugging
	if ( bodyIter.has() ) {
		do {
			switch( process(*bodyIter) ) {
			case EngineResult::Error:
				// Task stack and Stack are now invalid, so use previously saved info ONLY
				if ( stackTracePrintingEnabled )
					printFunctionError( funcID, tokenIdx, *bodyIter );
				return TaskResult::_error;
			// Only called via Engine::runFunction()
			case EngineResult::Done:
				//systemExitTrigger = true; // is this needed?
				return TaskResult::_done;
			default: break;
			}
			if ( functionReturnMailbox ) {
				functionReturnMailbox = false;
				break;
			}
			tokenIdx++; // for user-debugging
		} while ( ++bodyIter );
	}
	// This task is still on the task stack, and a number of tasks may not have finished.
	// To handle these, processing is continued by passing a special token (TT_function_end_run)
	// so that tasks can properly clean themselves up.
	const Token endRunToken(TT_function_end_run);
	while ( ! taskStack.getLast().areSameTask(&task) ) {
		switch( process(endRunToken) ) {
		case EngineResult::Error:
			// Task stack and Stack are now invalid, so use previously saved info ONLY
			if ( stackTracePrintingEnabled )
				printFunctionError( funcID, tokenIdx, endRunToken );
			return TaskResult::_error;
		case EngineResult::Done:
			return TaskResult::_done;
		default: break;
		}
	}

	stack.pop();
	return TaskResult::_skip;
}

// Convenience method
EngineResult::Value Engine::runFunction( FunctionContainer* pFunction, const List<Object*>& pParams ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::runFunction");
#endif
	Function* func;
	if ( isNull(pFunction) ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
		print(LogLevel::debug, "[C++ ERROR: Function-container-to-be-run is null.");
#endif
		lastObject.setWithoutRef(new FunctionContainer());
		return EngineResult::Error;
	}
	if ( ! pFunction->getFunction(func) ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
		print(LogLevel::debug, "[C++ ERROR: Function-to-be-run is null.");
#endif
		lastObject.setWithoutRef(new FunctionContainer());
		return EngineResult::Error;
	}
	if ( func->constantReturn ) {
		lastObject.set( func->result.raw() ); // Copy is created when saved to variable
		return EngineResult::Ok;
	}
	// Add temporary stack context
	stack.push();
#ifdef COPPER_ENABLE_CALLBACK_THIS_POINTER
	// Add self-reference to the temporary scope
	stack.getTop().getScope().setVariableFrom(CONSTANT_FUNCTION_SELF, pFunction, true);
#endif
	// Add parameters to temporary scope
	unsigned int pi=0;
	List<Object*>::ConstIter paramsIter = pParams.constStart();
	if ( pParams.size() > 0 ) {
		for (; pi < func->params.size(); ++pi, ++paramsIter ) {
			if ( pi >= pParams.size() ) {
				print( LogLevel::warning, EngineMessage::MissingFunctionCallParam );
				stack.getTop().getScope().addVariable(func->params[pi]);
				continue;
			}
			// Match parameters with parameter list items
			stack.getTop().getScope().setVariableFrom(func->params[pi], *paramsIter, true);
		}
	}
	if ( func->body.raw() == REAL_NULL ) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
		print(LogLevel::debug, "[C++ ERROR: Function body-to-be-run is null.");
#endif
		return EngineResult::Error;
	}
	// For enabling the proper removal of tasks created within this function.
	uint taskStackStartSize = taskStack.size();

	// Run body
	RefPtr<Body> bodyGuard;
	bodyGuard.set(func->body.raw());
	List<Token>::ConstIter bodyIter = func->body.raw()->getIterator();
	unsigned int tokenIdx = 1; // for user-debugging
	if ( bodyIter.has() ) {
		do {
			switch( process(*bodyIter) ) {
			case EngineResult::Error:
				// Task stack and Stack are now invalid, so use previously saved info ONLY
				if ( stackTracePrintingEnabled )
					printFunctionError( 0, tokenIdx, *bodyIter );
				return EngineResult::Error;
			// Only called via Engine::runFunction()
			case EngineResult::Done:
				// systemExitTrigger = true; // might be needed
				return EngineResult::Done;
			default: break;
			}
			if ( functionReturnMailbox ) {
				functionReturnMailbox = false;
				break;
			}
			tokenIdx++; // for user-debugging
		} while ( ++bodyIter );
	}
	// This task is not on the task stack, but it's possible that several of its sub-tasks
	// are and have not been completed, such as variable assignment: { this.a = a }
	// To handle these, processing is continued by passing a special token (TT_function_end_run)
	// so that tasks can properly clean themselves up.
	const Token endRunToken(TT_function_end_run);
	while ( taskStack.size() != taskStackStartSize ) {
		switch( process(endRunToken) ) {
		case EngineResult::Error:
			// Task stack and Stack are now invalid, so use previously saved info ONLY
			if ( stackTracePrintingEnabled )
				printFunctionError( 0, tokenIdx, endRunToken );
			return EngineResult::Error;
		case EngineResult::Done:
			return EngineResult::Done;
		default: break;
		}
	}

	stack.pop();
	return EngineResult::Ok;
}

TaskResult::Value Engine::FuncFound_processRunSysFunction(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FuncFound_processRunSysFunction");
#endif

	switch( task.cSideCallback ) {
	case SystemFunction::_unset:
		print(LogLevel::error, "SYSTEM ERROR: Attempting to run system function without setting it.");
		return TaskResult::_error;
	case SystemFunction::_return:
		process_sys_return(task);
		break;
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
		switch( process_sys_member(task) ) {
		case EngineResult::Error:
			return TaskResult::_error;
		default: break;
		}
		break;
	case SystemFunction::_member_count:
		switch( process_sys_member_count(task) ) {
		case EngineResult::Error:
			return TaskResult::_error;
		default: break;
		}
		break;
	case SystemFunction::_is_member:
		switch( process_sys_is_member(task) ) {
		case EngineResult::Error:
			return TaskResult::_error;
		default: break;
		}
		break;
	case SystemFunction::_set_member:
		switch( process_sys_set_member(task) ) {
		case EngineResult::Error:
			return TaskResult::_error;
		default: break;
		}
		break;
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
		switch( process_sys_assert(task) ) {
		case EngineResult::Error:
			return TaskResult::_error;
		default: break;
		}
		break;
	default:
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
		print(LogLevel::error, "SYSTEM ERROR: Could not find system function to run.");
#endif
		break;
	}
	return TaskResult::_skip;
}

void Engine::process_sys_return(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_return");
#endif
	functionReturnMailbox = true;

	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		lastObject.setWithoutRef(new FunctionContainer()); // Default
	} else {
		lastObject.set(*paramIter);
	}
	Object* obj = lastObject.raw();
	if ( isNull(obj) ) {
		lastObject.setWithoutRef(new FunctionContainer());
	}
}

void Engine::process_sys_not(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_not");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		lastObject.setWithoutRef(new ObjectBool(true));
		return;
	}
	// Only one parameter is used
	lastObject.setWithoutRef(new ObjectBool(! getBoolValue(**paramIter)));
}

void Engine::process_sys_all(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_all");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		lastObject.setWithoutRef(new ObjectBool(false));
		return;
	}
	do {
		if ( ! getBoolValue(**paramIter) ) {
			lastObject.setWithoutRef(new ObjectBool(false));
			return;
		}
	} while ( ++paramIter );
	lastObject.setWithoutRef(new ObjectBool(true));
}

void Engine::process_sys_any(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_any");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		lastObject.setWithoutRef(new ObjectBool(false));
		return;
	}
	do {
		if ( getBoolValue(**paramIter) ) {
			lastObject.setWithoutRef(new ObjectBool(true));
			return;
		}
	} while ( ++paramIter );
	lastObject.setWithoutRef(new ObjectBool(false));
}

void Engine::process_sys_nall(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_nall");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		lastObject.setWithoutRef(new ObjectBool(true));
		return;
	}
	do {
		if ( ! getBoolValue(**paramIter) ) {
			lastObject.setWithoutRef(new ObjectBool(true));
			return;
		}
	} while ( ++paramIter );
	lastObject.setWithoutRef(new ObjectBool(false));
}

void Engine::process_sys_none(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_none");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		lastObject.setWithoutRef(new ObjectBool(true));
		return;
	}
	do {
		if ( getBoolValue(**paramIter) ) {
			lastObject.setWithoutRef(new ObjectBool(false));
			return;
		}
	} while ( ++paramIter );
	lastObject.setWithoutRef(new ObjectBool(true));
}

void Engine::process_sys_are_fn(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_fn");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		lastObject.setWithoutRef(new ObjectBool(false));
		return;
	}
	// Check all parameters
	do {
		if ( ! isObjectFunction(**paramIter) ) {
			lastObject.setWithoutRef(new ObjectBool(false));
			return;
		}
	} while ( ++paramIter );
	lastObject.setWithoutRef(new ObjectBool(true));
}

void Engine::process_sys_are_empty(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_empty");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		lastObject.setWithoutRef(new ObjectBool(false));
		return;
	}
	// Check all parameters
	do {
		if ( ! isObjectEmptyFunction(**paramIter) ) {
			lastObject.setWithoutRef(new ObjectBool(false));
			return;
		}
	} while ( ++paramIter );
	lastObject.setWithoutRef(new ObjectBool(true));
}

void Engine::process_sys_are_same(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_same");
#endif
	// Accepts all parameters and compares their function pointers.
	// If some objects are not functions, the result is false.
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		lastObject.setWithoutRef(new ObjectBool(false));
		return;
	}
	bool out = false;
	FunctionContainer* funcContainer = REAL_NULL;
	do {
		if ( *paramIter == REAL_NULL ) {
			out = false;
			break;
		}
		if( (*paramIter)->getType() == ObjectType::Function ) {
			if ( funcContainer == REAL_NULL ) {
				funcContainer = (FunctionContainer*)(*paramIter);
				continue;
			} // else
			if ( funcContainer == (FunctionContainer*)(*paramIter) ) {
				out = true;
				continue;
			}
		} // else failed continue conditions: not object, not first function, not same container
		out = false;
		break;
	} while ( ++paramIter );
	lastObject.setWithoutRef(new ObjectBool(out));
}

EngineResult::Value	Engine::process_sys_member(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_member");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() || task.getParamCount() != 2 ) {
		print(LogLevel::error, EngineMessage::MemberWrongParamCount);
		return EngineResult::Error;
	}
	// First parameter is the parent function of the member sought
	if ( ! isObjectFunction(**paramIter) ) {
		print(LogLevel::error, EngineMessage::MemberParam1NotFunction);
		return EngineResult::Error;
	}
	FunctionContainer* fc = (FunctionContainer*)(*paramIter);
	Function* parentFunc;
	if ( ! fc->getFunction(parentFunc) ) {
		print(LogLevel::error, EngineMessage::DestroyedFuncAsMemberParam);
		return EngineResult::Error;
	}
	++paramIter;
	// Second parameter is the name of the member
	if ( !isObjectString(**paramIter) ) {
		print(LogLevel::error, EngineMessage::MemberParam2NotString);
		return EngineResult::Error;
	}
	ObjectString* objStr = (ObjectString*)(*paramIter);
	String& rawStr = objStr->getString();
	if ( ! isValidName( rawStr ) ) {
		print(LogLevel::error, EngineMessage::MemberFunctionInvalidNameParam);
		return EngineResult::Error;
	}
	Variable* var;
	parentFunc->getPersistentScope().getVariable( rawStr, var );
	lastObject.set( var->getRawContainer() );
	return EngineResult::Ok;
}

EngineResult::Value Engine::process_sys_member_count(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_member_count");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( task.getParamCount() == 0 ) {
		print(LogLevel::warning, EngineMessage::MemberCountWrongParamCount);
		lastObject.setWithoutRef(new ObjectNumber(String("0")));
		return EngineResult::Error;
	}
	// First parameter is the parent function of the members
	if ( ! isObjectFunction(**paramIter) ) {
		print(LogLevel::error, EngineMessage::MemberCountParam1NotFunction);
		return EngineResult::Error;
	}
	Function* parentFunc;
	FunctionContainer* fc = (FunctionContainer*)(*paramIter);
	if ( ! fc->getFunction(parentFunc) ) {
		print(LogLevel::error, EngineMessage::DestroyedFuncAsMemberCountParam);
		return EngineResult::Error;
	}
	unsigned int size = parentFunc->getPersistentScope().occupancy();
	// The bummer here is that it's cramming an unsigned int into an int.
	// On the bright side, no one should be creating 2.7 billion variables.
	lastObject.setWithoutRef(new ObjectNumber(String(int(size))));
	return EngineResult::Ok;
}

EngineResult::Value Engine::process_sys_is_member(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_is_member");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( task.getParamCount() != 2 ) {
		print(LogLevel::error, EngineMessage::IsMemberWrongParamCount);
		lastObject.setWithoutRef(new ObjectBool(false));
		return EngineResult::Error;
	}
	// First parameter is the parent function of the members
	if ( ! isObjectFunction(**paramIter) ) {
		print(LogLevel::error, EngineMessage::IsMemberParam1NotFunction);
		return EngineResult::Error;
	}
	Function* parentFunc;
	FunctionContainer* fc = (FunctionContainer*)(*paramIter);
	if ( ! fc->getFunction(parentFunc) ) {
		print(LogLevel::error, EngineMessage::DestroyedFuncAsIsMemberParam);
		return EngineResult::Error;
	}
	// Second parameter should be a string
	++paramIter;
	bool result = false;
	if ( ! isObjectString(**paramIter) ) {
		print(LogLevel::error, EngineMessage::IsMemberParam2NotString);
		return EngineResult::Error;
	}
	result = parentFunc->getPersistentScope().variableExists(
						((ObjectString*)*paramIter)->getString()
						);
	lastObject.setWithoutRef(new ObjectBool(result));
	return EngineResult::Ok;
}

EngineResult::Value Engine::process_sys_set_member(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_set_member");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( task.getParamCount() != 3 ) {
		print(LogLevel::error, EngineMessage::SetMemberWrongParamCount);
		lastObject.setWithoutRef(new FunctionContainer());
		return EngineResult::Error;
	}
	// First parameter should be the parent function of the members
	if ( ! isObjectFunction(**paramIter) ) {
		print(LogLevel::error, EngineMessage::SetMemberParam1NotFunction);
		return EngineResult::Error;
	}
	Function* parentFunc;
	FunctionContainer* fc = (FunctionContainer*)(*paramIter);
	if ( ! fc->getFunction(parentFunc) ) {
		print(LogLevel::error, EngineMessage::DestroyedFuncAsSetMemberParam);
		return EngineResult::Error;
	}
	// Second parameter should be a string
	++paramIter;
	if ( ! isObjectString(**paramIter) ) {
		print(LogLevel::error, EngineMessage::SetMemberParam2NotString);
		return EngineResult::Error;
	}
	String& memberName = ((ObjectString*)*paramIter)->getString();
	if ( !isValidName(memberName) ) {
		print(LogLevel::error, EngineMessage::SystemFunctionBadParam);
		return EngineResult::Error;
	}
	// Third parameter can be anything
	++paramIter;
	parentFunc->getPersistentScope().setVariableFrom( memberName, *paramIter, false );
	lastObject.setWithoutRef(new FunctionContainer());
	return EngineResult::Ok;
}

void Engine::process_sys_union(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_union");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		lastObject.setWithoutRef(new FunctionContainer());
		return;
	}
	FunctionContainer* finalFC = new FunctionContainer();
	Function* finalFunc;
	finalFC->getFunction(finalFunc); // Guaranteed
	FunctionContainer* usableFC;
	Function* usableFunc;
	do {
		if ( isObjectFunction(**paramIter) ) {
			usableFC = (FunctionContainer*)(*paramIter);
			if ( usableFC->getFunction(usableFunc) ) {
				finalFunc->getPersistentScope().copyMembersFrom( usableFunc->getPersistentScope() );
			} else {
				//print(LogLevel::warning, "Destroyed function passed to \"union\" function.");
				print(LogLevel::warning, EngineMessage::DestroyedFuncAsUnionParam);
			}
		} else {
			print(LogLevel::warning, EngineMessage::NonFunctionAsUnionParam);
		}
	} while( ++paramIter );
	lastObject.set(finalFC);
}

void Engine::process_sys_type(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_type");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		lastObject.setWithoutRef(new FunctionContainer());
		return;
	}
	// Get only the first parameter
	lastObject.setWithoutRef(new ObjectString( (*paramIter)->typeName() ));
}

void Engine::process_sys_are_same_type(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_same_type");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		lastObject.setWithoutRef(new FunctionContainer());
		return;
	}
	// First parameter, to which all other parameters are compared
	String first( (*paramIter)->typeName() );
	bool same = true;
	do {
		if ( ! first.equals( (*paramIter)->typeName() ) ) {
			same = false;
			break;
		}
	} while( ++paramIter );
	// Get only the first parameter
	lastObject.setWithoutRef(new ObjectBool(same));
}

void Engine::process_sys_are_bool(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_bool");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		lastObject.setWithoutRef(new ObjectBool(false));
		return;
	}
	// Check all parameters
	do {
		if ( ! isObjectBool(**paramIter) ) {
			lastObject.setWithoutRef(new ObjectBool(false));
			return;
		}
	} while ( ++paramIter );
	lastObject.setWithoutRef(new ObjectBool(true));
}

void Engine::process_sys_are_string(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_string");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		lastObject.setWithoutRef(new ObjectBool(false));
		return;
	}
	// Check all parameters
	do {
		if ( ! isObjectString(**paramIter) ) {
			lastObject.setWithoutRef(new ObjectBool(false));
			return;
		}
	} while ( ++paramIter );
	lastObject.setWithoutRef(new ObjectBool(true));
}

void Engine::process_sys_are_number(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_are_number");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		lastObject.setWithoutRef(new ObjectBool(false));
		return;
	}
	// Check all parameters
	do {
		if ( ! isObjectNumber(**paramIter) ) {
			lastObject.setWithoutRef(new ObjectBool(false));
			return;
		}
	} while ( ++paramIter );
	lastObject.setWithoutRef(new ObjectBool(true));
}

EngineResult::Value Engine::process_sys_assert(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::process_sys_assert");
#endif
	List<Object*>::Iter paramIter = task.createParamsIterator();
	if ( !paramIter.has() ) {
		return EngineResult::Ok;
	}
	do {
		if ( getBoolValue(**paramIter) ) {
			lastObject.setWithoutRef(new ObjectBool(true));
		} else {
			print(LogLevel::error, EngineMessage::AssertionFailed);
			return EngineResult::Error;
		}
	} while ( ++paramIter );
	lastObject.setWithoutRef(new ObjectBool(true));
	return EngineResult::Ok;
}

TaskResult::Value Engine::FunctionFound_processAssignmentObj(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FunctionFound_processAssignmentObj");
#endif
	Object* obj = REAL_NULL;
	Variable* var = REAL_NULL;
	if ( ! lastObject.obtain(obj) ) {
		//print( LogLevel::error, "ERROR: Failed to obtain an object when assigning." );
		print( LogLevel::error, EngineMessage::UnsetLastObjectInAsgn );
		return TaskResult::_error;
	}

	// Note: The last object MUST be set.
	// This is the user variable, unless a system or external function is used,
	// in which case, the last object is set to a new function.
	switch(task.type) {
	case TASK_FFTYPE_variable:
/*
		switch( obj->getType() ) {
		case ObjectType::Function:
			task.varPtr.raw()->setFunc((FunctionContainer*)obj, false);
			break;
		case ObjectType::Data:
			task.varPtr.raw()->setFuncReturn( obj );
			break;
		}
		lastObject.set(task.varPtr.raw()->getRawContainer());
*/
		switch( obj->getType() ) {
		case ObjectType::Function:
			if ( task.varPtr.obtain(var) ) {
				var->setFunc((FunctionContainer*)obj, false);
				lastObject.set(var->getRawContainer());
			} else {
				print(LogLevel::error, EngineMessage::TaskVarIsNull);
			}
			break;
		case ObjectType::Data:
			if ( task.varPtr.obtain(var) ) {
				var->setFuncReturn( obj );
				lastObject.set(var->getRawContainer());
			} else {
				print(LogLevel::error, EngineMessage::TaskVarIsNull);
			}
			break;
		}
		//lastObject.set(task.varPtr.raw()->getRawContainer());
		return TaskResult::_pop_loop;

	case TASK_FFTYPE_cside:
		//print( LogLevel::warning, "Warning: Cannot save over built-in function." );
		print( LogLevel::warning, EngineMessage::AttemptToOverrideBuiltinFunc );
		lastObject.setWithoutRef(new FunctionContainer());
		return TaskResult::_pop_loop;

	case TASK_FFTYPE_ext_cside:
		//print( LogLevel::warning, "Warning: Cannot save over external function." );
		print( LogLevel::warning, EngineMessage::AttemptToOverrideExternalFunc );
		lastObject.setWithoutRef(new FunctionContainer());
		return TaskResult::_pop_loop;

	default:
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
		print(LogLevel::error, "SYSTEM ERROR: Default case reached in FunctionFound_processAssignmentObj");
#endif
		return TaskResult::_error;
	}
}

TaskResult::Value Engine::FunctionFound_processPointerAssignmentObj(TaskFunctionFound& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::FunctionFound_processPointerAssignmentObj");
#endif
	Object* obj;
	if ( ! lastObject.obtain(obj) ) {
		//print( LogLevel::error, "ERROR: Failed to obtain an object when assigning to a pointer." );
		print( LogLevel::error, EngineMessage::UnsetLastObjectInPtrAsgn );
		return TaskResult::_error;
	}
	// Make sure the last object is a function
	if ( obj->getType() != ObjectType::Function ) {
		// Maybe this should merely be a warning and the system creates a wrapper function.
		//print( LogLevel::error, "ERROR: Can only save pointer to functions, not data." );
		print( LogLevel::error, EngineMessage::DataAssignedToPtr );
		return TaskResult::_error;
	}
	// Note: The last object MUST be set.
	// This is the user variable, unless a system or external function is used,
	// in which case, the last object is set to a new function.
	switch(task.type) {
	case TASK_FFTYPE_variable:
		task.varPtr.raw()->setFunc((FunctionContainer*)obj, true);
		lastObject.set(obj);
		return TaskResult::_pop_loop;

	case TASK_FFTYPE_cside:
		//print( LogLevel::warning, "Warning: Cannot save pointer over built-in function." );
		print( LogLevel::warning, EngineMessage::AttemptToOverrideBuiltinFunc );
		lastObject.setWithoutRef(new FunctionContainer());
		return TaskResult::_pop_loop;

	case TASK_FFTYPE_ext_cside:
		//print( LogLevel::warning, "Warning: Cannot save pointer over external function." );
		print( LogLevel::warning, EngineMessage::AttemptToOverrideExternalFunc );
		lastObject.setWithoutRef(new FunctionContainer());
		return TaskResult::_pop_loop;

	default:
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
		print(LogLevel::error, "SYSTEM ERROR: Default case reached in FunctionFound_processPointerAssignmentObj");
#endif
		return TaskResult::_error;
	}
}

TaskResult::Value Engine::processIfStatement(TaskIfStructure& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::processIfStatement");
#endif
	switch( task.state ) {
	case TASK_IF_find_cond: // has "if", search for '('
		return ifStatement_findCondition(task, lastToken);

	case TASK_IF_get_cond: // has '(', search for object, hoping for bool
		return ifStatement_getCondition(task, lastToken);

	case TASK_IF_seek_cond_end: // has object, search for ')'
		if ( lastToken.type == TT_parambody_close ) {
			task.openParamBodies -= 1;
			task.state = TASK_IF_seek_body;
		}
		return TaskResult::_cycle;

	case TASK_IF_seek_body: // has ')', search for '{'
		// After seeking the body, check if the body should process, else seek body end
		return ifStatement_seekBody(task, lastToken);

	case TASK_IF_run_body: // has '{', run til '}'
		if ( functionReturnMailbox )
			return TaskResult::_cycle;
		if ( lastToken.type == TT_body_close ) {
			task.openBodies -= 1;
			if ( task.openBodies == 0 ) {
				task.state = TASK_IF_seek_else;
				return TaskResult::_cycle;
			}
		}
		return TaskResult::_none;

	case TASK_IF_seek_body_end: // search for '}'
		return ifStatement_seekBodyEnd(task, lastToken);

	case TASK_IF_seek_else: // search for "else" or "elif"
		return ifStatement_seekElse(task, lastToken);

	default:
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
		print(LogLevel::error, "SYSTEM ERROR: Default case reached in processIfStatement");
#endif
		return TaskResult::_error;
	}
}

TaskResult::Value Engine::ifStatement_findCondition(TaskIfStructure& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ifStatement_findCondition");
#endif
	if ( lastToken.type == TT_parambody_open ) {
		if ( task.ran == false ) {
			task.state = TASK_IF_get_cond;
		} else {
			task.state = TASK_IF_seek_cond_end;
		}
		task.openParamBodies += 1;
		return TaskResult::_cycle;
	} else {
		//print(LogLevel::error, "ERROR: Invalid token after \"if\". Expected condition body.");
		print(LogLevel::error, EngineMessage::InvalidTokenAtIfStart);
		return TaskResult::_error;
	}
}

TaskResult::Value Engine::ifStatement_getCondition(TaskIfStructure& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ifStatement_getCondition");
#endif
	if ( lastToken.type == TT_parambody_open ) {
		task.openParamBodies += 1;
		return TaskResult::_none;
	}
	// Condition relies on getting an object, but user may short-circuit by immediately ending param body.
	if ( lastToken.type == TT_parambody_close ) {
		task.openParamBodies -= 1;
		if ( task.openParamBodies == 0 ) {
			if ( task.isConditionSet ) {
				task.state = TASK_IF_seek_body;
			} else {
				//print(LogLevel::warning, "Warning: No condition passed to if-structure. Default is false.");
				print(LogLevel::warning, EngineMessage::ConditionlessIf);
				task.condition = false;
				task.isConditionSet = true; // We have to set it here unfortunately
				task.state = TASK_IF_seek_body;
			}
			return TaskResult::_cycle;
		}
	}
	return TaskResult::_none;
}

TaskResult::Value Engine::ifStatement_seekBody(TaskIfStructure& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ifStatement_seekBody");
#endif
	if ( lastToken.type == TT_body_open ) {
		task.openBodies += 1;
		if ( task.condition && task.ran == false ) {
			task.state = TASK_IF_run_body;
			task.ran = true;
		} else {
			task.state = TASK_IF_seek_body_end;
		}
		return TaskResult::_cycle;
	}
	//print(LogLevel::error, "ERROR: Illegal token found while seeking if-structure body opening token.");
	print(LogLevel::error, EngineMessage::InvalidTokenBeforeIfBody);
	return TaskResult::_error;
}

TaskResult::Value Engine::ifStatement_seekBodyEnd(TaskIfStructure& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ifStatement_seekBodyEnd");
#endif
	// check for both { and }
	switch( lastToken.type ) {
	case TT_body_open:
		task.openBodies += 1;
		break;

	case TT_body_close:
		task.openBodies -= 1;
		if ( task.openBodies == 0 ) {
			task.state = TASK_IF_seek_else;
		}
		break;

	default: break;
	}
	return TaskResult::_cycle; // Absorb token
}

TaskResult::Value Engine::ifStatement_seekElse(TaskIfStructure& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::ifStatement_seekElse");
#endif
		// Note: This will be used whether the else/elif body is run or not
		// When encountering this, reset the condition and isConditionSet tokens
		// based on whether this is an "elif" or an "else".
		// Also, "else" should set the task.condition to true so the body will be run.
		// If neither "elif" nor "else" is encountered, pop.

	// Next token must be "elif" or "else". If not, pop.
	if ( lastToken.type == TT_elif ) {
		task.state = TASK_IF_find_cond; // Skipping the condition is handled in the find-condition part
	} else if ( lastToken.type == TT_else ) {
		task.state = TASK_IF_seek_body;
		task.condition = ! task.condition;
	} else {
		//taskStack.pop();
		//return performObjProcessAndCycle(lastToken);
		/*
		switch( process(lastToken) ) {
		case EngineResult::Error:
			return TaskResult::_error;
		case EngineResult::Done:
			return TaskResult::_done;
		default: break;
		}*/
		return TaskResult::_pop_loop;
	}
	return TaskResult::_cycle;
}

TaskResult::Value Engine::processObjForIfStatement(TaskIfStructure& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::processObjForIfStatement");
#endif
	if ( task.state == TASK_IF_get_cond ) {
		// Gets the first object value and considers it the condition.
		task.condition = getBoolValue( *(lastObject.raw()) );
		task.isConditionSet = true;
		task.state = TASK_IF_seek_cond_end;
	}
	// Should if-structures set the last object before popping?
	// Maybe not, as it may allow for inline if-statements such as:
	// a = if( gt(b() 10) ){ 4 }else{ 1 }
	// Could also allow problems.

	if ( functionReturnMailbox ) {
		taskStack.pop();
	}
	return TaskResult::_none;
}

TaskResult::Value Engine::processLoop(TaskLoopStructure& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::processLoop");
#endif
	switch( task.state ) {
	case TASK_LOOP_find_body:
		if ( lastToken.type == TT_body_open ) {
			task.openBodies = 1;
			task.state = TASK_LOOP_collect_body;
		}
		return TaskResult::_cycle;

	case TASK_LOOP_collect_body:
		if ( lastToken.type == TT_body_open ) {
			task.openBodies += 1;
		}
		else if ( lastToken.type == TT_body_close ) {
			task.openBodies -= 1;
			if ( task.openBodies == 0 ) {
				task.state = TASK_LOOP_run;
				return loopStructure_runBody(task);
			}
		}
		task.body.addToken(lastToken);
		return TaskResult::_cycle;

	case TASK_LOOP_run:
		return TaskResult::_none;

	default:
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
		print(LogLevel::error, "SYSTEM ERROR: Default case reached in Engine::processLoop");
#endif
		return TaskResult::_error;
	}
}

TaskResult::Value Engine::loopStructure_runBody(TaskLoopStructure& task) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::loopStructure_runBody");
#endif
	List<Token>::ConstIter tokenIter = task.body.getIterator();
	loopEndMailbox = false;
#ifdef COPPER_DEBUG_LOOP_STRUCTURE
int debug_max = 0;
#endif
	if ( tokenIter.has() )
	do {
		// "stop"/"endloop" and "skip" are handled by setting mailboxes (global booleans)
		if ( process( *tokenIter ) == EngineResult::Error ) {
			return TaskResult::_error;
		}
		if ( functionReturnMailbox ) {
			break;
		}
		// If at the end, restart
		if ( ! ++tokenIter || loopSkipMailbox ) {
			tokenIter.reset();
			loopSkipMailbox = false;
		}
#ifdef COPPER_DEBUG_LOOP_STRUCTURE
		debug_max++;
		if ( debug_max == 100 ) break;
#endif
	} while( !loopEndMailbox );
	loopEndMailbox = false; // Prevents cascading loop-end effect on outer loops
	loopSkipMailbox = false; // Prevents cascading loop-skip effect on outer loops
	taskStack.pop();
	return TaskResult::_cycle;
}

TaskResult::Value Engine::processNamed(TaskProcessNamed& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::processNamed");
#endif
	// The lastObject is, at best, only a function, not the name of the variable it came from (if any).
	// Therefore, to find the variable, search the scope for the given name.
	// At worst, the lastObject is some valid unhandled token.

	switch( task.state ) {
	case TASK_PROC_NAMED_find_opening:
		if ( lastToken.type == TT_parambody_open ) {
			task.state = TASK_PROC_NAMED_param_name;
			return TaskResult::_cycle;
		}
		print(LogLevel::error, EngineMessage::InvalidTokenBeforePtrHandlerParamBody);
		return TaskResult::_error;

	case TASK_PROC_NAMED_param_name:
		if ( lastToken.type == TT_name ) {
			// Get the variable. Should report it if the variable does not exist but create the variable anyways.
			// Needs to check both global and local scopes.
			processNamed_collectVariable(task, lastToken);
			task.state = TASK_PROC_NAMED_find_closing;
			return TaskResult::_cycle;
		}
		print(LogLevel::error, EngineMessage::NonNameFoundInPtrHandlerParamBody);
		return TaskResult::_error;

	// Note: This actually handles several cases:
	// 1) If the closing is found
	// 2) If the member-link token is found, which takes it to find_child_member
	case TASK_PROC_NAMED_find_closing:
		if ( lastToken.type == TT_member_link ) {
			task.state = TASK_PROC_NAMED_find_member;
			return TaskResult::_cycle;
		} // else
		if ( lastToken.type == TT_parambody_close ) {
			// This point should never be reached without a variable, so calling raw() is safe
			switch( task.name ) {
			case TaskName::_own:
				task.varPtr.raw()->getRawContainer()->changeOwnerTo(task.varPtr.raw());
				lastObject.setWithoutRef(new FunctionContainer());
				break;

			case TaskName::_is_ptr:
				lastObject.setWithoutRef(new ObjectBool( task.varPtr.raw()->isPointer() ));
				break;

			case TaskName::_is_owner:
				lastObject.setWithoutRef(new ObjectBool( ! task.varPtr.raw()->isPointer() ));
				break;

			default: break;
			}
			// If there is an error here, it's internal.
			taskStack.pop();
			return TaskResult::_skip;
		}
		print(LogLevel::error, EngineMessage::InvalidTokenForEndingPtrHandlerParamBody);
		return TaskResult::_error;

	case TASK_PROC_NAMED_find_member:
		// There should be a pointer to the parent scope saved in the task.
		if ( lastToken.type == TT_name ) {
			// Get the variable. Should report it if the variable does not exist but create the variable anyways.
			// Needs to check both global and local scopes.
			processNamed_collectMember(task, lastToken);
			task.state = TASK_PROC_NAMED_find_closing;
			return TaskResult::_cycle;
		}
		print(LogLevel::error, EngineMessage::NonNameFoundInPtrHandlerParamBody);
		return TaskResult::_error;

	default:
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
		print(LogLevel::debug, "[SYSTEM ERROR: default case reached in Engine::processNamed.");
#endif
		return TaskResult::_error;
	}
}

void Engine::processNamed_collectVariable(TaskProcessNamed& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::processNamed_collectVariable");
#endif
	Variable* var = REAL_NULL;
	if ( stack.getTop().getScope().findVariable(lastToken.name, var) ) {
		task.varPtr.set(var);
	}
		// bottom stack frame != top stack frame
	else if ( stack.getCurrLevel() > 1 && stack.getBottom().getScope().findVariable(lastToken.name, var) ) {
		task.varPtr.set(var);
	} else {
		print(LogLevel::warning, EngineMessage::NonexistentVariablePassedToPtrHandler);
		// Using new(no throw) for creating variables is not helpful. It should throw and catch elsewhere.
		var = stack.getTop().getScope().addVariable(lastToken.name);
		task.varPtr.set(var);
	}
}

void Engine::processNamed_collectMember(TaskProcessNamed& task, const Token& lastToken) {
#ifdef COPPER_DEBUG_ENGINE_MESSAGES
	print(LogLevel::debug, "[DEBUG: Engine::processNamed_collectMember");
#endif
	Variable* var = REAL_NULL;
	Variable* member = REAL_NULL;
	Function* f = REAL_NULL;
	if ( task.varPtr.obtain(var) ) {
		f = var->getFunction(logger);
		if ( f->getPersistentScope().findVariable(lastToken.name, member) ) {
			task.varPtr.set(member);
		} else {
			print(LogLevel::warning, EngineMessage::NonexistentVariablePassedToPtrHandler);
			f->getPersistentScope().getVariable(lastToken.name, member);
			task.varPtr.set(member);
		}
	}
}

}
