/************************************************************
			Course		: 	CSC/ECE506
			Source 		:	ssci.cc
			Owner		:	Ed Gehringer
			Email Id	:	efg@ncsu.edu
*************************************************************/
#include "ssci.h"

void SSCI::add_sharer_entry(int proc_no){
    std::list<int>::iterator itr;

	for (itr = cache_list.begin(); itr != cache_list.end(); std::advance(itr, 1)) {
        if (*itr == proc_no) {
            return;
        }
	}

	cache_list.push_back(proc_no);
}

void SSCI::remove_sharer_entry(int proc_num){
	// YOUR CODE HERE
	//
	// Remove the entry from the linked list
}

int SSCI::is_cached(int proc_num){
	// YOUR CODE HERE
	//
	// Return 1 if the linked list is not empty
	// else return 0
	return 0;
}

int SSCI::others_are_sharing(int proc_num, int num_proc) {
	std::list<int>::iterator itr;

	for (itr = cache_list.begin(); itr != cache_list.end(); std::advance(itr, 1)) {
		if (*itr != proc_num) {
            return true;
		}
	}

	return false;
}

void SSCI::sendInv_to_sharer(ulong addr, int num_proc, int proc_num){
	// YOUR CODE HERE
	//
	// Erase the entry from the list except for the latest entry
	// The latest entry will be for the processor which is invoking
	// this function
	// Invoke the sendInv function defined in the main function
}

void SSCI::sendInt_to_sharer(ulong addr, int num_proc, int proc_num){
	// YOUR CODE HERE
	//
	// Invoke the sendInt function defined in the main function
	// for all the entries in the list except for proc_num. 
}
