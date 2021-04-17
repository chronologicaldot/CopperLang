// For testing the hash table

// I need the String class.
// I really should test random inserts, and esp random names that end up conflicting.
// I should test the string and char list classes first.

#include <stdio.h>
#include <time.h>
#include "../src/RHHash.h"

using namespace util;

struct Int {
	unsigned int i;
	Int() : i(0) {}
	Int(unsigned int p) : i(p) {}
};

void test1();
void test2();

int main() {
	test1();

	return 0;
}

void test1() {
	setbuf(stdout, NULL);
	printf("\nLoaded.");
	RobinHoodHash<Int> hashTable(100);
	printf("\nFilling table...\n");

	// Uh... I need to auto-generate names.
	unsigned int i=1;
	unsigned int cap = 71; // Change to 103 so I can test resizing
	for( ; i < cap; ++i ) {
		//printf("i1(%i) \t", i);
		//if ( i % 4 == 3 )
		//	printf("\n");
		hashTable.insert(String(i), Int(i));
		printf("\ninterim Size: %u\t\tOccupancy: %u", hashTable.getSize(), hashTable.getOccupancy());
	}

	printf("\nSize: %u\nOccupancy: %u", hashTable.getSize(), hashTable.getOccupancy());

	String _40((unsigned)40);
	printf("\nString 40(%i) len = %u", _40.toInt(), _40.size());
	RobinHoodHash<Int>::BucketData* bucketData = hashTable.getBucketData( _40 );
	if ( bucketData != 0 ) {
		printf("\nBucket data not null.");
		printf("\nBucket data for \"%i\": %u", _40.toInt(), bucketData->item.i);
		hashTable.erase( bucketData->name );
	}
	printf("\nnew Size: %u\t\tnew Occupancy: %u\n", hashTable.getSize(), hashTable.getOccupancy());

	for( i=1; i < cap; ++i ) {
		//printf("i2(%i) \t", i);
		//if ( i % 4 == 3 )
		//	printf("\n");
		hashTable.insert(String(i+1000), Int(i+1000));
		printf("\ninterim Size: %u\t\tOccupancy: %u", hashTable.getSize(), hashTable.getOccupancy());
	}
	printf("\nnewer Size: %u\t\tnewer Occupancy: %u\n", hashTable.getSize(), hashTable.getOccupancy());

	for( i=1; i < cap; ++i ) {
		//printf("i2(%i) \t", i);
		//if ( i % 4 == 3 )
		//	printf("\n");
		hashTable.insert(String(i+2000), Int(i+2000));
	}
	printf("\nnewest Size: %u\t\tnewest Occupancy: %u\n", hashTable.getSize(), hashTable.getOccupancy());
}

void test2() {
	RobinHoodHash<Int> hashTable(100);
	unsigned int i = 0;
	unsigned int cap = 300;
	clock_t timeStart, timeEnd;
	timeStart = clock();
	for( ; i < cap; ++i ) {
		hashTable.insert(String(i*i), Int(0));
	}
	timeEnd = clock();

	float clockTime = ((float)(timeEnd-timeStart)/CLOCKS_PER_SEC)*1000;
	printf("\nHASH: Clock time for %i elements = %f\n", cap, clockTime);

	List<String> stringList;
	i = 0;
	timeStart = clock();
	for ( ; i < cap; ++i ) {
		stringList.push_back(String(i*i));
	}
	timeEnd = clock();

	clockTime = ((float)(timeEnd-timeStart)/CLOCKS_PER_SEC)*1000;
	printf("\nLIST: Clock time for %i elements = %f\n", cap, clockTime);
}
