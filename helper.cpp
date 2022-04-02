#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <thread>
using namespace std;
int requestNUM = 0;
#define MAX_THREAD 1000
void *handler(void *arg){
  requestNUM++;
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = "vcm-24828.vm.duke.edu";
  const char *port = "12345";
  const char *Filename = (char *)arg;
  //  if (argc < 2) {
  //  cout << "Syntax: client <hostname>\n" << endl;
  //  return 1;
  //}

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;  
  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } //if

  socket_fd = socket(host_info_list->ai_family,
             host_info_list->ai_socktype,
             host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } //if
  
  cout << "Connecting to " << hostname << " on port " << port << "..." << endl;
  
  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } //if
  
  // 
  string File(Filename);
  string lines;
  string s;
  ifstream ifs(File.c_str());
  if(ifs.fail()) {
    cerr << "File to open XML file." << endl;
    exit(EXIT_FAILURE);
  }
  while(getline(ifs, s)){
    lines += s;
  }
  ifs.close();

  //  const char *message = "Good Night! I'm listening to MAMAMOO";
  int length = lines.length();
  //cout << "Size of XML file is :" << length << endl; 
  send(socket_fd, &length, sizeof(length), 0);
  //cout << "Content of lines is :" << lines << endl;
  send(socket_fd, lines.c_str(),lines.length(), 0);
  char buffer[10240];
  int lengthRSP;
  recv(socket_fd, &lengthRSP, sizeof(lengthRSP), 0);
  recv(socket_fd, buffer, lengthRSP, 0);

  string rsp(buffer, buffer + lengthRSP);
  cout << requestNUM<< ": \n" << rsp <<"Don't be ridiculous"<< endl;
  freeaddrinfo(host_info_list);
  close(socket_fd);
  return 0;
}

int main(int argc, char **argv) {
  try {    
    //int MAX_THREAD = stoi(argv[2]);
    int threads[MAX_THREAD];
    pthread_attr_t thread_attr[MAX_THREAD];
    pthread_t thread_ids[MAX_THREAD];
    
    for (int i = 0; i < MAX_THREAD; ++i) {
      threads[i] = pthread_create(&thread_ids[i], NULL, handler, argv[1]);
      usleep(1000);
    }
    for (int i = 0; i < MAX_THREAD; ++i) {
      pthread_join(thread_ids[i], NULL);
    }
  } catch (const std::exception &e) {
    cerr << e.what() << std::endl;
    return 1;
  }
  cout << requestNUM <<endl;
  return 0;
}
