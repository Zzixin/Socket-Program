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
void convert_map_to_string(string &output_name, string &output_graph, vector<vector<string> > &connected_graph){
    unordered_map<string, int> temp_map;
    //output1 - names vector. like a map. the index of the name is its corresponding integer
    //output2 - connected_graph(int integer)
    int count=0; 
    for (auto vec:connected_graph){
        if (temp_map.find(vec[0])==temp_map.end()){
            output_name += vec[0] + " ";
            temp_map[vec[0]] = count;
            count ++;
        }
        if (temp_map.find(vec[1]) == temp_map.end()){
            output_name += vec[1] + " ";
            temp_map[vec[1]] = count;
            count ++;
        }
        output_graph += to_string(temp_map[vec[0]])+ " " + to_string(temp_map[vec[1]]) + "$";
    }
}


//get the connected graph. erase those nodes which are not connected
void get_connected_graph(unordered_map<string, int> &marks, vector<vector<string> > &connected_graph){
    vector<string> data0;
    ifstream infile;
    infile.open("edgelist.txt");
    string temp;
    while (getline(infile,temp)){
        data0.push_back(temp);
    }
    infile.close();

    for (auto i:data0){
        stringstream ss(i);
        char space = ' ';
        vector<string> p{};
        while (std::getline(ss, temp, space)){
            //temp.erase(std::remove_if(temp.begin(),temp.end(),ispunct), temp.end());
            p.push_back(temp);
        }
        if (marks[p[0]] == 1) {connected_graph.push_back(p);}
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
    char input[Bufferlen], output_N[2048], output_G[4096];
    struct sockaddr_storage their_addr;
	socklen_t addr_len;
    int recvfd, sendfd; //receive descriptor
    int sockfd = Bootup_UDP();
    printf("The ServerT is up and running using UDP on port %s.\n\n", ServerT_port);
    
    string inputA, inputB, inputB2;
    
    while (1){
        //receive the message from central server
        addr_len = sizeof their_addr;
        if ((recvfd = recvfrom(sockfd, input, sizeof input, 0,
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
            vector<vector<string> > connected_graph;
            unordered_map<string, int> marksA, marksB;
            string output_name, output_graph;
            getdata(edgemap);
            DFS(inputA, marksA, edgemap);
            //DFS(inputB, marksB, edgemap);
            //if (marksB[inputA] != 1) {flag_connected = 0;}
            if (marksA[inputB] != 1) {flag_connected = 0;}
            if (flag_connected == 0){output_graph = ""; output_name = "";}
            else{
                get_connected_graph(marksA, connected_graph);
                convert_map_to_string(output_name, output_graph, connected_graph);
                output_graph += "0$"; //clientB only has one input
            }
            strcpy(output_N, output_name.c_str());
            strcpy(output_G, output_graph.c_str());
        }
        else{ //two inputs of clientB(extra)
            inputA = p[0];
            inputB = p[1];
            inputB2 = p[2];

            //deal with the inputs
            bool flag_connected1 = 1; //to see if inputA and inputB are connected
            bool flag_connected2 = 1;
            unordered_map<string, set<string> > edgemap;
            vector<vector<string> > connected_graph;
            unordered_map<string, int> marksA, marksB, marksB2;
            string output_name, output_graph;

            getdata(edgemap);
            DFS(inputA, marksA, edgemap);
            //DFS(inputB, marksB, edgemap);
            //DFS(inputB2, marksB2, edgemap);
            if (marksA[inputB] != 1) {flag_connected1 = 0;}
            if (marksA[inputB2] != 1) {flag_connected2 = 0;}
            if (flag_connected1==0 && flag_connected2==0){output_graph = ""; output_name = "";}
            else if (flag_connected1==0){
                get_connected_graph(marksA, connected_graph);
                convert_map_to_string(output_name, output_graph, connected_graph);
                output_graph += "2$"; //imply that only clientB2 has connection
            }
            else if (flag_connected2==0){
                get_connected_graph(marksA, connected_graph);
                convert_map_to_string(output_name, output_graph, connected_graph);
                output_graph += "1$"; //imply that only clientB1 has connection
            }
            else{
                get_connected_graph(marksA, connected_graph);
                convert_map_to_string(output_name, output_graph, connected_graph);
                output_graph += "12$"; //imply that clientB1 and clientB2 has connection
            }
            strcpy(output_N, output_name.c_str());
            strcpy(output_G, output_graph.c_str());
        }

        
        

        //send the char[] message to the central server
        sockaddr_in addr_serverC;
        addr_serverC.sin_family = AF_INET;
        addr_serverC.sin_addr.s_addr = inet_addr(LocalHost);
        addr_serverC.sin_port = htons(24819); //central port
        sendfd = sendto(sockfd, output_N, sizeof output_N, 0, (struct sockaddr *)&addr_serverC, sizeof(addr_serverC));
        if (sendfd == -1){
            perror("serverT to serverC: sendto");
                exit(1);
        }
        sendfd = sendto(sockfd, output_G, sizeof output_G, 0, (struct sockaddr *)&addr_serverC, sizeof(addr_serverC));
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