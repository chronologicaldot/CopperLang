// Copyright 2017-2018, Nic Anderson

#ifndef _Listix_INCLUDED_
#define _Listix_INCLUDED_

// simple definitions for usage with this list

#ifndef REAL_NULL
#define REAL_NULL 0	// For return of null pointers
#endif

#ifndef LISTIX_SI32
#ifdef _MSC_VER
typedef __int32                 si32;
#else
typedef signed int              si32;
#endif
#define LISTIX_SI32
#endif // #ifndef LISTIX_SI32


// namespace containing the list
namespace util
{
	template<class Unk> // Should probably be template<typename Unk>
	class Listix
	{
	public:

		struct Node
		{
			friend Listix;

			Node* Prior; // the node preceding this one
			Node* Post; // the node after this one

			Unk Item; // the item stored here

			//! constructor
			Node( Unk item )
				: Prior(REAL_NULL)
				, Post(REAL_NULL)
			{
				Item = item;
			}

			//! Extracts the node from the list
			void extract() {
				if ( Prior != REAL_NULL ) {
					Prior->Post = Post;
				}
				if ( Post != REAL_NULL ) {
					Post->Prior = Prior;
				}
			}

			//! Destroys the node
			void destroy() {
				extract();
				delete this;
			}

		protected:
			//! Connector (ignores current Post)
			void setPost( Node* node ) {
				Post = node;
				node->Prior = this;
			}

			//! Connector (ignores current Prior)
			void setPrior( Node* node ) {
				Prior = node;
				node->Post = this;
			}
		};

		// Exception Class
		class NoItemException
		{
			si32 failIndex;
		public:
			NoItemException(si32 i=0)
				: failIndex(i)
			{}

			si32 getFailureIndex() const {
				return failIndex;
			}
		};

	private:
		si32 size; // list size
		Node* HeadNode; // leading node
		Node* TailNode; // last node

		// This list uses a "Selector" or "Midix" - a pointer that
		// is moved around the list and used to access all of the nodes.
		Node* Selector;
		si32 selectorIndex; // location of Selector


	public:
		// Iterator class for loops
		// Prior to using, you should reset the list's Selector
		class ForwardIter
		{
			Listix& srcList;
			bool done;
		public:
			ForwardIter(Listix& sourceList)
				: srcList(sourceList)
				, done(false)
			{}

			bool isDone() const {
				return done;
			}

			Unk& next() {
				if ( srcList.getSize() == 0 )
					throw NoItemException(0);
				Unk& item = srcList.getCurrentItem();
				done = ! srcList.incrementSelector();
				return item;
			}
		};

		// Iterator class for loops
		// Prior to using, you should reset the list's Selector
		class BackwardIter
		{
			Listix& srcList;
			bool done;
		public:
			BackwardIter(Listix& sourceList)
				: srcList(sourceList)
				, done(false)
			{}

			bool isDone() const {
				return done;
			}

			Unk& next() {
				if ( srcList.getSize() == 0 )
					throw NoItemException(0);
				Unk& item = srcList.getCurrentItem();
				done = ! srcList.decrementSelector();
				return item;
			}
		};


			//! Constructor
		Listix()
			: size(0)
			, HeadNode(REAL_NULL)
			, TailNode(REAL_NULL)
			, Selector(REAL_NULL)
			, selectorIndex(0)
		{}

			//! Constructor
		Listix(Listix<Unk>& other)
			: size(0)
			, HeadNode(REAL_NULL)
			, TailNode(REAL_NULL)
			, Selector(REAL_NULL)
			, selectorIndex(0)
		{
			si32 i=0;
			for ( ; i < other.getSize(); i++ )
			{
				push_back( other.copyOf(i) );
			}
		}

			//! Destructor
		~Listix()
		{
			this->clear();
		}

			// Operator =
		Listix<Unk>& operator= ( Listix<Unk>& other )
		{
			si32 i=0;
			for ( ; i < other.getSize(); i++ )
			{
				push_back( other.copyOf(i) );
			}
			return *this;
		}

