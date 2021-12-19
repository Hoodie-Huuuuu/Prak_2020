#ifndef PARSING_H
#define PARSING_H


char *read_line(int *size);
char **split_command(char *line, int pos, int size);
char *get_file_name(char *line, int pos, int size);

#endif