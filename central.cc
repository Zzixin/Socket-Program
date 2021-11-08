#include "share.h"

//the helper function of Bootup_TCP - from Beej's Guide
void sigchld_handler(int s){
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

//create, bind, listen - TCP of client - some from Beej's Guide
//port_num = Central_port_TCPA or Central_port_TCPB
//return the socket file descriptor
int Bootup_TCP(const char *port_name){
    int sockfd; //the socket file descriptor returned by socket()
    struct addrinfo hints, *servinfo, *p; // hints points to a struct addrinfo, serveinfo points to the results
    struct sigaction sa; 
    int yes = 1;
    int status;

    memset(&hints, 0, sizeof hints); //make sure the struct is empty
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP stream socket

    status = getaddrinfo(LocalHost, port_name, &hints, &servinfo);

    if (status != 0){
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));  
        return 1;  
    }

    //loop through all the results and bing the the first we can
    //Until the three equations are all right, we can go to the next step
    for (p=servinfo; p != NULL; p=p->ai_next){
        sockfd=socket(p->ai_family, p->ai_socktype,p->ai_protocol); //get the file descriptor
        if (sockfd == -1){
            perror("serverC: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("serverC: setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("serverC: bind");
            continue;
        }
        break;
    }//from Beej's Guide

    freeaddrinfo(servinfo); //all done with this structure

    if (p == NULL){
        fprintf(stderr, "serverC: failed to bind\n");
        exit(1);
    }//from Beej's Guide

    //listen
    if (listen(sockfd, BACKLOG) == -1){
        perror("serverC: listen");
        exit(1);
    }//from Beej's Guide

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("sigaction");
        exit(1);
    }//from Beej's Guide

    return sockfd;
}

//create, bind, listen - UDP of server - some from Beej's Guide
int Bootup_UDP(){
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(LocalHost, Central_port_UDP, &hints, &servinfo))!=0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    //loop through all the results and bind to the first we can
    for (p=servinfo; p!=NULL; p=p->ai_next){
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1){
            perror("serverC: socket");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("serverC:bind");
            continue;
        }
        break;
    }//from Beej's Guide
    
    if (p==NULL){
        fprintf(stderr, "serverC: failed to bind socket\n");
        return 2;
    }//from Beej's Guide

    freeaddrinfo(servinfo);
    return sockfd; //socket descriptor of UDP
}


