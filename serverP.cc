#include "share.h"
#include "math.h"

#define IMAX 2147483647

using namespace std;

//serverP - Server P will find a social connection that bridges Victor and Oliver with the smallest compatibility gap.

int Bootup_UDP(){
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(LocalHost, ServerP_port, &hints, &servinfo))!=0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    //loop through all the results and bind to the first we can
    for (p=servinfo; p!=NULL; p=p->ai_next){
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1){
            perror("serverP: socket");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("serverP:bind");
            continue;
        }
        break;
    }//from Beej's Guide
    
    if (p==NULL){
        fprintf(stderr, "serverP: failed to bind socket\n");
        return 2;
    }//from Beej's Guide

    freeaddrinfo(servinfo);
    return sockfd; //socket descriptor of UDP
}

//"$0 1$0 2$0 3$1 2$0$"
//Rachael 43#Victor 8#King 3#Oliver 94#
void convert_string_to_map(string &input_name,string &input_graph, unordered_map<int, string> &namemap, unordered_map<string, int> &namemap_str_int, vector<vector<int> > &edgemap, unordered_map<int, int> &score_map){
    //split the input into vector<string>. each string contains several nodes
    //first part is several strings which are edgemap, and the last string is score map
    
    //convert a long string to a map
    vector<string> scoremap_helper{};
    stringstream ss2(input_name);
    string temp;
    while(std::getline(ss2, temp, '#')){
        scoremap_helper.push_back(temp); //{"King 3", "Oliver 94", "Rachael 43"}
    }
    for (int i=0; i<scoremap_helper.size()-1; i++){
        stringstream ss3(scoremap_helper[i]);
        string temp0;
        string temp1;
        std::getline(ss3, temp0, ' ');
        std::getline(ss3, temp1, ' ');
        namemap_str_int[temp0] = i; //map name to int
        namemap[i] = temp0;
        score_map[i] = stoi(temp1); //map int to score
    }

    //split vector<string> into vector<vector<string>>. separate the nodes
    //a big vector contains small vectors. small vector contains one node and its neighbors
    stringstream ss(input_graph);
    vector<string> p{};
    while(std::getline(ss, temp, '$')){
        p.push_back(temp);
    }
    
    int len = p.size();
    vector<vector<int> > edgemap_helper;
    for (int i=0; i<len-1; i++){
        stringstream ss1(p[i]);
        vector<int> pp{};
        while (std::getline(ss1, temp, ' ')){
            pp.push_back(stoi(temp));
        }
        edgemap_helper.push_back(pp);
    }

    for (int i=0; i<score_map.size(); i++){
        edgemap.push_back({i});
    }
    //edgemap, node and its neighbors
    for (auto i:edgemap_helper){
        edgemap[i[0]].push_back(i[1]);
        edgemap[i[1]].push_back(i[0]);
    }
}

