#ifndef INPUT_H
#define INPUT_H

char *read_file(const char *filename);
char *replace_placeholders(const char *template, const char *placeholder, const char *value);

#endif  // INPUT_H
