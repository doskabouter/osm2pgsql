
#include "format.hpp"
#include "pgsql-helper.hpp"

idlist_t get_ids_from_result(pg_result_t const &result) {
    idlist_t ids;
    ids.reserve(result.num_tuples());

    for (int i = 0; i < result.num_tuples(); ++i) {
        ids.push_back(osmium::string_to_object_id(result.get_value(i, 0)));
    }

    return ids;
}

idlist_t get_ids_from_db(pg_conn_t const *db_connection, char const *stmt,
                         osmid_t id)
{
    auto const res = db_connection->exec_prepared(stmt, id);
    return get_ids_from_result(res);
}

void create_geom_check_trigger(pg_conn_t *db_connection,
                               std::string const &schema,
                               std::string const &table,
                               std::string const &geom_column)
{
    std::string func_name = qualified_name(schema, table + "_osm2pgsql_valid");

    db_connection->exec(
        "CREATE OR REPLACE FUNCTION {}()\n"
        "RETURNS TRIGGER AS $$\n"
        "BEGIN\n"
        "  IF ST_IsValid(NEW.{}) THEN \n"
        "    RETURN NEW;\n"
        "  END IF;\n"
        "  RETURN NULL;\n"
        "END;"
        "$$ LANGUAGE plpgsql IMMUTABLE;"_format(func_name, geom_column));

    db_connection->exec(
        "CREATE TRIGGER \"{}\""
        " BEFORE INSERT OR UPDATE"
        " ON {}"
        " FOR EACH ROW EXECUTE PROCEDURE"
        " {}();"_format(table + "_osm2pgsql_valid",
                        qualified_name(schema, table), func_name));
}

void analyze_table(pg_conn_t const &db_connection, std::string const &schema,
                   std::string const &name)
{
    auto const qual_name = qualified_name(schema, name);
    db_connection.exec("ANALYZE {}"_format(qual_name));
}
