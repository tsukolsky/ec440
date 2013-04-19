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
#define TIMER_NUM 	10		//max number of timers needed
#define MY_GPIO 	25
#define SUCCESS 	0
#define DEVICE_NAME	"morse-communicator"
#define BUF_LEN		32
#define DRIVER_AUTHOR	"Todd Sukolsky"
#define DRIVER_DESC	"This driver is meant to output morse code on an LED attached to GPIO25 on the Rasberry Pi"
#define LENGTH_CONSTANT 10
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
//Lookup tables. Duration of DASH is 3(dot). A space between letters in a word is equal to a DASH. Space between dot/dash is a dot. Space between words=7dots
//D represents D
unsigned int numTimers[26]={3,7,7,5,1,7,5,7,3,7,5,7,3,3,5,7,7,5,5,1,5,7,5,7,7,7};
unsigned int a[3]={1,2,5};
unsigned int b[7]={3,4,5,6,7,8,9};
unsigned int c[7]={3,4,5,6,9,10,11};
unsigned int d[5]={3,4,5,6,7};
unsigned int e[1]={1};
unsigned int f[7]={1,2,3,4,7,8,9};
unsigned int g[5]={3,4,7,8,9};
unsigned int h[7]={1,2,3,4,5,6,7};
unsigned int I[3]={1,2,3,};
unsigned int j[7]={1,2,5,6,9,10,13};
unsigned int k[5]={3,4,5,6,9};
unsigned int l[7]={1,2,5,6,7,8,9};
unsigned int m[3]={3,4,7};
unsigned int n[3]={3,4,5};
unsigned int o[5]={3,4,7,8,11};
unsigned int p[7]={1,2,5,6,9,10,11};
unsigned int q[7]={3,4,7,8,9,10,11};
unsigned int R[5]={1,2,5,6,7};
unsigned int s[5]={1,2,3,4,5};
unsigned int t[1]={3};
unsigned int u[5]={1,2,3,4,7};
unsigned int v[7]={1,2,3,4,5,6,9};
unsigned int w[5]={1,2,5,6,9};
unsigned int x[7]={3,4,5,6,7,8,11};
unsigned int y[7]={3,4,5,6,9,10,13};
unsigned int z[7]={3,4,7,8,9,10,11};
unsigned int zero[9]={3,4,7,8,11,12,15,16,19};
unsigned int one[9]={1,2,5,6,9,10,13,14,17};
unsigned int two[9]={1,2,3,4,7,8,11,12,15};
unsigned int three[9]={1,2,3,4,5,6,9,10,13};
unsigned int four[9]={1,2,3,4,5,6,7,8,11};
unsigned int five[9]={1,2,3,4,5,6,7,8,9};
unsigned int six[9]={3,4,5,6,7,8,9,10,11};
unsigned int seven[9]={3,4,7,8,9,10,11,12,13};
unsigned int eight[9]={3,4,7,8,11,12,13,14,15};
unsigned int nine[9]={3,4,7,8,11,12,15,16,19};

//Useful variables
static unsigned int majorNumber = 69;
static unsigned int current_gpio_level=0;
static int Device_Open=0;
/* length of the current message */
static int timer_len;
static unsigned capacity=BUF_LEN;
//Define global buffer that holds the message
static char globalBuffer[BUF_LEN];
static char *globalBuffer_Ptr;

// For my timer
static struct timer_list timers[TIMER_NUM];		//array of timers, 100 possible
static unsigned int numberOfTimers=0;
static unsigned int howManyTimersCompleted=0;

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
	howManyTimersCompleted++;
	if (howManyTimersCompleted==numberOfTimers){
		current_gpio_level=0;
		printk(KERN_ALERT"Finished\n");
		numberOfTimers=0;
		howManyTimersCompleted=0;
	} else {
		current_gpio_level=!current_gpio_level;
		printk(KERN_ALERT"Toggle\n");
	}
