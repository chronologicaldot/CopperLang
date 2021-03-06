// Copyright 2015 Nicolaus Anderson
// Note: Some of the function names in here have been changed from the original.

#ifndef UTILITY_LIST
#define UTILITY_LIST

namespace util {

class NodeCopyException {};
class IndexOutOfBoundsException {};
class NullListNodeException {};
class BadIteratorException {}; // Node could be null
class BadListInitException {};

// For debug
class InvalidListException {};

typedef unsigned long	uint;

template<typename T>
class List
{
public:
	class Node
	{
		friend List;
		Node* prev;
		Node* next;
		T item;
	public:
		Node( const T& pItem )
			: prev(0)
			, next(0)
			, item( pItem )
		{}

		Node( const Node& pOther )
		{
			throw NodeCopyException();
		}

		T& getItem()
		{
			return item;
		}

		const T& getConstItem() const
		{
			return item;
		}

		void setItem( const T& pItem )
		{
			item = pItem;
		}

		void insertBefore( const T& pItem )
		{
			Node* n = prev;
			prev = new Node(pItem);
			prev->next = this;
			if ( n ) {
				n->next = prev;
				prev->prev = n;
			}
		}

		void insertAfter( const T& pItem )
		{
			Node* n = next;
			next = new Node(pItem);
			next->prev = this;
			if ( n ) {
				n->prev = next;
				next->next = n;
			}
		}

		void delink()
		{
			if ( prev )
			{
				if ( next )
				{
					prev->next = next;
					next->prev = prev;
				} else {
					prev->next = 0;
				}
			} else if ( next )
			{
				next->prev = 0;
			}
			prev = 0;
			next = 0;
		}

		void destroy()
		{
			delink();
			delete this;
		}
	};

	class Iter
	{
		friend List;
		List* list;
		Node* node;
	public:
		Iter( List& pList )
			: list( &pList )
			, node( pList.head )
		{}

		Iter( const Iter& pOther )
			: list( pOther.list )
			, node( pOther.node )
		{}

		//T& curr()	// current node
		T& operator*()
		{
			return node->getItem();
		}

		// For explicitness
		T& getItem() {
			return node->getItem();
		}

		T* operator->()
		{
			return &node->getItem();
		}

		void setItem( T pItem )
		{
			if ( node )
				node->setItem( pItem );
		}

		bool operator==( const Iter& pOther ) const
		{
			return (list == pOther.list) && (node == pOther.node);
		}

		Iter& operator=( Iter pOther )
		{
			list = pOther.list;
			node = pOther.node;
			return *this;
		}

		// REQUIRED
		void set( Iter pOther )
		{
			list = pOther.list;
			node = pOther.node;
		}

		bool prev()	// previous node
		//bool operator--() // <- was this
		{
			//if ( start() ) return false;
			if ( !node || !(node->prev) ) return false;
			node = node->prev;
			return true; // there may be more nodes
		}

		bool next()	// next node
		//bool operator++() // <- was this
		{
			if ( !node || !(node->next) ) return false;
			node = node->next;
			return true; // there may be more nodes
		}
	
		T& peek() // UNSAFE
		{
			return node->next->getItem();
		}

		void reset()
		{
			node = list->head;
		}

		void makeLast()
		{
			node = list->tail;
		}

		bool atStart() const
		{
			if ( ! list->has() ) return true;
			return node == list->head;
		}

		bool atEnd() const
		{
			if ( ! list->has() ) return true;
			return node == list->tail;
		}

		bool has() const
		{
			return list->has();
		}

		void insertBefore( const T& item )
		{
			if ( ! list->has() )
			{
				list->push_back( item );
				node = list->head;
				return;
			}
			Node* n = node->prev;
			node->prev = new Node( item );
			node->prev->next = node;
			// Link to pre-previous node
			if ( n )
			{
				n->next = node->prev;
				node->prev->prev = n;
			}
			if ( node == list->head )
			{
				list->head = node->prev;
			}
			list->count++;
		}

