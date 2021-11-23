# Socket-Programs
This is a simple socket program project of USC-EE450, including two clients, a central server, and three processing servers.


c. I have successfully completed all the requirements of the project and the optional part. Whether the input of clientB are one username or two usernames, the servers can successfully processed the network and print the correct answers.


d. Code files:
 - share.h: This is the head file of all the programs, including the head files needed in this project.

 - central.cc: This is the central server. 
	Firstly, It receives usernames from clients and sends usernames to serverT.
	Secondly, from serverT, it receives "input_name" char array which contains the names of connected nodes and "input_graph" char array which contains the connected graph. The nodes of connected graph are in integers, the index of string in "input_name" corresponds to the integer in "input_graph".
	Then it sends "input_name" char array to serverS and get the username-score char array. The order of the array won't change.
	After that, it sends username-score and "input_graph" to serverP to get the path.
	Finally, it sends the results received from serverP to two clients.

 - serverT.cc: This is the Topology server. It reads edgelist.txt, receives usernames from central server, finds the connected graph between users and sends the names of connected nodes and the graph back to central server. If there is no connection between users, server T will send "null".

 - serverS.cc: This is the Score server. It reads scores.txt, receives the data of names from central server and sends username-score data back. If graph is null, then it will send "null".

 - serverP.cc: This is the Processing server. It received graph and username-score data from central server and sends the network path with the smallest matching gap back. If graph and score data is null, serverP will realize there is no connection.

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

   7) ./clientA Victor
	The client is up and running.
	The client sent Victor to the Central server.
	Found compatibility for 'Victor' and 'Oliver':
	Victor---Rachael---Oliver
	Matching Gap: 1.06

   8) ./clientB Oliver
	The client is up and running.
	The client sent Oliver to the Central server.
	Found compatibility for 'Oliver' and 'Victor':
	Oliver---Rachael---Victor
	Matching Gap: 1.06

---optional part (extra points)---

   9) ./clientA Victor
	The client is up and running.
	The client sent Victor to the Central server.
	Found compatibility for 'Victor' and 'Oliver':
	Victor---Rachael---Oliver
	Matching Gap: 1.06
	Found compatibility for 'Victor' and 'King':
	Victor---King
	Matching Gap: 0.45

   9) ./clientB Oliver King
	The client is up and running.
	The client sent Oliver and King to the Central server.
	Found compatibility for 'Oliver' and 'Victor':
	Oliver---Rachael---Victor
	Matching Gap: 1.06
	Found compatibility for 'King' and 'Victor':
	King---Victor
	Matching Gap: 0.45


g. I haven't noticed any idiosyncrasy yet.


h. Yes, I used code from "Beej’s Guide to Network Programming". I reused some codes in my int Bootup_TCP(), int Bootup_UDP() and main() functions. (The connection part)