//get the shortest path between start node and end node
float get_shortest_path(vector<int> &result, unordered_map<int, int> &score_map, vector<vector<int> > &edgemap, unordered_map<int, string> &namemap, const int &start, const int &end){
    int N = score_map.size();
    float nodes_dist[N][N]; // nodes_dist[i][j]: the distance of node i to node j
    float start_dist[N]; //start_dist[j]: the distance of start node to node j
    int marks[N]; //whehter the node j is been visited
    vector<int> previous(N);
    // INT_MAX: max distance

    //Dijkstra
    //reference: https://www.cnblogs.com/goldsunshine/p/12978305.html
    //vector<vector<int> > record_nodes_path;
    for (int i=0; i<N; i++){
        previous[i] = start;
    }

    for (int i=0; i<N; i++){
        for (int j=0; j<N; j++){
            nodes_dist[i][j] = IMAX;
        }
    }

    for (auto i:edgemap){
        int n1 = i[0];
        for (int j=0; j<i.size(); j++){
            int n2 = i[j];
            nodes_dist[n1][n2] = abs(score_map[n1]-score_map[n2]) / float(score_map[n1]+score_map[n2]);
        }
    }

    for (int i=0; i<N; i++){
        start_dist[i] = nodes_dist[start][i];
        marks[i] = 0;
    }
    marks[start] = 1;

    for (int i=0; i<N; i++){
        //if (i==start) {continue;}
        float min_value = IMAX;
        int node_min = 0;
        for (int j=0; j<N; j++){
            if (!marks[j] && start_dist[j]<min_value){
                min_value = start_dist[j];
                node_min = j;
            }
        }
        marks[node_min]=1;
        //record_nodes_path[node_min].push_back(node_min);

        if (node_min == end){break;}

        for (int j=0; j<N; j++){
            if (j!=node_min && start_dist[j] == start_dist[node_min]+nodes_dist[node_min][j]){
                if (previous[j]!=start && namemap[node_min][0] < namemap[previous[j]][0]){
                    previous[j] = node_min;
                }
            }
            
            if (start_dist[j] > start_dist[node_min]+nodes_dist[node_min][j]){
                //if the current min node can update nodeA's path, implying that current node is a node in the shortes path of node A
                previous[j] = node_min;  
                start_dist[j] = start_dist[node_min]+nodes_dist[node_min][j];
            }
        }

    }
    result.push_back(end);
    int temp = end;
    while (temp != start){
        result.push_back(previous[temp]);
        temp = previous[temp];
    }
    reverse(result.begin(), result.end());

    return start_dist[end];
}

// convert result vector to the output string in order to send the msg to serverC
void convert_vector_to_string(string &output, vector<int> &result, unordered_map<int, string> &namemap){
    output += namemap[result[0]];
    for (int i=1; i<result.size(); i++){
        output += "---";
        output += namemap[result[i]];
    }
    output += "$";

    reverse(result.begin(), result.end());
    output += namemap[result[0]];
    for (int i=1; i<result.size(); i++){
        output += "---";
        output += namemap[result[i]];
    }
    output += "$";
}


