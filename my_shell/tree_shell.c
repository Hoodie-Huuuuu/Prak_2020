
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "parsing.h"
#include "tree_shell.h"
#include "stack.h"



int
init_cmd_inf(Cmd_inf *p) {
    p->argv = NULL;
    p->infile = NULL;
    p->outfile = NULL;
    p->backgrnd = 0;
    p->psubcmd = NULL;
    p->pipe = NULL;
    p->next = NULL;
    p->append = 0;
    return 0;
}


#define MESSAGE(str) fprintf(stderr, str "\n")

Cmd_inf *
mk_pipe(Cmd_inf *prev)
{
	Cmd_inf *new = (Cmd_inf *)malloc(sizeof(Cmd_inf));
	if (!new) {
		MESSAGE("mk_pipe: bad allocation memory");
		return NULL;
	}
	init_cmd_inf(new);
	prev->pipe = new;
	return new;
}


Cmd_inf *
next_cmd(Cmd_inf *prev)
{
	
	Cmd_inf *new = (Cmd_inf *)malloc(sizeof(Cmd_inf));
	if (!new) {
		MESSAGE("next_cmd: bad allocation memory");
		return NULL;
	}
	init_cmd_inf(new);
	prev->next = new;
	return new;
}


Cmd_inf *
brack_open(Stack *stack, Cmd_inf *prev)
{
	Cmd_inf *new = (Cmd_inf *)malloc(sizeof(Cmd_inf));
	if (!new) {
		MESSAGE("brack_open: bad allocation memory");
		return NULL;
	}
	init_cmd_inf(new);
	prev->psubcmd = new;

	push_stack(stack, &prev, sizeof(Cmd_inf *));
	return new;
}


Cmd_inf *
brack_close(Stack *stack)
{
	Cmd_inf *opn_brack;
	pop_stack(stack, &opn_brack, sizeof(Cmd_inf *));
	return opn_brack;
}


void
final_tree(Cmd_inf *head)
{	
	if (head == NULL) return;
	if (head->psubcmd != NULL) {
		final_tree(head->psubcmd);
	}
	if (head->next != NULL) {
		final_tree(head->next);
	}
	if (head->pipe != NULL) {
		final_tree(head->pipe);
	}
	if (head->argv != NULL) {
		free(head->argv[0]);
		free(head->argv);
	}
	if (head->infile != NULL) {
		free(head->infile);
	}
	if (head->outfile != NULL) {
		free(head->outfile);
	}
	free(head);
	return;
}