			//! Get current node location
		/* Returns the index value of the last node normally accessed. */
		si32 getSelectorIndex() const {
			return selectorIndex;
		}

			//! Reset Selector
		/* Resets the pointer for grabbing items back to the head node */
		void resetSelector()
		{
			Selector = HeadNode;
			selectorIndex = 0;
		}

			//! Set Selector to end
		/* Sets the selector to the tail node. */
		void setSelectorToEnd() {
			Selector = TailNode;
			selectorIndex = (size>0? size - 1: 0);
		}

			//! Increment Selector position
		/* Attempts to increment the index of the Selector.
		Returns true if possible, false if not. */
		bool incrementSelector() {
			if ( Selector == REAL_NULL )
				return false;
			if ( selectorIndex + 1 < size ) {
				Selector = Selector->Post;
				selectorIndex++;
				return true;
			} else {
				return false;
			}
		}

			//! Decrement selector position
		/* Attempts to decrement the index of the Selector.
		Returns true if possible, false if not. */
		bool decrementSelector() {
			if ( Selector == REAL_NULL )
				return false;
			if ( selectorIndex > 0 ) {
				Selector = Selector->Prior;
				selectorIndex--;
				return true;
			} else {
				return false;
			}
		}

			//! Get current node
		/* Returns a pointer to the last node normally accessed. */
		Node* getCurrentNode() {
			return Selector;
		}

			//! Get current node item
		/* Returns a pointer to the item of the last node normally accessed. */
		Unk* getCurrentItemPtr() {
			return &(Selector->Item);
		}

			//! Get the current item
		Unk& getCurrentItem() {
			return (Selector->Item);
		}
	
			//! Check bounds (simple)
		bool isInBounds( si32 index ) const {
			return index > 0 && index < size;
		}

			//! Check bounds
		/* Check if the index (given as a pointer) is within the bounds of the list.
		\param index - The index to be checked.
		\param canAlterIndex - If the index should be altered to ensure it is in range.
		Returns false if the index is NOT out of bounds. Otherwise it returns true.
		NOTE: When LISTIX_SIZE_T is NOT defined, this can be used with negative indices.
		*/
		bool isOutOfBounds( si32* index, bool canAlterIndex=false) const
		{
			// This index cannot be used
			if ( size == 0 ) return true;

			if ( *index >= size
#if !(defined LISTIX_SIZE_T)
				|| *index < 0
#endif
				)
			{
				// Is this index fixable?
				if ( canAlterIndex )
				{
					// Don't worry about corrections if there are no items
					// - let the calling function handle this scenario
					if ( size == 0 ) return false;

#if !(defined LISTIX_SIZE_T)
					// Correction for low values
					while ( *index < 0 ) *index = *index + size;
#endif

					// Correction for high values
					while ( *index >= size ) *index = *index - size;

					// This index can be used
					return false;
				} else {
					// This index should not be used
					return true;
				}
			} else {
				// This index can be used
				return false;
			}
		}

			//! Return pointer-to
		/* Find and return the pointer to a node at this particular location */
		Node* at( si32 index )
		{
			// Ensure the given index is within the boundaries
			if ( isOutOfBounds( &index, true ) ) {
				throw NoItemException(index);
			}

			/* If the location is closer to the last link than either the
			the first one or the Selector, start from the last link to obtain it. */
			if ( index > (size + selectorIndex)/2 )
			{
				Selector = TailNode; // set the Selector to the tail
				selectorIndex = size - 1; // set the location to the end
			} else {
				/* If the location is closer to the first link than the
				Selector, start from the first link to obtain it. */
				if ( index < selectorIndex/2 )
				{
					Selector = HeadNode; // set the Selector to the head
					selectorIndex = 0; // set the location to the start
				}
			}

			while ( index > selectorIndex )
			{
				Selector = Selector->Post;
				selectorIndex = selectorIndex + 1;
			}
			while ( index < selectorIndex )
			{
				Selector = Selector->Prior;
				selectorIndex = selectorIndex - 1;
			}

			return Selector;
		}

