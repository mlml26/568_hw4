#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
using namespace std;
int invoke_client(const char *hostname, const char *port);
int invoke_server(const char *port);
int connection(int client_socket_fd, string * ip);
