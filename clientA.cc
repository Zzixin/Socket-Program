#include "share.h"

using namespace std;

int main(int argc, char *argv[]){
    int sockfd, numbytes;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    struct sockaddr_in A_addr;
    socklen_t sin_size; 

    //only 1 name input
    if (argc != 2){
        fprintf(stderr, "usage: ./clientA <username>");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; //IPv4=AF_INET
    hints.ai_socktype = SOCK_STREAM;

    if ((rv=getaddrinfo(LocalHost, Central_port_TCPA, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    //-- from Beej's Guide
    //loop through all the results and connect to the first we can
    for (p=servinfo; p!=NULL; p=p->ai_next){
        //create socket
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1){
            perror("client: socket");
            continue;
        }

        //connect()
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    if (p == NULL){
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    
    //get dynamically assigned portnumber -- from the project pdf
    sin_size = sizeof A_addr;
    int Getsock_check = getsockname(sockfd, (struct sockaddr*)&A_addr, &sin_size);
    if (Getsock_check == -1){
        perror("getsockname");
        exit(1);
    }

    //booting up
    printf("The client is up and running.\n\n");

    //send the input and function to central server
    //message: argv[1]
    numbytes = send(sockfd, argv[1], sizeof argv[1], 0); 
    if (numbytes == -1){
        perror("client: send");
        exit(1);
    }

    //successfully send
    printf("The client sent %s to the Central server.\n\n", argv[1]);


    // receive msg from the central server
	char input[Bufferlen];
	if ((numbytes = recv(sockfd, input, sizeof input, 0)) == -1) {
		perror("receiver from central server");
		exit(1);
	}

    
    //deal with the message
    stringstream ss(input);
    string temp;
    vector<string> record;
    while(std::getline(ss, temp, ' ')){
        record.push_back(temp); 
    }
    
    int len = record.size();
    string last_str = record[len-1];
    if (last_str!="0" && last_str!="1" && last_str!="2" && last_str!="12"){
        if (len ==2){
            printf("Found no compatibility for %s and %s.\n\n", argv[1], record[1].c_str());
        }
        else{
            printf("Found no compatibility for %s and %s.\n", argv[1], record[1].c_str());
            printf("Found no compatibility for %s and %s.\n\n", argv[1], record[2].c_str());
        }
    }
    else{
        //input1 = Victor---Rachael---Oliver Victor---King 1.06 0.45 Victor Oliver King 12;
        if (last_str == "0"){
            printf("Found compatibility for '%s' and '%s':\n", argv[1],record[len-2].c_str());
	        printf("%s\n", record[0].c_str());

            printf("Matching Gap: %s\n",record[1].c_str());
        }
        else if (last_str == "1"){
            //input1 = info_p[0]+" "+info_p[2]+" "+name1+" " + name2 +name3;
            printf("Found compatibility for '%s' and '%s':\n", argv[1],record[3].c_str());
	        printf("%s\n", record[0].c_str());
            printf("Matching Gap: %s\n\n",record[1].c_str());

            printf("Found no compatibility for %s and %s.\n", argv[1], record[4].c_str());

        }
        else if (last_str == "2"){
            printf("Found no compatibility for %s and %s.\n\n", argv[1], record[3].c_str());

            printf("Found compatibility for '%s' and '%s':\n", argv[1],record[4].c_str());
	        printf("%s\n", record[0].c_str());
            printf("Matching Gap: %s\n",record[1].c_str());            
        }
        else{
            //input1 = Victor---Rachael---Oliver Victor---King 1.06 0.45 Victor Oliver King 12;
            printf("Found compatibility for '%s' and '%s':\n", argv[1],record[5].c_str());
	        printf("%s\n", record[0].c_str());
            printf("Matching Gap: %s\n\n",record[2].c_str());

            printf("Found compatibility for '%s' and '%s':\n", argv[1],record[6].c_str());
	        printf("%s\n", record[1].c_str());
            printf("Matching Gap: %s\n",record[3].c_str());   
        }

        printf("\n");
    }
    

    freeaddrinfo(servinfo);
	close(sockfd);

}
