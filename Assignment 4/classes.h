#include <string>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <bitset>
#include <cmath>
#include <tuple>
using namespace std;

const int HEADER_SIZE = 128; //Header size for each page

class Record {
public:
    int id, manager_id;
    std::string bio, name;

    Record(vector<std::string> fields) {
        if (fields[0] == "no") {
          id = -1;
        }
        else {
          id = stoi(fields[0]);
        }

        name = fields[1];
        bio = fields[2];
        if (fields[3] == "no") {
          manager_id = -1;
        }
        else {
          manager_id = stoi(fields[3]);
        }

    }

    void print() {
        cout << "\tID: " << id << "\n";
        cout << "\tNAME: " << name << "\n";
        cout << "\tBIO: " << bio << "\n";
        cout << "\tMANAGER_ID: " << manager_id << "\n";
    }
};

// Grab a Record from Employee.csv

Record Grab_Record(fstream &empin) {
    std::string line, word;
    std::string id = "";
    std::string name = "";
    std::string bio = "";
    std::string manager_id = "";

    // grab entire line

    if (getline(empin, line, '\n')) {
        // turn line into a stream
        stringstream s(line);
        // gets everything in stream up to comma
        getline(s, word,',');
        id = word;
        getline(s, word, ',');
        name = word;
        getline(s, word, ',');
        bio = word;
        getline(s, word, ',');
        manager_id = word;
    } else {
        id = "no";  //no more records in the file
        manager_id = "no";

    }


    std::vector<std::string> v = {id,name,bio,manager_id};
    Record r1(v);
    return r1;
}


/*******************************************************************************
Creates a page header for a newly initialized bucket. A page header stores
the index of the overflow page, how much free space is left in the page,the
number of records, and the lengths of each record present in the page.
*******************************************************************************/

void createPageHead(int pageIndex, FILE* f, int pageSize) {
  int start = pageIndex * (pageSize + HEADER_SIZE); //locat where the page should be
  int overflow = -1; //the page currently doesn't have any overflow pages
  int freeSpace = pageSize;
  int nr = 0;

  if (! fseek(f, start, SEEK_SET)) {
    fwrite(&overflow, sizeof(int), 1, f);
    fwrite(&freeSpace,sizeof(int),1,f);
    fwrite(&nr,sizeof(int),1,f);
  }
  fseek(f, 0, SEEK_CUR);


}

/******************************************************************************/

/*******************************************************************************
Writes a record to a page given its page index
*******************************************************************************/

void writeToFile(int id, string name, string bio, int manager_id, int pageIndex, int pageSize, FILE* f, int* nextFreePage) {
  int bioSize = bio.size() + 1; //add 1 block becaus bio will become a c string
  int nameSize = name.size();
  int recordSize = (4 * sizeof(int)) + bioSize + nameSize; //storing 4 integers and 2 string fields per record
  int pageHeadPos =pageIndex * (pageSize + HEADER_SIZE);

  int overflow, freeSpace, nr;
  if (! fseek(f,pageHeadPos,SEEK_SET)) {
    fread(&overflow,sizeof(int),1,f);
    fread(&freeSpace,sizeof(int),1,f);
    fread(&nr,sizeof(int),1,f);
  }

  if (freeSpace < recordSize) { // not enough room, move to an overflow page
    fseek(f, 0, SEEK_CUR);
    if (overflow != -1) { //overflow page exists, write to it

      writeToFile(id,name,bio,manager_id,overflow,pageSize,f,nextFreePage);
    }
    else { //overflow page doesn't exist, create and write to it
      if (! fseek(f,pageHeadPos,SEEK_SET)) {
        overflow = *nextFreePage;
        fwrite(&overflow,sizeof(int),1,f);
      }
      *nextFreePage += 1;
      createPageHead(overflow,f,pageSize);
      writeToFile(id,name,bio,manager_id,overflow,pageSize,f,nextFreePage);


    }



  } else { //enough room on current page, write to it
    int recordPos = pageSize - freeSpace;
    int NewfreeSpace   = freeSpace - recordSize;


    nr += 1;

    fseek(f, 0, SEEK_CUR);

    if (! fseek(f,pageHeadPos,SEEK_SET)) {
      fwrite(&overflow,sizeof(int),1,f);
      fwrite(&NewfreeSpace,sizeof(int),1,f);
      fwrite(&nr,sizeof(int),1,f);
    }


    fseek(f,0,SEEK_CUR);

    //write record length to next available spot in the header
    if (! fseek(f,pageHeadPos + (3* sizeof(int)) + ((nr - 1) * sizeof(int)), SEEK_SET)) {
      fwrite(&recordSize,sizeof(int),1,f);
    }

    //write record to allocated record position
    if (! fseek(f, pageHeadPos + HEADER_SIZE + recordPos, SEEK_SET)) {

         fwrite(&nameSize, sizeof(int), 1, f);
         fwrite(&bioSize, sizeof(int),1,f);
         fwrite(&id,sizeof(int),1,f);
         fwrite(&manager_id,sizeof(int),1,f);
         fwrite(name.c_str(),sizeof(char),name.length() + 1,f);
         fwrite(bio.c_str(),sizeof(char),bio.length() + 1,f);
       }
       fseek(f, 0, SEEK_CUR);

  }


}

