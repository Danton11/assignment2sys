/*
 *  chardev.c: Creates a read-only char device that says how many times
 *  you've read from the dev file
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>    /* for put_user */
#include <charDeviceDriver.h>

MODULE_LICENSE("GPL");

DEFINE_MUTEX  (devLock);
//static int index = 0;
//static int i;

/*
 * This function is called when the module is loaded
 * You will not need to modify it for this assignment
 */
int init_module(void)
{
    Major = register_chrdev(0, DEVICE_NAME, &fops);

    if (Major < 0) {
      printk(KERN_ALERT "Registering char device failed with %d\n", Major);
      return Major;
    }

    printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
    printk(KERN_INFO "the driver, create a dev file with\n");
    printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
    printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
    printk(KERN_INFO "the device file.\n");
    printk(KERN_INFO "Remove the device file and module when done.\n");


    return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
    /*  Unregister the device */

    // unallocate mem
    clear(&myStack);
    //destroySack(&myStack);
	printk(KERN_ALERT "%s\n","clean");
    unregister_chrdev(Major, DEVICE_NAME);
}

/*
 * Methods
 */

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
	//printk(KERN_ALERT "%s\n","open");
    mutex_lock (&devLock);
    if (Device_Open) {
    mutex_unlock (&devLock);
    return -EBUSY;
    }
    Device_Open++;
    mutex_unlock (&devLock);
    msg_Ptr = top(&myStack);
    try_module_get(THIS_MODULE);
    
    return SUCCESS;
}

/* Called when a process closes the device file. */
static int device_release(struct inode *inode, struct file *file)
{
    mutex_lock (&devLock);
    Device_Open--;      /* We're now ready for our next caller */
    mutex_unlock (&devLock);
    /* 
     * Decrement the usage count, or else once you opened the file, you'll
     * never get get rid of the module. 
     */
    module_put(THIS_MODULE);
	//printk(KERN_ALERT "%s\n","release");

    return 0;
}

/* 
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 * This is just an example about how to copy a message in the user space
 * You will need to modify this function
 */
static ssize_t device_read(struct file *filp,   /* see include/linux/fs.h   */
               char *buffer,    /* buffer to fill with data */
               size_t length,   /* length of the buffer     */
               loff_t * offset)
{
	//printk(KERN_ALERT "%s\n","read");
    int result;

    if(size==0)
    {
		return -EAGAIN;
	}

    char *newStr = kmalloc(messageSize(top(&myStack)),GFP_KERNEL);
    strcpy(newStr,top(&myStack));
    strcat(newStr,"\n");
    mutex_lock (&devLock);
    result = copy_to_user(buffer,newStr,strlen(newStr)+1);
    mutex_unlock (&devLock);

    if (result != 0) {
        return -EFAULT;
    }
        
    decreaseSize(top(&myStack));
	pop(&myStack);
  

    return strlen(newStr)+1;
}




/* Called when a process writes to dev file: echo "hi" > /dev/hello  */
static ssize_t
device_write(struct file *filp,const char *buff, size_t len, loff_t * offset)
{
	ssize_t msg_len = len;

 	if(msg_len <= 0){
 		return 0;
 	}
	mutex_lock (&devLock);
	char *newMsg = kmalloc(len+1,GFP_KERNEL);
	if(!newMsg){
		return -ENOMEM;
	}
    strlcpy(newMsg,buff,len+1);
 	if(copy_from_user(newMsg,buff,len)){
 		printk(KERN_INFO "EFAULT\n");
 		mutex_unlock (&devLock);
 		kfree(newMsg);
 		return -EFAULT;
 	}
 	mutex_unlock (&devLock);

    if(checkMsgLen(newMsg) == 1){ 
      if(checkStackSize(newMsg) == 1){
        push(&myStack, newMsg);
       	increaseSize(top(&myStack)); 
      }else{ 
      	kfree(newMsg);
      	return -EAGAIN;
      }
    }else{
    	kfree(newMsg);
    	return -EINVAL;
    }
    kfree(newMsg);
    return len;
}
	