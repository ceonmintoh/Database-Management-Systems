/* This is a skeleton code for two-pass multi-way sorting, you can make modifications as long as you meet
   all question requirements*/
//Currently this program will just load the records in the buffers (main memory) and print them when filled up.
//And continue this process untill the full Emp.csv is read.

/*
Kaavya Subramanian
subramka@oregonstate.edu

Gretel Rajamoney
rajamong@oregonstate.edu

*/


#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
using namespace std;
#define buffer_size 22

//defines how many buffers are available in the Main Memory


struct EmpRecord {
    int eid;
    string ename;
    int age;
    double salary;
};

EmpRecord buffers[buffer_size]; // this structure contains 22 buffers; available as your main memory.
fstream run_files[buffer_size - 1]; //storse an array of filehandles that will point to each sorted run

//Comparator function that gets passed into C++ sort
bool compareRecord (EmpRecord const& e1, EmpRecord const& e2) {
  return e1.eid < e2.eid;
}

// Grab a single block from the Emp.csv file, in theory if a block was larger than
// one tuple, this function would return an array of EmpRecords and the entire if
// and else statement would be wrapped in a loop for x times based on block size
EmpRecord Grab_Emp_Record(fstream &empin) {
    string line, word;
    EmpRecord  emp;
    // grab entire line
    if (getline(empin, line, '\n')) {
        // turn line into a stream
        stringstream s(line);
        // gets everything in stream up to comma
        getline(s, word,',');
        emp.eid = stoi(word);
        getline(s, word, ',');
        emp.ename = word;
        getline(s, word, ',');
        emp.age = stoi(word);
        getline(s, word, ',');
        emp.salary = stod(word);
        return emp;
    } else {
        emp.eid = -1;
        return emp;
    }
}

//Printing whatever is stored in the buffers of Main Memory
//Can come in handy to see if you've sorted the records in your main memory properly.
void Print_Buffers(int cur_size) {
    for (int i = 0; i < cur_size; i++)
    {
        cout << i << " " << buffers[i].eid << " " << buffers[i].ename << " " << buffers[i].age << " " << buffers[i].salary << endl;
    }
}

//Sorting the buffers in main_memory based on 'eid' and storing the sorted records into a temporary file
//You can change return type and arguments as you want.
void Sort_in_Main_Memory(){
    std::sort(buffers,buffers + buffer_size,&compareRecord);
    return;
}

//You can use this function to merge your M-1 runs using the buffers in main memory and storing them in sorted file 'EmpSorted.csv'(Final Output File)
//You can change return type and arguments as you want.
void Merge_Runs_in_Main_Memory(int stream_ctr){

    fstream sorted_file;
    sorted_file.open("EmpSorted.csv", ios::out );

    //Open a filehandle to each sorted run
    for (int i = 0; i <= stream_ctr;i++) {
      std::string fileStart = "runs_", fileEnd = ".csv";
      std::stringstream s;
      s << fileStart << i+1 << fileEnd;
      std::string fileName = s.str();
      run_files[i].open(fileName,ios::in);
      buffers[i] = Grab_Emp_Record(run_files[i]); //fill the buffer with the first element in the run

    }


    int empty_files = 0;  //Keep track of how many empty buffers there are

    while(empty_files <= stream_ctr) {  // break the loop once all the buffesr are empty
    int min_index = 0;
    //find the buffer with the smallest eid
    for (int i = 1; i <= stream_ctr;i++) {
      if (buffers[min_index].eid == -1) {
        min_index += 1;
        continue;
      }
      if (buffers[i].eid== -1) continue;

      if (buffers[i].eid < buffers[min_index].eid) min_index = i;
    }

    //Copy the min_index into the output buffer
    buffers[buffer_size - 1].eid = buffers[min_index].eid;
    buffers[buffer_size - 1].ename = buffers[min_index].ename;
    buffers[buffer_size - 1].age = buffers[min_index].age;
    buffers[buffer_size - 1].salary = buffers[min_index].salary;

    //Write the output buffer to memory
    sorted_file << buffers[buffer_size - 1].eid << "," << buffers[buffer_size - 1].ename << "," << buffers[buffer_size - 1].age << "," << buffers[buffer_size - 1].salary << endl;

    //refill the empty buffer
    buffers[min_index] = Grab_Emp_Record(run_files[min_index]);
    if (buffers[min_index].eid == -1) //buffer is empty
    {
      empty_files += 1;

    }
  }
  sorted_file.close();
}

int main() {
  // open file streams to read and write
  fstream input_file;
  input_file.open("Emp.csv", ios::in);
  // ofstream output_file("pass1.csv");

  // flags check when relations are done being read
  bool flag = true;
  int cur_size = 0;
  int run_no = 1;
  int stream_ctr = 0;

  /*First Pass: The following loop will read each block put it into main_memory
    and sort them then will put them into a temporary file for 2nd pass */
  while (flag) {
      // FOR BLOCK IN RELATION EMP

      // grabs a block
      EmpRecord  single_EmpRecord  = Grab_Emp_Record(input_file);
      // checks if filestream is empty
      if (single_EmpRecord.eid == -1) {
          flag = false;
          Sort_in_Main_Memory();
          //Save run in a file
          std::string fileStart = "runs_", fileEnd = ".csv";
          std::stringstream s;
          s << fileStart << run_no << fileEnd;
          std::string fileName = s.str();
          run_files[stream_ctr].open(fileName,ios::out);
          for (int i = 0; i < cur_size; i++) {
                run_files[stream_ctr] << buffers[i].eid << "," << buffers[i].ename << "," << buffers[i].age << "," << buffers[i].salary << endl;
            }
      }
      if(cur_size + 1 <= buffer_size){
          //Memory is not full store current record into buffers.
          buffers[cur_size] = single_EmpRecord ;
          cur_size += 1;
      }
      else{
          //memory is full now. Sort the blocks in Main Memory and Store it in a temporary file (runs)
          // cout << "Main Memory is full. Time to sort and store sorted blocks in a temporary file" << endl;

          Sort_in_Main_Memory();
          //Save run in a file
          std::string fileStart = "runs_", fileEnd = ".csv";
          std::stringstream s;
          s << fileStart << run_no << fileEnd;
          std::string fileName = s.str();
          run_files[stream_ctr].open(fileName,ios::out);

          for (int i = 0; i < buffer_size; i++) {
                run_files[stream_ctr] << buffers[i].eid << "," << buffers[i].ename << "," << buffers[i].age << "," << buffers[i].salary << endl;
            }
            run_no +=1;
            stream_ctr += 1;


          //After sorting start again. Clearing memory and putting the current one into main memory.
          cur_size = 0;
          buffers[cur_size] = single_EmpRecord;
          cur_size += 1;
      }

  }

  for (int i = 0; i < buffer_size - 1; i++) {
    run_files[i].close();
  }

  /* Implement 2nd Pass: Read the temporary sorted files and utilize main_memory to store sorted runs into the EmpSorted.csv*/

  //Uncomment when you are ready to store the sorted relation
  //fstream sorted_file;
  //sorted_file.open("EmpSorted.csv", ios::out | ios::app);





  bool flag_sorting_done = false;
  while(!flag_sorting_done){


      Merge_Runs_in_Main_Memory(stream_ctr);
      flag_sorting_done = true;
  }

  //You can delete the temporary sorted files (runs) after you're done if you want. It's okay if you don't.

  return 0;
}
