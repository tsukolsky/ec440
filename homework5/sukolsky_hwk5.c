/*******************************************************************************\
| homework5_main.c
| Author: Todd Sukolsky
| Initial Build: 4/3/2013
| Last Revised: 4/3/2013
|================================================================================
| Description: This is the base/main module for EC440's homework5. The homework
|	calls for the creation of a character device driver that, when read from
|	will send out a character string in morse code, and when written to will
|	store the string in a static global variable.
|--------------------------------------------------------------------------------
| Revisions:
|	4/3- Initial build. Got everything to work. Small mistake using "copy_from_user"
|	     in the 'read' part, but was able to overcome. Issue is from the return value,
|	     something isn't correct (to small or to large) so it keeps reading. Checked off
|	     for part1.
|================================================================================
| *NOTES: (1)First part of assignment is basic char driver that reads string and writes
|	     string. Second part is adding timing and morse code.
|	  (2)This is a KERNEL MODULE!!!! Steps to compile...
\*******************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for put_user */

/****************************************/
/*	Forward Declarations		*/
/****************************************/
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);


#define SUCCESS 	0
#define DEVICE_NAME	"morseDriver"
#define BUF_LEN		32

/****************************************/
/*		Globals			*/
/****************************************/
static int Major;
static int Device_Open=0;

static char message[BUF_LEN];
static char *message_Ptr;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
	.owner = THIS_MODULE
};

//Module that is called when function is loaded
int init_module(void){
	//Register the device
	Major = register_chrdev(0,DEVICE_NAME,&fops);
	if (Major < 0){
		printk(KERN_ALERT "Registering char device failed with major = %d\n",Major);
		return Major;
	}
	printk("My major number is %d\n",Major);
	strncpy(message,"Miles to go before I sleep.\n\0",32);
	return SUCCESS;
}//end init_module

//Module that is called when the function is unloaded
void cleanup_module(void){
	unregister_chrdev(Major,DEVICE_NAME);
}


//Module called when we try and open (with 'cat').
static int device_open(struct inode *inode,struct file *file){
	if (Device_Open){return -EBUSY;}
	message_Ptr=message;
	Device_Open++;
	return SUCCESS;
}

//Module Called what a process closes the device file
static int device_release(struct inode *inode, struct file *file){
	Device_Open--;
	return 0;
}

//Called when something opens, then tries to read from the module
static ssize_t device_read(struct file *filp,char __user *buffer, size_t length, loff_t * offset){	

/*	int r;
	int L;
	printk("READ:Entering\n");
	L=strlen(message_Ptr);
	r=copy_to_user(buffer,message_Ptr,BUF_LEN);
	printk("READ:Ends with %d characters.\n",L);
	return length;*/
	/*
	 * Number of bytes actually written to the buffer 
	 */

	//THish is second method. First method repeatedly returns the string over and over, infinite looping.
	int bytes_read = 0;
	if (*message_Ptr == 0)
		return 0;

	
	while (length && *message_Ptr) {
		put_user(*(message_Ptr++), buffer++);

		length--;
		bytes_read++;
	}

	return bytes_read;
}

//Called when something tries to write to the device.
static ssize_t device_write(struct file *filp, const char __user *buffer, size_t length, loff_t * offset){
	int r,wr_sz;
	printk("WRITE:Entering\n");
	memset(message,'\0',BUF_LEN);
	if (length <= BUF_LEN){
		wr_sz= length;
	} else {wr_sz=BUF_LEN;}
	
	r=copy_from_user(message_Ptr,buffer,wr_sz);
	printk( KERN_ALERT"WRITE:Rx buf = %s\n",message_Ptr);
	
	return wr_sz;
}



















