/* Necessary includes for device drivers */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */

// For mytimer use
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/timer.h>

//For GPIO use
#include <asm/gpio.h>           /* Needed for gpio calls */
#define MY_GPIO 25

#define TIMER_NUM 100
MODULE_LICENSE("Dual BSD/GPL");

/* Declaration of memory.c functions */
static int mytimer_open(struct inode *inode, struct file *filp);
static int mytimer_release(struct inode *inode, struct file *filp);
static ssize_t mytimer_read(struct file *filp,
		char *buf, size_t count, loff_t *f_pos);
static ssize_t mytimer_write(struct file *filp,
		const char *buf, size_t count, loff_t *f_pos);
static void mytimer_exit(void);
static int mytimer_init(void);

// For my timer
static struct timer_list mytimer[TIMER_NUM];
static char message[TIMER_NUM][128];
static int realdelay[TIMER_NUM];
static char output[130];
static int num = 0;					//Number of timers that are currently active.


/* Structure that declares the usual file */
/* access functions */
struct file_operations mytimer_fops = {
	read: mytimer_read,
	write: mytimer_write,
	open: mytimer_open,
	release: mytimer_release
};

/* Declaration of the init and exit functions */
module_init(mytimer_init);
module_exit(mytimer_exit);

static unsigned capacity = 128;
//static unsigned byte = 128;
module_param(capacity, uint, S_IRUGO);
//module_param(byte, uint, S_IRUGO);

/* Global variables of the driver */
/* Major number */
static int mytimer_major = 61;

/* Buffer to store data */
static char *mytimer_buffer;
/* length of the current message */
static int mytimer_len;

//Declare Method Used when timer expires
void expire(unsigned long arg)
{
	printk(KERN_ALERT "Message : %s\n", message[arg]);
 	gpio_set_value(MY_GPIO,0);
}//end expire

static int mytimer_init(void)
{

	//Get the GPIO
 	int status;
 	if ((status=gpio_request(MY_GPIO,"FINGERS CROSSED"))<0){
 	   printk(KERN_INFO "grio_request failed \n");
 	   return -1;
  	} else {
 	printk(KERN_INFO "got gpio!\n");
 	gpio_direction_output(MY_GPIO,0); // set GPIO direction and initial value
	}
	int result;

	/* Registering device */
	result = register_chrdev(mytimer_major, "mytimer", &mytimer_fops);
	if (result < 0)
	{
		printk(KERN_ALERT
			"mytimer: cannot obtain major number %d\n", mytimer_major);
		return result;
	}

	/* Allocating mytimer for the buffer */
	mytimer_buffer = kmalloc(capacity, GFP_KERNEL); 
	if (!mytimer_buffer)
	{ 
		printk(KERN_ALERT "Insufficient kernel memory\n"); 
		result = -ENOMEM;
		goto fail; 
	} 
	memset(mytimer_buffer, 0, capacity);
	mytimer_len = 0;
	printk(KERN_ALERT "Inserting mytimer module\n");
	return 0;

fail: 
	mytimer_exit(); 
	return result;
}//end init

static void mytimer_exit(void)
{
	//Release GPIO
 	 gpio_set_value(MY_GPIO,0); // turn off GPIO
 	 gpio_free(MY_GPIO);
 	 printk(KERN_INFO "Goodbye, LED\n");

	/* Freeing the major number */
	unregister_chrdev(mytimer_major, "mytimer");

	/* Freeing buffer memory */
	if (mytimer_buffer)
	{
		kfree(mytimer_buffer);
	}

	printk(KERN_ALERT "Removing mytimer module.\n");

}//end exit

static int mytimer_open(struct inode *inode, struct file *filp)
{
	printk(KERN_ALERT "open called: process id %d, command %s\n",
		current->pid, current->comm);
	/* Success */
	return 0;
}//end open

static int mytimer_release(struct inode *inode, struct file *filp)
{
	printk(KERN_ALERT "release called: process id %d, command %s\n",
		current->pid, current->comm);
	/* Success */
	return 0;
}//end release

