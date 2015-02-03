#include "statement.h"
#include "database.h"

void Statement::_bind_methods()
{
	ObjectTypeDB::bind_method(_MD("prepare", "database", "statement"), &Statement::prepare);
	ObjectTypeDB::bind_method(_MD("finalize"), &Statement::finalize);
	ObjectTypeDB::bind_method(_MD("getSql"), &Statement::getSql);
	ObjectTypeDB::bind_method(_MD("getParamCount"), &Statement::getParamCount);
	ObjectTypeDB::bind_method(_MD("getParamName", "index"), &Statement::getParamName);
	ObjectTypeDB::bind_method(_MD("getParamIndex", "parameter_name"), &Statement::getParamIndex);
	ObjectTypeDB::bind_method(_MD("bind", "index", "value"), &Statement::bind);
	ObjectTypeDB::bind_method(_MD("step"), &Statement::step);
	ObjectTypeDB::bind_method(_MD("getRow", "method"), &Statement::getRow, DEFVAL(GET_BY_NAME));
	ObjectTypeDB::bind_method(_MD("getResults", "method"), &Statement::getResults, DEFVAL(GET_BY_NAME));
	ObjectTypeDB::bind_method(_MD("getColumnCount"), &Statement::getColumnCount);
	ObjectTypeDB::bind_method(_MD("getColumn", "column_index"), &Statement::getColumn);
	ObjectTypeDB::bind_method(_MD("getColumnAsBool", "column_index"), &Statement::getColumnAsBool);

	BIND_CONSTANT(STEP_ROW);
	BIND_CONSTANT(STEP_DONE);
	BIND_CONSTANT(STEP_BUSY);
	BIND_CONSTANT(STEP_MISUSE);

	BIND_CONSTANT(GET_BY_NAME);
	BIND_CONSTANT(GET_BY_INDEX);
	BIND_CONSTANT(GET_BY_BOTH);
}

Error Statement::prepare(Ref<Database>& db, const String& statement)
{
	int err = sqlite3_prepare16_v2(
		db->mpDatabase,
		statement.c_str(),
		statement.length()*2,
		&mpStatement,
		NULL);
	if (err == SQLITE_OK) {
		return OK;
	}
	else {
		// TODO: Error handling
		ERR_PRINT(("Prepare code: " + itos(err)).utf8().get_data());
		ERR_PRINT("Failed to prepare statement");
		return FAILED;
	}
}

void Statement::finalize()
{
	if (mpStatement) {
		sqlite3_finalize(mpStatement);
		mpStatement = nullptr;
	}
}

String Statement::getSql() const
{
	if (mpStatement) {
		return String::utf8(sqlite3_sql(mpStatement));
	}
	else {
		ERR_PRINT("Tried to get SQL of uninitialized Statement");
		return String();
	}
}

int Statement::getParamCount() const
{
	return sqlite3_bind_parameter_count(mpStatement);
}

String Statement::getParamName(int index) const
{
	const char* name = sqlite3_bind_parameter_name(mpStatement, index);
	if (name) {
		return String::utf8(name);
	}
	else {
		return String();
	}
}

int Statement::getParamIndex(const String& parameter_name) const
{
	return sqlite3_bind_parameter_index(mpStatement, parameter_name.utf8().get_data());
}

Error Statement::bind(const Variant& index, const Variant& value)
{
	int parameter_index;

	if (index.is_num()) {
		parameter_index = index;
	}
	else if (index.get_type() == Variant::STRING) {
		parameter_index = getParamIndex(index);
	}
	else {
		ERR_PRINT("Invalid type for parameter index, must be int or string");
		return ERR_INVALID_PARAMETER;
	}

	// We now have the correct index
	int err;
	if (value.get_type() == Variant::RAW_ARRAY) {
		typedef DVector<uint8_t> RawArray;
		const RawArray& array = static_cast<RawArray>(value);
		RawArray::Read read = array.read();
		err = sqlite3_bind_blob(mpStatement, parameter_index, read.ptr(), array.size(), SQLITE_TRANSIENT);
	}
	else if (value.get_type() == Variant::REAL) {
		err = sqlite3_bind_double(mpStatement, parameter_index, value);
	}
	else if (value.get_type() == Variant::INT || value.get_type() == Variant::BOOL) {
		err = sqlite3_bind_int(mpStatement, parameter_index, value);
	}
	else if (value.get_type() == Variant::STRING) {
		err = sqlite3_bind_text16(mpStatement, parameter_index, ((String)value).c_str(), ((String)value).length(), SQLITE_TRANSIENT);
	}
	else if (value.get_type() == Variant::NIL) {
		err = sqlite3_bind_null(mpStatement, parameter_index);
	}
	else {
		ERR_PRINT("Type supplied for parameter value is not supported");
		return ERR_INVALID_PARAMETER;
	}

	if (err == SQLITE_OK) {
		return OK;
	}
	else if (err == SQLITE_RANGE) {
		return ERR_PARAMETER_RANGE_ERROR;
	}
	else {
		// If this is coming up then we might need to add some more specific error handling.
		return FAILED;
	}
}

Statement::ReturnCode Statement::step()
{
	return static_cast<ReturnCode>(sqlite3_step(mpStatement));
}

Dictionary Statement::getRow(GetMethod method) const
{
	Dictionary res;

	int num_columns = getColumnCount();

	for (int i = 0; i < num_columns; ++i) {
		// Get the value
		Variant val = getColumn(i);

		// Store it in the result list
		if (method == GET_BY_NAME || method == GET_BY_BOTH) {
			res[String(static_cast<const CharType*>(sqlite3_column_name16(mpStatement, i)))] = val;
		}
		if (method == GET_BY_INDEX || method == GET_BY_BOTH) {
			res[i] = val;
		}
	}

	return res;
}

Array Statement::getResults(GetMethod method)
{
	Array res;

	ReturnCode code;
	do {
		code = step();

		if (code == STEP_ROW) {
			// Store the data
			res.push_back(getRow(method));
		}

		// TODO: Error handling

	} while (code == STEP_ROW);

	return res;
}

int Statement::getColumnCount() const
{
	return sqlite3_column_count(mpStatement);
}

Variant Statement::getColumn(int column_index) const
{
	int type = sqlite3_column_type(mpStatement, column_index);

	if (type == SQLITE_INTEGER) {
		return Variant(sqlite3_column_int(mpStatement, column_index));
	}
	else if (type == SQLITE_FLOAT) {
		return Variant(sqlite3_column_double(mpStatement, column_index));
	}
	else if (type == SQLITE_TEXT) {
		return Variant(String(static_cast<const CharType*>(sqlite3_column_text16(mpStatement, column_index))));
	}
	else if (type == SQLITE_BLOB) {
		typedef DVector<uint8_t> RawArray;
		RawArray array;

		// Convert value to blob
		const void* blob = sqlite3_column_blob(mpStatement, column_index);
		int bytes = sqlite3_column_bytes(mpStatement, column_index);

		// Copy data
		array.resize(bytes);
		RawArray::Write write = array.write();
		copymem(write.ptr(), blob, bytes);

		return Variant(array);
	}
	else {// else if (type == SQLITE_NULL) {
		return Variant();
	}
}

bool Statement::getColumnAsBool(int column_index)
{
	return sqlite3_column_int(mpStatement, column_index) != 0 ? true : false;
}

Statement::Statement()
: mpStatement(nullptr)
{
}

Statement::~Statement()
{
	finalize();
}