/******************************************************************************/

/*******************************************************************************
Removes a record from a page based on its search id and returns it
*******************************************************************************/

Record removeRecordFromPage(int pageIndex, int pageSize, FILE* f, int s_id) {
  string id = "-1", manager_id = "-1";
  string name,bio;
  std::vector<std::string> v = {id,name,bio,manager_id};
  Record r1(v);
  int offset;

  int start = pageIndex * (pageSize + HEADER_SIZE);
  int overflow, freeSpace, nr;
  if (! fseek(f, start, SEEK_SET)) {
    fread(&overflow,sizeof(int),1,f);
    fread(&freeSpace,sizeof(int),1,f);
    fread(&nr,sizeof(int),1,f);
  }


  bool found = false;
  int recordStart = start + HEADER_SIZE;
  for (int i = 0; i < nr; i++) { //go through all records in page

    int recordLength;

    if (! fseek(f, start + (3*sizeof(int) + (i*sizeof(int))), SEEK_SET)) {
      fread(&recordLength,sizeof(int),1,f);
    }

    int q_id;
    if (!fseek(f,recordStart + (2* sizeof(int)) ,SEEK_SET)) {
      fread(&q_id,sizeof(int),1,f);
    }


    if (( q_id != s_id) && (found == false)) { //record hasn't been found yet
      recordStart += recordLength;
      continue;
    }
    else if ((q_id == s_id) && (found == false)) { //record was found
      found = true;
      //Read information into a record object
      int nameSize, bioSize,id, manager_id;
      if (!fseek(f,recordStart,SEEK_SET)) {
        fread(&nameSize,sizeof(int),1,f);
      }

      if (!fseek(f,recordStart + sizeof(int) ,SEEK_SET)) {
        fread(&bioSize,sizeof(int),1,f);
      }

      if (!fseek(f,recordStart + (2* sizeof(int)) ,SEEK_SET)) {
        fread(&id,sizeof(int),1,f);
      }

      if (!fseek(f,recordStart + (3* sizeof(int)) ,SEEK_SET)) {
        fread(&manager_id,sizeof(int),1,f);
      }

      char name[201];

      if (!fseek(f,recordStart + (4* sizeof(int)) ,SEEK_SET)) {
        fread(name,nameSize+1,1,f);
      }
      char bio[501];
      if (!fseek(f,recordStart + (4* sizeof(int)) + nameSize + 1,SEEK_SET)) {
        fread(bio,bioSize+ 1,1,f);
      }




      recordStart += recordLength;
      offset = recordLength; //set offset equal to the length of the record that is being deleted


      fseek(f,0,SEEK_CUR);
      int remainingSpace = freeSpace + recordLength; //update amount of free space left in page
      int newRecords = nr - 1; //update amount of elements left in page
      if (! fseek(f, start + sizeof(int), SEEK_SET)) {
        fwrite(&remainingSpace,sizeof(int),1,f);
        fwrite(&newRecords,sizeof(int),1,f);
      }

      r1.name = name;
      r1.bio = bio;
      r1.manager_id = manager_id;
      r1.id = id;



    }
    else { // move the records to fill the space before ((i-1)*sizeof(int))

      fseek(f,0,SEEK_CUR);

      if (! fseek(f, start + (3*sizeof(int)) +((i-1)*sizeof(int)) , SEEK_SET)) {
        fwrite(&recordLength,sizeof(int),1,f); //overwrite record length
      }

      int oldPos = recordStart;
      int o_nameSize, o_bioSize,o_id, o_manager_id;

      //read old record information
      if (!fseek(f,oldPos,SEEK_SET)) {
        fread(&o_nameSize,sizeof(int),1,f);
      }

      if (!fseek(f,oldPos + sizeof(int) ,SEEK_SET)) {
        fread(&o_bioSize,sizeof(int),1,f);
      }

      if (!fseek(f,oldPos + (2* sizeof(int)) ,SEEK_SET)) {
        fread(&o_id,sizeof(int),1,f);
      }

      if (!fseek(f,oldPos + (3* sizeof(int)) ,SEEK_SET)) {
        fread(&o_manager_id,sizeof(int),1,f);
      }

      char o_name[201];

      if (!fseek(f,oldPos + (4* sizeof(int)) ,SEEK_SET)) {
        fread(o_name,o_nameSize+1,1,f);
      }
      char o_bio[501];
      if (!fseek(f,oldPos + (4* sizeof(int)) + o_nameSize + 1,SEEK_SET)) {
        fread(o_bio,o_bioSize+ 1,1,f);
      }

      //write to new position

      int rs = recordStart - offset;

      fseek(f,0,SEEK_CUR);
      if (!fseek(f,rs,SEEK_SET)) {
        fwrite(&o_nameSize,sizeof(int),1,f);
      }

      if (!fseek(f,rs + sizeof(int) ,SEEK_SET)) {
        fwrite(&o_bioSize,sizeof(int),1,f);
      }

      if (!fseek(f,rs + (2* sizeof(int)) ,SEEK_SET)) {
        fwrite(&o_id,sizeof(int),1,f);
      }

      if (!fseek(f,rs + (3* sizeof(int)) ,SEEK_SET)) {
        fwrite(&o_manager_id,sizeof(int),1,f);
      }



      if (!fseek(f,rs + (4* sizeof(int)) ,SEEK_SET)) {
        fwrite(o_name,o_nameSize+1,1,f);
      }
      if (!fseek(f,rs + (4* sizeof(int)) + o_nameSize + 1,SEEK_SET)) {
        fwrite(o_bio,o_bioSize+ 1,1,f);
      }

      recordStart += recordLength;


    }


    }



    return r1;


}

