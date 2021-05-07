// Copyright 2016, Nicolaus Anderson
#ifndef STRINGS_H
#define STRINGS_H

#include "utilList.h"

namespace util {

using util::uint;

char tolower( char pChar );
bool equals(const char* str1, const char* str2);
bool isAlphaNumeric(const char& c);

class String; // predeclaration

// Technically, a string builder, and it should probably be renamed as such
class CharList : public List<char>
{
public:
	CharList();
	explicit CharList( const char* pString );
	CharList( const char* pString, uint pLength );
	CharList( const String& pString );
	//explicit CharList( const int pValue );
	//explicit CharList( const unsigned long pValue );
	//explicit CharList( const float pValue );
	//explicit CharList( const double pValue );
	~CharList();
	CharList& operator= ( const CharList& pOther );
	CharList& append( const CharList& pOther );
	CharList& append( const String& pString );
	bool equals( const char* pString ); // string must be null-terminated
	bool equals( const CharList& pOther );
	bool equalsIgnoreCase( const CharList& pOther );
	void appendULong( const unsigned long pValue );
};

//! String
// A null-terminated string-containing class
class String
{
	char* str;
	uint len;
public:
	String();
	String( const char* pString );
	String( const String& pString );
	String( const CharList& pList );
	String( const char pChar );
	//String( const uint pValue ); //I wonder what kind of effect I'd have using just int
	//String( const int pValue ); // DEPRECATED - use CharList(const int)
	//String( const float pValue ); // DEPRECATED - use CharList(const float)
	virtual ~String();
	void steal( String& pSource ); // Steals the data from the source
	String& operator= ( const String& pString );
	String& operator= ( const char* pString );
	String& operator= ( const CharList& pList );
	String& operator+= ( const String& pString ); // may be removed
	char operator[] ( uint pIndex ) const;
	void set( uint pIndex, char pChar );
	const char* c_str() const;
	uint size() const;
	bool equals( const String& pString ) const;
	bool equals( const CharList& pList ) const;
	bool equals( const char* pString ) const;
	bool equalsIgnoreCase( const String& pOther ) const;
	int toInt() const;				// TODO: Speed up
	unsigned long toUnsignedLong() const;
	float toFloat() const;			// TODO: Handle NaN and infinity
	double toDouble() const;		// TODO: Handle NaN and infinity
	void fromInt( int );
	void fromDouble( double, uint prec=6 ); // Allows for precision setting
	void purgeNonPrintableASCII();
	bool contains( char c ) const;
	unsigned char numberType() const; // return 0 for no type, 1 for integer, 2 for decimal

	// Needs an equalsSubString( const String&, start, end )
	// Needs a getSubString( start, end )

	// Creates a key-value to be used for hash-tables
	uint keyValue() const;
};

}

#endif