static ssize_t mytimer_read(struct file *filp, char *buf, 
							size_t count, loff_t *f_pos)
{ 
	//Show that we have read
	printk(KERN_ALERT "read called: process id %, command %s\n",current->pid,current->comm);

	int i;
	signed long remain[100];
	char tbuf[128], *tbptr = tbuf;
	/* end of buffer reached */
	if (*f_pos >= mytimer_len)
	{
		return 0;
	}

	/* do not go over then end */
	if (count > mytimer_len - *f_pos)
		count = mytimer_len - *f_pos;

	if (copy_to_user(buf, mytimer_buffer + *f_pos, count))
	{
		return -EFAULT;
	}
	tbptr += sprintf(tbptr,								   
		"read called: process id %d, command %s, count %d, chars ",
		current->pid, current->comm, count);

	//printk(KERN_INFO "%s\n", tbuf);
	for (i = 0; i < 100; i++)
	{
		remain[i] = (mytimer[i].expires - jiffies)/100;
		if (mytimer[i].expires > jiffies)
		{	
			sprintf(output,"%s\t%lu\n", message[i], remain[i]);
			printk(KERN_ALERT"%s\n", output);
		}
                else continue;
		
	}
	/* Changing reading position as best suits */ 
	*f_pos += count; 
	return count; 
}//end read

static ssize_t mytimer_write(struct file *filp, const char *buf,
				    size_t count, loff_t *f_pos)
{

	printk(KERN_ALERT "write called: process id %, command %s\n",current->pid,current->comm);
	//We received a string and a time to wait until the string is printed.	
	int i=0;
        int j, k;
        int bound;
	int delay[4];
	char tbuf[256], *tbptr = tbuf;
        int swap, flag;
	//If buffer is full, return no process.
        if (*f_pos+count >= capacity)
	{
		printk(KERN_ALERT
			"write called: process id %d, command %s, count %d, buffer full\n",
			current->pid, current->comm, count);
		return -ENOSPC;
	}
	//Copy from the user, get buffer and bring into fpos
	if (copy_from_user(mytimer_buffer + *f_pos, buf, count))
	{
		return -EFAULT;
	}

	tbptr += sprintf(tbptr,								   
		"write called: process id %d, command %s, count %d, chars ",
		current->pid, current->comm, count);

	//printk(KERN_INFO "%s\n", tbuf);
	
	if (num == TIMER_NUM){
                printk(KERN_ALERT "timers are limited, now replacing the 0th one\n");
		flag= 1;
		swap=0;
	}

	// Split the delay in the message
	while (buf[i] != ' ')
	{
		delay[i] = buf[i] - '0';
		i++;
	}
	bound = i + 1;
	// Calculate the delay we need
	realdelay[num] = 0;
	j = 0;
	while (--i >= 0)
	{
		realdelay[num] = delay[j++] + realdelay[num]*10;
	}

	//If there is a message to be displayed with a timer, call the flag and assign a swap variable to that. Break from the loop.
	for (k = 0; k <= num; k++ )
	{
		if (strcmp(buf + bound,message[k]) == 0)
		{
			flag = 1;
			swap = k;

			break;
		}
		else flag = 0;
	}
	//What should be printed when the timer expires.
	sprintf(message[num], "%s", buf + bound);

	
	//If there is something we need to time, initialize a timer.
	if (flag == 1)
	{	

		del_timer(&mytimer[swap]);
		init_timer(&mytimer[swap]);
		mytimer[swap].expires = jiffies + realdelay[num]*100;
		mytimer[swap].data = swap;
		mytimer[swap].function = expire;
		add_timer(&mytimer[swap]);
 		gpio_set_value(MY_GPIO,1);
		flag = 0;
		swap = 0;
	}
	else
	{
 		gpio_set_value(MY_GPIO,1);
		init_timer(&mytimer[num]);
		mytimer[num].expires = jiffies + realdelay[num]*100;
		mytimer[num].data = num;
		mytimer[num].function = expire;
		add_timer(&mytimer[num]);
		num++;
	}
	
	*f_pos += count;
	mytimer_len = *f_pos;

	return count;
}//end write
