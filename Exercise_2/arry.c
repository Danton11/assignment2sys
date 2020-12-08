#include <stdio.h>
#include <stdlib.h>

#define BUF_LEN 6*1024


static char msg[BUF_LEN];	/* The msg the device will give when asked */
static char *msg_Ptr;
static int size = 0;


#define SIZE 4*1024*1024
#define iterations SIZE/BUF_LEN
#define EMPTY -1

#include <stdlib.h>
#include <string.h>

/**
 * Type for individual stack entry
 */
struct stack_entry {
  char *data;
  struct stack_entry *next;
};

/**
 * Type for stack instance
 */
struct stack_t
{
  struct stack_entry *head;
  size_t stackSize;  // not strictly necessary, but
                     // useful for logging
};

/**
 * Create a new stack instance
 */
struct stack_t *newStack(void)
{
  struct stack_t *stack = malloc(SIZE);
  if (stack)
  {
    stack->head = NULL;
    stack->stackSize = 0;
  }
  return stack;
};

/**
 * Make a copy of the string to be stored (assumes  
 * strdup() or similar functionality is not
 * available
 */
char *copyString(char *str)
{
  char *tmp = malloc(strlen(str) + 1);
  if (tmp)
    strcpy(tmp, str);
  return tmp;
}

/**
 * Push a value onto the stack
 */
void push(struct stack_t *theStack, char *value)
{
  struct stack_entry *entry = malloc(sizeof *entry); 
  if (entry)
  {
    entry->data = copyString(value);
    entry->next = theStack->head;
    theStack->head = entry;
    theStack->stackSize++;
  }
  else
  {
    // handle error here
  }
}

/**
 * Get the value at the top of the stack
 */
char *top(struct stack_t *theStack)
{
  if (theStack && theStack->head)
    return theStack->head->data;
  else
    return NULL;
}

/**
 * Pop the top element from the stack; this deletes both 
 * the stack entry and the string it points to
 */
void pop(struct stack_t *theStack)
{
  if (theStack->head != NULL)
  {
    struct stack_entry *tmp = theStack->head;
    theStack->head = theStack->head->next;
    free(tmp->data);
    free(tmp);
    theStack->stackSize--;
  }
}

/**
 * Clear all elements from the stack
 */
void clear (struct stack_t *theStack)
{
  while (theStack->head != NULL)
    pop(theStack);
}

/**
 * Destroy a stack instance
 */
void destroyStack(struct stack_t **theStack)
{
  clear(*theStack);
  free(*theStack);
  *theStack = NULL;
}

void increaseSize(char *mesg){
  size = size + messageSize(mesg);
}

int messageSize(char *mesg){
  return strlen(mesg) + 1;
}

int checkStackSize(char *mesg){
  if(messageSize(mesg) > ((SIZE)-size)){
    printf("stack too big\n");
    return -1;
  }else{
    return 1;
  }
}



int checkMsgLen(char *mesg){
  if(messageSize(mesg) > BUF_LEN){
    printf("message too big\n");
    return -1;
  }else{
    printf("smaller or same as %d\n",BUF_LEN);
    return 1;
  }
}

int main(void)
{
  struct stack_t *theStack = newStack();
  char *data;

  char *msgg;
  msgg = "aaaaaaaaaaaaaaa";

  // if((checkMsgLen(msgg) == 1) && (checkStackSize(msgg) == 1)){
  //   push(theStack, msgg);  
  // }else{
  //   return 0;
  // }
  // //printf("%s\n",top(theStack));
  // data = top(theStack);
  // increaseSize(data);
  // printf("msg size =%d\n",messageSize(data));
  // printf("stack size =%d\n",size);


  int i;
  for(i=0;i<5000000;i++){
        if(checkMsgLen(msgg) == 1){
          if(checkStackSize(msgg) == 1){
            push(theStack, msgg);  
          }else{
            return 0;
          }
        }else{
          return 0;
        }
        //printf("%s\n",top(theStack));
        data = top(theStack);
        increaseSize(data);
        printf("msg size =%d\n",messageSize(data));
        printf("stack size =%d\n",size);
        printf("---------------------------------------\n");

  }



  clear(theStack);
  destroyStack(&theStack);


  
}