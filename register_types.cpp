#include "register_types.h"
#include "object_type_db.h"

#include "database.h"
#include "statement.h"

void register_database_types() {
	ObjectTypeDB::register_type<Database>();
	ObjectTypeDB::register_type<Statement>();
}

void unregister_database_types() {

}
