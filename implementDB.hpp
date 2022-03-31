#include "pugiconfig.hpp"
#include "pugixml.hpp"
#include <pqxx/pqxx>

using namespace pqxx;
using namespace std;

int buildDB();
void createTable(char const *fileName);
void execute(string sql);
result run_query(string sql);
void query(int id, pugi::xml_node &node);
void delete_open(int id);
void insert_cancel(int id, string symbol, float amount, float price, time_t now, int account_id);
void update_add_account(int account_id, float price, float shares);
void update_minus_account(int account_id, float price, float shares);
void cancel(int id, pugi::xml_node &node);
string check_order(int account_id, string symbol, float shares, int price);
result search_seller(string symbol, int price);
result search_buyer(string symbol, int price);
void match_seller_order(int order_id, string symbol, float shares, float price);
void match_buyer_order(int order_id, string symbol, float shares, float price);
int add_order(int account_id, string symbol, float shares, float price);
void execute_order(int sellOrderID, int buyOrderID, string symbol, int price, float amount);
bool create_account(int accountID, float balance);
bool create_position(string symbol, float amount, int accountID);
void open_order(int id, string symbol, float amount, float price, int accountID);
void alter_open(float amount, int orderID);
void add_executed(int OrderID, string symbol, float amount, float price, time_t epoch, int accountID);
void change_balance(int acountID, int balance);
void change_amount(int ID, int amount);
