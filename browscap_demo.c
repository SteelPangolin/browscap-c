#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "browscap.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        char* prog_name = argc > 0 ? argv[0] : "browscap_demo";
        fprintf(stderr, "usage: %s <browscap.ini> [user agent] [...]\n", prog_name);
        return EXIT_FAILURE;
    }

    char* ini_path = argv[1];

    browscap* b = browscap_load(ini_path);
    if (b == NULL) {
        fprintf(stderr, "%s\n", "couldn't load browscap data");
        return EXIT_FAILURE;
    }

    for (int arg_index = 2; arg_index < argc; arg_index++) {
        char* user_agent = argv[arg_index];
            
        printf("%s\n", user_agent);

        browscap_result* br = browscap_search(b, user_agent);

        for (size_t prop_index = 0;
                prop_index < br->num_str_properties;
                prop_index++) {
            printf("\t%s = ", br->str_property_names[prop_index]);
            printf("%s", br->str_properties[prop_index]
                ? br->str_properties[prop_index] : "NULL");
            printf("\n");
        }

        for (size_t prop_index = 0;
                prop_index < br->num_int_properties;
                prop_index++) {
            printf("\t%s = ", br->int_property_names[prop_index]);
            printf("%" PRId32, br->int_properties[prop_index]);
            printf("\n");
        }

        for (size_t prop_index = 0;
                prop_index < br->num_bool_properties;
                prop_index++) {
            printf("\t%s = ", br->bool_property_names[prop_index]);
            printf("%s", br->bool_properties[prop_index]
                ? "true" : "false");
            printf("\n");
        }

        printf("\n");

        browscap_result_free(br);
    }

    browscap_free(b);

    return EXIT_SUCCESS;
}
