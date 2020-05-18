//Programmer: Michal Kowalczyk
//ZID: 1843873
//Assignment 7
//Purpose: Simulation of a File Allocation Table (FAT12)

#include <cstring>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <iomanip>
#include <vector>
#include <list>

#define HOWOFTEN 6
#define BLOCK 512
#define ENTRIES 12
#define ROWS 20
using namespace std;

//***************************************************************************************************************************************************************************
//Class: Entry
//Variables: string fname - file name
//           int fsize - file size
//           short fblock - short value to represent the first block of the entry/program
class Entry{ 
private:
	string fname;
        int fsize = -1;
        short fblock;
public:

	Entry(string x, short y, short z){
		fname = x;
		fsize = y;
		fblock = z;
	}
	Entry(string x, short y){
		fname = x;
		fsize = y;
		if(y == 0)
			clusters.push_back(-1);
	}

	void setfname(string); //setFileName
	void setfsize(short); //setFileSize
	void setfblock(short); //setFileblock (the first block for the file)

	string getfname();
	short getfsize();
	short getfblock();
	void printClusters();
	list<short> clusters;
};

void Entry::setfname(string fname){
	this->fname = fname;
}

void Entry::setfsize(short fsize){
        this->fsize = fsize;
}

void Entry::setfblock(short fblock){
        this->fblock = fblock;
}

string Entry::getfname(){
	return fname;
}

short Entry::getfsize(){
        return fsize;
}

short Entry::getfblock(){
        return fblock;
}

//Method: printClusters()
//Parameters: none
//returns: void (just a print statement)
//Purpose: a printer helper function to list out the clusters for the given class

void Entry::printClusters(){
	for(list<short>::iterator it = clusters.begin(); it != clusters.end(); ++it){
		if(*it < 0 || clusters.empty()){
			cout << setw(5) << "(none)";
			break;
		}else{
			cout << setw(5) << *it;
		}
	}
	cout << endl;
}


//************************************************END OF CLASS SPECIFIC FUNCTIONS*********************************************************************************************
//Function: printFAT(short [][])
//Parameters: fat - the 2D array to hold our values for our fat table
//Returns: void (just a print statement)
//Purpose: to print out the contents of our FAT table

void printFAT(short fat[][ENTRIES]){
	cout << "Contents of the File Allocation Table" << endl;
	for(int i = 0; i < ROWS; i++){
		cout << "#" << setw(3) << i*12 << " - " << setw(3) << ((i + 1)*12)-1;
		for(int j = 0; j < ENTRIES; j++){
			cout << setw(10) << fat[i][j];
		}
		cout << endl;
	}
}
//Function: printDirectoryList(list<Entry>)
//Parameters: dlist - a list of all the entries currently either inserted or copied into our FAT table
//Returns: void - (just a print statement)
//Purpose: to print out the entries, as well as their corresponding clusters, also prints out how many files total there are and the total space taken up in bytes
void printDirectoryList(list<Entry> dlist){
	cout << "Directory Listing " << endl;
	short counter = 0;
	int tSize = 0;
	for(list<Entry>::iterator it = dlist.begin(); it != dlist.end(); ++it){
		cout << "File Name: " << it->getfname() << setw(15) << "File Size: " << setw(5) << it->getfsize() << endl;
		cout << "Cluster(s) in use: ";
		it->printClusters();
		counter++;
		tSize+= it->getfsize();
	}
	cout << "Files: " << setw(5) << counter << " Total Size: " << setw(10) << tSize << " bytes " << endl << endl;
}

//Function: allocateBlocks(Entry&, short, short [][])
//Parameters:object - the Entry object that we are trying to allocate memory for in our FAT table
//	     clustersNeeded - lets us know how many clusters are needed for the Entry object, to allocate in our FAT table
//           fat - our FAT table that we will be adjusting
//Returns: void - alters our fat table
//Purpose: passes in our Entry object, as well as clustersNeeded (previously calculated),to be able to allocate how much space is needed in our FAT table 


void allocateBlocks(Entry &object, short clustersNeeded, short fat[][ENTRIES]){
        short counter = clustersNeeded;
        for(int i = 0; i < ROWS; i++){
                for(int j = 0; j < ENTRIES; j++){
                        if(fat[i][j] == 0){
                                short holder = (i* 12) + j; //tells us which cluster it's going to take up
				if(counter == clustersNeeded)
					object.setfblock(holder); //this is the first block of the program
                                object.clusters.push_back(holder);
                                counter--;
                        }
                if(counter == 0)
                        break;
                }
        if(counter == 0)
                break;
        }
        //Update FAT table
        for(list<short>::iterator it1 = object.clusters.begin(); it1 != object.clusters.end(); ++it1){
                short row = *it1 /12;
                short entry = *it1 % 12;
                if(next(it1) == object.clusters.end())
                        fat[row][entry] = -1; //if this is the end of list of clusters, make the FAT a -1 indicating end of file
                else
                        fat[row][entry] = *next(it1); //points to the next cluster
        }

}

