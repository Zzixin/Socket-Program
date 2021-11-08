#include "share.h"

using namespace std;

// serverT - social network graph

void getdata(unordered_map<string, set<string> > &edgemap){
    //original data, including the string of every line
    vector<string> data0;
    ifstream infile;
    infile.open("edgelist.txt");
    //assert(infile.is_open()); // make sure that the file is open
    string temp;
    //char space_char = ' ';
    while (getline(infile,temp)){
        data0.push_back(temp);
    }
    infile.close();

    //vector, contains vector of every node
    vector<vector<string> > data;
    for (auto i:data0){
        stringstream ss(i);
        char space = ' ';
        vector<string> p{};
        while (std::getline(ss, temp, space)){
            //temp.erase(std::remove_if(temp.begin(),temp.end(),ispunct), temp.end());
            p.push_back(temp);
        }
        data.push_back(p);
    }

    //edgemap, node and its neighbors
    for (auto i:data){
        edgemap[i[0]].insert(i[1]);
        edgemap[i[1]].insert(i[0]);
    }
}


//using dfs to find the connected_graph. (graph contains the inputA, and all other nodes connected)
void DFS(string root, unordered_map<string, int> &marks, unordered_map<string, set<string> > &edgemap) {
  marks[root] = 1;
  //std::cout << "visited: " << root << std::endl;
  // Iterate all children
  for (const string node : edgemap[root]) {
    // Visit unmarked ones
    if (marks[node] != 1) {
      DFS(node, marks,edgemap);
    }
  }
}

//conver the edgemap(map) to string(output to central server)
void convert_map_to_string(string &output, unordered_map<string, set<string> > &edgemap){
    string temp;
    for (auto iter=edgemap.cbegin(); iter!=edgemap.cend(); iter++){
        output += (*iter).first;
        output += " ";
        temp.clear();
        for (auto it=(*iter).second.cbegin(); it!=(*iter).second.cend(); it++){
            output += *it;
            output += " ";
        }
        output += "$";
    }
}


//get the connected graph. erase those nodes which are not connected
void get_connected_graph(unordered_map<string, int> &marks, unordered_map<string, set<string> > &edgemap){
    for (auto iter=edgemap.cbegin(); iter!=edgemap.cend(); iter++){
        if (marks[(*iter).first] != 1){
            edgemap.erase(iter);
        }
    }
}

int Bootup_UDP(){
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(LocalHost, ServerT_port, &hints, &servinfo))!=0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    //loop through all the results and bind to the first we can
    for (p=servinfo; p!=NULL; p=p->ai_next){
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1){
            perror("serverT: socket");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("serverT:bind");
            continue;
        }
        break;
    }//from Beej's Guide
    
    if (p==NULL){
        fprintf(stderr, "serverT: failed to bind socket\n");
        return 2;
    }//from Beej's Guide

    freeaddrinfo(servinfo);
    return sockfd; //socket descriptor of UDP
}


int main(void){
    //UDP
    char input[Bufferlen], outputT[Bufferlen];
    struct sockaddr_storage their_addr;
	socklen_t addr_len;
    int recvfd, sendfd; //receive descriptor
    int sockfd = Bootup_UDP();
    printf("The ServerT is up and running using UDP on port %s.\n\n", ServerT_port);
    
    string inputA, inputB, inputB2;
    
    while (1){
        //receive the message from central server
        addr_len = sizeof their_addr;
        if ((recvfd = recvfrom(sockfd, input, Bufferlen-1 , 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

        printf("The ServerT received a request from Central to get the topology.\n");

        string sT = input, temp;
        stringstream ssT(sT);
        vector<std::string> p{};
        while (std::getline(ssT, temp, ' ')){
            p.push_back(temp);
        }

        if (p.size()==2){ //one input of clientB
            //split input to inputA and inputB
            inputA = p[0];
            inputB = p[1];

            //deal with the inputs
            bool flag_connected = 1; //to see if inputA and inputB are connected
            unordered_map<string, set<string> > edgemap;
            unordered_map<string, int> marksA, marksB;
            string output;
            getdata(edgemap);
            DFS(inputA, marksA, edgemap);
            //DFS(inputB, marksB, edgemap);
            //if (marksB[inputA] != 1) {flag_connected = 0;}
            if (marksA[inputB] != 1) {flag_connected = 0;}
            if (flag_connected == 0){output = "";}
            else{
                get_connected_graph(marksA, edgemap);
                convert_map_to_string(output, edgemap);
                output += "0$"; //clientB only has one input
            }
            strcpy(outputT, output.c_str());
        }
        else{ //two inputs of clientB(extra)
            inputA = p[0];
            inputB = p[1];
            inputB2 = p[2];

            //deal with the inputs
            bool flag_connected1 = 1; //to see if inputA and inputB are connected
            bool flag_connected2 = 1;
            unordered_map<string, set<string> > edgemap;
            unordered_map<string, int> marksA, marksB, marksB2;
            string output;

            getdata(edgemap);
            DFS(inputA, marksA, edgemap);
            //DFS(inputB, marksB, edgemap);
            //DFS(inputB2, marksB2, edgemap);
            if (marksA[inputB] != 1) {flag_connected1 = 0;}
            if (marksA[inputB2] != 1) {flag_connected2 = 0;}
            if (flag_connected1==0 && flag_connected2==0){output = "";}
            else if (flag_connected1==0){
                get_connected_graph(marksA, edgemap);
                convert_map_to_string(output, edgemap);
                output += "2$"; //imply that only clientB2 has connection
            }
            else if (flag_connected2==0){
                get_connected_graph(marksA, edgemap);
                convert_map_to_string(output, edgemap);
                output += "1$"; //imply that only clientB1 has connection
            }
            else{
                get_connected_graph(marksA, edgemap);
                convert_map_to_string(output, edgemap);
                output += "12$"; //imply that clientB1 and clientB2 has connection
            }
            strcpy(outputT, output.c_str());
        }

        
        

        //send the char[] message to the central server
        sockaddr_in addr_serverC;
        addr_serverC.sin_family = AF_INET;
        addr_serverC.sin_addr.s_addr = inet_addr(LocalHost);
        addr_serverC.sin_port = htons(24819); //central port
        sendfd = sendto(sockfd, outputT, sizeof outputT, 0, (struct sockaddr *)&addr_serverC, sizeof(addr_serverC));
        if (sendfd == -1){
            perror("serverT to serverC: sendto");
                exit(1);
        }

        printf("The ServerT finished sending the topology to Central.\n");
        printf("\n"); //the termial won't show the last printf message??

    }

    close(sockfd);

    return 0;
}