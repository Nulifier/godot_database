#ifndef GODOT_VFS_FILE_H
#define GODOT_VFS_FILE_H

#include "sqlite3.h"
#include "os/file_access.h"

struct godot_vfs_File {
	sqlite3_file base;				// Base class, must be first
	FileAccess* file;				// Actual file

	static int write(sqlite3_file* pFile, const void* buffer, int nBytes, sqlite_int64 offset);
	static int close(sqlite3_file* pFile);
	static int read(sqlite3_file* pFile, void* buf, int nBytes, sqlite_int64 offset);
	static int truncate(sqlite3_file* pFile, sqlite_int64 size);
	static int sync(sqlite3_file* pFile, int flags);
	static int fileSize(sqlite3_file* pFile, sqlite_int64* size);
	static int lock(sqlite3_file* pFile, int eLock);
	static int unlock(sqlite3_file* pFile, int eLock);
	static int checkReservedLock(sqlite3_file* pFile, int *resOut);
	static int fileControl(sqlite3_file* pFile, int op, void* arg);
	static int sectorSize(sqlite3_file* pFile);
	static int deviceCharacteristics(sqlite3_file* pFile);
};

extern const sqlite3_io_methods godot_vfs_File_io_methods;

#endif
