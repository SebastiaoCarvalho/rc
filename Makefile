all:

	gcc -Wall client.cpp -o client
	gcc -Wall server.cpp -o server
	gcc -Wall gameserver.cpp -o gameserver
	gcc -Wall testeclient.cpp -o testeclient