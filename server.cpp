#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
//#include <error.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include "implementDB.hpp"
#include "threadpool.hpp"
//#include "pugiconfig.hpp"
using namespace std;
void handle_create(pugi::xml_document &doc, pugi::xml_document &response){
  pugi::xml_node node_res = response.append_child("results");
  for (pugi::xml_node node : doc.child("create")) {
    if (!strcmp(node.name(), "account")) {
      int accountID = 0;
      int balance = 0;
      int attr_index = 0;
      for (pugi::xml_attribute attri : node.attributes()) {
        if (attr_index == 0) {
          accountID = stoi(attri.value());
          attr_index++;
        } else if (attr_index == 1) {
          balance = stoi(attri.value());
        }
      }
      if(create_account(accountID, balance)){
          pugi::xml_node nodeCreated = node_res.append_child("created");
          nodeCreated.append_attribute("id") = accountID;
      }
      else{
          pugi::xml_node nodeError = node_res.append_child("error");
          nodeError.append_attribute("id") = accountID;
          nodeError.text().set("Account already exists");
      }
    }
    else if (!strcmp(node.name(), "symbol")) {
      string symbol = node.first_attribute().value();
      int accountID = 0;
      int amount = 0;
      for (pugi::xml_node symbol_child : node) {
        accountID = stoi(symbol_child.first_attribute().value());
        amount = stoi(symbol_child.text().data().value());
    //}
    cout << "current create symbol:" << accountID << ", " << amount << endl;
    if(create_position(symbol, amount, accountID)) {
      cout << "account exist" << endl;
      pugi::xml_node nodeCreated = node_res.append_child("created");
      nodeCreated.append_attribute("sym") = symbol.c_str();
      nodeCreated.append_attribute("id") = accountID;
    }
    else{
      cout << "account doesn't exist" << endl;
      pugi::xml_node nodeError = node_res.append_child("error");
      nodeError.append_attribute("sym") = symbol.c_str();
      nodeError.append_attribute("id") = accountID;
      nodeError.text().set("Account already exists");
    }
      }
    }
    else {
      cout << "Invalid Request" << endl;
    }
  }
}

void handle_transaction(pugi::xml_document &doc, pugi::xml_document &response) {
  cout << "Calling handle_transaction" << endl;
  // xml_document response store the response
  pugi::xml_node node_res = response.append_child("results");
  int account_id = stoi(doc.child("transactions").first_attribute().value());
  cout << "account_id = " << account_id << endl;
  // check whether account_id exist
  // if not exist, <error id="ACCOUNT_ID">"Invalid account_id"</error>
  for (pugi::xml_node node : doc.child("transactions")) {
    if (!strcmp(node.name(), "order")) {
      cout << "Enter order part" << endl;
      string symbol = "";
      float amount = 0;
      float price = 0;
      int attr_index = 0;
      for (pugi::xml_attribute attri : node.attributes()) {
    if (attr_index == 0) {
      symbol = attri.value();
      attr_index++;
    } else if (attr_index == 1) {
      amount = stoi(attri.value());
      attr_index++;
    } else if (attr_index == 2) {
      price = stoi(attri.value());
    }
      }
      string mes = check_order(account_id, symbol, amount, price);
      const char * mes_char = mes.c_str();
      if (strcmp(mes_char, "Valid.")) {
	pugi::xml_node nodeError = node_res.append_child("error");
	nodeError.append_attribute("sym") = symbol.c_str();
	nodeError.append_attribute("amount") = amount;
	nodeError.append_attribute("limit") = price;
	string ans = check_order(account_id, symbol, amount, price);
	char *ans_char = (char *)ans.c_str();
	nodeError.text().set(ans_char);
	continue;
      }
      int order_id = add_order(account_id, symbol, amount, price);
      pugi::xml_node newNode = node_res.append_child("opened");
      newNode.append_attribute("sym") = symbol.c_str();
      newNode.append_attribute("amount") = amount;
      newNode.append_attribute("limit") = price;
      newNode.append_attribute("id") = order_id;
      //<opened sym="SYM" amount="AMT" limit="LMT" id="TRANS_ID"/>
      //<error sym="SYM" amount="AMT" limit="LMT">Message</error>
      // string symbol = "";
      // int amount = 0;
      // int price = 0;
    }
    else if (!strcmp(node.name(), "query")) {
      cout << "Enter query part" <<    endl;
      int queryId = stoi(node.first_attribute().value());
      query(queryId, node_res);
    }
    else if (!strcmp(node.name(), "cancel")) {
      cout << "Enter cancel part" << endl;
      int order_id = stoi(node.first_attribute().value());
      cancel(order_id, node_res);
    }
    else {
      cout << "Invalid Request" << endl;
    }
  }
}

string handle_request(string request, int size) {
  cout<< "Calling handle_request!" << endl;
  pugi::xml_document doc;
  char * request_char = (char *)request.c_str();
  pugi::xml_parse_result res = doc.load_buffer_inplace(request_char, size);
  pugi::xml_document response;
  if (!res) {
    cout << "Error in Parsing XML" << endl;
  }
  if (doc.child("create")) {
    cout<< "Calling handle_create!" << endl;
    handle_create(doc, response);
  } else if (doc.child("transactions")) {
    handle_transaction(doc, response);
  }
  string str = "";
  stringstream buffer;
  response.save(buffer);
  str = buffer.str();
  // cout << str << endl;
    return str;
}

void server_handle_request(int client_connection_fd){
     int length;
     //&player_id, sizeof(player_id)
     recv(client_connection_fd, &length, sizeof(length), 0);
     char buffer[1024];
     // recv(client_connection_fd, buffer, , 0);
     recv(client_connection_fd, buffer, length, 0);
     // buffer[9] = 0;
     // string s1(st, st + strlen(st));

     string request(buffer, buffer + length);
     //cout << "Request[n-1]:" << request[strlen(buffer) - 1] << endl;
     //cout << "Request(str): " << request << endl;

     string rsp = handle_request(request, length);
     int lengthRSP = rsp.length();
     send(client_connection_fd, &lengthRSP, sizeof(lengthRSP), 0);
     send(client_connection_fd, rsp.c_str(), lengthRSP, 0);
     //cout << "Server received: " << buffer << endl;
     cout << "Response's size : " << lengthRSP << endl;
     cout << rsp << "Don't be ridiculous"<<endl;
     //freeaddrinfo(host_info_list);
     close(client_connection_fd);
}

int main(int argc, char *argv[])
{
  buildDB();
  // create_account(1, 300);
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  const char *port     = "12345";

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags    = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if

  socket_fd = socket(host_info_list->ai_family,
             host_info_list->ai_socktype,
             host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if

  status = listen(socket_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } //if
    
    threadpool runner{50};
    while(1){
        cout << "Waiting for connection on port " << port << endl;
        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        int client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
        if (client_connection_fd == -1) {
            cerr << "Error: cannot accept connection on socket" << endl;
            return -1;
        } // if
	
	runner.commit(server_handle_request, client_connection_fd).get();
        //thread t(server_handle_request, client_connection_fd);
        //t.join();
    }
    close(socket_fd);
    return 0;
}
