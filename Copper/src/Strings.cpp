// unfortunately copyright belongs to the author, Nicolaus Anderson, 2016
//#include <stdio.h>
#include <climits> // *sigh* Inescapable system dependency.
#include "Strings.h"

namespace util {

// The number of decimal places can be limited to 8 by uncommenting the following line:
//#define STRING_LIMIT_DECIMAL_DIGIT_SAVE

char tolower( char pChar )
{
	if ( pChar >= 'A' && pChar <= 'Z' )
	{
		return (pChar - 'A') + 'a'; // Technically, specs say this may not work everywhere, but it usually does
	}
	return pChar;
}

bool equals(const char* str1, const char* str2) {
	if ( str1 == 0 || str2 == 0 ) return false;

	const char* i1 = str1;
	const char* i2 = str2;
	while( *i1 != '\0' ) {
		if ( *i2 == '\0' || *i1 != *i2 )
			return false;
		++i1;
		++i2;
	}
	return *i2 == '\0';
}

bool isAlphaNumeric(const char& c) {
	return ('a' <= c ) && ( c <= 'z') && ( 'A' <= c ) && ( c <= 'Z' ) && ( '0' <= c ) && ( c <= '9' );
}

//---------------------------------------------
CharList::CharList()
{}

CharList::CharList( const char* pString )
{
	uint i = 0;
	for ( ; pString[i] != '\0'; i++ )
	{
		push_back( pString[i] );
	}
}

CharList::CharList( const char* pString, uint pLength )
{
	// ATTENTION: These strings may or may not contain 1 or more \0 characters.
	// As this does not check for buffer overrun, please use wiseless.
	// This function may be removed.
	uint i = 0;
	for ( ; i < pLength; i++ )
	{
		push_back( pString[i] );
	}
}

CharList::CharList( const String& pString )
{
	const uint s = pString.size();
	uint i = 0;
	for ( ; i+1 < s; i++ ) // recall String saves the null-terminator, but CharList doesn't
	{
		push_back( pString[i] );
	}
}
/*
CharList::CharList( const int pValue )
{
	// FIXME: This may be slow.
	int v = pValue;
	bool flip = v < 0;
	if ( flip ) v = -v;
	int part;
	while( v > 0 ) {
		part = v % 10;
		push_front('0' + part);
		v = (v - part) / 10;
	}
	if ( pValue == 0 ) {
		push_front('0');
	}
	else if ( flip ) {
		push_front('-');
	}
}

CharList::CharList( const unsigned long pValue )
{
	// FIXME: This may be slow.
	unsigned long v = pValue;
	unsigned long part;
	while( v > 0 ) {
		part = v % 10;
		push_front('0' + part);
		v = (v - part) / 10;
	}
	if ( pValue == 0 ) {
		push_front('0');
	}
}

CharList::CharList( const float pValue )
{
	// FIXME: This fails to account for large float sizes whose inaccurate precision in division
	// can result in incorrect values.
	// Furthermore, float values can be huge, causing this function to take a very long time to complete.
	// The accurate alternatives are C++11's stod(), string's to_string(float) (semi-accurate),
	// and std::ostringstream ostr; ostr << number;
	float v = pValue;
	bool flip = false;
	if ( v < 0 ) {
		flip = true;
		v = -v;
	}
	int front = int(v);
	float back = v - float(front);
	int part = 0;
	while( front > 0 ) {
		part = front % 10;
		push_front('0' + part);
		front = (front - part) / 10;
	}
	if ( count == 0 )
		push_back('0');
	if ( back > 0.000001f )
		push_back('.');
#ifdef STRING_LIMIT_DECIMAL_DIGIT_SAVE
	int i=0;
	for(; i < 8; back > 0.000001f; i++ ) {
#else
	while( back > 0.000001f ) { // <- this is the only reason for this version of this function
#endif
		back *= 10;
		part = int(back);
		push_back('0' + part);
		back -= part;
	}
	if ( flip )
		push_front('-');
}

CharList::CharList( const double pValue )
{
	// FIXME: This fails to account for large double sizes whose inaccurate precision in division
	// can result in incorrect values.
	// Furthermore, double values can be huge, causing this function to take a very long time to complete.
	// The accurate alternatives are C++11's stod(), string's to_string(double) (semi-accurate),
	// and std::ostringstream ostr; ostr << number;
	double v = pValue;
	bool flip = false;
	if ( v < 0 ) {
		flip = true;
		v = -v;
	}
	int front = int(v);
	double back = v - double(front);
	int part = 0;
	while( front > 0 ) {
		part = front % 10;
		push_front('0' + part);
		front = (front - part) / 10;
	}
	if ( count == 0 )
		push_back('0');
	if ( back > 0.00000001 )
		push_back('.');
#ifdef STRING_LIMIT_DECIMAL_DIGIT_SAVE
	int i=0;
	for(; i < 8; back > 0.00000001; i++ ) {
#else
	while( back > 0.00000001 ) {
#endif
		back *= 10;
		part = int(back);
		push_back('0' + part);
		back -= part;
	}
	if ( flip )
		push_front('-');
}
*/
CharList::~CharList()
{
	clear();
}

CharList& CharList::operator= ( const CharList& pOther )
{
	if ( pOther.head == this->head )
		return *this;
	clear();
	ConstIter i = pOther.constStart();
	if ( i.atEnd() ) return *this;
	do {
		push_back( *i );
	} while ( i.next() );
	return *this;
}

CharList& CharList::append( const CharList& pOther )
{
	ConstIter i = pOther.constStart();
	if ( i.atEnd() && pOther.size() != 1 ) return *this;
	do {
		push_back( *i );
	} while ( i.next() );
	return *this;
}

CharList& CharList::append( const String& pString )
{
	uint i = 0;
	uint s = pString.size();
	for ( ; i < s; ++i )
	{
		push_back( pString[i] );
	}
	return *this;
}

bool CharList::equals( const char* pString )
{
	uint si = 0;
	if ( !has() && pString[si] != '\0' )
		return false;

	Iter i = start();
	while ( (*i) == pString[si] )
	{
		if ( pString[si] == '\0' && ! i.next() )
			return false;
		if ( i.atEnd() )
			return false;
	}
	return true;
}

bool CharList::equals( const CharList& pOther )
{
	// We want equality if the two strings are empty
	if ( ! has() )
	{
		if ( ! pOther.has() )
			return true;
		return false;
	} else if ( ! pOther.has() )
		return false;

	Iter i_me = start();
	ConstIter i_ot = pOther.constStart();

	while( *i_me == *i_ot )
	{
		if ( i_me.next() != i_ot.next() )
			return false;
	}
	return true;
}

bool CharList::equalsIgnoreCase( const CharList& pOther )
{
	// We want equality if the two strings are empty
	if ( ! has() )
	{
		if ( ! pOther.has() )
			return true;
		return false;
	} else if ( ! pOther.has() )
		return false;

	Iter i_me = start();
	ConstIter i_ot = pOther.constStart();

	while( tolower(*i_me) == tolower(*i_ot) )
	{
		if ( i_me.next() != i_ot.next() )
			return false;
	}
	return true;
}

void
CharList::appendULong( const unsigned long pValue )
{
	// FIXME: This may be slow.
	unsigned long v = pValue;
	unsigned long part;
	while( v > 0 ) {
		part = v % 10;
		push_front('0' + part);
		v = (v - part) / 10;
	}
	if ( pValue == 0 ) {
		push_front('0');
	}
}

//---------------------------------------------
String::String()
	: str(0)
	, len(0)
{
	str = new char[len+1]; // fixed bad assignement thanks to valgrind
	str[0] = '\0'; // only for returning as c-strings
}

String::String( const char* pString )
	: str(0)
	, len(0)
{
	while ( pString[len] != '\0' )
		len++;
	str = new char[len+1];
	uint i = 0;
	for ( ; i < len; ++i )
		str[i] = pString[i];
	str[len] = '\0'; // only for returning as c-strings
}

String::String( const String& pString )
	: str( 0 )
	, len( 0 )
{
	str = new char[pString.len+1];
	while ( len < pString.len )
	{
		str[len] = pString.str[len];
		len++;
	}
	str[len] = '\0'; // only for returning as c-strings
}

String::String( const CharList& pList )
	: str(0)
	, len(0)
{
	if ( ! pList.has() ) {
		str = new char[1];
		str[0] = '\0';
		return;
	}
	str = new char[pList.size() + 1];
	CharList::ConstIter i = pList.constStart();
	do {
		str[len] = *i;
		len++;
	} while ( i.next() );
	str[len] = '\0'; // only for returning as c-strings
}

String::String( const char pChar )
	: str(0)
	, len(0)
{
	str = new char[2];
	len = 1;
	str[0] = pChar;
	str[1] = '\0'; // only for returning as c-strings
}

String::~String()
{
	if ( str )
		delete[] str;
}

void
String::steal( String& pSource ) {
	if ( pSource.str == str )
		return;

	if ( str )
		delete[] str;
	str = pSource.str;
	pSource.str = new char[1];
	len = pSource.len;
	pSource.len = 0;
}

String& String::operator= ( const String& pString )
{
	if ( pString.str == str )
		return *this;

	len = 0;
	delete[] str;
	str = new char[pString.len+1];
	char* s = str;
	char* s2 = pString.str;
	while ( len < pString.len )
	{
		*s = *s2; //str[len] = pString.str[len];
		len++;
		++s;
		++s2;
	}
	*s = '\0'; //str[len] = '\0'; // only for returning as c-strings
	return *this;
}

String& String::operator= ( const char* pString )
{
	if ( pString == str )
		return *this;

	len = 0;
	delete[] str;
	while ( pString[len] != '\0' )
		len++;
	str = new char[len+1];
	uint i = 0;
	char* s = str;
	const char* s2 = pString;
	for ( ; i < len; ++i )
	{
		*s = *s2; //str[i] = pString[i];
		++s;
		++s2;
	}
	*s = '\0'; //str[len] = '\0'; // only for returning as c-strings
	return *this;
}

String& String::operator= ( const CharList& pList )
{
	delete[] str;
	len = 0;
	if ( ! pList.has() ) {
		str = new char[1];
		str[0] = '\0';
		return *this;
	}
	//str = new char[pList.size() + 1];
	len = pList.size();
	str = new char[len + 1];
	CharList::ConstIter i = pList.constStart();
	char* s = str;
	do {
		*s = *i; // str[len] = *i;
		//len++; // Now is set before loop
		++s;
	} while ( i.next() );
	*s = '\0'; //str[len] = '\0'; // only for returning as c-strings
	return *this;
}

String& String::operator+= ( const String& pString )
{
	const char* temp = str;
	uint templen = len;
	len = len + pString.len;
	str = new char[len+1];
	uint i, i2;
	for ( i=0; i < templen; i++ ) {
		str[i] = temp[i];
	}
	for ( i2=0; i2 < pString.len; i++, i2++ ) {
		str[i] = pString[i2];
	}
	str[len] = '\0';
	delete[] temp;
	return *this;
}

char String::operator[] ( uint pIndex ) const
{
	if ( pIndex >= len )
		return '\0';
	return str[pIndex];
}

void String::set( uint pIndex, char pChar )
{
	if ( pIndex < len )
		str[pIndex] = pChar;
}

const char* String::c_str() const
{
	return str;
}

uint String::size() const
{
	return len;
};

bool String::equals( const String& pOther ) const
{
	if ( str == pOther.str ) // Pointer comparison
		return true;
	// We want equality if the two strings are empty
	if ( len != pOther.len )
		return false;
	if ( len == 0 ) return true; // Both are zero and thus equal

	uint i=0;
	char* s = str;
	char* s2 = pOther.str;
	while( i < len )
	{
		if ( *s != *s2 )
			return false;
		++i;
		++s;
		++s2;
	}
	return true;
}

bool String::equals( const CharList& pList ) const
{
	if ( len <= 1 ) { // null character
		if ( pList.has() ) // list wins
			return false;
		return true; // Equally empty
	} // else len > 1
	CharList::ConstIter ci = pList.constStart();
	uint si = 0;
	char* s = str;
	for( ; si < len; ++si, ++s ) {
		if ( *s != ci.getItem() )
			return false;
		if ( !ci.next() && si+1 != len ) // reached end of CharList before String end
			return false;
	}
	return true;
}

bool String::equals( const char* pString ) const
{
	// Was:
	//return equals( String(pString) );
	// Should probably be:
	return util::equals(str, pString);
}

bool String::equalsIgnoreCase( const String& pOther ) const
{
	// We want equality if the two strings are empty
	if ( len != pOther.len )
		return false;
	if ( len == 0 ) return true; // Both are zero and thus equal

	uint i = 0;
	char* s = str;
	char* s2 = pOther.str;
	while( i < len )
	{
		if ( tolower(*s) != tolower(*s2) )
			return false;
		++i;
		++s;
		++s2;
	}
	return true;
}

int String::toInt() const
{
	if ( len == 0 )
		return 0;
	unsigned out = 0; // Hurray for unsigned overflow being "defined behavior"!
	unsigned part;
	uint i = 0;
	bool flip = false;
	if ( str[0] == '-' ) {
		flip = true;
		++i;
	}
	for ( ; i < len; ++i )
	{
		if ( str[i] == '.' ) break;
		if ( out > UINT_MAX / 10 || out > INT_MAX ) {
			out = INT_MAX;
			break;
		}
		out *= 10;
		part = (unsigned)str[i] - '0';
		if ( out > INT_MAX - part ) {
			out = INT_MAX;
			break;
		}
		out += part;
	}
	int ret = out;
	if ( flip )
		ret *= -1;
	return ret;
}

unsigned long String::toUnsignedLong() const
{
	if ( len == 0 )
		return 0;
	unsigned long out = 0;
	unsigned long part;
	uint i = 0;
	if ( str[0] == '-' ) // Awesome! Short curcuit here. Alternatively but stupidly we could return 2's compl.
		return 0;
	for ( ; i < len; ++i )
	{
		if ( str[i] == '.' ) break;
		if ( out > ULONG_MAX / 10 ) {
			out = ULONG_MAX;
			break;
		}
		out *= 10;
		part = (unsigned long)str[i] - '0';
		if ( out > ULONG_MAX - part ) {
			out = ULONG_MAX;
			break;
		}
		out += part;
	}
	return out;
}

float String::toFloat() const
{
	// TODO: Doesn't handle infinity or NaN bits.
	// Pain: https://stackoverflow.com/questions/570669/checking-if-a-double-or-float-is-nan-in-c
	if ( len == 0 )
		return 0;
	float little = 0;
	float big = 0;
	float digit = 1;
	uint i = 0;
	bool flip = false;
	bool deci = false;
	if ( str[0] == '-' ) {
		i++;
		flip = true;
	}
	for ( ; i < len; ++i ) {
		if ( str[i] == '.' ) {
			deci = true;
			++i;
			break;
		}
		big *= 10;
		big += float(str[i] - '0');
	}
	if ( deci )
	for ( ; i < len; ++i ) {
		digit /= 10;
		little += float(str[i] - '0') * digit;
	}
	big += little;
	if ( flip )
		big = -big;
	return big;
}

double String::toDouble() const
{
	// TODO: Doesn't handle infinity or NaN bits.
	// Pain: https://stackoverflow.com/questions/570669/checking-if-a-double-or-float-is-nan-in-c
	if ( len == 0 )
		return 0;
	double little = 0;
	double big = 0;
	double digit = 1;
	uint i = 0;
	bool flip = false;
	bool deci = false;
	if ( str[0] == '-' ) {
		i++;
		flip = true;
	}
	for ( ; i < len; ++i ) {
		if ( str[i] == '.' ) {
			deci = true;
			++i;
			break;
		}
		big *= 10;
		big += double(str[i] - '0');
	}
	if ( deci )
	for ( ; i < len; ++i ) {
		digit /= 10.f;
		little += double(str[i] - '0') * digit;
	}
	big += little;
	if ( flip )
		big = -big;
	return big;
}

void String::fromInt( int  value )
{
	if ( value == 0 ) {
		delete[] str;
		str = new char[2];
		str[0] = '0';
		str[1] = '\0';
		len = 1;
		return;
	}

	CharList total;
	bool isNeg = value < 0;
	int n = isNeg? -value : value;
	while ( n != 0 ) {
		total.push_front( char( n%10 + '0' ) );
		n /= 10;
	}
	if ( isNeg )
		total.push_front( char('-') );

	*this = total;
}

void String::fromDouble( double  value, uint prec )
{
	if ( value < .000001 && value > -.000001 ) {
		delete[] str;
		str = new char[2];
		str[0] = '0';
		str[1] = '\0';
		len = 1;
		return;
	}

	// We only preserve accuracy up to 6 places behind the decimal.
	CharList total;
	bool isNeg = value < 0;
	double ntop = isNeg? -value : value;
	long top = ntop;
	double fraction = ntop - top;
	uint i = 0;

	while ( i != prec ) {
		fraction *= 10;
		total.push_back( char( int(fraction) + '0' ) ); // floor(fraction)
		fraction -= int(fraction); // Chop top
		++i;
	}
	total.push_front('.');
	while ( top > 0 ) {
		total.push_front( char( top % 10 + '0' ) );
		top /= 10;
	}
	if ( isNeg )
		total.push_front('-');

	*this = total;
}

String String::convertBinary() const
{
	if ( len == 0 ) {
		return String();
	}

	if ( (len - 1)%8 != 0 ) {
		//fromInt(0); return false; // Fail
		return String();
	}
	
	uint numBytes = len / 8;
	uint i = 0;
	CharList bytes;
	unsigned char byte = 0;
	for(; i < numBytes; ++i) {
		byte |= (str[i] == '1' ? 1 : 0) << 7;
		byte |= (str[i+1] == '1' ? 1 : 0) << 6;
		byte |= (str[i+2] == '1' ? 1 : 0) << 5;
		byte |= (str[i+3] == '1' ? 1 : 0) << 4;
		byte |= (str[i+4] == '1' ? 1 : 0) << 3;
		byte |= (str[i+5] == '1' ? 1 : 0) << 2;
		byte |= (str[i+6] == '1' ? 1 : 0) << 1;
		byte |= (str[i+7] == '1' ? 1 : 0);
		bytes.append(byte);
	}
	return String(bytes);
}

void String::purgeNonPrintableASCII()
{
	// Using a list because we want the array to be the exact size of its occupancy
	CharList cleaned;
	uint i = 0;
	char* s = str;
	for ( ; i < len; ++i, ++s ) {
		if ( *s >= 32 )
			cleaned.push_back(*s);
	}
	*this = String(cleaned);
}

bool String::contains( char c ) const {
	uint i = 0;
	char* s = str;
	for ( ; i < len; ++i, ++s ) {
		if ( *s == c )
			return true;
	}
	return false;
}

unsigned char String::numberType() const {
	uint i = 0;
	char* s = str;
	unsigned char type = 1; // Integer type
	bool binary_ok = true;
	for ( ; i < len; ++i, ++s ) {
		if ( (*s >= '0' && *s <= '9') ) {
			if ( *s != '0' && *s != '1' )
				binary_ok = false;
			continue;
		} else if ( *s == '.' ) {
			binary_ok = false;
			if ( type == 2 ) // Second decimal found! Bad number format!
				return 0;
			type = 2; // Decimal type
			continue;
		} else {
			if ( i == len - 1 && *s == 'b' && binary_ok && (len-1)%8==0 ) {
				type = 3; // Binary type
				break;
			}
			return 0; // No numeric type
		}
	}
	return type;
}

uint String::keyValue() const {
	uint i = 0;
	uint key = 0;
	char* s = str;
	for ( ; i < len; ++i, ++s ) {
		key += (uint)(*s);
	}
	return key;
}

}
