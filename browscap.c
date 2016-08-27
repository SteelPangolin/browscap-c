#include <fnmatch.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iniparser.h>

#include "browscap.h"

typedef enum {
    browscap_file_type_lite,
    browscap_file_type_regular,
    browscap_file_type_full,
} browscap_file_type;

struct browscap {
    dictionary* ini;
    browscap_file_type file_type;
};

static const size_t browscap_num_str_properties[] = {
    5,
    6,
    20,
};

static const char* browscap_str_property_names[] = {
    // Lite
    "Comment",
    "Browser",
    "Version",
    "Platform",
    "Device_Type",
    // regular
    "Device_Pointing_Method",
    // full
    "Browser_Type",
    "Browser_Maker",
    "Browser_Modus",
    "Platform_Version",
    "Platform_Description",
    "Platform_Maker",
    "Device_Name",
    "Device_Maker",
    "Device_Code_Name",
    "Device_Brand_Name",
    "RenderingEngine_Name",
    "RenderingEngine_Version",
    "RenderingEngine_Description",
    "RenderingEngine_Maker",
};

static const size_t browscap_num_int_properties[] = {
    0,
    2,
    6,
};

static char* browscap_int_property_names[] = {
    // lite
    // regular
    "MajorVer", // except for "Basic" and "Home"
    "MinorVer", // except for "0b"
    // full
    "Browser_Bits",
    "Platform_Bits",
    "CssVersion",
    "AolVersion",
};

static const size_t browscap_num_bool_properties[] = {
    2,
    3,
    21,
};

static char* browscap_bool_property_names[] = {
    // lite
    "isMobileDevice",
    "isTablet",
    // regular
    "Crawler",
    // full
    "Alpha",
    "Beta",
    "Win16",
    "Win32",
    "Win64",
    "Frames",
    "IFrames",
    "Tables",
    "Cookies",
    "BackgroundSounds",
    "JavaScript",
    "VBScript",
    "JavaApplets",
    "ActiveXControls",
    "isSyndicationReader",
    "isFake",
    "isAnonymized",
    "isModified",
};

browscap* browscap_load(char* ini_path) {
    browscap* b = calloc(1, sizeof(browscap));
    if (b == NULL) {
        fprintf(stderr, "%s\n", "calloc failed");
        return b;
    }
    b->ini = iniparser_load(ini_path);

    char* file_type_str = iniparser_getstring(
        b->ini,
        "GJK_Browscap_Version:Type",
        NULL);
    if (file_type_str == NULL) {
        fprintf(stderr, "%s\n", "couldn't find file type");
        return b;
    } else if (strncmp(file_type_str, "LITE", sizeof("LITE")) == 0) {
        b->file_type = browscap_file_type_lite;
    } else if (strncmp(file_type_str, "FULL", sizeof("FULL")) == 0) {
        b->file_type = browscap_file_type_full;
    } else {
        b->file_type = browscap_file_type_regular;
    }

    return b;
}

void browscap_free(browscap* b) {
    iniparser_freedict(b->ini);
    free(b);
}

browscap_result* browscap_result_init(browscap* b) {
    browscap_result* br = (browscap_result*)calloc(1, sizeof(browscap_result));
    if (br == NULL) {
        fprintf(stderr, "%s\n", "calloc failed");
        return br;
    }

    br->num_str_properties = browscap_num_str_properties[b->file_type];
    br->str_property_names = (char**)browscap_str_property_names;
    br->str_properties = (char**)calloc(br->num_str_properties, sizeof(char*));
    if (br->str_properties == NULL) {
        fprintf(stderr, "%s\n", "calloc failed");
        return br;
    }

    br->num_int_properties = browscap_num_int_properties[b->file_type];
    br->int_property_names = (char**)browscap_int_property_names;
    br->int_properties = (int32_t*)calloc(br->num_int_properties, sizeof(int32_t));
    if (br->int_properties == NULL) {
        fprintf(stderr, "%s\n", "calloc failed");
        return br;
    }

    br->num_bool_properties = browscap_num_bool_properties[b->file_type];
    br->bool_property_names = (char**)browscap_bool_property_names;
    br->bool_properties = (bool*)calloc(br->num_bool_properties, sizeof(bool));
    if (br->bool_properties == NULL) {
        fprintf(stderr, "%s\n", "calloc failed");
        return br;
    }

    return br;
}