			//! Return address of
		/* Find and return the address of the object at this particular location.
		NOTE: It does NOT check for out-of-bounds errors. */
		Unk& operator[] ( si32 index )
		{
			return (this->at(index))->Item;
		}

			//! Return copy-of
		/* Find and return a copy of the object at this particular location.
		Throws NoItemException if there are no items in the list. */
		Unk copyOf ( si32 index )
		{
			if ( size == 0 )
				throw NoItemException(index);
			return (this->at(index))->Item;
		}

			//! Item at
		/* Get a pointer to the item at the given location.
		Throws NoItemException if there are no items in the list. */
		Unk* getItemPtrAt( si32 index )
		{
			if ( size == 0 )
				throw NoItemException(index);
			return &((this->at(index))->Item);
		}
		
			//! Is this the beginning?
		/* Returns true if the given index equals 0. */
		bool isIndexTheStart( si32 index ) const
		{
			return (index == 0);
		}
		
			//! Is this the end?
		/* Returns true if the given index equals the size - 1 */
		bool isIndexTheEnd( si32 index ) const
		{
			return (index == (size>0? size - 1: 0));
		}

			//! Is the end reached?
		/* Returns true if the midix is at the end. */
		bool isSelectorAtEnd() const
		{
			return selectorIndex == (size>0? size - 1: 0);
		}

			//! Size
		/* Returns the size of this list */
		si32 getSize() const {
			return size;
		}

			//! Set head node
		/* Sets the head node of the list if altered by some function outside
		of this class. */
		void setHeadNode( Node* newHeadNode )
		{
			this->HeadNode = newHeadNode;
		}

			//! Set tail node
		/* Sets the tail node of the list if altered by some function outside
		of this class. */
		void setTailNode( Node* newTailNode )
		{
			this->TailNode = newTailNode;
		}

			//! Replace item
		/* Find and return the node at this particular location */
		void replace( si32 index, Unk item )
		{
			if ( isOutOfBounds( &index, false ) )
				return;

			(this->at(index))->Item = item;
		}

			//! Insert
		/* Insert a link at a specific location in the chain. The new link
		will possess the location index of the location parameter given. */
		void insert( Unk item, si32 index )
		{
			// Ensure non-negative index
			if ( size > 0 )
				while ( index < 0 ) index = size + index;

			// if this is not one of the first links to be added
			if ( size >= 2 )
			{
				// go to this location if it is not the last one
				if ( index < size )
				{
					// if this is to be the first link
					if ( index == 0 )
					{
						HeadNode->Prior = new Node( item );
						HeadNode->Prior->Post = HeadNode; // connect the links
						HeadNode = HeadNode->Prior; // set the new link as the head
						selectorIndex = selectorIndex + 1; // account for a new link placed before Selector
					} else {
						// go to this location
						this->at(index);

						// save another pointer
						Node* preceder = Selector->Prior;

						// create a new node
						Selector->Prior = new Node( item );

						// connect the new link to the...
						Selector->Prior->Post = Selector; // ... one following it
						Selector->Prior->Prior = preceder; // ... one preceding it

						// connect the link following the new link to the new link
						preceder->Post = Selector->Prior;

						// account for the new link
						selectorIndex = selectorIndex + 1;
					}

				} else { // insertion after the last link

					// tack on a new link
					TailNode->Post = new Node( item );

					// set the tail link's preceding link to this one
					TailNode->Post->Prior = TailNode;

					// set the tail link to the new tail
					TailNode = TailNode->Post;
				}

				size += 1; // account for the newly added link
			} else {
				// if this is the first link
				if ( size == 0 )
				{
					HeadNode = new Node( item );
					TailNode = HeadNode;
					Selector = HeadNode;
					selectorIndex = 0;
				} else { // there is at least one link, so add a new one
					if ( index == 0 ) // new head
					{
						TailNode->Prior = new Node( item );
						TailNode->Prior->Post = TailNode; // connect the links
						HeadNode = TailNode->Prior; // save this new link to the head
						Selector = TailNode;
						selectorIndex = 1; // the Selector has moved up one
					} else {
						HeadNode->Post = new Node( item );
						HeadNode->Post->Prior = HeadNode; // connect the links
						TailNode = HeadNode->Post; // save this new link to the head
						Selector = HeadNode;
						selectorIndex = 0; // the Selector starts from the beginning
					}
				}

				size += 1; // account for the newly added link
			}
		}

