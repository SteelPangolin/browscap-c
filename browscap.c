#include <stdio.h>
#include <string.h>
#include <fnmatch.h>
#include <ctype.h>

#include <iniparser.h>

#define NUM_PROPERTIES 7
static char* property_names[NUM_PROPERTIES] = {
    "Comment",
    "Browser",
    "Version",
    "Platform",
    "isMobileDevice",
    "isTablet",
    "Device_Type",
};

#define KEY_BUFFER_SIZE 1024

void search(dictionary* ini, char* user_agent);

int main(int argc, char** argv) {
    dictionary* ini = iniparser_load("lite_php_browscap.ini");
    //iniparser_dump(ini, stdout);

    for (int arg_index = 1; arg_index < argc; arg_index++) {
        search(ini, argv[arg_index]);
    }

    iniparser_freedict(ini);

    return 0;
}

void search(dictionary* ini, char* user_agent) {
    char key_buffer[KEY_BUFFER_SIZE];
    char* properties[NUM_PROPERTIES] = {NULL};

    int num_sections = iniparser_getnsec(ini);
    char* current_section_name = NULL;
    for (int section_index = 0; section_index < num_sections; section_index++) {
        char* section_name = iniparser_getsecname(ini, section_index);
        if (fnmatch(section_name, user_agent, FNM_CASEFOLD) == 0) {
            current_section_name = section_name;
            break;
        }
    }

    while (current_section_name != NULL) {
        int key_length = strlcpy(key_buffer, current_section_name, KEY_BUFFER_SIZE);
        if (key_length >= KEY_BUFFER_SIZE) {
            fprintf(stderr, "ran out of key buffer");
            exit(1);
        }
        key_length = strlcat(key_buffer, ":", KEY_BUFFER_SIZE);
        if (key_length >= KEY_BUFFER_SIZE) {
            fprintf(stderr, "ran out of key buffer");
            exit(1);
        }
        int prefix_length = key_length;

        for (int property_index = 0; property_index < NUM_PROPERTIES; property_index++) {
            if (properties[property_index] != NULL) {
                continue;
            }

            key_length = strlcat(key_buffer, property_names[property_index], KEY_BUFFER_SIZE);
            if (key_length >= KEY_BUFFER_SIZE) {
                fprintf(stderr, "ran out of key buffer");
                exit(1);
            }

            properties[property_index] = iniparser_getstring(ini, key_buffer, NULL);

            key_buffer[prefix_length] = '\0';
        }

        key_length = strlcat(key_buffer, "Parent", KEY_BUFFER_SIZE);
        if (key_length >= KEY_BUFFER_SIZE) {
            fprintf(stderr, "ran out of key buffer");
            exit(1);
        }
        current_section_name = iniparser_getstring(ini, key_buffer, NULL);
        if (current_section_name != NULL) {
            for (char* c = current_section_name; *c != '\0'; c++) {
                *c = (char)tolower(*c);
            }
        }
    }

    printf("%s\n", user_agent);
    for (int property_index = 0; property_index < NUM_PROPERTIES; property_index++) {
        printf("\t%s = ", property_names[property_index]);
        if (properties[property_index] != NULL) {
            printf("%s", properties[property_index]);
        }
        printf("\n");
    }
    printf("\n");
}
