#include "implementDB.hpp"
#include <fstream>
#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <ctime>
connection *C;
int order_id = 0;
int buildDB() {
  try {
    // Establish a connection to the database
    // Parameters: database name, user name, user password
    C = new connection("dbname=mydb user=postgres password=passw0rd");
    if (C->is_open()) {
      cout << "Opened database successfully: " << C->dbname() << endl;
    } else {
      cout << "Can't open database" << endl;
      return 1;
    }
  } catch (const std::exception &e) {
    cerr << e.what() << std::endl;
    return 1;
  }
  createTable("sharesdb.sql");
  return 0;
}

void createTable(char const *fileName) {
  std::string sql;
  ifstream file(fileName);
  std::string line;
  if (file.is_open()) {
    while (getline(file, line)) {
      sql += line;
    }
    file.close();
    execute(sql);
  }
  else {
    std::string fileName_str = fileName;
    cerr << "Unable to open " + fileName_str << endl;
  }
}

void execute(string sql) {
  transaction<serializable,read_write> W(*C); 
  W.exec(sql);
  W.commit();
}

void query(int id, int account_id, pugi::xml_node & rst) {
  //  pugi::xml_node rst;
  string sql1 = "SELECT AMOUNT, PRICE FROM OPENTB WHERE id = " +
    to_string(id) + " AND ACCOUNTID = " + to_string(account_id) + ";";
  string sql2 = "SELECT AMOUNT, PRICE, EPOCH FROM CANCELED WHERE id = " +
               to_string(id) + " AND ACCOUNTID = " + to_string(account_id) + ";";
  string sql3 = "SELECT AMOUNT, PRICE, EPOCH FROM EXECUTED WHERE id = " +
               to_string(id) + " AND ACCOUNTID = " + to_string(account_id) + ";";
  transaction<serializable,read_write> W(*C);
  W.commit();
  nontransaction N(*C);
  result OPENTB(N.exec(sql1));
  result CANCELED(N.exec(sql2));
  result EXECUTED(N.exec(sql3));
  if (OPENTB.empty() && CANCELED.empty() && EXECUTED.empty()) {
    pugi::xml_node err = rst.append_child("error");
    //err.append_attribute("id") = to_string(id).data();
    err.append_child(pugi::node_pcdata).set_value("Invalid Order Id");
    return;
  }
  else {
    auto Status = rst.append_child("status");
    Status.append_attribute("id") = to_string(id).data();
      //new_node.append_attribute("shares") =
    if (!OPENTB.empty()) {
      auto new_node = Status.append_child("open");
      new_node.append_attribute("shares") = OPENTB[0][0].as<string>().data();
    }
    if (!EXECUTED.empty()) {
      auto new_node = Status.append_child("executed");
      new_node.append_attribute("shares") = EXECUTED[0][0].as<string>().data();
      new_node.append_attribute("price") = EXECUTED[0][1].as<string>().data();
      new_node.append_attribute("time") = EXECUTED[0][2].as<string>().data();
    }
    if (!CANCELED.empty()) {
      auto new_node = Status.append_child("canceled");
      new_node.append_attribute("shares") = CANCELED[0][0].as<string>().data();
      new_node.append_attribute("time") = CANCELED[0][2].as<string>().data();
    }
  }
  // return rst;
}

void delete_open(int id) {
  string sql = "DELETE FROM OPENTB WHERE id = " + to_string(id) + ";";
  transaction<serializable,read_write> W(*C);
  //work W(*C);
  W.exec(sql);
  W.commit();
}

void insert_cancel(int id, string symbol, float amount, float price, time_t now, int account_id) {
  //work W(*C);
  transaction<serializable,read_write> W(*C);
  string s = "INSERT INTO CANCELED values(" + to_string(id) + ", " + W.quote(symbol)+ ", " + to_string(amount) + ", " + to_string(price) + ", " + to_string(now) + ", " + to_string(account_id) + ");";
  W.exec(s);
  W.commit();
}