/******************************************************************************/


/*******************************************************************************
Given a page index, returns all overflow pages linked to it and the bucket
that they all correspond to
*******************************************************************************/
vector<std::tuple<int,int,int>> returnAllPages(FILE* f, int pageSize,int pageIndex,int bucket){

  int pg = pageIndex;
  vector<std::tuple<int,int,int>> pageIndices;

  int start = pageIndex * (pageSize + HEADER_SIZE);
  int overflow, freeSpace, nr;
  fseek(f,0,SEEK_CUR);
  if (! fseek(f, start, SEEK_SET)) {
    fread(&overflow,sizeof(int),1,f);
    fread(&freeSpace,sizeof(int),1,f);
    fread(&nr,sizeof(int),1,f);
  }

  if (freeSpace > 0) { //only add the page if it is nonempty
    pageIndices.push_back(tuple<int,int,int>(pageIndex,pg,bucket));
  }

  while (overflow != -1) {
    pageIndex = overflow;
    start = overflow * (pageSize + HEADER_SIZE);
    if (! fseek(f, start, SEEK_SET)) {
      fread(&overflow,sizeof(int),1,f);
      fread(&freeSpace,sizeof(int),1,f);
      fread(&nr,sizeof(int),1,f);
    }
    if (freeSpace > 0) {
      pageIndices.push_back(tuple<int,int,int>(pageIndex,pg,bucket));

  }

  }


  return pageIndices;

  }
