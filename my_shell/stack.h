#ifndef STACK_H
#define STACK_H
#include <stddef.h>

#define BAD_MALLOC(p) if ((p) == NULL) {                             \
                        fprintf(stderr, "=====bad malloc in polis\n");\
                        return 1;                                      \
                    }
enum
{
    MUL = 1,
    DEG = 2
};

typedef struct Stack
{
    void *data;
    size_t size;
    size_t cap;
} Stack;


int init_stack(Stack *stack);
void final_stack(Stack *stack);

int pop_stack(Stack *stack, void *resp, size_t size_res);
int push_stack(Stack *stack, const void *data, size_t size_data);

#endif