browscap_result* browscap_search(browscap* b, char* user_agent) {
    char key_buffer[1024]; // Should be bigger than any pattern in any browscap file so far.

    browscap_result* br = browscap_result_init(b);

    // Keep track of which properties have already been set,
    // so that we don't overwrite a child's properties with a parent's.
    bool str_properties_set[br->num_str_properties];
    bool int_properties_set[br->num_int_properties];
    bool bool_properties_set[br->num_bool_properties];
    memset(str_properties_set, 0, sizeof(str_properties_set) * sizeof(bool));
    memset(int_properties_set, 0, sizeof(int_properties_set) * sizeof(bool));
    memset(bool_properties_set, 0, sizeof(bool_properties_set) * sizeof(bool));

    // Find a section with a glob pattern that matches the user agent.
    int num_sections = iniparser_getnsec(b->ini);
    char* current_section_name = NULL;
    for (int section_index = 0; section_index < num_sections; section_index++) {
        char* section_name = iniparser_getsecname(b->ini, section_index);
        if (fnmatch(section_name, user_agent, FNM_CASEFOLD) == 0) {
            current_section_name = section_name;
            break;
        }
    }

    // Collect properties from that section and its parents.
    while (current_section_name != NULL) {
        // Begin the key buffer with the section name, followed by a colon.
        size_t key_length = strlcpy(
            key_buffer, current_section_name, sizeof(key_buffer));
        if (key_length >= sizeof(key_buffer)) {
            fprintf(stderr, "%s\n", "ran out of space in key buffer");
            return br;
        }
        key_length = strlcat(key_buffer, ":", sizeof(key_buffer));
        if (key_length >= sizeof(key_buffer)) {
            fprintf(stderr, "%s\n", "ran out of space in key buffer");
            return br;
        }
        size_t prefix_length = key_length;

        // String properties.
        for (size_t prop_index = 0;
                prop_index < br->num_str_properties;
                prop_index++) {
            if (str_properties_set[prop_index]) {
                continue;
            }

            // Append the property name to the end of the key buffer.
            key_length = strlcat(key_buffer,
                browscap_str_property_names[prop_index],
                sizeof(key_buffer));
            if (key_length >= sizeof(key_buffer)) {
                fprintf(stderr, "%s\n", "ran out of space in key buffer");
                return br;
            }

            char* value = iniparser_getstring(b->ini, key_buffer, NULL);
            if (value != NULL) {
                br->str_properties[prop_index] = value;
                str_properties_set[prop_index] = true;
            }

            // Reset the end of the key buffer to the colon.
            key_buffer[prefix_length] = '\0';
        }

        // Int properties.
        for (size_t prop_index = 0;
                prop_index < br->num_int_properties;
                prop_index++) {
            if (int_properties_set[prop_index]) {
                continue;
            }

            // Append the property name to the end of the key buffer.
            key_length = strlcat(key_buffer,
                browscap_int_property_names[prop_index],
                sizeof(key_buffer));
            if (key_length >= sizeof(key_buffer)) {
                fprintf(stderr, "%s\n", "ran out of space in key buffer");
                return br;
            }

            int not_set = -1; // browscap doesn't use negative int values, so this is safe.
            int value = iniparser_getint(b->ini, key_buffer, not_set);
            if (value != not_set) {
                br->int_properties[prop_index] = (int32_t)value;
                int_properties_set[prop_index] = true;
            }

            // Reset the end of the key buffer to the colon.
            key_buffer[prefix_length] = '\0';
        }

        // Bool properties.
        for (size_t prop_index = 0;
                prop_index < br->num_bool_properties;
                prop_index++) {
            if (bool_properties_set[prop_index]) {
                continue;
            }

            // Append the property name to the end of the key buffer.
            key_length = strlcat(key_buffer,
                browscap_bool_property_names[prop_index],
                sizeof(key_buffer));
            if (key_length >= sizeof(key_buffer)) {
                fprintf(stderr, "%s\n", "ran out of space in key buffer");
                return br;
            }

            int not_set = -1;
            int value = iniparser_getboolean(b->ini, key_buffer, -1);
            if (value != not_set) {
                br->bool_properties[prop_index] = (bool)value;
                bool_properties_set[prop_index] = true;
            }

            // Reset the end of the key buffer to the colon.
            key_buffer[prefix_length] = '\0';
        }

        // Find this section's parent.
        key_length = strlcat(key_buffer, "Parent", sizeof(key_buffer));
        if (key_length >= sizeof(key_buffer)) {
            fprintf(stderr, "%s\n", "ran out of space in key buffer");
            return br;
        }
        current_section_name = iniparser_getstring(b->ini, key_buffer, NULL);
    }

    return br;
}

void browscap_result_free(browscap_result* br) {
    free(br->str_properties);
    free(br->int_properties);
    free(br->bool_properties);
    free(br);
}
