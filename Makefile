all:

	g++ -Wall -g client.cpp utils.cpp -o player
	g++ -Wall -g server.cpp utils.cpp filehandling.cpp -o server
	
