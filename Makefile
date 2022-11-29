all:

	g++ -Wall client.cpp utils.cpp -o player
	g++ -Wall server.cpp utils.cpp filehandling.cpp -o server
	
