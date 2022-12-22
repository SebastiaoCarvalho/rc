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

All images used for hint files are stored on a folder called images.

Project made by Sebastião Carvalho 99326 and Frederico Silva 99222