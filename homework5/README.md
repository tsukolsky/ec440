To make the files, run 'make' followd by 'insmod ./sukolsky_hwk5.ko'. Then check to see the major number in 'cat /proc/devices', usually goes to major 249, then 'mknod /dev/sukolskyDriver c 249 0'. This file was checked off by the GTF one week ago, April 5th during the office hours.

To remove a node, execute command 'rmmod <module>'