/******************************************************************************/

/*******************************************************************************
Auxilary function to print contents of a page. Not used in main programs
*******************************************************************************/

void printPage(int pageIndex, int pageSize, FILE * f) {
  int start = start = pageIndex * (pageSize + HEADER_SIZE);
  int overflow, freeSpace, nr;
  int sum = 0;
  if (! fseek(f, start, SEEK_SET)) {
    fread(&overflow,sizeof(int),1,f);
    fread(&freeSpace,sizeof(int),1,f);
    fread(&nr,sizeof(int),1,f);
  }

  cout << "Header Information: " << endl;
  cout << "Overflow: " << overflow << "  free space: " << freeSpace << "  nr: " << nr << endl;
  cout << "-------------------" << endl;

  int recordStart = start + HEADER_SIZE;
  for (int i = 0; i < nr; i++) {
    int recordLength;
    int nameSize, bioSize,id, manager_id;
    if (! fseek(f, start + (3*sizeof(int) + (i*sizeof(int))), SEEK_SET)) {
      fread(&recordLength,sizeof(int),1,f);
    }
    sum += recordLength;

    if (!fseek(f,recordStart,SEEK_SET)) {
      fread(&nameSize,sizeof(int),1,f);
    }

    if (!fseek(f,recordStart + sizeof(int) ,SEEK_SET)) {
      fread(&bioSize,sizeof(int),1,f);
    }

    if (!fseek(f,recordStart + (2* sizeof(int)) ,SEEK_SET)) {
      fread(&id,sizeof(int),1,f);
    }

    if (!fseek(f,recordStart + (3* sizeof(int)) ,SEEK_SET)) {
      fread(&manager_id,sizeof(int),1,f);
    }

    char name[201];

    if (!fseek(f,recordStart + (4* sizeof(int)) ,SEEK_SET)) {
      fread(name,nameSize+1,1,f);
    }
    char bio[501];
    if (!fseek(f,recordStart + (4* sizeof(int)) + nameSize + 1,SEEK_SET)) {
      fread(bio,bioSize+ 1,1,f);
    }
    int hashFxn = id % (int)pow(2,16);
    string binaryHash = std::bitset<16>(hashFxn).to_string();

    cout << "id: " << id << " " << binaryHash << endl;


    recordStart += recordLength;
  }


}

/******************************************************************************/



class LinearHashIndex {

private:
    const int PAGE_SIZE = 4096;

    vector<int> pageDirectory;  // Where pageDirectory[h(id)] gives page index of block
                                // can scan to pages using index*PAGE_SIZE as offset (using seek function)
    int numBlocks; // n
    int bits;
    int numRecords; // Records in index
    int nextFreePage; // Next page to write to
    std::string fName;
    int block_size;


    /* Calculate the average bytes filled per bucket to determine
       whether or not to add a boucket */
    double calcCapacity(FILE* f) {

      int totalSpace = numBlocks * PAGE_SIZE; //total allocate space, disregarding oveflow blocks
      int nr = 0;
      int occupiedSpace  = 0;
      for (int i = 0; i < numBlocks; i++) {
        int pageIndex = pageDirectory[i];
        int start = pageIndex * (PAGE_SIZE + HEADER_SIZE);
        int pageFreeSpace,n;
        if (! fseek(f, start + sizeof(int), SEEK_SET)) {
          fread(&pageFreeSpace,sizeof(int),1,f);
          fread(&n,sizeof(int),1,f);
        }
        int usedSpace = PAGE_SIZE - pageFreeSpace;
        occupiedSpace += usedSpace; //add up free space in each page
        nr += n;

      }


      return (double) occupiedSpace / totalSpace;

    }
    void insertRecord(Record record, FILE* hash_index) {

        // No records written to index yet
        if (numRecords == 0) {
          bits = 1;
          numBlocks = 2;
          pageDirectory.push_back(0);
          createPageHead(0, hash_index, PAGE_SIZE);
          pageDirectory.push_back(1);
          createPageHead(1, hash_index, PAGE_SIZE);
          nextFreePage = 2;

        }

        //get binary value of hash(id)
        int hashFxn = record.id % (int)pow(2,16);
        string binaryHash = std::bitset<16>(hashFxn).to_string();
        string b = binaryHash.substr(binaryHash.length() - bits);
        int bx = stoi(b,NULL,2);


        if (bx >= numBlocks){ //if key doesn't exist yet flip most significant bit
          b[0] = '0';
          bx = stoi(b,NULL,2);
        }

        int pageToWrite = pageDirectory[bx];

        writeToFile(record.id, record.name, record.bio, record.manager_id, pageToWrite, PAGE_SIZE, hash_index, &nextFreePage);


    }