		void insertAfter( const T& item )
		{
			if ( ! list->has() )
			{
				list->push_back( item );
				node = list->head;
				return;
			}
			Node* n = node->next;
			node->next = new Node( item );
			node->next->prev = node;
			// Link to previous next node
			if ( n )
			{
				node->next->next = n;
				n->prev = node->next;
			}
			if ( node == list->tail )
			{
				list->tail = node->next;
			}
			list->count++;
		}

		// Invalidates the node
		void destroy() {
			if ( node == list->head )
				list->head = node->next;
			if ( node == list->tail )
				list->tail = node->prev;
			node->destroy();
			node = 0;
		}
	};

	class ConstIter
	{
		friend List;
		const List* list;
		const Node* node;
	public:
		// Bad setup? I don't know how to save a const list
		ConstIter( const List& pList )
			: list( &pList )
			, node( pList.head )
		{}

		ConstIter( const ConstIter& pOther )
			: list( pOther.list )
			, node( pOther.node )
		{}

		const T& operator*() const
		{
			return node->getConstItem();
		}

		// For explicitness
		const T& getItem() {
			return node->getConstItem();
		}

		ConstIter& operator=( ConstIter pOther )
		{
			list = pOther.list;
			node = pOther.node;
			return *this;
		}

		bool operator==( const Iter& pOther ) const
		{
			return (list == pOther.list) && (node == pOther.node);
		}

		bool prev()
		//bool operator--() // <- was
		{
			if ( !node || !(node->prev) ) return false;
			node = node->prev;
			return true; // there may be more nodes
		}

		bool next()
		//bool operator++() // <- was
		{
			if ( !node || !(node->next) ) return false;
			node = node->next;
			return true; // there may be more nodes
		}

		void reset()
		{
			node = list->head;
		}

		void makeLast()
		{
			node = list->tail;
		}

		bool atStart() const
		{
			if ( ! list->has() ) return true;
			return node == list->head;
		}

		bool atEnd() const
		{
			if ( ! list->has() ) return true;
			return node == list->tail;
		}

		bool has() const
		{
			return list->has();
		}
	};

protected:
	Node* head;
	Node* tail;
	uint count;

public:
	friend Iter;

	List()
		: head(0)
		, tail(0)
		, count(0)
	{}

	List( const List& pOther )
		: head(0)
		, tail(0)
		, count(0)
	{
		ConstIter other( pOther );
		if ( other.has() )
		do {
			push_back( *other );
		} while ( other.next() );
	}

	~List()
	{
		clear();
	}

	//! Optimized index search
	T& operator[] ( uint pIndex )
	{
		// should throw exceptions when no items
		Node* n = head;
		uint i=0;
		if ( pIndex >= count )
			throw IndexOutOfBoundsException();
		if ( pIndex > count / 2 ) // index is over halfway, so start from back instead
		{
			n = tail;
			i = count - 1;
			for ( ; i > pIndex && i > 0; i-- )
			{
				n = n->prev;
			}
		} else {
			for ( ; i < pIndex && i < count; i++ )
			{
				n = n->next;
			}
		}
		return n->getItem();
	}

	//! Optimized index search
	const T& operator[] ( uint pIndex ) const
	{
		// should throw exceptions when no items
		const Node* n = head;
		uint i=0;
		if ( pIndex >= count )
			throw IndexOutOfBoundsException();
		if ( pIndex > count / 2 ) // index is over halfway, so start from back instead
		{
			n = tail;
			i = count - 1;
			for ( ; i > pIndex && i > 0; i-- )
			{
				n = n->prev;
			}
		} else {
			for ( ; i < pIndex && i < count; i++ )
			{
				n = n->next;
			}
		}
		return n->getConstItem();
	}

	T& getFromBack( uint pReverseIndex )
	{
		// should throw exceptions when no items
		Node* n = tail;
		uint i=0;
		if ( pReverseIndex >= count )
			throw IndexOutOfBoundsException();
		if ( !tail )
			throw NullListNodeException();
		uint index = count - 1 - (pReverseIndex >= count? count-1 : pReverseIndex);
		i = count - 1;
		for ( ; i > index && i > 0; i-- )
		{
			n = n->prev;
		}
		return n->getItem();
	}

