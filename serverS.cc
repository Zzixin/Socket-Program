#include "share.h"

using namespace std;

//serverS - the score of nodes

int Bootup_UDP(){
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(LocalHost, ServerS_port, &hints, &servinfo))!=0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    //loop through all the results and bind to the first we can
    for (p=servinfo; p!=NULL; p=p->ai_next){
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1){
            perror("serverS: socket");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("serverS:bind");
            continue;
        }
        break;
    }//from Beej's Guide
    
    if (p==NULL){
        fprintf(stderr, "serverS: failed to bind socket\n");
        return 2;
    }//from Beej's Guide

    freeaddrinfo(servinfo);
    return sockfd; //socket descriptor of UDP
}

//input: "King Rachael Victor $Oliver Rachael $Rachael King Oliver Victor $Victor King Rachael $ 12"
void convert_string_to_map(string &input, vector<string> &connected_graph){
    //split the input into vector<string>. each string contains several nodes
    stringstream ss(input);
    string temp;
    while(std::getline(ss, temp, ' ')){
        connected_graph.push_back(temp);
    }
}

//get the score map of users
void getdata(unordered_map<string, string> &score_map){
    //original data, including the string of every line
    vector<string> data0;
    ifstream infile;
    infile.open("scores.txt");
    string temp;
    while (getline(infile,temp)){
        data0.push_back(temp);
    }
    infile.close();

    //split the string into nodes string."Victor 1"->"Victor","1"
    vector<string> data;
    for (auto j:data0){
        stringstream ss(j);
        while (std::getline(ss, temp, ' ')){
            data.push_back(temp);
        }
    }

    //get the map
    for (int i=0; i<data.size(); i+=2){
        score_map[data[i]] = data[i+1];
    }
}

//get the scores of nodes need (filter the score map)
void get_scores(unordered_map<string, string> &score_map, vector<string> &connected_graph, vector<vector<string> > &result){
    for (auto vec:connected_graph){
        result.push_back({vec, score_map[vec]});        
    }
}

//conver the scoremap(map) to string(output to central server)
void convert_map_to_string(vector<vector<string> > &result, string &output){
    for (auto vec:result){
        output += vec[0];
        output += " ";
        output += vec[1];
        output += "#";
    }
}

int main(void){
    //UDP
    char inputS[2048], outputS[2048];
    struct sockaddr_storage their_addr;
	socklen_t addr_len;
    int recvfd, sendfd; //receive/send descriptor
    int sockfd = Bootup_UDP();
    printf("The ServerS is up and running using UDP on port %s.\n\n", ServerS_port);

    while (1){
        //receive the message from central server
        addr_len = sizeof their_addr;
        if ((recvfd = recvfrom(sockfd, inputS, sizeof inputS, 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("serverS recvfrom serverC");
			exit(1);
		}

        printf("The ServerS received a request from Central to get the scores.\n");

        //input: "King Rachael Victor $Oliver Rachael $Rachael King Oliver Victor $Victor King Rachael $12"
        //get the socre map of the needed nodes
        if (strlen(inputS)==0) {strcpy(outputS,"");}
        else{
            vector<string> connected_graph;
            unordered_map<string, string> score_map;
            vector<vector<string>> result;
            string input = inputS, output;

            convert_string_to_map(input, connected_graph);//convert input to edgemap
            getdata(score_map); // get the general score map
            get_scores(score_map, connected_graph, result); //filter the score map
            convert_map_to_string(result, output);
            strcpy(outputS,output.c_str());
        }
        

        //send the message to the central server
        sockaddr_in addr_serverC;
        addr_serverC.sin_family = AF_INET;
        addr_serverC.sin_addr.s_addr = inet_addr(LocalHost);
        addr_serverC.sin_port = htons(24819); //central port
        sendfd = sendto(sockfd, outputS, sizeof outputS, 0, (struct sockaddr *)&addr_serverC, sizeof(addr_serverC));
        if (sendfd == -1){
            perror("serverS to serverC: sendto");
                exit(1);
        }

        printf("The ServerS finished sending the scores to Central.");
        printf("\n\n"); //the termial won't show the last printf message??

    }
    
    close(sockfd);
    return 0;
}