void update_add_account(int account_id, string symbol, float price, float shares) {
  string s;
  if(shares >= 0) {//buy order
    //refund buyer's account for price
    s = "UPDATE ACCOUNT SET BALANCE = BALANCE + " + to_string(shares * price) + " WHERE ACCOUNTID = " + to_string(account_id) + ";";
  }
  else {//sell order
    s = "UPDATE POSITIONTB SET AMOUNT = AMOUNT + " + to_string(shares) + " WHERE ACCOUNTID = " + to_string(account_id) + " AND SYMBOL = '" + symbol + "';";
  }
  transaction<serializable,read_write> W(*C);
  //  work W(*C);
  W.exec(s);
  W.commit();
}
void cancel(int id, int account_ID, pugi::xml_node & answer) {
  cout << "Calling cancel function." << endl;
  //pugi::xml_document res;
  //pugi::xml_node answer = res.append_child("result");
  //pugi::xml_node answer;
  string sql1 = "SELECT * FROM OPENTB WHERE id = " + to_string(id) + " AND ACCOUNTID = " + to_string(account_ID) +";";
  string sql2 = "SELECT AMOUNT, PRICE, EPOCH FROM CANCELED WHERE id = " + to_string(id) + " AND ACCOUNTID = " + to_string(account_ID) +";";
  string sql3 = "SELECT * FROM EXECUTED WHERE id = " + to_string(id) + " AND ACCOUNTID = " + to_string(account_ID) +";";
  work W(*C);
  result OPENTB(W.exec(sql1));
  result CANCELED(W.exec(sql2));
  result EXECUTED(W.exec(sql3));
  W.commit(); //正式提交给数据库
  if (OPENTB.empty()) {
    //cout << "Invalid Order ID" << endl;
    pugi::xml_node err = answer.append_child("error");
    //    err.append_attribute("id") = to_string(id).data();;
    err.append_child(pugi::node_pcdata).set_value("Invalid Order Id");
    //   answer.append_copy(err);
    return;
  }
  pugi::xml_node Status = answer.append_child("canceled");
  Status.append_attribute("id") = to_string(id).data();
  //executed: unchanged, copy it to the xml
  if(!EXECUTED.empty()) {
    for(auto exe: EXECUTED){
      pugi::xml_node new_node = Status.append_child("executed");
      new_node.append_attribute("shares") = exe[2].as<string>().data();
      new_node.append_attribute("price") = exe[3].as<string>().data();
      new_node.append_attribute("time") = exe[4].as<string>().data();
    }
  }
  //open:store and delect it from OPENTB, insert a line into cancelled table, update account balance
  float shares = OPENTB[0][2].as<float>();
  string symbol = OPENTB[0][1].as<string>();
  float price = OPENTB[0][3].as<float>();
  time_t now = time(NULL) + 18000;
  int account_id = OPENTB[0][4].as<int>();
  
  pugi::xml_node share_time = Status.append_child("canceled");
  share_time.append_attribute("shares") = shares;
  time_t curr_time = time(NULL) + 18000;
  share_time.append_attribute("time") = now;
  
  //delect it from openTB
  delete_open(id);
  //insert a line to cancelled
  insert_cancel(id, symbol, shares, price, now, account_id);
  //update account balance
  update_add_account(account_id, symbol, price, shares);
  
  //string response;
  //stringstream  buffer;
  //res.save(buffer);
  //response = buffer.str();
  //return response;
  //return answer;
}
 
result run_query(string sql) {
    transaction<serializable,read_write> W(*C);
    W.commit();
    nontransaction N(*C);
    result R(N.exec(sql));
    return R;
}

void alter_open(float amount, int orderID){
    string sql;
    if(amount > 0){//buyer: execute order: amount(buyer<seller), buyer amount -= amount
        sql =  "UPDATE OPENTB SET AMOUNT = AMOUNT - " + to_string(amount) + " WHERE ID = " + to_string(orderID) + ";";
    }
    else{//seller: execute order: amount
        sql =  "UPDATE OPENTB SET AMOUNT = AMOUNT + " + to_string((-1)*amount) + " WHERE ID = " + to_string(orderID) + ";";
    }
    execute(sql);
}

void add_executed(int OrderID, string symbol, float amount, float price, time_t epoch, int accountID){
  transaction<serializable,read_write> W(*C);
  //work W(*C);
  string sql = "INSERT INTO EXECUTED(ID, SYMBOL, AMOUNT, PRICE, EPOCH, ACCOUNTID) VALUES (" + to_string(OrderID) + ", " + W.quote(symbol) + ", " + to_string(amount) + ", " + to_string(price) + ", " + to_string(epoch) + ", " + to_string(accountID) + ");";
  W.exec(sql);
  W.commit();
  //execute(sql);
}

//已经确定了足够改
//从来没有调用过
void change_balance(int acountID, int balance){
    string sqlStatement;
    if(balance>0){
      sqlStatement =  "UPDATE ACCOUNT SET balance = balance + " + to_string(balance) + " WHERE ID = " + to_string(acountID) + ";";
    }
    execute(sqlStatement);
}

void change_amount(int ID, string symbol, int amount){
  string sqlStatement;
  sqlStatement =  "UPDATE POSITIONTB SET amount = amount + " + to_string(amount) + " WHERE ID = " + to_string(ID) + " AND SYMBOL = '" + symbol + "';";
  execute(sqlStatement);
}