	T& getFirst() {
		if ( !head )
			throw NullListNodeException();
		return head->getItem();
	}

	T& getLast() {
		if ( !tail )
			throw NullListNodeException();
		return tail->getItem();
	}

	const T& getConstFirst() const {
		if ( !head )
			throw NullListNodeException();
		return head->getConstItem();
	}

	List& operator= ( const List& pOther )
	{
		clear();
		ConstIter i = pOther.constStart();
		if ( i.has() )
		do {
			push_back(*i);
		} while ( i.next() );
		return *this;
	}

	uint size() const
	{
		return count;
	}

	bool has() const
	{
		return count >= 1;
	}

	void clear()
	{
		if ( count == 0 ) return;

		Node* n;
		while ( tail )
		{
			n = tail->prev;
			delete tail;
			tail = n;
		}
		head = 0; // tail should be 0 already
		count = 0;
	}

	void remove( const uint pIndex )
	{
		if ( count == 0 ) return;
		Node* n = head;
		Node* n2 = 0;
		for ( uint i = 0; i < pIndex; i++ )
		{
			if ( i == count - 1 )
				return;
			n = n->next;
		}
		if ( n == head )
		{
			if ( head != tail )
			{
				head = head->next;
			} else {
				head = 0;
				tail = 0;
			}
		}
		n->destroy();
		count--;
	}

	// Removes all nodes PRIOR to the given iterator
	void removeUpTo( const Iter& stopIter )
	{
		if ( stopIter.list != this )
			throw BadIteratorException();

		while ( stopIter.node != head )
			pop_front();
	}

	void push_back( const T& pItem )
	{
		if ( !tail )
		{
			if ( head )
				throw BadListInitException();
			tail = new Node( pItem );
			head = tail;
		} else {
			tail->insertAfter( pItem );
			tail = tail->next;
		}
		count++;
	}

	void push_front( const T& pItem )
	{
		if ( !head )
		{
			if ( tail )
				throw BadListInitException();
			head = new Node( pItem );
			tail = head;
		} else {
			head->insertBefore( pItem );
			head = head->prev;
		}
		count++;
	}

	void pop()
	{
		if ( count == 0 )
			return;
		Node* n = tail->prev;
		if ( head == tail )
			head = n;
		tail->destroy();
		tail = n; // Could be 0
		count--;
	}

	void pop_front()
	{
		if ( count == 0 )
			return;
		Node* n = head->next;
		if ( head == tail )
			tail = n;
		head->destroy();
		head = n; // Could be 0
		count--;
	}

	void append( const List& pOther )
	{
		ConstIter other( pOther );
		if ( !other.has() ) return;
		do {
			push_back( *other );
		} while ( other.next() );
	}

	Iter start()
	{
		return Iter( *this );
	}

	ConstIter constStart() const
	{
		return ConstIter( *this );
	}

	Iter end() {
		Iter i(*this);
		i.makeLast();
		return i;
	}

	// Primarily for debug stack-trace
	uint indexOf( const Iter& pIter )
	{
		if ( ! pIter.list->has() )
			return 0;
		if ( pIter.list != this )
			throw BadIteratorException();
		uint i=0;
		ConstIter it = constStart();
		if ( it.has() )
		do {
			if ( it == pIter )
				return i;
			++i;
		} while ( it.next() );
		throw BadIteratorException();
		//return count-1;
	}

	void validate() {
		if ( !head && !tail )
			return;

		if ( head && !tail )
			throw InvalidListException();
		if ( tail && !head )
			throw InvalidListException();

		if ( head->prev )
			throw InvalidListException();
		if ( tail->next )
			throw InvalidListException();

		if ( head == tail ) {
			return;
		}

		Node* n = head;
		while ( n != tail ) {
			if ( !( n->next ) )
				throw InvalidListException();
			n = n->next;
		}
		n = tail;
		while ( n != head ) {
			if ( !( n->prev ) )
				throw InvalidListException();
			n = n->prev;
		}
	}

};

}

#endif