Cmd_inf *
split_line_and_mktree(char *line, int size)
{
	#define END fprintf(stderr, "my_shell: unexpected token '%c'\n", line[i]);\
					 final_stack(&stack); final_tree(head); return NULL
	#define ITSCOMMAND(c) ((c) == '|' || (c) == '>' || \
            (c) == '<' || (c) == ';' || (c) == '&' ||    \
            (c) == '(' || (c) == ')' || (c) == '\0')

	#define NEXT_ARG(c) if (line[i] == '>' && i + 1 < size && line[i + 1] == '>'){\
							c = i + 2;                                             \
						} else { c = i + 1; }


	Cmd_inf *head = malloc(sizeof(Cmd_inf));
	if (!head) {
		return NULL;
	}
	Cmd_inf *cur_cmd = head;
	init_cmd_inf(head);

	Stack stack;
	init_stack(&stack);

	int flag_command = 0; //флаг для отслеживания ошибок (два служебных подряд)
	int flag_operation = 0; //какая именно была команда в 
							//предыдущей итерации
	
	int pos_arg = 0; //позиция с которой начинается аргумент

   	for (int i = 0; i <= size; ++i) {//читаем до служебного символа и смотрим
   									//на псоледний встреченный служебный символ
   		if (ITSCOMMAND(line[i])) {

   			switch (flag_operation) {

   			case START:
   				cur_cmd->argv = split_command(line, pos_arg, size);
   				break;

   			case PIPE://|
   				//после ) может быть команда
   				if (flag_command && line[i] != '('){
   					END;
   				}
   				cur_cmd = mk_pipe(cur_cmd);//создаем ответвление в пайп
   				cur_cmd->argv = split_command(line, pos_arg, size);
 				break;
   			
   			case LESS://<
	   			if (flag_command) {
	   				END;
	   			}	
   				cur_cmd->infile = get_file_name(line, pos_arg, size);
   				break;

   			case MORE://>
   				if (flag_command) {//после > может быть > 
   					END;
   				}					
   				cur_cmd->outfile = get_file_name(line, pos_arg, size);
   				break;

   			case SEMICOLON://;
	   			if (flag_command && line[i] != '(') {
	   				END;
	   			}
   				cur_cmd = next_cmd(cur_cmd);
   				cur_cmd->argv = split_command(line, pos_arg, size);
   				break;

   			case AMPER://&
   				if (flag_command && line[i] != '(' && line[i] != '\0'){
   					END;
   				}
   				cur_cmd->backgrnd = 1;
   				if (line[i] != '\0') {
   					cur_cmd = next_cmd(cur_cmd);
   					cur_cmd->argv = split_command(line, pos_arg, size);
   				}
   				break;

   			case BRACK_OPN:
   				if (flag_command && line[i] != '('){
   					END;
   				}
   				cur_cmd = brack_open(&stack, cur_cmd);
   				cur_cmd->argv = split_command(line, pos_arg, size);
   				break;

   			case BRACK_CLS:
   				if (line[i] == '('){
   					END;
   				}
   				cur_cmd = brack_close(&stack);
   				break;

   			case DUB_MORE:
   				if (flag_command){
   					END;
   				}
   				cur_cmd->outfile = get_file_name(line, pos_arg, size);
   				cur_cmd->append = 1;
   				break;
   			default:
   				break;
   			}
   			NEXT_ARG(pos_arg)
   		}

   		switch (line[i]) {

   		case '|' :
   			flag_operation = PIPE;
   			flag_command = PIPE;
   			if (i == size - 1){
   				END;
   			}
   			break;
   			
   		case '>' :
   			flag_operation = MORE;
   			flag_command = MORE;
	   		if (i == size - 1){
	   			END;
	   		}
   			if (line[i + 1] == '>') {
   				++i;
   				flag_operation = DUB_MORE;
   				flag_command = DUB_MORE;

   			}
   			break;
   			
   		case '<' :
   			flag_operation = LESS;
   			flag_command = LESS;
   			if (i == size - 1) {
   				END;
   			}
   			break;

   		case '&' :
   			flag_operation = AMPER;
   			flag_command = AMPER;
   			break;

   		case ';' :
   			flag_operation = SEMICOLON;
   			flag_command = SEMICOLON;
   			break;

   		case '(' :
   			if (i == size - 1) {
   				END;
   			}
   			flag_operation = BRACK_OPN;
            flag_command = BRACK_OPN;
            break;
   		
        case ')' :
        	flag_operation = BRACK_CLS;
        	flag_command = BRACK_CLS;
        	break;

        case ' ' :
            break;

   		default :
   			flag_command = 0;
   		}
   		
   	}
   	final_stack(&stack);
   	return head;
}
#undef MESSAGE


