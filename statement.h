#ifndef STATEMENT_H
#define STATEMENT_H

#include "reference.h"
#include "sqlite3.h"

class Database;

class Statement : public Reference {
	OBJ_TYPE(Statement, Reference);

	sqlite3_stmt* mpStatement;

protected:

	static void _bind_methods();

public:

	enum ReturnCode {
		STEP_ROW = SQLITE_ROW,
		STEP_DONE = SQLITE_DONE,
		STEP_BUSY = SQLITE_BUSY,
		STEP_MISUSE = SQLITE_MISUSE
	};

	enum GetMethod {
		GET_BY_NAME,
		GET_BY_INDEX,
		GET_BY_BOTH
	};

	Error prepare(Ref<Database>& db, const String& statement);
	void finalize();

	String getSql() const;

	int getParamCount() const;
	String getParamName(int index) const;
	int getParamIndex(const String& parameter_name) const;
	Error bind(const Variant& index, const Variant& value);

	ReturnCode step();

	Dictionary getRow(GetMethod method = GET_BY_NAME) const;
	Array getResults(GetMethod method = GET_BY_NAME);

	int getColumnCount() const;
	Variant getColumn(int column_index) const;
	bool getColumnAsBool(int column_index);	// Specifically not const as it can change the column type

	Statement();
	~Statement();
};

VARIANT_ENUM_CAST(Statement::ReturnCode);
VARIANT_ENUM_CAST(Statement::GetMethod);

#endif
