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
#define DEVICE_NAME "charkmod-out" // Device name.
#define CLASS_NAME "char_out"	  ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");						 ///< The license type -- this affects available functionality
MODULE_AUTHOR("Johnathan Sewell");					 ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("lkmasg2 Kernel Module"); ///< The description -- see modinfo
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
static ssize_t read(struct file *, char *, size_t, loff_t *);

static char tmpBuffer[1024];

typedef struct shared_memory {
    struct mutex lock;
    char BUFFER[1024];
	int BUF_START;
	int BUF_LEN; // this is the length of used buffer space, NOT 1024
} shared_memory;

extern shared_memory mem;

/**
 * File operations structure and the functions it points to.
 */
static struct file_operations fops =
	{
		.owner = THIS_MODULE,
		.open = open,
		.release = close,
		.read = read,
};

/**
 * Initializes module at installation
 */
int init_module(void)
{
	printk(KERN_INFO "charkmod-out: installing module.\n");

	// Allocate a major number for the device.
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0)
	{
		printk(KERN_ALERT "charkmod-out: could not register number.\n");
		return major_number;
	}
	printk(KERN_INFO "charkmod-out: registered correctly with major number %d\n", major_number);

	// Register the device class
	lkmasg2Class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(lkmasg2Class))
	{ // Check for error and clean up if there is
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(lkmasg2Class); // Correct way to return an error on a pointer
	}
	printk(KERN_INFO "charkmod-out: device class registered correctly\n");

	// Register the device driver
	lkmasg2Device = device_create(lkmasg2Class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(lkmasg2Device))
	{								 // Clean up if there is an error
		class_destroy(lkmasg2Class); // Repeated code but the alternative is goto statements
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(lkmasg2Device);
	}
	printk(KERN_INFO "charkmod-out: Reader module successfully installed\n"); // Made it! device was initialized

	return 0;
}

/*
 * Removes module, sends appropriate message to kernel
 */
void cleanup_module(void)
{
	printk(KERN_INFO "charkmod-out: removing module.\n");
	device_destroy(lkmasg2Class, MKDEV(major_number, 0)); // remove the device
	class_unregister(lkmasg2Class);						  // unregister the device class
	class_destroy(lkmasg2Class);						  // remove the device class
	unregister_chrdev(major_number, DEVICE_NAME);		  // unregister the major number
	printk(KERN_INFO "charkmod-out: Goodbye from the charkmod-out!\n");
	unregister_chrdev(major_number, DEVICE_NAME);
	return;
}

/*
 * Opens device module, sends appropriate message to kernel
 */
static int open(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "charkmod-out: device opened.\n");
	return 0;
}

/*
 * Closes device module, sends appropriate message to kernel
 */
static int close(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "charkmod-out: device closed.\n");
	return 0;
}

/*
 * Reads from device, displays in userspace, and deletes the read data
 */
static ssize_t read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int i;

    printk(KERN_INFO "charkmod-out: Reader - Entered read()");
    mutex_lock(&mem.lock); 
	printk(KERN_INFO "charkmod-out: Reader - Acquired the lock.");

	if (mem.BUF_LEN == 0)
	{
		printk(KERN_INFO "charkmod-out: Reader - Buffer is empty, unable to read");
	}

	for (i = 0; i < len; i++)
	{
		// buffer is empty
		if (mem.BUF_LEN == 0)
		{
			printk(KERN_INFO "charkmod-out: Reader - Buffer is now empty, unable to read");
			break;
		}

		// start reading at the start
		tmpBuffer[i] = mem.BUFFER[mem.BUF_START];
		
		// Decrease buffer length and move start up
		mem.BUF_LEN--;
		mem.BUF_START = (mem.BUF_START + 1) % 1024;
	}
	tmpBuffer[i] = '\0';


	if(copy_to_user(buffer, tmpBuffer, i + 1));

	printk(KERN_INFO "charkmod-out: Reader - Read %d bytes (%s) from the buffer.", i, tmpBuffer);
    mutex_unlock(&mem.lock);
	printk(KERN_INFO "charkmod-out: Reader - Exiting read() function");
	return i;
}
