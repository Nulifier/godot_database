#include "register_types.h"
#include "object_type_db.h"

#include "database.h"

void register_database_types() {
	ObjectTypeDB::register_type<Database>();
}

void unregister_database_types() {

}
