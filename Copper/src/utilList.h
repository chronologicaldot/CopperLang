// Copyright 2015-2016 Nicolaus Anderson
// Note: Some of the function names in here have been changed from the original.

#ifndef UTILITY_LIST
#define UTILITY_LIST

namespace util {

class IndexOutOfBoundsException {};
class NullListNodeException {};

typedef unsigned int	uint;

template<class T>
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
		}

		void destroy()
		{
			delink();
			delete this;
		}

		void removeBefore()
		{
			if ( !prev ) return;
			Node* n;
			if ( prev->prev ) {
				n = prev->prev;
				n->next = this;
				delete prev;
				prev = n;
				return;
			} // else
			delete prev;
			prev = 0;
		}

		void removeAfter()
		{
			if ( !next ) return;
			Node* n;
			if ( next->next ) {
				n = next->next;
				n->prev = this;
				delete next;
				next = n;
				return;
			} // else
			delete next;
			next = 0;
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

		T* operator->()
		{
			return &node->getItem();
		}

		void setItem( T pItem )
		{
			if ( node )
				node->setItem( pItem );
		}

		Iter& operator=( Iter pOther )
		{
			list = pOther.list;
			node = pOther.node;
			return *this;
		}

		//bool prev()	// previous node
		bool operator--()
		{
			//if ( start() ) return false;
			if ( !node || !(node->prev) ) return false;
			node = node->prev;
			return true; // there may be more nodes
		}

		//bool next()	// next node
		bool operator++()
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

		//bool start() const	// at the start.
		bool atStart() const // should be named this
		{
			if ( ! list->has() ) return true;
			return node == list->head;
		}

		//bool end() const	// at the end
		bool atEnd() const // should be named this
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
			list->count++;
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

		ConstIter& operator=( ConstIter pOther )
		{
			list = pOther.list;
			node = pOther.node;
			return *this;
		}

		bool operator--()
		{
			if ( !node || !(node->prev) ) return false;
			node = node->prev;
			return true; // there may be more nodes
		}

		bool operator++()
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

		//bool start() const	// at the start
		bool atStart() const // should be named this
		{
			if ( ! list->has() ) return true;
			return node == list->head;
		}

		//bool end() const	// at the end
		bool atEnd() const // should be named this
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

	// May not be needed
/*
	List( List& pOther )
	{
		Iter other( pOther );
		if ( other.has() )
		do {
			push_back( *other );
		} while ( ++other );
	}
*/

	List( const List& pOther )
	{
		ConstIter other( pOther );
		if ( other.has() )
		do {
			push_back( *other );
		} while ( ++other );
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

	// Should be reduced to merely operator() (uint) and always go from the back.
	// This function may be removed soon.
/*
	T& operator() ( uint pIndex, bool pFromBack )
	{
		// should throw exceptions when no items
		Node* n;
		uint i=0;
		if ( pFromBack )
		{
			n = tail;
			i = count - 1;
			for ( ; i > pIndex && i > 0; i-- )
			{
				n = n->prev;
			}
		} else {
			n = head;
			for ( ; i < pIndex && i < count; i++ )
			{
				n = n->next;
			}
		}
		return n->getItem();
	}
*/

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

	List& operator= ( const List& pOther )
	{
		clear();
		ConstIter i = pOther.constStart();
		if ( i.has() )
		do {
			push_back(*i);
		} while ( ++i );
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

	void remove( Iter& pIter )
	{
		if ( pIter->node == head )
		{
			if ( head != tail )
			{
				head = head->next;
			} else {
				head = 0;
				tail = 0;
			}
		} else if ( pIter->node == tail )
		{
			if ( head != tail )
			{
				tail = tail->prev;
			} else {
				head = 0;
				tail = 0;
			}
		}
		if ( ! pIter->node )
			throw NullListNodeException();
		pIter->node->destroy();
		count--;
	}
/*
	// This ends up removing all copies of the same item (because the iterator resets),
	// which may or may not be desired activity.
	// Besides, this may not even be needed.
	void remove( const List<T>& pOther )
	{
		Iter me(*this);
		ConstIter other(pOther);
		if ( other.has() )
		do {
			if ( !me.has() ) break;
			me.reset();
			do {
				if ( *me == *other )
				{
					remove(me); break;
				}
			} while ( ++me );
		} while ( ++other );
	}
*/
	void push_back( const T& pItem )
	{
		if ( !head )
		{
			head = new Node( pItem );
			tail = head;
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
			head = new Node( pItem );
			tail = head;
		} else {
			head->insertBefore( pItem );
			head = head->prev;
		}
		count++;
	}

	// This method may be removed if found incompatible with types that need to use the list.
	// Segfaults when the list is clear unless I create and uncomment the EmptyListException.
	/*T pop()
	{
		// should throw exceptions when no items
		//if ( count == 0 ) throw new EmptyListException();
		T item = tail->getItem();
		Node* n = tail->prev;
		if ( head == tail )
			head = 0;
		tail->destroy();
		if ( n )
			tail = n;
		else
			tail = 0;
		count--;
		return item;
	}*/

	void pop()
	{
		if ( count == 0 )
			return;
		Node* n = tail->prev;
		if ( head == tail )
			head = 0;
		tail->destroy();
		if ( n )
			tail = n;
		else
			tail = 0;
		count--;
	}

	void append( const List& pOther )
	{
		ConstIter other( pOther );
		if ( !other.has() ) return;
		do {
			push_back( *other );
		} while ( ++other );
	}

	Iter start()
	{
		return Iter( *this );
	}

	ConstIter constStart() const
	{
		return ConstIter( *this );
	}

	// Primarily for debug stack-trace
	uint indexOf( const Iter& pIter )
	{
		if ( ! pIter.list->has() )
			return 0;
		if ( pIter.list != this || pIter.atEnd() )
			return size()-1;
		uint i=0;
		Iter it = start();
		for ( ; ! it.atEnd(); ++it, i++ )
		{
			if ( *it == *pIter )
			{
				return i;
			}
		}
		return size()-1;
	}

/*
	// This method is a useful walkaround for copy-construction of a list on the
	// heap to a list on the stack and vice versa.
	// You will need to delete any iterators you have to a class after this.
	void stealFrom(List& pOther) {
		clear();
		head = pOther.head;
		tail = pOther.tail;
		count = pOther.count;
		pOther.head = 0;
		pOther.tail = 0;
		pOther.count = 0;
	}
*/
};

}

#endif
