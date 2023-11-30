/**
 * File:	lkmasg2.c
 * Adapted for Linux 5.15 by: John Aedo
 * Class:	COP4600-SP23
 */

#include <linux/module.h>	  // Core header for modules.
#include <linux/device.h>	  // Supports driver model.
#include <linux/kernel.h>	  // Kernel header for convenient functions.
#include <linux/fs.h>		  // File-system support.
#include <linux/uaccess.h>	  // User access copy function support.
#include <linux/mutex.h>
#define DEVICE_NAME "charkmod-in" // Device name.
#define CLASS_NAME "char_in"	  ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");						 ///< The license type -- this affects available functionality
MODULE_AUTHOR("Johnathan Sewell");					 ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("lkmasg2 Kernel Module");           ///< The description -- see modinfo
MODULE_VERSION("0.1");						 ///< A version number to inform users

/**
 * Important variables that store data and keep track of relevant information.
 */
static int major_number;

static struct class *lkmasg2Class = NULL;	///< The device-driver class struct pointer
static struct device *lkmasg2Device = NULL; ///< The device-driver device struct pointer

/**
 * Prototype functions for file operations.
 */
static int open(struct inode *, struct file *);
static int close(struct inode *, struct file *);
static ssize_t write(struct file *, const char *, size_t, loff_t *);

static char tmpBuffer[1024];

typedef struct shared_memory {
    struct mutex lock;
    char BUFFER[1024];
	int BUF_START;
	int BUF_LEN; // this is the length of used buffer space, NOT 1024
} shared_memory;

shared_memory mem;
EXPORT_SYMBOL(mem);

/**
 * File operations structure and the functions it points to.
 */
static struct file_operations fops =
	{
		.owner = THIS_MODULE,
		.open = open,
		.release = close,
		.write = write,
};

/**
 * Initializes module at installation
 */
int init_module(void)
{
	printk(KERN_INFO "charkmod-in: installing module.\n");
	mem.BUF_START = 0;
	mem.BUF_LEN = 0;
    mutex_init(&mem.lock);

	// Allocate a major number for the device.
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0)
	{
		printk(KERN_ALERT "charkmod-in could not register number.\n");
		return major_number;
	}
	printk(KERN_INFO "charkmod-in: registered correctly with major number %d\n", major_number);

	// Register the device class
	lkmasg2Class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(lkmasg2Class))
	{ // Check for error and clean up if there is
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(lkmasg2Class); // Correct way to return an error on a pointer
	}
	printk(KERN_INFO "charkmod-in: device class registered correctly\n");

	// Register the device driver
	lkmasg2Device = device_create(lkmasg2Class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(lkmasg2Device))
	{								 // Clean up if there is an error
		class_destroy(lkmasg2Class); // Repeated code but the alternative is goto statements
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(lkmasg2Device);
	}
	printk(KERN_INFO "charkmod-in Writer module successfully installed\n"); // Made it! device was initialized
	return 0;
}

/*
 * Removes module, sends appropriate message to kernel
 */
void cleanup_module(void)
{
	printk(KERN_INFO "charkmod-in: removing module.\n");
	device_destroy(lkmasg2Class, MKDEV(major_number, 0)); // remove the device
	class_unregister(lkmasg2Class);						  // unregister the device class
	class_destroy(lkmasg2Class);						  // remove the device class
	unregister_chrdev(major_number, DEVICE_NAME);		  // unregister the major number
	printk(KERN_INFO "charkmod-in: Goodbye from the LKM!\n");
	unregister_chrdev(major_number, DEVICE_NAME);
	return;
}

/*
 * Opens device module, sends appropriate message to kernel
 */
static int open(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "charkmod-in: device opened.\n");
	return 0;
}

/*
 * Closes device module, sends appropriate message to kernel
 */
static int close(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "charkmod-in: device closed.\n");
	return 0;
}

/*
 * Writes to the device
 */
static ssize_t write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	int i;
	int BUF_END;

    printk(KERN_INFO "charkmod-in Writer - Entered write()");
    mutex_lock(&mem.lock); 
	printk(KERN_INFO "charkmod-in Writer - Acquired the lock.");
	
	len = len <= 1024? len : 1024;
	if (copy_from_user(tmpBuffer, buffer, len))
	{
		printk(KERN_INFO "charkmod-in Writer - Buffer is full, unable to write.");
		return -1;
	}

	for (i = 0; i < len; i++)
	{
		// buffer is full
		if (mem.BUF_LEN == 1024)
		{
			break;
		}

		// start writing at the end (i.e. next unused index)
		BUF_END = (mem.BUF_START + mem.BUF_LEN) % 1024;
		mem.BUFFER[BUF_END] = tmpBuffer[i];

		// Increase buffer length
		mem.BUF_LEN++;
	}

	printk(KERN_INFO "charkmod-in Writer - Wrote %d bytes (%s) to the buffer.", i, tmpBuffer);
    mutex_unlock(&mem.lock);
	printk(KERN_INFO "charkmod-in Writer - Exiting write() function");
	return i;
}
