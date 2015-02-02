#ifndef DATABASE_H
#define DATABASE_H

#include "reference.h"
#include "sqlite3.h"

class Database : public Reference {
	OBJ_TYPE(Database, Reference);

	sqlite3* mpDatabase;

protected:

	static void _bind_methods();

public:

	enum Mode {
		MODE_OPEN_READONLY		= SQLITE_OPEN_READONLY,
		MODE_OPEN_READWRITE		= SQLITE_OPEN_READWRITE,
		MODE_OPEN_CREATE		= SQLITE_OPEN_CREATE
	};

	Error open(const String& database_file, Mode mode_flags);
	void close();

	Database();
	~Database();
};

#endif
