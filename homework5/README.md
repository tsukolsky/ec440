To make the files, run 'make' followd by 'insmod ./sukolsky_hwk5.ko'. Then check to see the major number in 'cat /proc/devices', usually goes to major 249, then 'mknod /dev/sukolskyDriver c 249 0'. This file was checked off by the GTF one week ago, April 5th during the office hours.

To remove a node, execute command 'rmmod <module>'


On April 19th the sukolsky_hwk5.c script was modified to fit the requirements of the second checkoff. Those requirements have been met. The GPIO line used is GPIO25. Currently, a DASH is 3 seconds and a DOT is 1 second. It accepts lowercase characters and signle digit numbers, whcih represent all of morse code values. On some systems it recognizes Upper case letters, however not on the Rasberry Pi. (BeagleBone it does). The mdoule is not named the same. It is known as 'morse-communicator' with major number 69.


To compile, just run make, then 'sudo insmod ./sukolsky_hwk.ko', followed by 'sudo mknod /dev/morse c 69 0'. TO view a default string in terminal, open the file with 'cat /dev/morse', and to see morse code connect GPIO25 to a led-resistor-ground, become root, then 'echo <char> > /dev/morse' and watch. Run dmesg |tail -f for last commands, then echo to the device as root if you wish