void
print_tree(Cmd_inf *tree, int i)
{
	if (tree == NULL) return;
	printf("<====%d====>\n", i);
	if (tree->argv != NULL) {
		char *z = tree->argv[0];
        int j = 1;
        while (z != NULL) {
            printf("%s\n", z);
            z = tree->argv[j++];
        }
    } else {
    	printf("argv 0\n");
    }
    if (tree->infile != NULL) {
    	printf("%s\n", tree->infile);
    } else {
    	printf("infile 0\n");
    }
    if (tree->outfile != NULL) {
    	printf("%s\n", tree->outfile);
    } else {
    	printf("outfile 0\n");
    }
    printf("backgrnd == %d\n", tree->backgrnd);
    printf("append == %d\n", tree->append);
    if (tree->psubcmd == NULL) {
    	printf("psubcmd  0\n");
    } else {
    	printf("psubcmd  1\n");
    }
    if (tree->pipe == NULL) {
    	printf("pipe 0\n");
    } else {
    	printf("pipe 1\n");
    }
    if (tree->next == NULL) {
    	printf("next 0\n");
    } else {
    	printf("next 1\n");
    }
    if (tree->psubcmd != NULL) {
    	print_tree(tree->psubcmd, i + 100);
    }
    if (tree->pipe != NULL) {
    	print_tree(tree->pipe, i + 200);
    }
    if (tree->next != NULL) {
    	print_tree(tree->next, ++i);
    }

    return;
}

#define SAFE(k) if ((k) != 0) return 1

int
calculate_tree(Cmd_inf *proc, int in, int out, int flag)
{
	
	if (proc->pipe != NULL && proc->psubcmd != NULL) {
		int fd[2];
		SAFE(pipe(fd/*, O_CLOEXEC*/));
		
    flag = 1;
		SAFE(calculate_tree(proc->pipe, fd[0], out, flag));
		close(fd[0]);

    int pid;
		if ((pid = fork()) == 0) {
			if (proc->infile != NULL) {
				in = open(proc->infile, O_RDONLY, 0666);
			}

			SAFE(calculate_tree(proc->psubcmd, in, fd[1], 0));
			close(fd[1]);
			exit(0);	
		}
		close(fd[1]);
    waitpid(pid, NULL, 0);


	} else if (proc->pipe != NULL) {
		int fd[2];
		SAFE(pipe(fd/*, O_CLOEXEC*/));
		
		if (fork() == 0) {
			dup2(in, 0);
			dup2(fd[1], 1);
			close(fd[1]);
			close(fd[0]);
			execvp(proc->argv[0], proc->argv);
			fprintf(stderr, "my_shell -- %s: No such command\n", proc->argv[0]);
			exit(1);
		}
		close(fd[1]);
		SAFE(calculate_tree(proc->pipe, fd[0], out, 0));
		close(fd[0]);


	} else if (proc->psubcmd != NULL) {
    int pid;
		if ((pid = fork()) == 0) {
			if (proc->outfile != NULL) {
				if (proc->append == 0) {
					out = open(proc->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
				} else {
					out = open(proc->outfile, O_WRONLY | O_CREAT | O_APPEND, 0666);
				}
			}

			if (proc->infile != NULL) {
				in = open(proc->infile, O_RDONLY, 0666);
			}

			SAFE(calculate_tree(proc->psubcmd, in, out, 0));
			if (in != 0) {
				close(in);
			}
			if (out != 1) {
				close(out);
			}
			
			exit(0);	
		}
    if (!proc->backgrnd) {
      waitpid(pid, NULL, 0);
    } 

	} else {
		int pid;
		if ((pid = fork()) == 0) {

			if (proc->outfile != NULL) {
				if (proc->append == 0) {
					out = open(proc->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
				} else {
					out = open(proc->outfile, O_WRONLY | O_CREAT | O_APPEND, 0666);
				}
				
			}
			if (proc->infile != NULL) {
				in = open(proc->infile, O_RDONLY, 0666);
			}
      dup2(out, 1);
      dup2(in, 0);

      if (out != 1) {
        close(out);
      }

      if (in != 0) {
        close(in);
      }

		
			execvp((proc->argv)[0], proc->argv);
			fprintf(stderr, "my_shell -- %s: No such command\n", proc->argv[0]);
			exit(1);
		}
		if (!proc->backgrnd && !flag) {
			waitpid(pid, NULL, 0);
		}	
	}		

	if (proc->next != NULL) {
		SAFE(calculate_tree(proc->next, in, out, 0));
	}

	int status;
    while (waitpid(-1, &status, WNOHANG) > 1) {
    }

    return 0;

}
#undef SAFE