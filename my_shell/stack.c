#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"

int
init_stack(Stack *stack)
{
    if (stack != NULL){
        stack->data = NULL;
        stack->size = 0;
        stack->cap = 0;
        return 0;
    }
    fprintf(stderr, "=====bad init stack\n");
    return 1;
}


void
final_stack(Stack *stack)
{
    if (stack != NULL){
        free(stack->data);
        stack->size = 0;
        stack->cap = 0;
    }   
}


int
pop_stack(Stack *stack, void *data, size_t pop_size)
{
    char *pcpy;
    if (stack->size < pop_size) {
        fprintf(stderr, "=====error pop_stack\n");
        return 1;
    }
    pcpy = ((char *)stack->data) + stack->size - pop_size;
    memcpy(data, pcpy, pop_size);
    stack->size -= pop_size;
    return 0;

}


int
push_stack(Stack *stack, const void *data, size_t push_size)
{
    char *pcpy;
    if (stack->data == NULL) {
        stack->cap = MUL * push_size;
        stack->data = malloc(stack->cap);
        pcpy = (char *)stack->data;
        BAD_MALLOC(pcpy)
    } else if (stack->cap < stack->size + push_size){
        stack->cap = (stack->cap + push_size)*DEG;
        stack->data = realloc(stack->data, stack->cap);
        BAD_MALLOC(stack->data)
        pcpy = ((char *)stack->data) + stack->size;
    } else {
        pcpy = ((char *)stack->data) + stack->size;
    }
    if (pcpy == NULL) {
            perror("=====bad malloc in stack\n");
            return 1;
    }
    stack->size += push_size;
    memcpy(pcpy, data, push_size);
    return 0;
}