    /* Reorganizes the hash index once a new bucket is added */
    void reorganizeHashIndex(FILE* f, int page_size, int bx) {

      //create list of pages to reorganize
      vector<std::tuple<int,int,int>> pagesToOrganize;
      for (int i = 0; i < pageDirectory.size() -1; i++) { // - 1 because we just added a new bucket
        vector<std::tuple<int,int,int>> p = returnAllPages(f,page_size,pageDirectory[i],i);
        pagesToOrganize.insert(pagesToOrganize.end(),p.begin(),p.end());
      }


      for (int i = 0; i < pagesToOrganize.size(); i++) {
        int start = std::get<0>(pagesToOrganize[i]) * (page_size + HEADER_SIZE);
        int overflow, freeSpace, nr;
        fseek(f,0,SEEK_CUR);
        if (! fseek(f, start, SEEK_SET)) {
          fread(&overflow,sizeof(int),1,f);
          fread(&freeSpace,sizeof(int),1,f);
          fread(&nr,sizeof(int),1,f);
        }

        int running_length  = 0;
        int recordLength = 0;

        //iterate through each record in a page and check if it is in the right bucket
        vector<int> idsToRemove;
        for (int j = 0; j < nr; j++ ){


          running_length += recordLength;

          int recordStart = start + HEADER_SIZE + running_length;
          int s_id;

          fseek(f,0,SEEK_CUR);
          if (! fseek(f, recordStart + 2*sizeof(int), SEEK_SET)) {
            fread(&s_id,sizeof(int),1,f);
          }

          //convert to binary and get string
          int hashFxn = s_id % (int)pow(2,16);
          string binaryHash = std::bitset<16>(hashFxn).to_string();
          string b = binaryHash.substr(binaryHash.length() - bx);
          int f_id = stoi(b,NULL,2);

          if (f_id != std::get<2>(pagesToOrganize[i])) { //not in the right bucket
            idsToRemove.push_back(s_id);

           }

           if (! fseek(f, start + 3*sizeof(int) + j*sizeof(int), SEEK_SET)) {
             fread(&recordLength,sizeof(int),1,f);
           }


        }

        //remove all records from the page and insert them into the right bucket
        for (int j = 0 ; j < idsToRemove.size(); j++) {
          Record r1 = removeRecordFromPage(std::get<0>(pagesToOrganize[i]), PAGE_SIZE, f, idsToRemove[j]);
          insertRecord(r1,f);
        }


      }

    }

public:
    LinearHashIndex(string indexFileName) {
        numBlocks = 0;
        bits = 0;
        numRecords = 0;
        block_size = 0;
        fName = indexFileName;
    }



