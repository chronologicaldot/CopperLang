// (C) 2020 Nicolaus Anderson

#include "../../Copper/src/Copper.h"
#include <stdio.h>

#ifndef CU_FILEMODE_TYPE
#define CU_FILEMODE_TYPE 12
#endif

#ifndef CU_FILE_TYPE
#define CU_FILE_TYPE 13
#endif

namespace Cu {
namespace FileIO {

namespace Basics {

/* Creates the following functions and adds them to the given engine:

file_mode_read()
file_mode_write()
file_mode_append()
	Return a file access mode (FileModeObject) for usage as the second argument of file_open()

file_open( [String file_path], [FileModeObject mode] )
	Attempts to open a file. Returns a FileObject.

file_name( [FileObject file] )
	Returns a string of the file name.

file_position( [FileObject file] )
	Returns an int value of the seek position in the given file object.

file_size( [FileObject file] )
	Returns the size of the given file.

file_seek( [FileObject file], [int shift], [bool from_start] )
	Shifts the position of access in the file by the given shift. If from_start, the shift is relative to the current position. Otherwise, the shift is relative to the start of the file. Returns true if seek performed.

file_read( [FileObject file], [string buffer], [int num_bytes] )
	Reads num_bytes amount of bytes from the file and stores them in the given buffer string. Returns the given number of bytes read.

file_write( [FileObject file], [string buffer] )
	Writes the buffer string to the given file. Finalizing is done with file_flush() and/or file_close(). Returns the number of bytes written.

file_flush( [FileObject file] )
	Completes any writing to file being done. Returns true if done, false otherwise.

file_close( [FileObject file] )
	Closes the file of the given FileObject. Automatically flushes.
*/
	void addFunctionsToEngine( Engine& );

} // end namespace Basics

//============================

enum MODE {
	_NONE,
	_READ,
	_WRITE,
	_APPEND
};

//============================

struct FileModeObject : public Object {

	MODE mode;

	FileModeObject( MODE m )
		: mode(m)
	{}

	virtual Object*
	copy() {
		return new FileModeObject( mode );
	}

	virtual void
	writeToString( String& out ) const;

	static const char*
	StaticTypeName() {
		return "filemode";
	}

	virtual const char*
	typeName() const {
		return StaticTypeName();
	}

	virtual bool
	supportsInterface( ObjectType::Value  typeValue ) const {
		return typeValue == StaticFileModeType();
	}

	static ObjectType::Value
	StaticFileModeType() {
		return static_cast<ObjectType::Value>( CU_FILEMODE_TYPE );
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "FileModeObject";
	}
#endif
};

//----------------------------

ForeignFunc::Result
GetReadMode( FFIServices& );

ForeignFunc::Result
GetWriteMode( FFIServices& );

ForeignFunc::Result
GetAppendMode( FFIServices& );


//============================

class FileObject : public Object {

	util::String filename;
	FILE* file;
	Integer filesize;
	MODE mode;

public:

	FileObject( const util::String& filepath, MODE );
	~FileObject();
	bool isOpen();
	String getFileName();
	Integer getPosition();
	Integer getSize();
	bool seek( Integer, bool fromStart );
	Integer read( String&, Integer bytecount ); // Read n bytes, store them in CharList. Returns bytes written.
	size_t read( void* filebuffer, size_t bytecount ); // For use by buffer classes.
	Integer write( const util::String& ); // Returns bytes written.
	bool flush();
	void close();

	virtual Object*
	copy() {
		// Copy of file handles is not possible
		this->ref();
		return this;
	}

	virtual void
	writeToString( String& out ) const;

	static const char*
	StaticTypeName() {
		return "file";
	}

	virtual const char*
	typeName() const {
		return StaticTypeName();
	}

	virtual bool
	supportsInterface( ObjectType::Value  typeValue ) const {
		return typeValue == StaticFileType();
	}

	static ObjectType::Value
	StaticFileType() {
		return static_cast<ObjectType::Value>( CU_FILE_TYPE );
	}

#ifdef COPPER_USE_DEBUG_NAMES
	virtual const char* getDebugName() const {
		return "FileObject";
	}
#endif

protected:
	void openFile( MODE m );
};

//----------------------------

ForeignFunc::Result
OpenFile( FFIServices& );

ForeignFunc::Result
FileIsOpen( FFIServices& );

ForeignFunc::Result
FileName( FFIServices& );

ForeignFunc::Result
FilePosition( FFIServices& );

ForeignFunc::Result
FileSize( FFIServices& );

ForeignFunc::Result
FileSeek( FFIServices& );

ForeignFunc::Result
FileRead( FFIServices& );

ForeignFunc::Result
FileWrite( FFIServices& );

ForeignFunc::Result
FileFlush( FFIServices& );

ForeignFunc::Result
CloseFile( FFIServices& );

} // end namespace FileIO
} // end namespace Cu
