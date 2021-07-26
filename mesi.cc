/************************************************************
                        Course		: 	CSC/ECE506
                        Source 		:	mesi.cc
                        Owner		:	Ed Gehringer
                        Email Id	:	efg@ncsu.edu
*************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
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

    cache_state state;
    current_cycle++;
    reads++;
    cache_line *line = find_line(addr);
    if ((line == NULL) || (line->get_state() == I)) {
        //printf("P:%d | A:%ld\n",processor_number,addr);
        cache_line *newline = allocate_line(addr);
        read_misses++;
        if (sharers_exclude(addr, processor_number) > 0) {
            cache2cache++;
            newline->set_state(S);
        } else {
            memory_transactions++;
            newline->set_state(E);
        }

        signal_rds++;

        signalRd(addr, processor_number);
    } else {
        state = line->get_state();
        if (state == E || state == M || state == S) {
            update_LRU(line);
        }
    }
}

void MESI::PrWr(ulong addr, int processor_number) {
    // YOUR CODE HERE
    // Refer comments for PrRd
    cache_state state;
    current_cycle++;
    writes++;
    cache_line *line = find_line(addr);

    if ((line == NULL) || (line->get_state() == I)) {
        write_misses++;
        cache_line *newline = allocate_line(addr);
        if (sharers_exclude(addr, processor_number) > 0) {
            cache2cache++;
        } else {
            memory_transactions++;
        }
        newline->set_state(M);
        signal_rdxs++;
        signalRdX(addr, processor_number);
    } else {
        state = line->get_state();
        if (state == E) {
            line->set_state(M);
            update_LRU(line);
        }

        else if (state == M) {
            update_LRU(line);
        }

        else if (state == S) {
            line->set_state(M);
            update_LRU(line);
            signal_upgrs++;
            signalUpgr(addr, processor_number);
        }
    }
}

cache_line *MESI::allocate_line(ulong addr) {
    ulong tag;
    cache_state state;

    cache_line *victim = find_line_to_replace(addr);
    assert(victim != 0);
    state = victim->get_state();
    if (state == M) {
        write_back(addr);
    }

    ulong victim_tag = victim->get_tag();
    dir_entry *dir_line = directory->find_dir_line(victim_tag);
    if (dir_line != NULL) {
        ////printf("%ld Line wasn't null, removing state\n",victim_tag);
        dir_line->remove_sharer_entry(cache_num);
        int present = 0;
        present = dir_line->is_cached(num_processors);
        if (!present) dir_line->state = U;
        
    }

    tag = tag_field(addr);
    victim->set_tag(tag);
    //dir_line->set_dir_tag(tag);
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
/*
    //cache_state state;
    cache_line *line = find_line(addr);
    dir_state state;
    //dir_entry *dir_line = directory->find_dir_line(addr);


    if (line != NULL) {
        ulong currentTag = line->get_tag();
        //printf("P%d is Looking for Tag:%ld\n",processor_number,currentTag);
        dir_entry *entry = directory->find_dir_line(currentTag);
        if(entry==NULL){
            //printf("Didn't find %ld\n",currentTag);
            entry = directory->find_empty_line(currentTag);
            entry->set_dir_tag(currentTag);
            entry->set_dir_state(EM);
            //printf("Updated tag:%ld with a state of %d\n",entry->get_dir_tag(),entry->get_state());
            entry->add_sharer_entry(processor_number);
        }else{
            //printf("Found Tag %ld\n",currentTag);
            entry->add_sharer_entry(processor_number);
            entry->set_dir_state(S_);
        }
        //printf("Entry State:%d\n",entry->get_state());
    
        state = entry->get_state();
        if (state == EM) {
            interventions++;
            Int(addr);
            flushes++;
            // write_backs++;
            memory_transactions++;
            entry->set_dir_state(S_);
        }

        else if (state == S) {}
        else if (state == E) {
            interventions++;
            line->set_state(S);
            Int(addr);
        }
    
    }
    */

   cache_line *line = find_line(addr);
   
   if(line != NULL){
       ulong currentTag = line -> get_tag();
       dir_entry *entry = directory->find_dir_line(currentTag);
       if(entry == NULL){ //Directory doesn't contain an entry for this data.
            entry = directory->find_empty_line(currentTag);// Find me an empty spot in the directory to place this new line
            entry->set_dir_tag(currentTag);//Set the entry tag so we can find it later
            entry->set_dir_state(EM);//Set to exclusive since this is the first processor to ask for it
            entry->add_sharer_entry(processor_number);//Update vector bit to represent this processor's cache state
        }else{//We did find it, so we need to check some thing before we proceed.
            switch(entry->get_state()){
                case EM:
                    printf("%d requested a block that was in EM\n",processor_number);
                    //interventions++;
                    sendInt(addr,processor_number);
                    entry->set_dir_state(S_);
                    break;
                case S_:
                    printf("%d requested a block that was in S_\n",processor_number);
                    break;
                default:
                    printf("Default action taken?");
            }

        }
   }
}

void MESI::signalRdX(ulong addr, int processor_number) {
    // YOUR CODE HERE
    // Refer to signalRd description in the handout
    cache_state state;
    cache_line *line = find_line(addr);

    if (line != NULL) {
        state = line->get_state();

        if (state == S) {
            invalidations++;
            line->set_state(I);
        }

        else if (state == M) {
            flushes++;
            write_backs++;
            memory_transactions++;
            invalidations++;
            line->set_state(I);
        }

        else if (state == E) {
            invalidations++;
            line->set_state(I);
        }
    }
}

void MESI::signalUpgr(ulong addr, int processor_number) {
    // YOUR CODE HERE
    // Refer to signalUpgr description in the handout
    cache_state state;
    cache_line *line = find_line(addr);

    if (line != NULL) {
        state = line->get_state();

        if (state == S) {
            line->set_state(I);
            invalidations++;
        }
    }
}

void MESI::Int(ulong addr) {
    // YOUR CODE HERE
    // The below comments are for reference and might not be
    // sufficient to match the debug runs.

    // Update the relevant counter, if the cache copy is dirty,
    // same needs to be written back to main memory. This is
    // achieved by simply updating the writeback counter
    cache_state state;
    cache_line *line = find_line(addr);
    if(line != NULL){
        state = line->get_state();
        if(state==M||state==E){
            interventions++;
            line->set_state(S);
            write_backs++;
        }
    }
}

void MESI::Inv(ulong addr) {
    // YOUR CODE HERE
    // Refer Inv description in the handout

    // Update the relevant counter, if the cache copy is dirty,
    // same needs to be written back to main memory. This is
    // achieved by simply updating the writeback counter
    write_backs++;
}