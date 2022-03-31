#include "implementDB.hpp"
#include <iostream>
using namespace std;
int main(void) {
  buildDB();
  //create_account(0, 1000);
  //create_account(1, 300);
  //create_account(2, 300);
  //create_account(3, 300);
  //create_account(4, 300);
  //create_account(5, 300);

  //create_position("apple", 36, 5);
  //open_order(1, "apple", 10, 2, 0);
  //open_order(2, "apple", -10, 1, 1);
  //open_order(3, "apple", 20, 1, 3);
  //alter_open(10, 1);//1->amount0
  //alter_open(5, 2);//2->amount -15
  //  delete_open(3);
  //time_t now = time(NULL) + 18000;
  //insert_cancel(3, "apple", 20, 1, now, 3);
  //add_executed(3, "apple", 20, 1, now, 3); 
  //update_account(3, 10, 10);//account3: balance=300+100=400
  //update_account(1, 1, -10);//position:account1->"apple", 0, 
  //change_amount(1, 4);//position:1: 6
  //test execute_order()
  //  create_account(1,100);
  //create_account(2,100);
  //open_order(1, "apple", 10, 2, 1);
  //open_order(2, "apple", -8, 1, 2);
  //create_position("apple", 10, 2);
  //execute_order(2, 1, "apple", 1, 8);

  //add_order(1, "apple", 10, 1 );//buyer
  //add_order(2, "apple", 10, 2);
  //add_order(3, "apple", 10, 3);
  //add_order(4, "apple", 10, 4);
  //add_order(5, "apple", -35, 1);//seller
  
  //open_order(5, "apple", -44, 1, 5);//seller
  //match_buyer_order(5, "apple", -44, 1);
  //search_seller("apple", 5);
  //result R = search_buyer("apple", 2);
  //cout << "I'm in the main" << endl;
  //if(R.empty()) cout << "error" << endl;
  //for(auto line:R) {
  //  cout << line[0] << " " << line[1] << " " << line[2] << " " << line[3] << endl;
  //}
  /* 
  if(check_order(1, "apple", 10, 10)== "Valid."){
    cout << "valid buyer order" << endl;
  }
  if(check_order(2, "apple", -10, 10)== "Valid."){
    cout << "valid seller order" << endl;
  }
  //cout << check_order(1, "apple", 10, 10) << endl;
  //cout << check_order(1, "apple", -10, 10) << endl;
  return 0;
  */
}
