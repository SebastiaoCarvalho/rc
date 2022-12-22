RC WORD GAME

This project consists on an application using a client and a server communicating via socket interface.


To run this problem, write "make" on your terminal to compile it and the execute it the following way:

Server
- to run the server write ./server word_file [-p GSport] [-v] , where word_file is the name of your file that contains the words and image names used.
- the server has the following flags:
    
    -v: execute on verbose mode
    -p PORT: select a custom port number for your server to use, default is 58002
    -s: set the reading for the word file to sequential reading. When not used, reading is random.

Player
- to run the player write ./player [-n GSIP] [-p GSport]
- the player has the following flags:
    -p PORT: change the port where your server is, default is 58002
    -n IP: change the IP address where your server is, default is localhost (127.0.0.1)
- the timers for the udp and tcp commands are implemented in the following functions: "rcvMessageUdp" and "readTcp" located on the lines 567 and 803, respectively. To deactivate the timers the variable "tv.tv_sec" must be set to 0 in each function. To increase or deacrese the timeout time, the "tv.tv_sec" and "tv.tv_usec" can be set to bigger values, taking into account that the first one represents time in seconds and the second represents time in microseconds.

All images used for hint files are stored on a folder called images.

Project made by Sebastião Carvalho 99326 and Frederico Silva 99222