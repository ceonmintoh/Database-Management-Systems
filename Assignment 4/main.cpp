/*
Skeleton code for linear hash indexing
*/

#include <string>
#include <ios>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include "classes.h"
using namespace std;


int main(int argc, char* const argv[]) {

    // Create the index
    LinearHashIndex emp_index("EmployeeIndex");
    emp_index.createFromFile("Employee.csv");

    // Loop to lookup IDs until user is ready to quit

    bool done = false;
    while (done == false) {
      int id;
      cout << "Enter an id to search for, or type -1 to quit: " ;
      cin >> id;
      cout << endl;

      if (id == -1) {
        done = true;
      } else {
        vector<Record> records = emp_index.findRecordById(id);
        if (records.size() == 0) {
          cout << "This record does not exist. " << endl;
        }
        else {
          for (int i = 0; i < records.size(); i++) {
            cout <<"---------------------" << endl;
            cout << "id: " << records[i].id << endl;
            cout << "name: " << records[i].name << endl;
            cout << "bio: " << records[i].bio << endl;
            cout << "manager id: " << records[i].manager_id << endl;
            cout <<"---------------------" << endl;

          }


        }

      }

    }





    return 0;
}
