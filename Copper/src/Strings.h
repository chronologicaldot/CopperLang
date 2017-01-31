// unfortunately copyright belongs to the author, Nicolaus Anderson, 2016
#ifndef STRINGS_H
#define STRINGS_H

#include "utilList.h"

namespace util {

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
	explicit CharList( const int pValue );
	explicit CharList( const unsigned long pValue );
	explicit CharList( const float pValue );
	explicit CharList( const double pValue );
	~CharList();
	CharList& operator= ( const CharList& pOther );
	CharList& append( const CharList& pOther );
	CharList& append( const String& pString );
	bool equals( const char* pString ); // string must be null-terminated
	bool equals( const CharList& pOther );
	bool equalsIgnoreCase( const CharList& pOther );
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
	String& operator= ( const String& pString );
	String& operator= ( const char* pString );
	String& operator= ( const CharList& pList );
	String& operator+= ( const String& pString ); // may be removed
	char operator[] ( uint pIndex ) const;
	const char* c_str() const;
	uint size() const;
	bool equals( const String& pString ) const;
	bool equals( const CharList& pList ) const;
	bool equals( const char* pString ) const;
	bool equalsIgnoreCase( const String& pOther ) const;
	int toInt() const;				// DO NOT USE!
	unsigned long toUnsignedLong() const;
	float toFloat() const;			// DO NOT USE!
	double toDouble() const;		// DO NOT USE!
	void purgeNonPrintableASCII();
	bool contains( char c ) const;

	// Needs an equalsSubString( const String&, start, end )
	// Needs a getSubString( start, end, String& )
};

}

#endif