int main(){
    //TCP and UDP socket
    struct sockaddr_storage A_addr, B_addr, server_addr; //connector's address information
    socklen_t sin_size;
    int newfd_clientA, newfd_clientB; //child socket of clients
    int recvfdA, recvfdB, recvfdT, recvfdS, recvfdP; //receiver descriptor
    int sendT; // send descriptor
    char inputA[512], inputB[512], inputT[Bufferlen], inputS[Bufferlen], inputP[Bufferlen];
    
    int tcp_socket_A = Bootup_TCP(Central_port_TCPA); //A
    int tcp_socket_B = Bootup_TCP(Central_port_TCPB); //B
    int udp_socket = Bootup_UDP(); //servers

    printf("The Central server is up and running.\n\n");

    
    //loop - A - some from Beej's Guide
    while (1){
        /*------------------Phase 1------------------*/

        /*------------------connect and receive msg from Client A------------------*/
        sin_size = sizeof A_addr;
        newfd_clientA = accept(tcp_socket_A, (struct sockaddr *)&A_addr, &sin_size);
        if (newfd_clientA == -1){
            perror("ServerC: accept clientA");
            continue;
        }//from Beej's Guide
        recvfdA = recv(newfd_clientA, inputA, sizeof inputA, 0); //receive the msg from Client A - in buffer
        if (recvfdA == -1) {
			perror("ServerC: receive clientA");
			exit(1);
		}
		printf("The Central server received input='%s' from the client using TCP over port %s.\n", inputA, Central_port_TCPA);


        /*------------------connect and receive msg from Client B------------------*/ 
        sin_size = sizeof B_addr;
        newfd_clientB = accept(tcp_socket_B, (struct sockaddr *)&B_addr, &sin_size);
        if (newfd_clientB == -1){
            perror("ServerC: accept clientB");
            continue;
        }//from Beej's Guide
        recvfdB = recv(newfd_clientB, inputB, sizeof inputB, 0); //receive msg from Client B
        if (recvfdB == -1) {
			perror("ServerC: receive clientB");
			exit(1);
		}
        //if inputB contains two names, split them
        std::string temp_inputB = inputB, tempBB;
        std::stringstream ss(temp_inputB);
        std::vector<std::string> inputB_vec{};
        while(std::getline(ss, tempBB, ' ')){
            inputB_vec.push_back(tempBB);
        }
        if (inputB_vec.size()==1){
            printf("The Central server received input='%s' from the client using TCP over port %s.\n", inputB, Central_port_TCPB);
        }
        else{
            printf("The Central server received input='%s' and '%s' from the client using TCP over port %s.\n", inputB_vec[0].c_str(), inputB_vec[1].c_str(), Central_port_TCPB);
        }
		
        /*------------------Phase 2------------------*/

        /*------------------connect and send msg to server T------------------*/
        strcat(inputA, " ");
        strcat(inputA, inputB); //inputA: "Victor Oliver King". put inputB into inputA

        sockaddr_in addr_serverT;
        addr_serverT.sin_family = AF_INET;
        addr_serverT.sin_addr.s_addr = inet_addr(LocalHost);
        addr_serverT.sin_port = htons(21819); //serverT port
        sendT = sendto(udp_socket, inputA, sizeof inputA, 0, (struct sockaddr *)&addr_serverT, sizeof(addr_serverT));
        if (sendT == -1){
            perror("serverC to serverT: sendto");
            exit(1);
        }
        printf("The Central server sent a request to Backend-Server T\n");


        /*------------------receive msg from serverT------------------*/
        sin_size = sizeof server_addr;
        recvfdT = recvfrom(udp_socket, inputT, Bufferlen-1 , 0, (struct sockaddr *)&server_addr, &sin_size);
        if (recvfdT == -1){
            perror("recvfrom serverT");
            exit (1);
        }
        printf("The Central server received information from Backend-Server %s using UDP over port %s.\n", "T", Central_port_UDP);
        //inputT: "King Rachael Victor $Oliver Rachael $Rachael King Oliver Victor $Victor King Rachael $12$"
        //msg from serverT: edgemap(connected graph), and a flag(about whether inputA has connections with inputB)
    

        /*------------------send msg to serverS------------------*/
        sockaddr_in addr_serverS;
        addr_serverS.sin_family = AF_INET;
        addr_serverS.sin_addr.s_addr = inet_addr(LocalHost);
        addr_serverS.sin_port = htons(22819); //serverS port
        sendT = sendto(udp_socket, inputT, sizeof inputT, 0, (struct sockaddr *)&addr_serverS, sizeof(addr_serverS));
        if (sendT == -1){
            perror("serverC to serverS: sendto");
            exit(1);
        }
        printf("The Central server sent a request to Backend-Server S\n");


        /*------------------receive msg from serverS------------------*/
        sin_size = sizeof server_addr;
        recvfdT = recvfrom(udp_socket, inputS, Bufferlen-1 , 0, (struct sockaddr *)&server_addr, &sin_size);
        if (recvfdT == -1){
            perror("recvfrom serverS");
            exit (1);
        }
        printf("The Central server received information from Backend-Server %s using UDP over port %s.\n", "S", Central_port_UDP);
        //inputS="King 3#Oliver 94#Rachael 43#Victor 8#"
        //msg from servers: the scores of the nodes


        /*------------------send msg to serverP------------------*/
        sockaddr_in addr_serverP;
        addr_serverP.sin_family = AF_INET;
        addr_serverP.sin_addr.s_addr = inet_addr(LocalHost);
        addr_serverP.sin_port = htons(23819); //serverP port
        if (strlen(inputT) != 0){ //if =0, means there's no connection
            strcat(inputT, inputS); // Connect the string S to the string T. the result is filled in the serverT
            strcat(inputT,"$");
            strcat(inputT,inputA); //last words in inputT are the inputs from clients
        }
        //inputT="King Rachael Victor $Oliver Rachael $Rachael King Oliver Victor $Victor King Rachael $12$King 3#Oliver 94#Rachael 43#Victor 8#$Victor Oliver King"
        sendT = sendto(udp_socket, inputT, sizeof inputT, 0, (struct sockaddr *)&addr_serverP, sizeof(addr_serverP));
        if (sendT == -1){
            perror("serverC to serverP: sendto");
            exit(1);
        }
        printf("The Central server sent a processing request to Backend-Server P.\n");


        /*------------------receive msg from serverP------------------*/
        sin_size = sizeof server_addr;
        recvfdT = recvfrom(udp_socket, inputP, Bufferlen-1 , 0, (struct sockaddr *)&server_addr, &sin_size);
        if (recvfdT == -1){
            perror("recvfrom serverP");
            exit (1);
        }
        printf("The Central server received the results from backend server P.\n");

        /*------------------Phase 3 and 4------------------*/


        /*------------------process the msg------------------*/
        char outputA[Bufferlen], outputB[Bufferlen];
        //inputP: "Victor---Rachael---Oliver$Oliver---Rachael---Victor$Victor---King$King---Victor$1.06$0.45$12"

        if (strlen(inputP)!=0){
            std::string inputAB = inputP, temp;
            std::string input1, input2, compat_score,compat_score2;
            std::stringstream ss(inputAB);
            std::vector<std::string> info_p{};
            while (std::getline(ss, temp, '$')){
                info_p.push_back(temp);
            }
            //"v--b--v$v--b--v$12"
            int len = info_p.size();
            temp = info_p[len-1];
            if (temp=="0" || temp=="1" || temp=="2"){
                input1 = info_p[0]+" "+info_p[2]+" ";
                input2 = info_p[1]+" "+info_p[2]+" ";

                strcpy(outputA, input1.c_str()); //outputA
                strcat(outputA, inputA);
                strcat(outputA, " ");
                strcat(outputA, temp.c_str());

                strcpy(outputB, input2.c_str()); //outputB
                strcat(outputB, inputA);
                strcat(outputB, " ");
                strcat(outputB, temp.c_str());
    
            }
            else{ //temp = "12"
                input1 = info_p[0]+" "+info_p[2]+" "+info_p[4]+" "+info_p[5]+" ";
                input2 = info_p[1]+" "+info_p[3]+" "+info_p[4]+" "+info_p[5]+" ";

                strcpy(outputA, input1.c_str()); //outputA
                strcat(outputA, inputA);
                strcat(outputA, " ");
                strcat(outputA, temp.c_str());

                strcpy(outputB, input2.c_str()); //outputB
                strcat(outputB, inputA);
                strcat(outputB, " ");
                strcat(outputB, temp.c_str());
            }
        }
        else{
            strcpy(outputA, inputA);
            strcpy(outputB, inputA);
        }
        

        /*------------------send msg to serverA------------------*/
        recvfdA = send(newfd_clientA, outputA, sizeof outputA, 0);
        if (recvfdA == -1){
            perror("central: send to clientA");
            exit(1);
        }
        printf("The Central server sent the results to client %s.\n", "A");
        close(newfd_clientA);


        /*------------------send msg to serverA------------------*/
        recvfdB = send(newfd_clientB, outputB, sizeof outputB, 0);
        if (recvfdB == -1){
            perror("central: send to clientB");
            exit(1);
        }
        printf("The Central server sent the results to client %s.\n", "B");
        printf("\n"); //the termial won't show the last printf message??
        close(newfd_clientB);
    }

    close(tcp_socket_A);
	close(tcp_socket_B);
    close(udp_socket);
	return 0;
}