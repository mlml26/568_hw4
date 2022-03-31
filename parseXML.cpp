#include "parseXML.hpp"
#include <cstring>

void handle_request(char *request, int size) {
  pugi::xml_document doc;
  pugi::xml_parse_result res = doc.load_buffer_inplace(request, size);
  pugi::xml_document response;
  if (!res) {
    cout << "Error in Parsing XML" << endl;
    // response
  }
  if (doc.child("create")) {
    handle_create(doc, response);
  } else if (doc.child("transactions")) {
    // transaction
  } else {
    cout << "Invalid Request" << endl;
    // response
  }
}

void handle_create(pugi::xml_document &doc, pugi::xml_document &response) {
  pugi::xml_node node = response.append_child("result");
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
      // create acount不一定成功后(db方法判断已存在的账户) --> response
      create_account(accountID, balance);
      pugi::xml_node nodeCreated = node.append_child("created");
      nodeCreated.append_attribute("id") = (char *)(&accountID);
    } else if (!strcmp(node.name(), "symbol")) {
      string symbol = node.first_attribute().value();
      int accountID = 0;
      int amount = 0;
      for (pugi::xml_node node : doc.child("symbol")) {
        accountID = stoi(node.first_attribute().value());
        amount = stoi(node.text().data().value());
      }
      // create position不一定成功后(db方法判断已存在的账户) --> response
      create_position(symbol, amount, accountID);
      pugi::xml_node nodeCreated = node.append_child("created");
      nodeCreated.append_attribute("sym") = symbol.c_str();
      nodeCreated.append_attribute("id") = (char *)(&accountID);
    } else {
      cout << "Invalid Request" << endl;
      // response
    }
  }
}
