/*
 *  chardev.c: Creates a read-only char device that says how many times
 *  you've read from the dev file!
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h> /* kmalloc */
#include <asm/uaccess.h>    /* for put_user */
#include <charDeviceDriver.h>

MODULE_LICENSE("GPL");

/* 
 * This function is called whenever a process tries to do an ioctl on our
 * device file. We get two extra parameters (additional to the inode and file
 * structures, which all device functions get): the number of the ioctl called
 * and the parameter given to the ioctl function.
 *
 * If the ioctl is write or read/write (meaning output is returned to the
 * calling process), the ioctl call returns the output of this function.
 *
 */


DEFINE_MUTEX  (devLock);

/* Node helper functions */

int totalSize(void) {
    
    int total_size = 0;
    message_list *temp;

    list_for_each_entry(temp, &dev_message_list, list) {
        total_size += strlen(temp->message);
    }
    printk(KERN_INFO "Total size: %d\n", total_size);
    return total_size;
}

void freeList(void) {
    message_list *cursor, *temp;
    list_for_each_entry_safe(cursor, temp, &dev_message_list, list) {
        printk(KERN_INFO "Deleting node with message '%s'\n", cursor->message);
        list_del(&cursor->list);
        kfree(cursor);
    }
}


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
    unregister_chrdev(Major, DEVICE_NAME);
    freeList();
    printk(KERN_INFO "Removed module.\n");
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
    try_module_get(THIS_MODULE);    
    return SUCCESS;
}

/* Called when a process closes the device file. */
static int device_release(struct inode *inode, struct file *file)
{
    module_put(THIS_MODULE);
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
    printk(KERN_INFO "Entering device_read()\n");

    if(list_empty(&dev_message_list)) {
        printk(KERN_ALERT "Zero messages stored.\n");
        return -EAGAIN;
    } else {
        message_list *temp = dev_message_list.next;
        char *out_message = temp->message;
        int messageLength = strlen(out_message)+1;
        printk(KERN_INFO "Reading message with length %d\n", strlen(out_message));
        mutex_lock (&devLock);
        int copy = copy_to_user(buffer,out_message,messageLength);
        mutex_unlock (&devLock);
        if(copy != 0)
            return -EFAULT;
        list_del(&temp->list);
        kfree(temp);
        return strlen(out_message);
    }
}



/* Called when a process writes to dev file: echo "hi" > /dev/hello  */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{

    mutex_lock(&devLock);
    int total_size = totalSize();
    mutex_unlock(&devLock);

    printk(KERN_INFO "Entering device_write()\n");
    if(len > 6144) {
        printk(KERN_ALERT "Message size too large.\n");
        return -EINVAL;
    } else if(total_size + len > 4194304) {
        printk(KERN_ALERT "Too little memory available (would be %d bytes)\n", total_size + len);
        return -EAGAIN;
    } else {
        int strLength = (int)len+1;

        char *in_message = kmalloc(strLength, GFP_KERNEL);
        if(!in_message)
            return -ENOMEM;
        mutex_lock(&devLock);
        strlcpy(in_message, buff, strLength);

        printk(KERN_INFO "Writing message (with length %d): %s\n", len, in_message);

        message_list *new_node = (message_list*)kmalloc(sizeof(message_list), GFP_KERNEL);    
        new_node->message=in_message;
        INIT_LIST_HEAD(&new_node->list);
        list_add(&new_node->list, &dev_message_list);    
        mutex_unlock(&devLock);
        return len;
    }
}





