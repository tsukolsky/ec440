/*  
 *  hello-led.c - Demonstrates module documentation.
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */

#include <asm/gpio.h>           /* Needed for gpio calls */

#define DRIVER_AUTHOR "Roscoe Giles <roscoe@bu.edu>"
#define DRIVER_DESC   "A sample driver"


#define MY_GPIO 25


static int __init init_hello_led(void){
  int status;

  printk(KERN_INFO "Hello, LED\n");
  if ((status=gpio_request(MY_GPIO,"FINGERS CROSSED"))<0){
    printk(KERN_INFO "grio_request failed \n");
    return -1;
  } else {
  printk(KERN_INFO "got gpio!\n");
  gpio_direction_output(MY_GPIO,1); // set GPIO direction and initial value

  return 0;
  }
}

static void __exit cleanup_hello_led(void){

  gpio_set_value(MY_GPIO,0); // turn off GPIO
  gpio_free(MY_GPIO);
  printk(KERN_INFO "Goodbye, LED\n");
}

module_init(init_hello_led);
module_exit(cleanup_hello_led);

/*  
 *  You can use strings, like this:
 */

/* 
 * Get rid of taint message by declaring code as GPL. 
 */
MODULE_LICENSE("GPL");

/*
 * Or with defines, like this:
 */
MODULE_AUTHOR(DRIVER_AUTHOR);	/* Who wrote this module? */
MODULE_DESCRIPTION(DRIVER_DESC);	/* What does this module do */

/*  
 *  This module uses /dev/testdevice.  The MODULE_SUPPORTED_DEVICE macro might
 *  be used in the future to help automatic configuration of modules, but is 
 *  currently unused other than for documentation purposes.
 */
MODULE_SUPPORTED_DEVICE("testdevice");



