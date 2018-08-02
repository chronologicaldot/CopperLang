// Copyright 2016 Nicolaus Anderson

#ifndef ROBIN_HOOD_HASH_H
#define ROBIN_HOOD_HASH_H

#include "Strings.h"

namespace util {

template<class T>
class RobinHoodHash {
public:
	struct BucketData {
		const String name;
		T item;
		uint delay; // Time to reach this data

		BucketData(const String& pName, const T& pItem)
			: name(pName)
			, item(pItem)
			, delay(0)
		{}

		BucketData(const BucketData& pOther)
			: name(pOther.name)
			, item(pOther.item)
			, delay(pOther.delay)
		{}
	};

	struct Bucket {
		BucketData* data;
		bool wasOccupied;

		Bucket() : data(0), wasOccupied(false) {}

		~Bucket() {
			clear();
		}

		void clear() {
			if ( data ) {
				delete data;
				data = 0;
				wasOccupied = true;
			}
		}
	};

private:
	uint size;
	uint occupancy;
	Bucket* buckets;
protected:
	void jump(uint& pIndex);
	uint getInitKey(const String& pName);
	void resizeTable();
public:
	RobinHoodHash(uint pInitSize);
	RobinHoodHash(const RobinHoodHash<T>& pOther);
	~RobinHoodHash();
	void appendCopyOf(RobinHoodHash<T>& pOther);
	Bucket* get(uint pIndex);
	BucketData* getBucketData(const String& pName);
	T* insert(const String& pName, T pItem);
	uint getSize() const;
	uint getOccupancy() const;
	void erase(const String& pName);
};

template<class T>
RobinHoodHash<T>::RobinHoodHash(uint pInitSize)
	: size(pInitSize)
	, occupancy(0)
{
	buckets = new Bucket[size]();
}

template<class T>
RobinHoodHash<T>::RobinHoodHash(const RobinHoodHash<T>& pOther) // Which is best: const or non-const?
	: size(pOther.size)
	, occupancy(pOther.occupancy)
{
	buckets = new Bucket[size]();
	BucketData* bucketData;
	uint i=0;
	for(; i < pOther.size; ++i) {
		bucketData = pOther.buckets[i].data;
		if ( bucketData ) {
			buckets[i].data = new BucketData(*bucketData);
		} else {
			buckets[i].data = 0;
		}
	}
}

template<class T>
void RobinHoodHash<T>::appendCopyOf(RobinHoodHash<T>& pOther) {
	uint i=0;
	BucketData* bucketData;
	for(; i < pOther.size; ++i) {
		bucketData = pOther.buckets[i].data;
		if ( bucketData ) {
			insert(bucketData->name, bucketData->item);
		}
	}
}

template<class T>
RobinHoodHash<T>::~RobinHoodHash() {
	delete[] buckets;
}

template<class T>
typename RobinHoodHash<T>::Bucket* RobinHoodHash<T>::get(uint pIndex) {
	return &(buckets[pIndex]);
}

template<class T>
void RobinHoodHash<T>::jump(uint& pIndex) {
	pIndex = (pIndex + 1) % size; // Safe jump
}

template<class T>
uint RobinHoodHash<T>::getInitKey(const String& pName) {
	return pName.keyValue() % size;
}

template<class T>
typename RobinHoodHash<T>::BucketData* RobinHoodHash<T>::getBucketData(const String& pName) {
	if ( occupancy == 0 ) // short-circuit
		return 0;
	uint idx = getInitKey(pName);
	uint jumpCount = 0;
	Bucket* initBucket = get(idx);
	Bucket* bucket = initBucket;
	while ( bucket->data != 0 || bucket->wasOccupied ) {
		if ( bucket->data->name.equals(pName) ) {
			return bucket->data;
		}
		// This if-statement's safety unconfirmed
		if ( bucket->data->delay < jumpCount ) {
			// If the bucket existed, it would have been inserted earlier.
			return 0;
		}
		jump(idx);
		jumpCount++;
		bucket = get(idx);
		if ( bucket == initBucket ) // This is problematic if the jump doesn't cycle thru all open spots
			return 0;
	}
	return 0;
}

template<class T>
T* RobinHoodHash<T>::insert(const String& pName, T pItem) {
	uint idx = getInitKey(pName);
	Bucket* initBucket = get(idx);
	Bucket* bucket = initBucket;
	BucketData* floatData = new BucketData(pName, pItem);
	T* itemPtr = &(floatData->item);
	BucketData* tempData = 0;
	while( true ) {
		//if ( bucket == 0 ) throw 1;
		if ( bucket->data == 0 ) {
			bucket->data = floatData;
			occupancy++;
			return itemPtr;
		}
		// else data != 0
		if ( bucket->data->name.equals(floatData->name) ) {
			// Replacing data should only be done directly, via getBucketdata()
			return itemPtr;
		}
		// Compare delays to reach
		// May be problematic if index jump exceeds 1.
		if ( bucket->data->delay < floatData->delay ) {
			tempData = bucket->data;
			bucket->data = floatData;
			floatData = tempData;
		}
		jump(idx);
		bucket = get(idx);
		floatData->delay++;
		// If all spots are checked, try to resize the table.
		if ( bucket == initBucket ) {
			resizeTable();
			// Restart search for a location because the table has changed
			idx = getInitKey(floatData->name);
			initBucket = get(idx);
			bucket = initBucket;
		}
	}
}

template<class T>
uint RobinHoodHash<T>::getSize() const {
	return size;
}

template<class T>
uint RobinHoodHash<T>::getOccupancy() const {
	return occupancy;
}

template<class T>
void RobinHoodHash<T>::erase(const String& pName) {
	if ( occupancy == 0 ) // short-circuit
		return;
	uint idx = getInitKey(pName);
	Bucket* initBucket = get(idx);
	Bucket* bucket = initBucket;
	while ( bucket->data != 0 || bucket->wasOccupied ) {
		if ( bucket->data->name.equals(pName) ) {
			bucket->clear();
			occupancy--;
			return;
		}
		jump(idx);
		bucket = get(idx);
		if ( bucket == initBucket ) // This is problematic if the jump doesn't cycle thru all open spots
			return;
	}
}

template<class T>
void RobinHoodHash<T>::resizeTable() {
	// Attempt to make the table larger (double the size)
	// Unfortunately, buckets can't be simply copied to a new array because their position
	// depends on a jump that cycles based on the current array size.
	Bucket* oldBuckets = buckets;
	uint oldSize = size;
	size *= 2;
	buckets = new Bucket[size]();
	uint i=0;
	occupancy = 0; // Reset because everything must be re-added
	for (; i < oldSize; ++i) {
		Bucket& oldBucket = oldBuckets[i];
		if ( oldBucket.data != 0 ) {
			insert( oldBucket.data->name, oldBucket.data->item );
		}
	}
	delete[] oldBuckets;
}

}

#endif