    // Read csv file and add records to the index
    void createFromFile(string csvFName) {

      fstream input_file;
      input_file.open(csvFName, ios::in);
      FILE* hash_index;
      hash_index = fopen(fName.c_str(), "w+"); //use fopen() so we can use fseek()



      bool flag = true;
      int cur_size = 0;

      while (flag) {
        Record  single_EmpRecord  = Grab_Record(input_file);
        cur_size += 1;
        if (single_EmpRecord.id == -1) { // no more files left
          flag = false;


        }
        else {

          insertRecord(single_EmpRecord, hash_index);
          numRecords += 1;

          double lc = calcCapacity(hash_index); // calculate capacity

          if (lc > 0.7) { //add a new bucket

            numBlocks += 1;
            createPageHead(nextFreePage, hash_index, PAGE_SIZE);
            pageDirectory.push_back(nextFreePage);
            nextFreePage += 1;

            if ((int)pow(2,bits) < numBlocks) { //increments  bits if needed
              bits += 1;
            }

            //reorganize hash index
            reorganizeHashIndex(hash_index, PAGE_SIZE, bits);

          }

        }

      }

      input_file.close();

      fclose(hash_index);

    }

    // Given an ID, find the relevant record and print it
    vector<Record> findRecordById(int id) {
      FILE* f;
      vector<Record> records;
      f = fopen(fName.c_str(), "r");
      bool found = false;

      //convert string to binary
      int hashFxn = id % (int)pow(2,16);
      string binaryHash = std::bitset<16>(hashFxn).to_string();
      string b = binaryHash.substr(binaryHash.length() - bits);
      int idx = stoi(b,NULL,2);
      if (idx >= numBlocks){ //key doesn't exist in the hash index
        b[0] = '0';
        idx = stoi(b,NULL,2);
      }



      int pageIndex = pageDirectory[idx];
      vector<std::tuple<int,int,int>> pagesToSearch = returnAllPages(f, PAGE_SIZE,pageIndex,idx);

      for (int i = 0; i < pagesToSearch.size(); i++) {
        int start = std::get<0>(pagesToSearch[i]) * (PAGE_SIZE + HEADER_SIZE);
        int overflow, freeSpace, nr;
        fseek(f,0,SEEK_CUR);
        if (! fseek(f, start, SEEK_SET)) {
          fread(&overflow,sizeof(int),1,f);
          fread(&freeSpace,sizeof(int),1,f);
          fread(&nr,sizeof(int),1,f);
        }

        int running_length  = 0;
        int recordLength = 0;

        for (int j = 0; j < nr; j++ ){

          running_length += recordLength;
          int recordStart = start + HEADER_SIZE + running_length;
          int s_id;
          fseek(f,0,SEEK_CUR);
          if (! fseek(f, recordStart + 2*sizeof(int), SEEK_SET)) {
            fread(&s_id,sizeof(int),1,f);
          }

          if (id == s_id) { //found an instance of the record
            int manager_id,qid;
            int nameSize, bioSize;
            found = true;
            if (!fseek(f,recordStart,SEEK_SET)) {
              fread(&nameSize,sizeof(int),1,f);
            }

            if (!fseek(f,recordStart + sizeof(int) ,SEEK_SET)) {
              fread(&bioSize,sizeof(int),1,f);
            }

            if (!fseek(f,recordStart + (2* sizeof(int)) ,SEEK_SET)) {
              fread(&qid,sizeof(int),1,f);
            }

            if (!fseek(f,recordStart + (3* sizeof(int)) ,SEEK_SET)) {
              fread(&manager_id,sizeof(int),1,f);
            }

            char name[201];

            if (!fseek(f,recordStart + (4* sizeof(int)) ,SEEK_SET)) {
              fread(name,nameSize+1,1,f);
            }
            char bio[501];
            if (!fseek(f,recordStart + (4* sizeof(int)) + nameSize + 1,SEEK_SET)) {
              fread(bio,bioSize+ 1,1,f);
            }

            std::vector<std::string> v = {to_string(qid),name,bio,to_string(manager_id)};
            Record r1(v);
            records.push_back(r1);

            r1.id = qid;
            r1.name = name;
            r1.manager_id = manager_id;
            r1.bio = bio;

          }


           if (! fseek(f, start + 3*sizeof(int) + j*sizeof(int), SEEK_SET)) {
             fread(&recordLength,sizeof(int),1,f);
           }


        }


      }
      fclose(f);
      return records;
    }
};
