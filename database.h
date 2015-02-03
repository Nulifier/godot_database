#ifndef DATABASE_H
#define DATABASE_H

#include "reference.h"
#include "sqlite3.h"

// TODO: Look into using UTF-16 in database
// TODO: Get sqlite to use godot memory allocation
class Database : public Reference {
	friend class Statement;
	OBJ_TYPE(Database, Reference);

	sqlite3* mpDatabase;

protected:

	static void _bind_methods();

public:

	enum Mode {
		MODE_OPEN_DEFAULT		= SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
		MODE_OPEN_READONLY		= SQLITE_OPEN_READONLY,
		MODE_OPEN_READWRITE		= SQLITE_OPEN_READWRITE,
		MODE_OPEN_CREATE		= SQLITE_OPEN_CREATE
	};

	Error open(const String& database_file, Mode mode_flags = MODE_OPEN_DEFAULT);
	void close();

	bool opened() const;

	String getErrorMessage() const;

	Database();
	~Database();
};

VARIANT_ENUM_CAST(Database::Mode);

#endif
