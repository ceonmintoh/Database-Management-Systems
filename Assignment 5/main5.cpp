/* This is a skeleton code for Optimized Merge Sort, you can make modifications as long as you meet
   all question requirements*/


/*

Kaavya Subramanian, subramka@oregonstate.edu
Gretel Rajamoney, rajamong@oregonstate.edu
Mar 1 2022
Assignment #5 

*/

#include <bits/stdc++.h>
#include <iomanip>
#include <stdio.h>
#include "record_class.h"


using namespace std;

//defines how many blocks are available in the Main Memory
#define buffer_size 22

Records buffers[buffer_size]; //use this class object of size 22 as your main memory

//Comparator functions for C++ sort
bool compareEmp (Records const& e1, Records const& e2) {
  return e1.emp_record.eid < e2.emp_record.eid;
}

bool compareDept (Records const& d1, Records const& d2) {
  return d1.dept_record.managerid < d2.dept_record.managerid;
}

//Sorting the buffers in main_memory and storing the sorted records into a temporary file (runs)
void Sort_Buffer(string recordType,int size){
    if (recordType == "emp") {
      std::sort(buffers,buffers + size,&compareEmp);

    }
    else if (recordType == "dept") {
      std::sort(buffers,buffers + size,&compareDept);


    }
    //Remember: You can use only [AT MOST] 22 blocks for sorting the records / tuples and create the runs
    return;
}


//Split and sort Department Relation files
int splitDeptFile(fstream& f) {
  bool flag = true;
  int cur_size = 0;
  int run_no = 1;
  int stream_ctr = 0;

  while (flag) {
      // FOR BLOCK IN RELATION EMP

      // grabs a block
      Records  single_DeptRecord  = Grab_Dept_Record(f);
      // checks if filestream is empty
      if (single_DeptRecord.no_values == -1) {
          flag = false;
          Sort_Buffer("dept",cur_size);
          //Save run in a file
          std::string fileStart = "dept_", fileEnd = ".csv";
          std::stringstream s;
          s << fileStart << run_no << fileEnd;
          std::string fileName = s.str();
          fstream output;
          output.open(fileName,ios::out);
          for (int i = 0; i < cur_size; i++) {
                output << buffers[i].dept_record.did << "," << buffers[i].dept_record.dname << "," << buffers[i].dept_record.budget << "," << buffers[i].dept_record.managerid << endl;
            }
          output.close();
      }

      if(cur_size + 1 <= buffer_size){
          //Memory is not full store current record into buffers.
          buffers[cur_size] = single_DeptRecord;
          cur_size += 1;
      }
      else{
          //memory is full now. Sort the blocks in Main Memory and Store it in a temporary file (runs)
          // cout << "Main Memory is full. Time to sort and store sorted blocks in a temporary file" << endl;

          Sort_Buffer("dept",buffer_size);
          //Save run in a file
          std::string fileStart = "dept_", fileEnd = ".csv";
          std::stringstream s;
          s << fileStart << run_no << fileEnd;
          std::string fileName = s.str();
          fstream output;
          output.open(fileName,ios::out);

          for (int i = 0; i < buffer_size; i++) {
                output << buffers[i].dept_record.did << "," << buffers[i].dept_record.dname << "," << buffers[i].dept_record.budget << "," << buffers[i].dept_record.managerid << endl;
            }
            run_no +=1;
            stream_ctr += 1;


          //After sorting start again. Clearing memory and putting the current one into main memory.
          cur_size = 0;
          buffers[cur_size] = single_DeptRecord;
          cur_size += 1;
      }

  }

  return run_no;

}


//Split and sort Employee Relation files
int splitEmpFile(fstream& f) {
  bool flag = true;
  int cur_size = 0;
  int run_no = 1;
  int stream_ctr = 0;

  while (flag) {
      // FOR BLOCK IN RELATION EMP

      // grabs a block
      Records  single_EmpRecord  = Grab_Emp_Record(f);
      // checks if filestream is empty
      if (single_EmpRecord.no_values == -1) {
          flag = false;
          Sort_Buffer("emp",cur_size);
          //Save run in a file
          std::string fileStart = "emp_", fileEnd = ".csv";
          std::stringstream s;
          s << fileStart << run_no << fileEnd;
          std::string fileName = s.str();
          fstream output;
          output.open(fileName,ios::out);
          for (int i = 0; i < cur_size; i++) {
                output << buffers[i].emp_record.eid << "," << buffers[i].emp_record.ename << "," << buffers[i].emp_record.age << "," << buffers[i].emp_record.salary << endl;
            }
          output.close();
      }
      if(cur_size + 1 <= buffer_size){
          //Memory is not full store current record into buffers.
          buffers[cur_size] = single_EmpRecord;
          cur_size += 1;
      }
      else{
          //memory is full now. Sort the blocks in Main Memory and Store it in a temporary file (runs)
          // cout << "Main Memory is full. Time to sort and store sorted blocks in a temporary file" << endl;

          Sort_Buffer("emp",buffer_size);
          //Save run in a file
          std::string fileStart = "emp_", fileEnd = ".csv";
          std::stringstream s;
          s << fileStart << run_no << fileEnd;
          std::string fileName = s.str();
          fstream output;
          output.open(fileName,ios::out);

          for (int i = 0; i < buffer_size; i++) {
                output << buffers[i].emp_record.eid << "," << buffers[i].emp_record.ename << "," << buffers[i].emp_record.age << "," << buffers[i].emp_record.salary << endl;
            }
            run_no +=1;
            stream_ctr += 1;


          //After sorting start again. Clearing memory and putting the current one into main memory.
          cur_size = 0;
          buffers[cur_size] = single_EmpRecord;
          cur_size += 1;
      }

  }

  return run_no;

}

