#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static void trim(char* s) {
    char* start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start) + 1);
    char* end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) *end-- = '\0';
}

void config_load_defaults(GameConfig* cfg) {
    cfg->scale = 3;
}

int config_load(GameConfig* cfg, const char* path) {
    config_load_defaults(cfg);

    FILE* f = fopen(path, "r");
    if (!f) {
        return -1;
    }

    char line[256];
    int line_num = 0;
    while (fgets(line, sizeof(line), f)) {
        line_num++;
        trim(line);

        if (line[0] == '\0' || line[0] == '#') continue;

        char* eq = strchr(line, '=');
        if (!eq) {
            fprintf(stderr, "config.cfg:%d: missing '='\n", line_num);
            continue;
        }

        *eq = '\0';
        char* key = line;
        char* value = eq + 1;
        trim(key);
        trim(value);

        if (strcmp(key, "scale") == 0) {
            int val = atoi(value);
            if (val < 1) val = 1;
            if (val > 8) val = 8;
            cfg->scale = val;
        } else {
            fprintf(stderr, "config.cfg:%d: unknown key '%s'\n", line_num, key);
        }
    }

    fclose(f);
    return 0;
}
