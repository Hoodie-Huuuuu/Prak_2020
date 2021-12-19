#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "parsing.h"


//ЧТЕНИЕ
char *
read_line(int *size)
{
	#define SIZE 256
 	int bufsize = SIZE;
  	char *buf = malloc(sizeof(char) * bufsize);
  	
  	if (!buf) {
    	fprintf(stderr, "read_line: error allocation memory\n");
    	return NULL;
  	}

  	int c;
  	int pos = 0;
  	int balance = 0;
  	while (1) {
    	// Читаем символ
    	c = getchar();
    	// При встрече с EOF заменяем его \0 и возвращаем буфер
    	if (c == EOF || c == '\n') {
      		buf[pos] = '\0';
      		*size = pos;
      		if (balance) {//нарушен баланс скобок
      			free(buf);
      			*size = 0;
      			fprintf(stderr, "my_shell: the balance of brackets is broken\n");
	        	return NULL;
      		}
      		return buf;
    	} else {
    		if (c == '(') ++balance;
    		if (c == ')') --balance;
      		buf[pos] = c;
    	}
    	++pos;

    	if (pos >= bufsize) {
      		bufsize += SIZE;
      		buf = realloc(buf, bufsize);
	      	if (!buf) {
	        	fprintf(stderr, "read_line: error realoc\n");
	        	free(buf);
	        	return NULL;
      		}
    	}
  	}
  	#undef SIZE
}


#define BUFSIZE 8
#define MESSAGE(str) fprintf(stderr, str "\n")

#define CMD_SYM(c) (c != '&' && c != '|' && c != '<' && c != '>' && c != ';' &&\
                                                    c != ')' && c != '(')

#define END(c) free(help_str); free(args); return (c)

//Расщепить комманду на аргументы
char **
split_command(char *line, int pos, int size)
{
    
    char *argum;
	char *help_str = (char *)malloc(2 * size * sizeof(char));
    
    
    if (!help_str) {
        MESSAGE("split_command: bad allocation memory");
        return NULL;
    }

    int bufsize = BUFSIZE;
    char **args = (char **)malloc(bufsize * sizeof(char *));
    if (!args) {
        MESSAGE("split_command: bad allocation memory");
        free(help_str);
        return NULL;
    }

    argum = help_str; //первый аргумент команды
    
    while (pos < size && line[pos] == ' ') { //пропускаем первые пробелы
    	++pos;
    }

    if (!CMD_SYM(line[pos]) || pos >= size) {
    	END(NULL);
    }
   
    int position = 0, num_arg = 0;
    while (pos < size && CMD_SYM(line[pos])){

    	if (num_arg >= bufsize) {
            bufsize += BUFSIZE;
            args = realloc(args, bufsize * sizeof(char*));
            if (!args) {
                MESSAGE("split_command: bad realoc memory");
                return NULL;
            }
        }

        //c = line[pos];//первый 100% не служебный и находится в строке
        if (line[pos] != ' ') {
            help_str[position++] = line[pos];
            ++pos;
            if ((pos < size && !CMD_SYM(line[pos])) || line[pos] == '\0') {
            	help_str[position++] = '\0';
            	args[num_arg++] = argum;
            }
        } else {

            help_str[position++] = '\0';
            args[num_arg++] = argum;
            argum = &help_str[position];
            ++pos;

            while (pos < size && (line[pos]) == ' ') { 
            	++pos;   
            }

        }
    }
    args[num_arg] = NULL;
    return args;
}
#undef MESSAGE
#undef BUFSIZE
#undef END



#define NAME_LEN 50
#define MESSAGE(str) fprintf(stderr, str "\n")
#define END(c)  free(help_str); return (c)

char *
get_file_name(char *line, int pos, int size)
{
	int name_len = NAME_LEN;
	char *help_str = (char *)malloc(name_len * sizeof(char));

	if (!help_str) {
        MESSAGE("get_file_name: bad allocation memory");
        return NULL;
    }

    while (pos < size && line[pos] == ' ') { //пропускаем первые пробелы
    	++pos;
    }

    if (pos >= size || !CMD_SYM(line[pos]) ) {
    	END(NULL);
    }

    int i = 0;
    while (pos < size && CMD_SYM(line[pos]) && line[pos] != ' ') {
    	help_str[i++] = line[pos++];
    	if (i == name_len) {
    		name_len *= 2;
    		help_str = (char *)realloc(help_str, name_len);
    	}
    }
    
    help_str[i] = '\0';
    return help_str;

}
#undef NAME_LEN
#undef MESSAGE
#undef END