bool create_account(int accountID, float balance) {
    string sqlCheckAccount = "SELECT * FROM ACCOUNT WHERE ACCOUNTID = " + to_string(accountID) + ";";
    result res = run_query(sqlCheckAccount);
    if(res.empty()){
        string sqlStatement = "INSERT INTO ACCOUNT(ACCOUNTID, BALANCE) VALUES (" + to_string(accountID) + ", " + to_string(balance) + ");";
        execute(sqlStatement);
        return true;
    }
    else{
        return false;
    }
}

void update_minus_account(int account_id, string symbol, float price, float shares) {
  string s;
  if(shares >= 0) {//buy order
    //refund buyer's account for price
    s = "UPDATE ACCOUNT SET BALANCE = BALANCE - " + to_string(shares * price) + " WHERE ACCOUNTID = " + to_string(account_id) + ";";
  }
  else {//sell order
    s = "UPDATE POSITIONTB SET AMOUNT = AMOUNT + " + to_string(shares) + " WHERE ACCOUNTID = " + to_string(account_id) + " AND SYMBOL = '" + symbol + "';";
  }
  execute(s);
}
//
bool create_position(string symbol, float amount, int accountID){
  string sqlCheckAccount = "SELECT * FROM ACCOUNT WHERE ACCOUNTID = " + to_string(accountID) + ";";
  result res = run_query(sqlCheckAccount);
  if(!res.empty()) {//accountid exist
    string sql2 = "SELECT * FROM POSITIONTB WHERE ACCOUNTID = " + to_string(accountID) + " AND SYMBOL = '" + symbol + "';";
    result res2 = run_query(sql2);
    string sqlStatement;
    if(!res2.empty()) {//add it to existing position
      sqlStatement = "UPDATE POSITIONTB SET AMOUNT = AMOUNT + " + to_string(amount) + " WHERE ACCOUNT\
ID = " + to_string(accountID) + " AND SYMBOL = '" + symbol + "';";
    }
    else{
      sqlStatement = "INSERT INTO POSITIONTB(SYMBOL, AMOUNT, ACCOUNTID) VALUES ('" + symbol + "', " + to_string(amount) + ", " + to_string(accountID) + ");";
    }
    execute(sqlStatement);
    return true;
  }
  else {
    return false;
  }
}

void execute_order(int sellOrderID, int buyOrderID, string symbol, int price, float amount){
  //  cout<< "in executed_order function, the amount = " << amount << endl;
  string sql_sellerID = "SELECT ACCOUNTID, AMOUNT FROM OPENTB WHERE id = " + to_string(sellOrderID) + ";";
  string sql_buyerInfo = "SELECT ACCOUNTID, AMOUNT, PRICE FROM OPENTB WHERE id = " + to_string(buyOrderID) + ";";
  int sellerID = run_query(sql_sellerID)[0][0].as<int>();
  float sellerAmount = run_query(sql_sellerID)[0][1].as<float>();
  int buyerID = run_query(sql_buyerInfo)[0][0].as<int>();
  float buyerAmount = run_query(sql_buyerInfo)[0][1].as<float>();
  float buyerPrice = run_query(sql_buyerInfo)[0][2].as<float>();
  //卖家+钱; 买家refund
  string sql_addBalance =  "UPDATE ACCOUNT SET balance = balance + " + to_string(price*amount) + " WHERE ACCOUNTID = " + to_string(sellerID) + ";";
  if(buyerPrice>price) {
    sql_addBalance += "UPDATE ACCOUNT SET balance = balance + " + to_string((buyerPrice - price) * amount) + " WHERE ACCOUNTID = " + to_string(buyerID) + ";";
    cout << "REFUND!!!!!!!!!" << endl;
  }
  execute(sql_addBalance);
  //买家+amount
  string sql_buyerPositionID = "SELECT ID FROM POSITIONTB WHERE ACCOUNTID = " + to_string(buyerID) + " AND SYMBOL = '" + symbol + "';";
  result buyerPositionID_res = run_query(sql_buyerPositionID);
  if(buyerPositionID_res.empty()){
    create_position(symbol, amount, buyerID);
  }
  else{
    change_amount(buyerPositionID_res[0][0].as<int>(), symbol, amount);
  }
  //executedTB插入; openTB删减/修改amount
  time_t curr_time = time(NULL);
  add_executed(sellOrderID, symbol, amount, price, curr_time, sellerID);
  add_executed(buyOrderID, symbol, -amount, price, curr_time, buyerID);
  if(buyerAmount>amount) {
    alter_open(amount, buyOrderID);
  }
  else if(buyerAmount==amount){
    delete_open(buyOrderID);
  }
  if((-1)*sellerAmount>amount) {
    alter_open((-1)*amount,sellOrderID);
  }
  else if((-1)*sellerAmount==amount) {
    delete_open(sellOrderID);
  }
}




