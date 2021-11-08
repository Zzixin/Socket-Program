#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <utility> 
#include <set>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <numeric>


// USCID 819
#define ServerT_port "21819" //UDP
#define ServerS_port "22819" //UDP
#define ServerP_port "23819" //UDP
#define Central_port_UDP "24819"
#define Central_port_TCPA "25819"
#define Central_port_TCPB "26819"

#define LocalHost "127.0.0.1"
#define BACKLOG 10 //how many pending connections queue will hold (queue limit)
#define Bufferlen 1024