all:

	g++ -Wall client.cpp -o client
	g++ -Wall server.cpp utils.cpp -o server
	
