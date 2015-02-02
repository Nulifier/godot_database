#include "database.h"

void Database::_bind_methods()
{
	ObjectTypeDB::bind_method(_MD("open", "database_file", "mode_flags"), &Database::open);
	ObjectTypeDB::bind_method(_MD("close"), &Database::close);

	BIND_CONSTANT(MODE_OPEN_READONLY);
	BIND_CONSTANT(MODE_OPEN_READWRITE);
}

Error Database::open(const String& database_file, Mode mode_flags)
{
	if (!mpDatabase) {
		int err = sqlite3_open_v2(
			database_file.utf8().get_data(),	// SQLite requires UTF-8 on windows
			&mpDatabase,
			mode_flags,
			NULL);								// TODO: Add in VFS support for godot filesystem
		return err == SQLITE_OK ? OK : FAILED;	// TODO: Better error return codes
	}
	else {
		// Database connection already open
		return ERR_ALREADY_IN_USE;
	}
}

void Database::close()
{
	if (mpDatabase) {
		sqlite3_close_v2(mpDatabase);
		mpDatabase = nullptr;
	}
}

Database::Database()
: mpDatabase(nullptr)
{
}

Database::~Database()
{
	close();
}