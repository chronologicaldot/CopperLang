// Copyright 2018 Nicolaus Anderson
#ifndef UTILITY_DYN_ARRAY
#define UTILITY_DYN_ARRAY

namespace util {

typedef unsigned long	PileSize_t;

class BadPileIndexException {
	PileSize_t  idx;
public:
	BadPileIndexException( PileSize_t index )
		: idx(index)
	{}

	PileSize_t
	getIndex() const {
		return idx;
	}
};

/*
	A dynamically-sized array that cannot be shrunk until destroyed.
	Unlike other arrays, it occupies only the space that it needs.
*/
template<class T>
class Pile {
	T*  data;
	PileSize_t  count;

public:

	Pile()
		: data(0)
		, count(0)
	{}

	Pile( const Pile&  pOther )
		: data(0)
		, count(0)
	{
		data = new T[pOther.size()];
		for (; count < pOther.size(); ++count) {
			data[count] = T(pOther.data[count]);
		}
	}

	~Pile() {
		while( count > 0 ) {
			--count;
			data[count].~T();
		}
	}

	PileSize_t
	size() const {
		return count;
	}

	void
	push_back( const T&  item ) {
		T* temp = data;
		data = new T[++count];
		PileSize_t  i=0;
		for (; i < count - 1; ++i ) {
			data[i] = temp[i];
		}
		data[count-1] = T(item);
	}

	T&
	operator[] ( PileSize_t  index ) {
		if ( index >= count )
			throw BadPileIndexException(index);
		return data[index];
	}

	const T&
	operator[] ( PileSize_t  index ) const {
		if ( index >= count )
			throw BadPileIndexException(index);
		return data[index];
	}
};

}

#endif