//Function: newEntry(string, short, list<Entry>&, short[][])
//Parameters:f1 - name of the new program
//           size - size of the new program in bytes
//           entries - reference to our list of entries, so we can insert a new Entry object
//           fat - our fat table to adjust, once we've created our Entry Object
//Returns: int - used in the error portion of our code (0 - failed, 1 - successful)
//Purpose: used to create a new Entry object and allocate the memory in our fat table, once completed, the Entry object is added to our list of Entries

int newEntry(string f1, short size, list<Entry> &entries, short fat[][ENTRIES]){
	//check if name already exists 
	for(list<Entry>::iterator it = entries.begin(); it != entries.end(); ++it){
                if(f1.compare(it->getfname()) == 0){
                        cerr << "File Name already exists!" << endl;
			return 0;
			}
        }
	short clustersNeeded;
	if (size % BLOCK == 0)
		clustersNeeded = size / BLOCK;
	else
		clustersNeeded = (size / BLOCK) + 1;
	Entry object(f1, size);
	allocateBlocks(object, clustersNeeded, fat);
	entries.push_back(object);
	return 1;
}

//Function: deleteEntry(string, list<Entry>&, short[][])
//Parameters: fname - string name of the Entry/program we want to delete
//	      entries - a reference to our list that we want to remove the one program if it exists
//            fat - the FAT table that we want to alter by freeing up clusters if the program we wish to delete exists
//Returns: int - used in the error portion of our code (0 - failed, 1 - successful)
//Purpose:  to delete the program with the corresponding name, only if it exists


int deleteEntry(string fname, list<Entry> &entries, short fat[][ENTRIES]){
	bool copy = false;
        //parse entire list, if you get to the end, error.
        for(list<Entry>::iterator it = entries.begin(); it != entries.end(); ++it){
                if(it->getfname() == fname){
			Entry object = *it;
			for(list<short>::iterator it1 = object.clusters.begin(); it1 != object.clusters.end(); ++it1){
				short row = (*it1 / 12);
				short column (*it1 % 12);
				fat[row][column] = 0;
			}
			it = entries.erase(it);
                        copy = true;
                        break;
                }
        }
        if (copy == false){
                cerr << " File doesn't exist to copy! " << endl;
                return 0;
        }
	return 1;
}
//Function: renameEntry(string, string, list<Entry>)
//Parameters:f1 - string name of our first program
//           f2 - string name of our second program
//           entries - reference to our list of entries, so we can check if the old file name exisists to replace, and if the new file name exisists, so we dont have two files with the same name
//Returns: int - used in the error portion of our code (0 - failed, 1 - successful)
//Purpose: to rename an existing program
int renameEntry(string f1, string f2, list<Entry> &entries){
	bool copy = false; //check if the new name we want to put in exists,
	for(list<Entry>::iterator it = entries.begin(); it != entries.end(); ++it){
                if(it->getfname() == f2){
			cerr << "File name already exists!" << endl;
			return 0;
		}
	} //look for the old file we want.
	for(list<Entry>::iterator it = entries.begin(); it != entries.end(); ++it){
		if(it->getfname() == f1){
                        copy = true;
			it->setfname(f2);
                        break;
                }
        }
        if (copy == false){
                cerr << " File doesn't exist to copy! " << endl;
                return 0;
        }
	return 1; 
}

//Function: modifyEntry(string, short, list<Entry>, short)
//Parameters:f1 - string name of the program
//           size - the integer representation of the file size in bytes
//           entries - the list of Entry objects we have, so we can reference to see if a file exists currently, in order to modify it
//           fat - reference to our FAT table if we alter a program, allocating it new blocks/clusters
//Returns: int - used in the error portion of our code (0 - failed, 1 - successful)
//Purpose: to be able to modify a current program, if it exists, to a new size

int modifyEntry(string f1, short size, list<Entry> &entries, short fat[][ENTRIES]){
	//check if file exists
	bool exists = false;
	for(list<Entry>::iterator it = entries.begin(); it != entries.end(); ++it){
                if(it->getfname() == f1){
                        exists = true;
                        break;
                }
        }
        if (exists == false){
                cerr << " File doesn't exist to copy! " << endl;
                return 0;
        }
	//make a temporary new file
	string temp = "temp";
	int temp2 = 1;   
	temp2 = newEntry(temp, size, entries, fat);
	if(temp2 == 0)
		return 0;//checks if each of these fail
	//delete the old file
	temp2 = deleteEntry(f1, entries, fat);
	if(temp2 == 0)
		return 0;
	// rename the temporary file to the one of the new one
	temp2 = renameEntry(temp, f1, entries);
	if(temp2 == 0)
		return 0;
	return temp2;

}

