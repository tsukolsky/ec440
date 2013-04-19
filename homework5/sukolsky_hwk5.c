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
//General use
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h> 
#include <linux/fs.h> 
#include <linux/errno.h> 
#include <linux/types.h> 
#include <linux/proc_fs.h>
#include <linux/fcntl.h> 
#include <asm/system.h> 
#include <asm/uaccess.h> 
// For timer use
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/timer.h>
//For GPIO use
#include <asm/gpio.h>           /* Needed for gpio calls */

//Defines for various things
#define TIMER_NUM 100
#define MY_GPIO 25
#define SUCCESS 	0
#define DEVICE_NAME	"morse-communicator"
#define BUF_LEN		32
#define desiredMajor 	31337
#define DRIVER_AUTHOR	"Todd Sukolsky"
#define DRIVER_DESC	"This driver is meant to output morse code on an LED attached to GPIO25 on the Rasberry Pi"

/****************************************/
/*	Forward Declarations		*/
/****************************************/
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);
//Define struct
static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
	.owner = THIS_MODULE
};






/****************************************/
/*		Globals			*/
/****************************************/
static int majorNumber = desiredMajor;
static int Device_Open=0;

/* length of the current message */
static int timer_len;
static unsigned capacity=BUF_LEN;
//Define global buffer that holds the message
static char globalBuffer[BUF_LEN];
static char *globalBuffer_Ptr;

// For my timer
static struct timer_list timers[TIMER_NUM];		//array of timers, 100 possible
static int realdelay[TIMER_NUM];			//read delay, 100 long.
static int num = 0;					//Number of timers that are currently active.

//Take care of module license.
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);	
MODULE_DESCRIPTION(DRIVER_DESC);
//Defile module size
module_param(capacity, uint, S_IRUGO);	
/********************************************************************************************************/
/*					Begin Methods							*/
/********************************************************************************************************/


/*******************
expire
********************/
//Declare Method Used when timer expires
void expire(unsigned long arg)
{
	printk(KERN_ALERT "Message : %s\n",globalBuffer_Ptr);				//Print the message in the buffer.		
 	gpio_set_value(MY_GPIO,0);
}//end expire


/*******************
init_module
********************/
//Module that is called when function is loaded
int init_module(void){

	//Get the GPIO
 	int status;
 	if ((status=gpio_request(MY_GPIO,"FINGERS CROSSED"))<0){
 	   	printk(KERN_INFO "grio_request failed \n");
 	   	return -1;
  	} else {
 		printk(KERN_INFO "got gpio!\n");
 		gpio_direction_output(MY_GPIO,0); // set GPIO direction and initial value
	}//end gpio request

	int result=0;
	//Register the device with major number 31337
	result = register_chrdev(majorNumber,DEVICE_NAME,&fops);
	if (result < 0){
		printk(KERN_ALERT "Registering char device failed with major = %d\n",result);
		return result;
	} else {
		printk(KERN_ALERT "Registered character device: %s\n",DEVICE_NAME);
	}
	//Get storacy space and copy a buffer to start with.
	globalBuffer_Ptr = kmalloc(capacity, GFP_KERNEL);
	strncpy(globalBuffer,"Miles to go before I sleep.\n\0",32);

	return SUCCESS;
}//end init_module


/*******************
cleanup_module
********************/
//Module that is called when the function is unloaded
void cleanup_module(void){
	//Release GPIO
 	gpio_set_value(MY_GPIO,0); // turn off GPIO
 	gpio_free(MY_GPIO);
 	printk(KERN_INFO "Goodbye, LED\n");
		
	// Freeing buffer memory 
	if (globalBuffer_Ptr){kfree(globalBuffer_Ptr);}

	//Unregister the device
	printk(KERN_ALERT "Removing %s module.\n",DEVICE_NAME);
	unregister_chrdev(majorNumber,DEVICE_NAME);
}


/*******************
device_open
********************/
//Module called when we try and open (with 'cat').
static int device_open(struct inode *inode,struct file *file){
	if (Device_Open){return -EBUSY;}
	globalBuffer_Ptr=globalBuffer;
	Device_Open++;
	return SUCCESS;
}


/*******************
device_release
********************/
//Module Called what a process closes the device file
static int device_release(struct inode *inode, struct file *file){
	Device_Open--;
	return 0;
}


/*******************
device_read
********************/
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
	if (*globalBuffer_Ptr == 0)
		return 0;

	
	while (length && *globalBuffer_Ptr) {
		put_user(*(globalBuffer_Ptr++), buffer++);

		length--;
		bytes_read++;
	}

	return bytes_read;
}


/*******************
device_write
********************/
//Called when something tries to write to the device.
static ssize_t device_write(struct file *filp, const char __user *buffer, size_t length, loff_t * offset){
	int r,wr_sz;
	printk("WRITE:Entering\n");
	memset(globalBuffer,0,capacity);
	if (length <= BUF_LEN){
		wr_sz= length;
	} else {wr_sz=BUF_LEN;}
	
	if (wr_sz > 1){
		r=copy_from_user(globalBuffer_Ptr,buffer,wr_sz);
	}
	printk( KERN_ALERT"WRITE:Rx buf = %s\n",globalBuffer_Ptr);
	
	realdelay[num]=5;	//however many seconds you want it to wait. Timer interprets this as ms.

	//Here is where we need to set up timers.
	init_timer(&timers[num]);
	timers[num].expires = jiffies + realdelay[num]*100;
	timers[num].data = num;
	timers[num].function = expire;
	add_timer(&timers[num]);
 	gpio_set_value(MY_GPIO,1);
	num++;

	timer_len = *offset;
	return wr_sz;
}



