int main(void){
    //UDP
    char input_name[2048], input_graph[4096], outputP[Bufferlen];
    struct sockaddr_storage their_addr;
	socklen_t addr_len;
    int recvfd, sendfd; //receive/send descriptor
    int sockfd = Bootup_UDP();
    printf("The ServerP is up and running using UDP on port %s.\n\n", ServerP_port);

    while (1){
        //receive the message from central server
        addr_len = sizeof their_addr;
        if ((recvfd = recvfrom(sockfd, input_name, sizeof input_name , 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("serverP recvfrom serverC");
			exit(1);
		}
        if ((recvfd = recvfrom(sockfd, input_graph, sizeof input_graph, 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("serverP recvfrom serverC");
			exit(1);
		}
        printf("The ServerP received the topology and score information.\n");

        //"Rachael Victor$Rachael King$Rachael Oliver$Victor King$12$Rachael 43#Oliver 94#King 3#Victor 8#$Victor Oliver King"
        if (strlen(input_name)==0) {strcpy(outputP,"");}
        else{
            //get the flag bit
            //string input_temp = input_graph;
            stringstream ss(input_graph);
            //vector<string> p{};
            string temp;
            string flag_bit;
            while(std::getline(ss, temp, '$')){
                flag_bit = temp;
            }

            //get the start and end names
            stringstream ss2(input_name);
            //vector<string> p{};
            string name_inputs; //Victor Oliver
            std::getline(ss2, name_inputs, '$');
            std::getline(ss2, name_inputs, '$');            

            int start, end1, end2;
            unordered_map<int, string> namemap_int_str;
            unordered_map<string, int> namemap_str_int;
            vector<vector<int> > edgemap;
            unordered_map<int, int> score_map;
            string input_N = input_name, input_G = input_graph, output;
            vector<int> result1, result2;
            float compat_score,compat_score2;

            if (flag_bit == "0"){ //A B
                convert_string_to_map(input_N, input_G, namemap_int_str, namemap_str_int, edgemap, score_map);//convert input to edgemap and scoremap

                //split the string by space ' ' to get the usernames
                stringstream ss4(name_inputs);
                std::getline(ss4, temp, ' ');
                start = namemap_str_int[temp];
                std::getline(ss4, temp, ' ');
                end1 = namemap_str_int[temp];
            
                compat_score = get_shortest_path(result1, score_map, edgemap, namemap_int_str, start, end1);

                convert_vector_to_string(output, result1, namemap_int_str);

                char str[20];
                sprintf(str, "%.2f", compat_score);

                output = output+str+"$"+flag_bit;

                strcpy(outputP,output.c_str());
            }
            else if (flag_bit == "1"){ //only clientB1 has connection
                convert_string_to_map(input_N, input_G, namemap_int_str, namemap_str_int, edgemap, score_map);//convert input to edgemap and scoremap

                stringstream ss4(name_inputs);
                std::getline(ss4, temp, ' ');
                start = namemap_str_int[temp];
                std::getline(ss4, temp, ' ');
                end1 = namemap_str_int[temp];
                std::getline(ss4, temp, ' ');
                end2 = namemap_str_int[temp];
            
                compat_score = get_shortest_path(result1, score_map, edgemap, namemap_int_str, start, end1);

                convert_vector_to_string(output, result1, namemap_int_str);

                char str[20];
                sprintf(str, "%.2f", compat_score);

                output = output+str+"$"+flag_bit;

                strcpy(outputP,output.c_str());
            }
            else if (flag_bit =="2"){ //only clientB2 has connection
                convert_string_to_map(input_N, input_G, namemap_int_str, namemap_str_int, edgemap, score_map);//convert input to edgemap and scoremap

                stringstream ss4(name_inputs);
                std::getline(ss4, temp, ' ');
                start = namemap_str_int[temp];
                std::getline(ss4, temp, ' ');
                end1 = namemap_str_int[temp];
                std::getline(ss4, temp, ' ');
                end2 = namemap_str_int[temp];
            
                compat_score = get_shortest_path(result2, score_map, edgemap, namemap_int_str, start, end2);

                convert_vector_to_string(output, result2, namemap_int_str);

                char str[20];
                sprintf(str, "%.2f", compat_score);

                output = output+str+"$"+flag_bit;

                strcpy(outputP,output.c_str());
            }
            else{ //(temp=="12") //clientB1 and clientB2 all have connection
                convert_string_to_map(input_N, input_G, namemap_int_str, namemap_str_int, edgemap, score_map);//convert input to edgemap and scoremap

                stringstream ss4(name_inputs);
                std::getline(ss4, temp, ' ');
                start = namemap_str_int[temp];
                std::getline(ss4, temp, ' ');
                end1 = namemap_str_int[temp];
                std::getline(ss4, temp, ' ');
                end2 = namemap_str_int[temp];

                compat_score = get_shortest_path(result1, score_map, edgemap, namemap_int_str, start, end1);
                compat_score2 = get_shortest_path(result2, score_map, edgemap, namemap_int_str, start, end2);

                convert_vector_to_string(output, result1, namemap_int_str);
                convert_vector_to_string(output, result2, namemap_int_str);

                char str1[20], str2[20];
                sprintf(str1, "%.2f", compat_score);
                sprintf(str2, "%.2f", compat_score2);

                output = output+str1+"$"+str2+"$"+flag_bit;

                strcpy(outputP,output.c_str());
            }
        }

        
        //send the message to the central server
        sockaddr_in addr_serverC;
        addr_serverC.sin_family = AF_INET;
        addr_serverC.sin_addr.s_addr = inet_addr(LocalHost);
        addr_serverC.sin_port = htons(24819); //central port
        sendfd = sendto(sockfd, outputP, sizeof outputP, 0, (struct sockaddr *)&addr_serverC, sizeof(addr_serverC));
        if (sendfd == -1){
            perror("serverP to serverC: sendto");
                exit(1);
        }

        printf("The ServerP finished sending the results to the Central.");
        printf("\n\n"); //the termial won't show the last printf message??
    }

    close(sockfd);
    return 0;

}