			//! Add to back
		/* Add an item to the end of the chain */
		void push_back( Unk item )
		{
			insert( item, this->size );
		}

			//! Add to front
		/* Add an item to the beginning of the chain */
		void push_front( Unk item )
		{
			insert( item, 0 );
		}
		
			//! Add to back successively
		/* Adds an item to the end of the chain */
		void operator << ( Unk item )
		{
			push_back( item );
		}

			//! Delete
		/* Remove an item from somewhere in the list */
		void deleteAt( si32 index )
		{
			// if there are any links
			if ( size > 0 )
			{
				// account for indexes that are out of bounds
				if ( isOutOfBounds( &index, true ) )
					return;

				// if this is the only link in the chain
				if ( size == 1 )
				{
					HeadNode->destroy(); // delete the only item
					size = 0;
					HeadNode = REAL_NULL;
					TailNode = REAL_NULL;
					Selector = REAL_NULL;
					selectorIndex = 0;
				} else {
					// if the head
					if ( index == 0 )
					{
						// the Selector may need to move
						if ( Selector == HeadNode )
						{
							Selector = Selector->Post;
						} else {
							selectorIndex -= 1; // account for the position change
						}

						HeadNode = HeadNode->Post;
						HeadNode->Prior->destroy();

						// Prevent access to it
						HeadNode->Prior = REAL_NULL;
					} else {
						// if the tail
						if ( index == size - 1 )
						{
							// the Selector may need to move
							if ( Selector == TailNode )
							{
								Selector = Selector->Prior;
								selectorIndex = size - 2;
							}

							TailNode = TailNode->Prior;
							TailNode->Post->destroy();

							// Prevent access to it
							TailNode->Post = REAL_NULL;
						} else {
							Selector = at(index);
							decrementSelector();
							Selector->Post->destroy(); // Handles relinking other nodes
						}
					}

					size -= 1; // account for the loss of a link
				}
			}
		}

			//! Empty
		/* Clear the contents of the list */
		void clear()
		{
			if ( HeadNode != REAL_NULL ) {
				while ( HeadNode != TailNode ) {
					HeadNode = HeadNode->Post;
					HeadNode->Prior->destroy();
				}
				HeadNode->destroy();
				size = 0;
				selectorIndex = 0;
				HeadNode = REAL_NULL;
				TailNode = REAL_NULL;
				Selector = REAL_NULL;
			}
		}

			//! Find
		/* Attempts to locate an item in the list that matches the given
		item. Returns -1 if the item could not be found.
		The Selector is left at the location of the match. */
		si32 find( Unk& match )
		{
			resetSelector();
			for ( si32 i = 0; i < size; i++ )
			{
				Selector = Selector->Post;
				selectorIndex++;
				if ( Selector->Item == match )
					return i;
			}
			return -1;
		}

			//! Swap
		/* Exchange two items in the list. */
		void swap( si32 item1_index, si32 item2_index )
		{
			// Account for indexes that are out of bounds
			if ( isOutOfBounds( &item1_index, true ) || isOutOfBounds( &item2_index, true ) )
				return;

			// save the first item to be moved
			Unk item = *( getItemPtrAt(item1_index) );

			// relocate the other item to be moved
			*( getItemPtrAt( item1_index ) ) = *( getItemPtrAt( item2_index ) );

			// resave the item in temporary storage
			*( getItemPtrAt( item2_index ) ) = item;
		}

