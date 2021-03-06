/* Global definition for the example character device driver */
int init_module(void);
void cleanup_module(void);
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


static int Major;		


typedef struct  message_list 
{
	struct list_head list;
	char *message;
}message_list;

LIST_HEAD(dev_message_list);

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

