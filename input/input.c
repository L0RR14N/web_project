#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "input.h"

char *read_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    char *res = NULL;
    if (f) {
        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);
        rewind(f);
        res = (char *)malloc(size + 1);
        if (res) {
            fread(res, 1, size, f);
            res[size] = '\0';
        }
        fclose(f);
    }
    return res;
}

char *replace_placeholders(const char *template, const char *placeholder, const char *value) {
    char *pos = strstr(template, placeholder);
    if (!pos) return NULL;

    size_t prefix_len = pos - template;
    size_t suffix_len = strlen(pos + strlen(placeholder));

    char *result = malloc(prefix_len + strlen(value) + suffix_len + 1);
    if (!result) return NULL;

    memcpy(result, template, prefix_len);
    memcpy(result + prefix_len, value, strlen(value));
    memcpy(result + prefix_len + strlen(value), pos + strlen(placeholder), suffix_len);
    result[prefix_len + strlen(value) + suffix_len] = '\0';

    return result;
}