//Grabs the next block from the right relation
int getBlock(fstream &f, int start, int end, string recordType) {

  int numRecords = 0;

  for (int i = start; i < end; i++ ) {
    Records r1;
    if (recordType == "emp") {
      r1 = Grab_Emp_Record(f);
    }

    else if (recordType == "dept") {
      r1 = Grab_Dept_Record(f);
    }

    if (r1.no_values == -1) {
      f.close();
      return numRecords; //no more records left in the file
    }

    buffers[i] = r1; //fill up the right buffers with the record
    numRecords += 1;


  }

  return numRecords;


}

//Prints out the attributes from empRecord and deptRecord when a join condition is met
//and puts it in file Join.csv
void PrintJoin(int e, int d, fstream &j) {
  //Print to screen
  while (buffers[e].emp_record.eid == buffers[d].dept_record.managerid) {
    cout << buffers[d].dept_record.did << "," << buffers[d].dept_record.dname << "," <<
         fixed << setprecision(2) <<
         buffers[d].dept_record.budget <<  "," << buffers[d].dept_record.managerid << "," <<
         buffers[e].emp_record.eid << "," << buffers[e].emp_record.ename << ", " <<
         buffers[e].emp_record.age << "," << buffers[e].emp_record.salary << endl;

    //write to file
    j << buffers[d].dept_record.did << "," << buffers[d].dept_record.dname << "," <<
         fixed << setprecision(2) <<
         buffers[d].dept_record.budget <<  "," << buffers[d].dept_record.managerid << "," <<
         buffers[e].emp_record.eid << "," << buffers[e].emp_record.ename << ", " <<
         buffers[e].emp_record.age << "," << buffers[e].emp_record.salary << endl;

    e += 1; //increment emp index
  }

}

void Merge_Join_Runs(int numEmpRuns,int numDeptRuns, fstream & f){


  int d = 0;
  for (int i = 1; i <= numDeptRuns; i++) { //iterate through each stored dept run file
    std::string fileStart = "dept_", fileEnd = ".csv";
    std::stringstream s;
    s << fileStart << i << fileEnd;
    fstream deptFile;
    deptFile.open(s.str(),ios::in);



    while (deptFile.is_open()) { //grab blocks while the file is open

      //split the buffer equally so the dept records are stored in the first 11, and the employee
      //records are stored in the next 11
      int deptsInBlock = getBlock(deptFile,0,buffer_size/2, "dept");

      if (deptsInBlock == 0) continue; //no files found


      for (int j  = 1; j <= numEmpRuns; j++) { //iterate through each emp file
        std::string fileStart = "emp_", fileEnd = ".csv";
        std::stringstream s;
        s << fileStart << j << fileEnd;
        fstream empFile;
        empFile.open(s.str(),ios::in);




        while(empFile.is_open()) { //grab blocks while the file is open
          int empsInBlock = getBlock(empFile,buffer_size/2,buffer_size,"emp");

          if (empsInBlock == 0) continue;

          int empIdx = buffer_size/2;
          int deptIdx = 0;

          //perform join computation within block
          while (empIdx < buffer_size && deptIdx < buffer_size/2) { //idx stay within array bounds
            if (buffers[deptIdx].dept_record.managerid > buffers[empIdx].emp_record.eid){
                empIdx += 1;
            }
            else if (buffers[deptIdx].dept_record.managerid < buffers[empIdx].emp_record.eid) {
                deptIdx += 1;
            }

            else {

              PrintJoin(empIdx, deptIdx,f);
              deptIdx += 1;


            }

          }


        }


      }


    }


  }


    return;
}

int main() {

    //Open file streams to read and write
    //Opening out two relations Emp.csv and Dept.csv which we want to join
    fstream empin;
    fstream deptin;
    empin.open("Emp.csv", ios::in);
    deptin.open("Dept.csv", ios::in);



    int numEmpRuns = splitEmpFile(empin);
    int numDeptRuns = splitDeptFile(deptin);


    empin.close();
    deptin.close();
    fstream joinout;
    joinout.open("Join.csv", ios::out);

    Merge_Join_Runs(numEmpRuns, numDeptRuns, joinout);

    joinout.close();

    //Remove run files

    for (int i  = 1; i <= numEmpRuns; i++) {
      std::string fileStart = "emp_", fileEnd = ".csv";
      std::stringstream s;
      s << fileStart << i << fileEnd;
      std::string fileName = s.str();

      if (remove(fileName.c_str()) == 0) continue;
      else cout << "Couldn't remove file " << fileName << endl;

    }

    for (int i  = 1; i <= numDeptRuns; i++) {
      std::string fileStart = "dept_", fileEnd = ".csv";
      std::stringstream s;
      s << fileStart << i << fileEnd;
      std::string fileName = s.str();

      if (remove(fileName.c_str()) == 0) continue;
      else cout << "Couldn't remove file " << fileName << endl;

    }


    return 0;
}
