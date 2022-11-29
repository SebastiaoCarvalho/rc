all:

	g++ -Wall client.cpp utils.cpp -o client
	g++ -Wall server.cpp utils.cpp -o server
	
