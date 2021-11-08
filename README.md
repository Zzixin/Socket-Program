# Socket-Programs
This is a simple socket program project of USC-EE450, including two clients, a central server, and three processing servers.


c. I have successfully completed the optional part and all the requirements of the project.
 - In phase1, I established the connections between Server C and the Clients as well as all other servers(T, S, P). The server and client programs boot up and the clients can send usernames to the server C.
 - In phase2, server C send the received data to server T to get the connected graph, send the graph to server S to get the scores, and then send the graph and scores to server P to get the network path.
 - In phase3, server C receive the network path from server P and send it to client A and client B. Finally client A and B show the path received on the screen. 
 - In phase4, client B can provide two usernames and the system can compare both usernames with client A username and show the message on the screen.


d. Code files:
 - share.h: This is the head file of all the programs, including the head files needed in this project.

 - central.cc: This is the central server. It receives usernames from clients, sends the data to server T, server S, and server P, and then it will receiver the information(graph, scores, results) from these three servers. After receiving result from server P, it will send to two clients.

 - serverT.cc: This is the Topology server. It reads edgelist.txt, receives usernames from central server, finds the connected graph between users and sends the graph back to central server. If there is no connection between users, server T will send "null".

 - serverS.cc: This is the Score server. It reads scores.txt, receives graph data from central server and sends scores of users back. If graph is null, then it will send "null".

 - serverP.cc: This is the Processing server. It received graph, score, username data from central server and sends the network path with the smallest matching gap back. If graph and score data is null, serverP will realize there is no connection.

 - clientA.cc: This is the client A. It can receive one username and send it to central server. After receiving the network path, it will show it on the screen.

 - clientB.cc: This is the Client B. It can receive either one or two usernames and send it/them to central server. After receiving the network path, it will show it on the screen.


e. I mainly use std::string to deal with the data in the program.
   1) Open 1 terminal and make all
	g++ -o serverC central.cc -std=c++11
	g++ -o serverT serverT.cc -std=c++11
	g++ -o serverS serverS.cc -std=c++11
	g++ -o serverP serverP.cc -std=c++11
	g++ -o clientA clientA.cc -std=c++11
	g++ -o clientB clientB.cc -std=c++11
   2) Open 7 terminals

   3) make serverC
	./serverC
	The Central server is up and running.
	The Central server received input='Victor' from the client using TCP over port 25819.
	The Central server received input='Oliver' from the client using TCP over port 26819.
	The Central server sent a request to Backend-Server T
	The Central server received information from Backend-Server T using UDP over port 24819.
	The Central server sent a request to Backend-Server S
	The Central server received information from Backend-Server S using UDP over port 24819.
	The Central server sent a processing request to Backend-Server P.
	The Central server received the results from backend server P.
	The Central server sent the results to client A.
	The Central server sent the results to client B.

   4) make serverT
	./serverT
	The ServerT is up and running using UDP on port 21819.
	The ServerT received a request from Central to get the topology.
	The ServerT finished sending the topology to Central.

   5) make serverS
	./serverS
	The ServerS is up and running using UDP on port 22819.
	The ServerS received a request from Central to get the scores.
	The ServerS finished sending the scores to Central.

   6) make serverP
	./serverP
	The ServerP is up and running using UDP on port 23819.
	The ServerP received the topology and score information.
	The ServerP finished sending the results to the Central.

   7) ./clientA <username1>
	The client is up and running.
	The client sent Victor to the Central server.
	Found compatibility for 'Victor' and 'Oliver':
	Victor---Rachael---Oliver
	Matching Gap: 1.06

   8) ./clientB <username2>
	The client is up and running.
	The client sent Oliver to the Central server.
	Found compatibility for 'Oliver' and 'Victor':
	Oliver---Rachael---Victor
	Matching Gap: 1.06

   9) ./clientB <username2> <username3>
	The client is up and running.
	The client sent Oliver and Rachael to the Central server.
	Found compatibility for 'Oliver' and 'Victor':
	Oliver---Rachael---Victor
	Matching Gap: 1.06
	Found compatibility for 'Rachael' and 'Victor':
	Rachael---Victor
	Matching Gap: 0.69


g. I haven't noticed any idiosyncrasy yet.


h. Yes, I used code from "Beej’s Guide to Network Programming". I reused some codes in my int Bootup_TCP(), int Bootup_UDP() and main() functions. (The connection part)