string check_order(int account_id, string symbol, float shares, int price) {
  //account里的balance >= shares * price
  //symbol: position里对应account_id有symbol
  //shares<0: seller-> amount > -shares
  //shares>0: buyer-> balance >= amount * price
  string sql1 = "SELECT BALANCE FROM ACCOUNT WHERE ACCOUNTID = " + to_string(account_id) + ";";
  result R1 = run_query(sql1);
  string sql2 = "SELECT AMOUNT FROM POSITIONTB WHERE ACCOUNTID = " + to_string(account_id) + " AND SYMBOL = '" + symbol + "';";
  result R2 = run_query(sql2);
  if(shares < 0) { //seller
    if(R2.empty()) return "Seller doesn't exit or doesn't have such symbol.";
    float amount = R2[0][0].as<float>();
    //shares<0: seller-> amount > -shares
    if(amount > -shares) return "Valid.";
    else return "Seller doesn't have enough amount symbol.";
  }
  else if(shares > 0) { //buyer
    if(R1.empty()) return "Buyer doesn't exist.";
    float Balance = R1[0][0].as<float>();
    //shares > 0: buyer-> balance >= amount * price
    if(Balance >= shares * price) return "Valid.";
    else return "Buyer doesn't have enough balance";
  }
  return "Invalid amount.";
}

result search_seller(string symbol, int price) {
  string sql = "SELECT ID, SYMBOL, AMOUNT, PRICE FROM OPENTB WHERE SYMBOL = '" + symbol + "' AND PRICE <= " + to_string(price) + "AND AMOUNT < 0 ORDER BY PRICE ASC;";
  return run_query(sql);
}
result search_buyer(string symbol, int price) {
  string sql = "SELECT ID, SYMBOL, AMOUNT, PRICE FROM OPENTB WHERE SYMBOL = '" + symbol + "' AND PRICE >= " + to_string(price) + "AND AMOUNT > 0 ORDER BY PRICE DESC;";
  return run_query(sql);
}
//I am a buyer.
void match_seller_order(int order_id, string symbol, float shares, float price) {
    result R = search_seller(symbol, price);
    if(R.empty()) return;
    for(int i = 0; i < R.size(); ++i) {
        int seller_order_id = R[i][0].as<int>();
        //string seller_symbol = seller[1].as<string>();
        float seller_shares = R[i][2].as<float>();
        float seller_price = R[i][3].as<float>();
        //int seller_account_id = R[i][4].as<int>();
        if(shares <= -seller_shares) {
            execute_order(seller_order_id, order_id, symbol, seller_price, shares);
            break;
        }
        else{//seller_shares <shares(buyer)
            execute_order(seller_order_id, order_id, symbol, seller_price, -seller_shares);
            shares += seller_shares;
        }
    }
}
void match_buyer_order(int order_id, string symbol, float shares, float price) {
    result R = search_buyer(symbol, price);
    if(R.empty()) return;
    for(size_t i = 0; i < R.size(); ++i) {
        int buyer_order_id = R[i][0].as<int>();
        //string seller_symbol = seller[1].as<string>();
        float buyer_shares = R[i][2].as<float>();
        float buyer_price = R[i][3].as<float>();
        //int buyer_account_id = R[i][4].as<int>();
        if(-shares <= buyer_shares) {
            execute_order(order_id, buyer_order_id, symbol, buyer_price, -shares);
            break;
        }
        else{//-shares > buyer_shares
            execute_order(order_id, buyer_order_id, symbol, buyer_price, buyer_shares);
            shares += buyer_shares;
        }
    }
    
}
void open_order(int id, string symbol, float amount, float price, int accountID){
  string sqlStatement = "INSERT INTO OPENTB(ID, SYMBOL, AMOUNT, PRICE, ACCOUNTID) VALUES (" + to_string(id) + ", '" + symbol + "', " + to_string(amount) + ", " + to_string(price) + ", " + to_string(accountID) + ");";
  //    work W(*C);
  execute(sqlStatement);
}
int add_order(int account_id, string symbol, float shares, float price) {
 //check order成立，若成立加入open table，
 //int order_id = check_order(account_id, symbol, amount, price);
 if(check_order(account_id, symbol, shares, price)!= "Valid.") {
  //invalid order
  return -1;
 }
 update_minus_account(account_id, symbol, price, shares);
 ++order_id;
 //insert into openTB
 open_order(order_id, symbol, shares, price, account_id);
 if(shares > 0) {//buy, iterate the open order and find matching buying orders
  match_seller_order(order_id, symbol, shares, price);
 }
 else { //sell, iterate the open order and find matching selling orders
  match_buyer_order(order_id, symbol, shares, price);
 }
 return order_id;
}