//	printk(KERN_ALERT "Message : %s\n",globalBuffer_Ptr);				//Print the message in the buffer.		
	gpio_set_value(MY_GPIO,current_gpio_level);
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
 		current_gpio_level=0;
		gpio_direction_output(MY_GPIO,current_gpio_level); // set GPIO direction and initial value		
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
	current_gpio_level=0;
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
	int r,wr_sz,i=0;
	printk("WRITE:Entering\n");
	wr_sz=length;
	char *string;
	string=kmalloc(2,GFP_KERNEL);
	if (length <=2){
		wr_sz=length;
	} else {wr_sz=2;}
	
	r=copy_from_user(string,buffer,wr_sz);
	
	int whichLetter=(int)string[0];
	if (whichLetter >= 65 && whichLetter <= 90){
		whichLetter+=22;		//converts to lowercase.
	}
	if ((whichLetter >= 48 && whichLetter <= 57) || (whichLetter <=122 && whichLetter >=97)){
		if (whichLetter >=48 && whichLetter <= 57){
			numberOfTimers=9;
		} else {
			numberOfTimers=numTimers[whichLetter-97];
		}
		switch (whichLetter){
			case 48: {
				for (i=0; i<9;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+zero[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 49: {
				for (i=0; i<9;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+one[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 50: {
				for (i=0; i<9;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+two[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 51: {
				for (i=0; i<9;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+three[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 52: {
				for (i=0; i<9;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+four[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 53: {
				for (i=0; i<9;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+five[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 54: {
				for (i=0; i<9;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+six[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 55: {
				for (i=0; i<9;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+seven[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 56: {
				for (i=0; i<9;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+eight[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 57: {
				for (i=0; i<9;i++){
						init_timer(&timers[i]);
					timers[i].expires=jiffies+nine[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);			}
				break;
				}
			case 97: {
				for (i=0; i<3;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+a[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 98: {
				for (i=0; i<7;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+b[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 99: {
				for (i=0; i<7;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+c[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 100: {
				for (i=0; i<5;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+d[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 101: {
				for (i=0; i<1;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+e[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 102: {
				for (i=0; i<7;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+f[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 103: {
				for (i=0; i<5;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+g[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 104: {
				for (i=0; i<7;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+h[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 105: {
				for (i=0; i<3;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+I[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 106: {
				for (i=0; i<7;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+j[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 107: {
				for (i=0; i<5;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+k[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 108: {
				for (i=0; i<7;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+l[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 109: {
				for (i=0; i<3;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+m[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 110: {
				for (i=0; i<3;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+n[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 111: {
				for (i=0; i<5;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+o[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 112: {
				for (i=0; i<7;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+p[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 113: {
				for (i=0; i<7;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+q[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 114: {
				for (i=0; i<5;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+R[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 115: {
				for (i=0; i<5;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+s[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 116: {
				for (i=0; i<1;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+t[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 117: {
				for (i=0; i<5;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+u[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 118: {
				for (i=0; i<7;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+v[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 119: {
				for (i=0; i<5;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+w[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 120: {
				for (i=0; i<7;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+x[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);			}
				break;
				}
			case 121: {
				for (i=0; i<7;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+y[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);				}
				break;
				}
			case 122: {
				for (i=0; i<7;i++){
					init_timer(&timers[i]);
					timers[i].expires=jiffies+z[i]*LENGTH_CONSTANT*10;
					timers[i].data=i;
					timers[i].function=expire;
					add_timer(&timers[i]);	
				}
				break;
				}
			default: break;
		}//end switch
		current_gpio_level=1;
	}//end if we got a valid character
	else {
		current_gpio_level=0;
	}

	howManyTimersCompleted=0;

	kfree(string);
	
	//Bring the level to where it should be
	gpio_set_value(MY_GPIO,current_gpio_level);
	timer_len = *offset;
	return wr_sz;
}



















