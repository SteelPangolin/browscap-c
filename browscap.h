#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * A loaded Browscap data file.
 *
 * Note that these are not thread safe because iniparser isn't either.
 */
typedef struct browscap browscap;

/**
 * Properties for a given user agent.
 *
 * String properties may be null.
 */
typedef struct browscap_result {
    size_t num_str_properties;
    char** str_property_names;
    char** str_properties;
    size_t num_int_properties;
    char** int_property_names;
    int32_t* int_properties;
    size_t num_bool_properties;
    char** bool_property_names;
    bool* bool_properties;
} browscap_result;

/**
 * Load a Browscap data file.
 */
browscap* browscap_load(char* ini_path);

/**
 * Free a Browscap data file.
 *
 * Any browscap_results created from b will not be safe to access afterwards,
 * because the string properties are references to data managed by iniparser.
 */
void browscap_free(browscap* b);

/**
 * Search for a user agent in a Browscap data file.
 */
browscap_result* browscap_search(browscap* b, char* user_agent);

/**
 * Free user agent properties.
 */
void browscap_result_free(browscap_result* br);
