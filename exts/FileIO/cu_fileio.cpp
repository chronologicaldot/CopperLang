// (C) 2020 Nicolaus Anderson

#include "cu_fileio.h"

namespace Cu {
namespace FileIO {

namespace Basics {

void addFunctionsToEngine( Engine&  engine ) {
	addForeignFuncInstance(engine, "file_mode_read", &GetReadMode);
	addForeignFuncInstance(engine, "file_mode_write", &GetWriteMode);
	addForeignFuncInstance(engine, "file_mode_append", &GetAppendMode);
	addForeignFuncInstance(engine, "file_open", &OpenFile);
	addForeignFuncInstance(engine, "file_is_open", &FileIsOpen);
	addForeignFuncInstance(engine, "file_name", &FileName);
	addForeignFuncInstance(engine, "file_position", &FilePosition);
	addForeignFuncInstance(engine, "file_size", &FileSize);
	addForeignFuncInstance(engine, "file_seek", &FileSeek);
	addForeignFuncInstance(engine, "file_read", &FileRead);
	addForeignFuncInstance(engine, "file_write", &FileWrite);
	addForeignFuncInstance(engine, "file_flush", &FileFlush);
	addForeignFuncInstance(engine, "file_close", &CloseFile);
}

} // end namespace Basics

//---------------------------------------------

void
FileModeObject::writeToString( String& out ) const {
	switch( mode ) {
	case _READ:
		out = "read"; break;

	case _WRITE:
		out = "write"; break;

	case _APPEND:
		out = "append"; break;

	default:
		out = "none"; break;
	}
}

//---------------------------------------------

ForeignFunc::Result
GetReadMode( FFIServices& ffi ) {
	ffi.setNewResult( new FileModeObject(_READ) );
	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
GetWriteMode( FFIServices& ffi ) {
	ffi.setNewResult( new FileModeObject(_WRITE) );
	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
GetAppendMode( FFIServices& ffi ) {
	ffi.setNewResult( new FileModeObject(_APPEND) );
	return ForeignFunc::FINISHED;
}

//---------------------------------------------

FileObject::FileObject( const util::String& filepath, MODE m )
	: filename(filepath)
	, file(0)
	, filesize(0)
	, mode(_NONE)
{
	openFile(m);
}

FileObject::~FileObject() {
	if ( getRefCount() == 0 )
		close();
}

void
FileObject::openFile( MODE m ) {
	if ( isOpen() )
		close();

	if ( filename.size() == 0 )
	{
		file = 0;
		filesize = 0;
		mode = _NONE;
		return;
	}

	switch ( m ) {
	case _READ:
		file = fopen(filename.c_str(), "rb");
		mode = _READ;
		break;

	case _WRITE:
		file = fopen(filename.c_str(), "wb");
		mode = _WRITE;
		break;

	case _APPEND:
		file = fopen(filename.c_str(), "ab");
		mode = _APPEND;
		break;

	default:
		mode = _NONE;
		break;
	}

	if ( file ) {
		fseek(file, 0, SEEK_END);
		filesize = getPosition();
		fseek(file, 0, SEEK_SET);
	}
}

bool
FileObject::isOpen() {
	return file != 0;
}

String
FileObject::getFileName() {
	return filename;
}

Integer
FileObject::getPosition() {
	if ( !isOpen() )
		return 0;

	return (Integer) ftell(file);
}

Integer
FileObject::getSize() {
	if ( !isOpen() )
		return 0;

	size_t i;
	if ( mode == _WRITE ) {
		i = getPosition(); // Save last access position because the user expects it.
		fseek(file, 0, SEEK_END);
		filesize = getPosition();
		fseek(file, i, SEEK_SET);
	}

	return filesize;
}

bool
FileObject::seek( Integer steps, bool fromStart ) {
	if ( isOpen() ) {
		return fseek(file, steps, fromStart? SEEK_SET : SEEK_CUR) == 0;
	}
	return false;
}

Integer
FileObject::read( String&  out, Integer  bytecount ) {
	if ( !isOpen() || mode != _READ )
		return 0;

	char* buffer = new char[bytecount];
	size_t r = fread(buffer, 1, bytecount, file);
	out = String(buffer);
	delete[] buffer;
	return (Integer)r;
}

size_t
FileObject::read( void* filebuffer, size_t bytecount ) {
	if ( !isOpen() || mode != _READ )
		return 0;

	return fread(filebuffer, 1, bytecount, file);
}

Integer
FileObject::write( const util::String&  in ) {
	if ( !isOpen() || (mode != _WRITE && mode != _APPEND) )
		return 0;

	Integer written = (Integer) fwrite(in.c_str(), 1, (size_t)in.size(), file);

	if ( mode == _APPEND )
		filesize += written;

	return written;
}

bool
FileObject::flush() {
	if ( isOpen() ) {
		return fflush(file) != 0;
	}
	return false;
}

void
FileObject::close() {
	if ( isOpen() ) {
		if ( mode == _WRITE ) {
			fflush(file);
		}
		fclose(file);
		file = 0;
	}
}

void
FileObject::writeToString( String& out ) const {
	out = "{file object}"; // To acquire the contents, you need to read the file.
}

//---------------------------------------------

ForeignFunc::Result
OpenFile( FFIServices&  ffi ) {
	if ( !ffi.demandArgCount(2)
		|| !ffi.demandArgType(0, ObjectType::String)
		|| !ffi.demandArgType(1, FileModeObject::StaticFileModeType())
	) {
		return ForeignFunc::NONFATAL;
	}

	String& filename = ((StringObject&) ffi.arg(0)).getString();
	FileModeObject& mode = (FileModeObject&) ffi.arg(1);
	ffi.setNewResult( new FileObject(filename, mode.mode) );

	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
FileIsOpen( FFIServices& ffi ) {
	if ( !ffi.demandArgCount(1)
		|| !ffi.demandArgType(0, FileObject::StaticFileType())
	) {
		return ForeignFunc::NONFATAL;
	}

	bool yes = ((FileObject&) ffi.arg(0)).isOpen();
	ffi.setNewResult( new BoolObject(yes) );

	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
FileName( FFIServices& ffi ) {
	if ( !ffi.demandArgCount(1)
		|| !ffi.demandArgType(0, FileObject::StaticFileType())
	) {
		return ForeignFunc::NONFATAL;
	}

	String s = ((FileObject&) ffi.arg(0)).getFileName();
	ffi.setNewResult( new StringObject(s) );

	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
FilePosition( FFIServices& ffi ) {
	if ( !ffi.demandArgCount(1)
		|| !ffi.demandArgType(0, FileObject::StaticFileType())
	) {
		return ForeignFunc::NONFATAL;
	}

	Integer position = ((FileObject&) ffi.arg(0)).getPosition();
	ffi.setNewResult( new IntegerObject(position) );

	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
FileSize( FFIServices& ffi ) {
	if ( !ffi.demandArgCount(1)
		|| !ffi.demandArgType(0, FileObject::StaticFileType())
	) {
		return ForeignFunc::NONFATAL;
	}

	Integer size = ((FileObject&) ffi.arg(0)).getSize();
	ffi.setNewResult( new IntegerObject(size) );

	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
FileSeek( FFIServices& ffi ) {
	if ( !ffi.demandArgCount(3)
		|| !ffi.demandArgType(0, FileObject::StaticFileType())
		|| !ffi.demandArgType(1, ObjectType::Integer)
		|| !ffi.demandArgType(2, ObjectType::Bool)
	) {
		return ForeignFunc::NONFATAL;
	}

	FileObject& fileObject = (FileObject&) ffi.arg(0);
	Integer index = ((IntegerObject&) ffi.arg(1)).getIntegerValue();
	bool fromStart = ((BoolObject&) ffi.arg(2)).getValue();
	bool sought = fileObject.seek(index, fromStart);
	ffi.setNewResult( new BoolObject(sought) );

	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
FileRead( FFIServices& ffi ) {
	if ( !ffi.demandArgCount(3)
		|| !ffi.demandArgType(0, FileObject::StaticFileType())
		|| !ffi.demandArgType(1, ObjectType::String)
		|| !ffi.demandArgType(2, ObjectType::Integer)
	) {
		return ForeignFunc::NONFATAL;
	}

	FileObject& fileObject = (FileObject&) ffi.arg(0);
	util::String& buffer = ((StringObject&) ffi.arg(1)).getString();
	Integer numBytesToRead = ((IntegerObject&) ffi.arg(2)).getIntegerValue();

	Integer amountRead = fileObject.read(buffer, numBytesToRead);
	ffi.setNewResult( new IntegerObject( amountRead ) );

	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
FileWrite( FFIServices& ffi ) {
	if ( !ffi.demandArgCount(2)
		|| !ffi.demandArgType(0, FileObject::StaticFileType())
		|| !ffi.demandArgType(1, ObjectType::String)
	) {
		return ForeignFunc::NONFATAL;
	}

	FileObject& fileObject = (FileObject&) ffi.arg(0);
	String& buffer = ((StringObject&) ffi.arg(1)).getString();

	Integer amountWritten = fileObject.write(buffer);
	ffi.setNewResult( new IntegerObject( amountWritten ) );

	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
FileFlush( FFIServices& ffi ) {
	if ( !ffi.demandArgCount(1)
		|| !ffi.demandArgType(0, FileObject::StaticFileType())
	) {
		return ForeignFunc::NONFATAL;
	}

	bool completed = ((FileObject&) ffi.arg(0)).flush();
	ffi.setNewResult( new BoolObject(completed) );

	return ForeignFunc::FINISHED;
}

ForeignFunc::Result
CloseFile( FFIServices& ffi ) {
	if ( !ffi.demandArgCount(1)
		|| !ffi.demandArgType(0, FileObject::StaticFileType())
	) {
		return ForeignFunc::NONFATAL;
	}

	((FileObject&) ffi.arg(0)).close();

	return ForeignFunc::FINISHED;
}



} // end namespace FileIO
} // end namespace Cu
