#include "godot_vfs.h"
#include "os/os.h"
#include "math_funcs.h"

#define MAX_PATH_NAME 512

#include "godot_vfs_file.h"

int godot_vfs_open(sqlite3_vfs* vfs, const char* filename, sqlite3_file* file, int flags, int* outFlags)
{
	godot_vfs_File* p = reinterpret_cast<godot_vfs_File*>(file);
	
	ERR_FAIL_COND_V(filename == NULL, SQLITE_IOERR);	// TODO: Support temp databases

	// Get flags
	// TODO: Support exclusive and create
	int godot_flags = 0;
	if (flags & SQLITE_OPEN_READONLY)	godot_flags |= FileAccess::READ;
	if (flags & SQLITE_OPEN_READWRITE)	godot_flags |= FileAccess::READ_WRITE;

	// Open file
	p->file = FileAccess::open(String::utf8(filename), godot_flags, NULL);

	ERR_FAIL_COND_V(!p->file, SQLITE_CANTOPEN);

	// Return flags, we fail if they request the wrong access
	if (outFlags) {
		*outFlags = flags;
	}

	p->base.pMethods = &godot_vfs_File_io_methods;	// This must be at end to prevent a call to xClose
	return SQLITE_OK;
}

int godot_vfs_delete(sqlite3_vfs* vfs, const char* name, int syncDir)
{
	return SQLITE_OK;
}

int godot_vfs_access(sqlite3_vfs* vfs, const char* name, int flags, int* resultOut)
{
	return SQLITE_OK;
}

int godot_vfs_fullPathname(sqlite3_vfs* vfs, const char* name, int nOut, char* str_out)
{
	for (int i = 0; i < nOut; ++i) {
		str_out[i] = name[i];
		if (name[i] == '\0') {
			break;
		}
	}

	return SQLITE_OK;
}

void* godot_vfs_dlOpen(sqlite3_vfs* vfs, const char* filename)
{
	return 0;
}

void godot_vfs_dlError(sqlite3_vfs* vfs, int nBytes, char* errMsg)
{
	sqlite3_snprintf(nBytes, errMsg, "Loadable extensions are not supported");
	errMsg[nBytes-1] = '\0';
}

void (*godot_vfs_dlSym(sqlite3_vfs* vfs, void* data, const char* symbol))(void)
{
	return 0;
}

void godot_vfs_dlClose(sqlite3_vfs* vfs, void* data)
{
	return;
}

int godot_vfs_randomness(sqlite3_vfs* vfs, int nBytes, char* byte)
{
	for (int i = 0; i < nBytes; ++i) {
		byte[i] = Math::rand();
	}
	return SQLITE_OK;
}

int godot_vfs_sleep(sqlite3_vfs* vfs, int microseconds)
{
	OS::get_singleton()->delay_usec(microseconds);
	return microseconds;
}

int godot_vfs_currentTime(sqlite3_vfs* vfs, double* time)
{
	uint64_t unix_time = OS::get_singleton()->get_unix_time();
	*time = unix_time/86400.0 + 2440587.5;	// Add the number of days since julian time
	return SQLITE_OK;
}

int godot_vfs_getLastError(sqlite3_vfs* vfs, int nBuf, char* buf)
{
	// TODO: Implement properly
	return 0;
}

int godot_vfs_currentTimeInt64(sqlite3_vfs* vfs, sqlite3_int64* now)
{
	uint64_t unix_time = OS::get_singleton()->get_unix_time();
	*now = unix_time + 210866760000;	// Add the number of ms since julian time
	return SQLITE_OK;
}

sqlite3_vfs* sqlite3_godot_vfs()
{
	static sqlite3_vfs godot_vfs = {
		3,								// Struct Version Number
		sizeof(godot_vfs_File),			// File struct size
		MAX_PATH_NAME,					// 512 characters in a filename is good enough for anyone
		0,								// pNext, set automatically
		"godot",						// Name of this vfs
		NULL,							// User data if needed
		&godot_vfs_open,				// Open function
		&godot_vfs_delete,				// Delete function
		&godot_vfs_access,				// Access function
		&godot_vfs_fullPathname,		// Converts relative pathnames to full pathnames
		&godot_vfs_dlOpen,				// Not used
		&godot_vfs_dlError,				// Not used
		&godot_vfs_dlSym,				// Not used
		&godot_vfs_dlClose,				// Not used
		&godot_vfs_randomness,			// Provides randomness
		&godot_vfs_sleep,				// Sleep function
		&godot_vfs_currentTime,			// Gets the current time in a double
		&godot_vfs_getLastError,		// Gets the last error from the vfs
		&godot_vfs_currentTimeInt64,	// Gets the curent time in an int64
		NULL,							// Not used
		NULL,							// Not used
		NULL							// Not used
	};
	return &godot_vfs;
}