			//! Relocate
		/* Moves the link at the given location to the new location.
		\param index - Current index of the link.
		\param newIndex - Index where to move the link.
		Returns true if the relocation was successful. */
		bool relocate( si32 index, si32 newIndex )
		{
			// Account for indexes that are out of bounds
			if ( isOutOfBounds( &index, true ) || isOutOfBounds( &newIndex, true ) )
				return false;

			// Insert the item at the specific slot
			if ( index < newIndex )
				insert( *(getItemPtrAt( index )), newIndex + 1 );
			else
				this->insert( *(getItemPtrAt( index )), newIndex );

			/* Delete the old link, depending on its location relative
			to the new one, since the index of the old one will change
			if the new link is inserted before it. */
			if ( newIndex < index )
				deleteAt( index + 1 );
			else
				deleteAt( index );

			// Successful relocation of the link
			return true;
		}

			//! Create Forward Iterator
		/*
			Example:
		Listix<MyClass>::ForwardIterator i = list->createForwardIterator();
		while( ! i.isDone() ) {
			std::printf("%s", i.next().toString() );
		}
		*/
		ForwardIter createForwardIterator() {
			this->resetSelector();
			return ForwardIter(*this);
		}

			//! Create Backward Iterator
		BackwardIter createBackwardIterator() {
			this->setSelectorToEnd();
			return BackwardIter(*this);
		}

		//********** COPY, COMBINE, OR BREAK LIST FUNCTIONS ***********
		/* The following functions define procedures merging lists or parts
		of lists or breaking lists into pieces. */

			//! Append list
		/* Appends the data from the given list to this one. */
		void append( Listix<Unk>& list )
		{
			for ( si32 i = 0; i < list.getSize(); i++ )
			{
				push_back( list.copyOf(i) );
			}
		}

			//! Append range
		/* Copies part of the other list and append it to this one. */
		void appendRange( Listix<Unk>& fromList, si32 startIndex, si32 endIndex)
		{
			for ( si32 i = startIndex; i < endIndex; i++ ) {
				push_back( fromList.copyOf(i) );
			}
		}

			//! Destroy list front
		void destroyFront( si32 newStartIndex )
		{
			if ( newStartIndex < 0 )
				return;
			if ( newStartIndex >= size ) {
				clear();
				return;
			}
			Node* oldHead = HeadNode;
			HeadNode = at(newStartIndex);
			while( oldHead != HeadNode ) {
				oldHead = oldHead->Post;
				oldHead->Prior->destroy();
				size--;
			}
			resetSelector();
		}

			//! Destroy list back
		void destroyBack( si32 newEndIndex ) {
			if ( newEndIndex >= size )
				return;
			if ( newEndIndex < 0 ) {
				clear();
				return;
			}
			Node* oldTail = TailNode;
			TailNode = at(newEndIndex);
			while( oldTail != TailNode ) {
				oldTail = oldTail->Prior;
				oldTail->Post->destroy();
				size--;
			}
			resetSelector();
		}


			//! Delete section of list
		/* Removes the specified section of the list between the given
		indexes and deletes those values.
		\param start - Index of the first node to cut out.
		\param end - Index of the last node to cut out. */
		void destroyRange( si32 start, si32 end ) {
			if ( size == 0 )
				return;

			// Fix out-of-bounds indices
			if ( isOutOfBounds( &start, true ) || isOutOfBounds( &end, true ) )
				return;

			// Flip the indexes if need be
			si32 temp;
			if ( start > end )
			{
				temp = start;
				start = end;
				end = temp;
			}

			Node* preHead = HeadNode;
			Node* postTail = TailNode;
			Node* marked = REAL_NULL;

			if ( start > 0 )
				preHead = at(start - 1);
			if ( end < size - 1 )
				postTail = at(end + 1);

			for ( si32 i = start; i <= end; i++ ) {
				marked = at(i);
				marked->destroy();
			}
			// Front part is cut out
			if ( start == 0 )
				HeadNode = postTail;
			// Back end is cut out
			if ( end == size - 1 )
				TailNode = preHead;

			size -= end - start + 1;
			resetSelector();
		}

	};
}

#endif
