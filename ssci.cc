/************************************************************
			Course		: 	CSC/ECE506
			Source 		:	ssci.cc
			Owner		:	Ed Gehringer
			Email Id	:	efg@ncsu.edu
*************************************************************/
#include "ssci.h"

void SSCI::add_sharer_entry(int proc_no){
	printf("add_sharer_entry");
    std::list<int>::iterator itr;

	for (itr = cache_list.begin(); itr != cache_list.end(); std::advance(itr, 1)) {
        if (*itr == proc_no) {
            return;
        }
	}

	cache_list.push_back(proc_no);
}

void SSCI::remove_sharer_entry(int proc_num){
	printf("remove_sharer_entry");
	cache_list.remove(proc_num);
}

int SSCI::is_cached(int proc_num){	
	printf("is_cached");
	if (cache_list.size() > 0) return 1;
	return 0;
}

int SSCI::others_are_sharing(int proc_num, int num_proc) {
	printf("others_are_sharing")
	std::list<int>::iterator itr;

	for (itr = cache_list.begin(); itr != cache_list.end(); std::advance(itr, 1)) {
		if (*itr != proc_num) {
            return true;
		}
	}

	return false;
}

void SSCI::sendInv_to_sharer(ulong addr, int num_proc, int proc_num){
	printf("sendInv_to_sharer");
	// YOUR CODE HERE
	//
	// Erase the entry from the list except for the latest entry
	// The latest entry will be for the processor which is invoking
	// this function
	cache_list.clear();
	cache_list.push_back(proc_num);
	// Invoke the sendInv function defined in the main function
	sendInv(addr, proc_num);

}

void SSCI::sendInt_to_sharer(ulong addr, int num_proc, int proc_num){
	printf("sendInt_to_sharer");
	// YOUR CODE HERE
	//
	// Invoke the sendInt function defined in the main function
	// for all the entries in the list except for proc_num. 
	for (int p = 0; p < num_proc; p++) {
		if ((p != proc_num)) {
			sendInv(addr, p);
		}
	}
}
