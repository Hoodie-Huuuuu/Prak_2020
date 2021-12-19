#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include "stack.h"
#include "parsing.h"
#include "tree_shell.h"

#define CHECK(c) if ((c) == NULL) exit(1)
int
main(void)
{
	
	while (1) {
		
		printf("USER$ \n");
		//fflush(stdout);
		int pid;
		if ((pid = fork()) == 0) {
			char *line;
		   	int size;
		   	line = read_line(&size);
		   	CHECK(line);
		   	Cmd_inf *tree = NULL;
		   	if (strlen(line) != 0) {
		   		tree = split_line_and_mktree(line, size);
			   	if (tree == NULL) {
			   		free(line);
			   		exit(1);
			   	}
		   	}
		   	
		   	//int i = 1;
		   	//print_tree(tree, i);
			free(line);
			if (tree != NULL) {
				int flag = calculate_tree(tree, 0, 1, 0);
				final_tree(tree);
				if (flag != 0) {
					exit(1);
				} else {
					exit(0);
				}
			}
			
		}

		wait(NULL);
		
		
		
	}
}


