/* Global definition for the example character device driver */
#include <linux/slab.h>
int init_module(void);
void cleanup_module(void);
void increaseSize(char *mesg);
void decreaseSize(char *mesg);
static int messageSize(char *mesg);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "opsysmem"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 6*1024		/* Max length of the message from the device */
#define SIZE 4*1024*1024
/* 
 * Global variables are declared as static, so are global within the file. 
 */
struct cdev *my_cdev;
       dev_t dev_num;


static int Major;		/* Major number assigned to our device driver */
static int Device_Open = 0;	/* Is device open?  
				 * Used to prevent multiple access to device */
static char msg[BUF_LEN];	/* The msg the device will give when asked */
static unsigned long msg_len = 0;
static char *msg_Ptr;
static int size = 0;
//static char *msgList[LIS_LEN];



static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

void increaseSize(char *mesg){
  size = size + messageSize(mesg);
}

void decreaseSize(char *mesg){
  size = size - messageSize(mesg);
}

int messageSize(char *mesg){
  return strlen(mesg)+1;
}

int checkStackSize(char *mesg){
  if(messageSize(mesg) > (SIZE-size)){
    return -1;
  }else{
    return 1;
  }
}

int checkMsgLen(char *mesg){
  if(strlen(mesg) > BUF_LEN){
  	printk(KERN_INFO "%d\n", messageSize(mesg));
    return -1;
  }else{
    return 1;
  }
}

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
  struct stack_t *stack = kmalloc(sizeof(*stack),GFP_KERNEL);
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
  char *tmp = kmalloc(strlen(str)+1,GFP_KERNEL);
  if (tmp){
    strscpy(tmp, str,strlen(str));
  }

  return tmp;
}

/**
 * Push a value onto the stack
 */
void push(struct stack_t *theStack, char *value)
{
  struct stack_entry *entry = kmalloc(sizeof(*entry),GFP_KERNEL);
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
    kfree(tmp->data);
    kfree(tmp);
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
  kfree(*theStack);
  *theStack = NULL;
}


struct stack_t myStack = {NULL,0};

// struct stack_t *myStack = kmalloc(6*1024*1024,GFP_KERNEL);
// myStack.head = NULL;
// myStack.stackSize = 0;