//Function: copyEntries(string, string, list<Entry>, short [][])
//Parameters:f1 - string name of our first program
//           f2 - string name of our second program
//           entries - list of our Entry objects, to reference to, to see if either our first or second program names already exist in the list of programs
//Returns: int - used in the error portion of our code (0 - failed, 1 - successful)
//Purpose: to be able to copy an existing program and create a new one with a new name, but same size as our pre-existing one

int copyEntries(string f1, string f2, list<Entry> &entries, short fat[][ENTRIES]){
	bool copy = false;
	short newSize;
	short clustersNeeded;
	//parse entire list, if you get to the end, error. 
	for(list<Entry>::iterator it = entries.begin(); it != entries.end(); ++it){
		if(it->getfname() == f1){
			if(it->getfsize() % BLOCK == 0)
				clustersNeeded = it->getfsize() / BLOCK;
			else
				clustersNeeded = (it->getfsize() / BLOCK) + 1;
			newSize = it->getfsize();
			copy = true; 
			break;
		}
	}
	if (copy == false){
		cerr << " File doesn't exist to copy! " << endl; 
		return 0;
	}

	for(list<Entry>::iterator it = entries.begin(); it != entries.end(); ++it){
		if(it->getfname() == f2){
			cerr << "File Name already exists!" << endl; 
			return 0;
		}
	}
	Entry object(f2, newSize);
	allocateBlocks(object, clustersNeeded, fat);
	entries.push_back(object);
	return 1;  
}
int main(){
	list<Entry> entries; 
	short fat[ROWS][ENTRIES] = {};

	//Default Entries
	Entry root(".", BLOCK, 0);
	root.clusters.push_back(0);
	entries.push_back(root);
	fat[0][0] = -1;

	Entry parent("..", 0, -1);
	parent.clusters.push_back(-1);
	entries.push_back(parent);

	cout << "Beginning of the FAT simulation" << endl << endl;
	printDirectoryList(entries);
	printFAT(fat);

	char holder;
	string fname1, fname2;
        short size;
	int counter = 0;
	int error;
	fstream file;
	string filename = "data7.txt";
	file.open(filename.c_str());

	file >> holder;
	//Loop would start here
	while(holder != '?'){
		switch(holder){
			case 'C':
				file >> fname1;
				file >> fname2;
				cout << "Transaction: Copy a file\n";
				error = copyEntries(fname1, fname2, entries, fat);
				if( error == 1)
					cout << "Successfully copied an existing file, " << fname1 << " to a new file, " << fname2 << endl;
				else
					 cout << "Unsuccesful in copying an existing file, " << fname1 << " to a new file, " << fname2 << endl;
				break;

			case 'D':
				file >> fname1;
				cout << "Transaction: Delete a file\n";
				error = deleteEntry(fname1, entries, fat); 
				if( error == 1)
					cout << "Succesfully deleted a file, " << fname1 << endl;
				else
					cout << "Unsuccesful in deleting a file " << fname1 << endl;
				break;

			case 'N':file >> fname1;
				file >> size;
				cout << "Transaction: Add a new file\n"; 
				error = newEntry(fname1, size, entries, fat);
				if( error == 1)
					cout << "Successfuly added a new file, " << fname1 << ", of size " << size << endl;
				else
					cout << "Unsuccesful in adding a new file, " << fname1 << ", of size" << size << endl;
				break;

			case 'M':file >> fname1;
				file >> size;
				cout << "Transaction: Modify a file\n";
				error = modifyEntry(fname1, size, entries, fat);
				if( error == 1)
					cout << "Succesfully modified a file, " << fname1 << endl;
				else
					cout << "Unsuccesful in modifying a file, " << fname1 << endl; 
				break;

			case 'R':
				file >> fname1;
				file >> fname2;
				error = renameEntry(fname1, fname2, entries);
				if ( error == 1)
					cout << "Succesfully changed the file name " << fname1 << " to " << fname2 << endl;
				else
					cout << "Unsuccesful in changing the file name " << fname1 << " to " << fname2 << endl;
				break;
		}
		counter++;
		if(counter == 5){
			cout << endl;
			printDirectoryList(entries);
			printFAT(fat);
			counter = 0;
		}
		file >> holder;
	}
	cout << "\n\nEnd of the FAT simulation\n\n";
	printDirectoryList(entries);
	printFAT(fat);
	return 0; 
}


