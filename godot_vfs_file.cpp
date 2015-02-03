#include "godot_vfs_file.h"

const sqlite3_io_methods godot_vfs_File_io_methods = {
	1,										// Version
	&godot_vfs_File::close,					// Close function
	&godot_vfs_File::read,					// Read function
	&godot_vfs_File::write,					// Write function
	&godot_vfs_File::truncate,				// Truncate function
	&godot_vfs_File::sync,					// Sync function
	&godot_vfs_File::fileSize,				// File size function
	&godot_vfs_File::lock,					// Lock function
	&godot_vfs_File::unlock,				// Unlock function
	&godot_vfs_File::checkReservedLock,		// Check reserved lock
	&godot_vfs_File::fileControl,			// File control
	&godot_vfs_File::sectorSize,			// Sector size
	&godot_vfs_File::deviceCharacteristics,	// Device characteristics

	NULL,									// xShmMap, not used
	NULL,									// xShmLock, not used
	NULL,									// xShmBarrier, not used
	NULL,									// xShmUnmap, not used

	NULL,									// xFetch, not used
	NULL									// xUnfetch, not used
};

int godot_vfs_File::close(sqlite3_file* pFile)
{
	godot_vfs_File* p = reinterpret_cast<godot_vfs_File*>(pFile);
	ERR_FAIL_COND_V(!p->file, SQLITE_IOERR_CLOSE);

	p->file->close();
	memdelete(p->file);
	p->file = NULL;

	return SQLITE_OK;
}

int godot_vfs_File::read(sqlite3_file* pFile, void* buf, int nBytes, sqlite_int64 offset)
{
	godot_vfs_File* p = reinterpret_cast<godot_vfs_File*>(pFile);
	ERR_FAIL_COND_V(!p->file, SQLITE_IOERR_READ);

	// Seek to position
	p->file->seek(offset);
	ERR_FAIL_COND_V(p->file->get_pos() != offset, SQLITE_IOERR_READ);

	// Read the data
	int nRead = p->file->get_buffer(static_cast<uint8_t*>(buf), nBytes);

	if (nRead == nBytes) {
		return SQLITE_OK;
	}
	else if (nRead >= 0) {
		return SQLITE_IOERR_SHORT_READ;
	}

	ERR_FAIL_V(SQLITE_IOERR_READ);
}

int godot_vfs_File::write(sqlite3_file* pFile, const void* buffer, int nBytes, sqlite_int64 offset)
{
	godot_vfs_File* p = reinterpret_cast<godot_vfs_File*>(pFile);
	ERR_FAIL_COND_V(!p->file, SQLITE_IOERR_WRITE);

	// Move to the correct place in the file
	p->file->seek(offset);

	// Write the data
	p->file->store_buffer(static_cast<const uint8_t*>(buffer), nBytes);

	// Sanity check
	size_t bytes_written = p->file->get_pos() - offset;
	ERR_FAIL_COND_V(bytes_written != nBytes, SQLITE_IOERR_WRITE);

	return SQLITE_OK;
}

int godot_vfs_File::truncate(sqlite3_file* pFile, sqlite_int64 size)
{
	// This is a no-op for this VFS as it is not required as long as the journal mode is not set to truncate
	return SQLITE_OK;
}

int godot_vfs_File::sync(sqlite3_file* pFile, int flags)
{
	// We have no control over flushing
	return SQLITE_OK;
}

int godot_vfs_File::fileSize(sqlite3_file* pFile, sqlite_int64* size)
{
	godot_vfs_File* p = reinterpret_cast<godot_vfs_File*>(pFile);
	ERR_FAIL_COND_V(!p->file, SQLITE_IOERR);

	// Nice and trivial
	*size = p->file->get_len();

	return SQLITE_OK;
}

int godot_vfs_File::lock(sqlite3_file* p, int eLock)
{
	// TODO: Implement locks
	return SQLITE_OK;
}

int godot_vfs_File::unlock(sqlite3_file* p, int eLock)
{
	// TODO: Implement locks
	return SQLITE_OK;
}

int godot_vfs_File::checkReservedLock(sqlite3_file* p, int *resOut)
{
	// As we don't implement locks, we always show that no other process holds a reserved lock
	//		on this file. This ensures that the hot-journal is alwasy rolled back
	*resOut = 0;
	return SQLITE_OK;
}

int godot_vfs_File::fileControl(sqlite3_file* p, int op, void* arg)
{
	// We don't implement any verbs
	return SQLITE_OK;
}

int godot_vfs_File::sectorSize(sqlite3_file* p)
{
	// We don't know so we return the safe value
	return 0;
}

int godot_vfs_File::deviceCharacteristics(sqlite3_file* p)
{
	// We don't know so we return the safe value
	return 0;
}
