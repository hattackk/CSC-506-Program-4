/************************************************************
			Course		: 	CSC/ECE506
			Source 		:	mesi.cc
			Owner		:	Ed Gehringer
			Email Id	:	efg@ncsu.edu
*************************************************************/
		     
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
using namespace std;
#include "cache.h"

#include "main.h"
#include "mesi.h"

void MESI::PrRd(ulong addr, int processor_number) {
	// YOUR CODE HERE
	// The below comments are for reference and might not be
	// sufficient to match the debug runs. 
 
	// Update the Per-cache global counter to maintain LRU 
	// order among cache ways, updated on every cache access

	// Increment the global read counter.

	// Check whether the line is cached in the processor cache. 
	// If not cached, allocate a fresh line and update the state. (M,E,S,I)
	// Do not forget to update miss/hit counter

	// Increment the directory operation counter like signalrds, 
	// response_replies etc... Invoke the relevant directory
	// signal functions like signalRd or signalRdX etc...

	// If the line is cached in the processor cache, do not forget
	// to update the LRU
}

void MESI::PrWr(ulong addr, int processor_number) {
	// YOUR CODE HERE
	// Refer comments for PrRd
}

cache_line * MESI::allocate_line(ulong addr) { 
    ulong tag;
    cache_state state;
   
    cache_line *victim = find_line_to_replace(addr);
    assert(victim != 0);
    state = victim->get_state();
    if (state == M) {
        write_back(addr);
    }

	ulong victim_tag = victim->get_tag();	
	dir_entry* dir_line = directory->find_dir_line(victim_tag);
	if(dir_line!=NULL)	{
		dir_line->remove_sharer_entry(cache_num);
		int present = 0;
		present = dir_line->is_cached(num_processors);
		if(!present)
			dir_line->state = U;
	}

    tag = tag_field(addr);   
    victim->set_tag(tag);
    victim->set_state(I);
    return victim;
}

void MESI::signalRd(ulong addr, int processor_number) {
	// YOUR CODE HERE
	// The below comments are for reference and might not be
	// sufficient to match the debug runs.

	// Check whether the directory entry is updated. If not updated,
	// create a fresh entry in the directory, update the sharer vector or list.
	
	// Check the directory state and update the cache2cache counter
	// Update the directory state (U, EM, S_).

	// Send Intervention or Invalidation

	// Update the vector/list
}

void MESI::signalRdX(ulong addr, int processor_number) {
	// YOUR CODE HERE
	// Refer to signalRd description in the handout
}

void MESI::signalUpgr(ulong addr, int processor_number) {
	// YOUR CODE HERE
	// Refer to signalUpgr description in the handout
}

void MESI::Int(ulong addr) {
	// YOUR CODE HERE
	// The below comments are for reference and might not be
	// sufficient to match the debug runs.

	// Update the relevant counter, if the cache copy is dirty,
	// same needs to be written back to main memory. This is 
	// achieved by simply updating the writeback counter
}

void MESI::Inv(ulong addr) { 
	
	// YOUR CODE HERE
	// Refer Inv description in the handout

	// Update the relevant counter, if the cache copy is dirty,
	// same needs to be written back to main memory. This is 
	// achieved by simply updating the writeback counter
}