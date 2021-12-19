#ifndef TREE_SHELL_H
#define TREE_SHELL_H

#define _GNU_SOURCE

typedef struct cmd_inf {
	char ** argv; // список из имени команды и аргументов //при очитске free(argv[0]) free(argv)
 	char *infile; // переназначенный файл стандартного ввода
 	char *outfile; // переназначенный файл стандартного вывода
 	int backgrnd; // ==1, если команда подлежит выполнению в фоновом режиме
 	struct cmd_inf* psubcmd; // команды для запуска в дочернем shell
 	struct cmd_inf* pipe; // следующая команда после “|”
 	struct cmd_inf* next; // следующая после “;” (или после “&”)
 	int append;
} Cmd_inf;

enum
{
	//const for flag_operation
	START = 0,
	PIPE = 1,     //|
	LESS = 2,     //<
	MORE = 3,     //>
	SEMICOLON = 4,//;
	AMPER = 5,    //&
	BRACK_OPN = 6,//(
	BRACK_CLS = 7,//)
	DUB_MORE = 8  //>>
};

Cmd_inf *split_line_and_mktree(char *line, int size);
void print_tree(Cmd_inf *tree, int i);
void final_tree(Cmd_inf *head);
int calculate_tree(Cmd_inf *proc, int in/*for canals*/, int out/*for (...) */, int flag);


#endif