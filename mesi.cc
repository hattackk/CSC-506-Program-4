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

static const bool debug = false;
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

void MESI::PrRd(ulong addr, int processor_number) {
    cache_state state;
    current_cycle++;
    reads++;
    // See if we have the line in our cache
    cache_line *line = find_line(addr);
    if ((line == NULL) || (line->get_state() == I)) {
        // No line found, lets allocate a new one.
        cache_line *newline = allocate_line(addr);
        read_misses++;
        // check to see if the directory knows about this line
        dir_entry *entry = directory->find_dir_line(newline->get_tag());
        if (entry != NULL) {
            // We got back a line from the directory for this. Someone else must
            // have it. Go check.
            if (entry->get_state() == U) {
                // We found a line, but it isn't owned by anyone. So we will
                // take it over. We are now exclusive
                newline->set_state(E);
                // We will turn out bit on in the directory line
                entry->add_sharer_entry(processor_number);
                // We will now set the directory state to EM
                entry->set_dir_state(EM);
                // we will now assign our tag for the directory line.
                entry->set_dir_tag(newline->get_tag());
            } else {
                // We got back a line from the directory for this. But it is in
                // a valid state.
                newline->set_state(S);
                // We now need to generate a signal read so all other processors
                // know to update their states.
                signal_rds++;
                signalRd(addr, processor_number);
            }
        } else {
            // Directory entry was null. So we didn't find our tag in the
            // directory. No one has it so we now own it.
            newline->set_state(E);
            dir_entry *newentry =
                directory->find_empty_line(newline->get_tag());
            // check to make sure the new line's vector is all 0's, this is just
            // a sanity check
            bool check = newentry->is_cached(num_processors);
            if (check) {
                printf("new directory line was not empty?\n");
            }
            // We will now add ourselves as a owner to the directory's bit
            // vector
            newentry->add_sharer_entry(processor_number);
            // We will now set the directory entry state to EM since we are the
            // only ones who own it.
            newentry->set_dir_state(EM);
            // We will set the directory entry tag to our cached value.
            newentry->set_dir_tag(newline->get_tag());
        }
    } else {
        // Our cache line wasn't null. This means we already have this value in
        // our cache.
        // What is our current cache state?
        state = line->get_state();
        if (state == E || state == M || state == S) {
            // We are only performing a read so as long as our state isn't I we
            // should be good to read it and update the LRU
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
    // See if we already have this line in our cache
    cache_line *line = find_line(addr);
    if ((line == NULL) || (line->get_state() == I)) {
        write_misses++;
        // No line found. We didn't have it so allocate a new one.
        cache_line *newline = allocate_line(addr);
        // check to see if the directory knows about this line
        dir_entry *entry = directory->find_dir_line(newline->get_tag());
        if (entry != NULL) {
            // We got back a line from the directory for this. Someone else must
            // have it. Go check.
            if (entry->get_state() == U) {
                // We found a line, but it isn't owned by anyone. So we will
                // take it over. We are now Modified.
                newline->set_state(M);
                // We will turn our bit on in the directory line.
                entry->add_sharer_entry(processor_number);
                // We will now set the directory state to EM
                entry->set_dir_state(EM);
                // We will now assign our tag for the directory line.
                entry->set_dir_tag(newline->get_tag());
            } else {
                // We got back a line from the directory for this. But it is in
                // a valid state. We need to set our state to M for modify, and
                // send a signalRdx so the directory can invalidate everyone
                // else.
                newline->set_state(M);
                // We now need to generate a signal write so all other
                // processors can invalidate their states
                signal_rdxs++;
                signalRdX(addr, processor_number);
            }
        } else {
            // Directory entry was null. So we didn't find out tag in the
            // directory. No one has it so we now own it.
            newline->set_state(M);
            dir_entry *newentry =
                directory->find_empty_line(newline->get_tag());
            // check to make sure the new line's vector is all 0's, this is just
            // a sanity check
            bool check = newentry->is_cached(num_processors);
            if (check) {
                printf("new directory line was not empty?\n");
            }
            // We will now add ourselves as an owner and set the vector bits
            newentry->add_sharer_entry(processor_number);
            // We will now set the directory entry state to EM since we are the
            // only ones who own it.
            newentry->set_dir_state(EM);
            // We will set the diretory entry tag to our cached value.
            newentry->set_dir_tag(newline->get_tag());
        }
    } else {
        // Our cache line wasn't null. This means we already have this value in
        // our cache.
        // What is our current cache state?
        state = line->get_state();
        switch (state) {
            case E:
                // We are in exclusive state. That means we can change at will.
                // Set the state to M.
                line->set_state(M);
                update_LRU(line);
                break;
            case M:
                // We are in modified state. That means we can change at will
                // and keep the same state.
                update_LRU(line);
                break;
            case S:
                // We are in shared state. We need to perform a state upgrade so
                // we can change to state M. We are now going to M state
                line->set_state(M);
                // update the last recenty used line so we know it is less
                // likely to get evicted.
                update_LRU(line);
                // We now need to drive a signal upgrade to the interconnect so
                // it is able to invalidate all other sharers.
                signal_upgrs++;
                signalUpgr(addr, processor_number);
                break;
            default:
                /*noop*/;
        }
    }
}

cache_line *MESI::allocate_line(ulong addr) {
    // We have been asked to allocate a new line
    ulong tag;
    cache_state state;
    // Find us a line that can be replaced
    cache_line *victim = find_line_to_replace(addr);
    assert(victim != 0);
    state = victim->get_state();
    // if the line to be replaced is in Modify state, then we need to write it
    // back
    if (state == M) {
        write_back(addr);
    }
    // What is the tag of our vicitim?
    ulong victim_tag = victim->get_tag();
    // Did our victim live in the directory?
    dir_entry *dir_line = directory->find_dir_line(victim_tag);
    if (dir_line != NULL) {
        // Yes our victim had a valid directory entry
        // We need to remove that victim from that entry
        dir_line->remove_sharer_entry(cache_num);
        int present = 0;
        // is the current directory entry shared by anyone else?
        present = dir_line->is_cached(num_processors);
        if (!present) {
            // No, so set it to uncached so it can be used as a free line later
            dir_line->state = U;
            dir_line->set_dir_tag(0);
        }
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

    // Find the corresponding tag for the address in question
    ulong requestedTag = tag_field(addr);
    // Find the corresponding directory entry for the given tag
    dir_entry *entry = directory->find_dir_line(requestedTag);

    if (entry != NULL) {
        // We found a tag in the directory.
        // What is the state of that entry?
        dir_state requestedDirectoryEntryState = entry->get_state();
        if (requestedDirectoryEntryState == EM) {
            // Directory entry state was EM which means someone either has it in
            // M or E state. Since I'm coming here to do a Read I need to
            // downgrade any other caches to S and call a cache2cache flush for
            // processors with state M.

            // Someone has this in either E or M state. The directory doesn't
            // know so that processor do a cache to cache transfer.
            cache2cache++;
            // Now we need to send an intervention to the other caches to let
            // them know they need to downgrade their states
            entry->sendInt_to_sharer(addr, processor_number, num_processors);
            // Downgrade the state of the directory entry since we are all
            // sharers now.
            entry->set_dir_state(S_);
            // We now need to turn on our bit in the vector since we are a new
            // sharer.
            entry->add_sharer_entry(processor_number);

        } else if (requestedDirectoryEntryState == S_) {
            // Directory entry state was S_ which means we have 1 or more
            // processors with this cache in shared state.

            // So since we are in shared state, no one else must have this block
            // in E or M state We should only have to add ourselves as a new
            // sharer.
            entry->add_sharer_entry(processor_number);

        } else {
            // Directory state must be in Uncached state and is free to use
            // Surpised we got an Uncached state. This invoking processor should
            // have done that before calling us. But just in case the full moon
            // is out, fix it here.
            bool check = entry->is_cached(num_processors);
            // Once again, we do a sanity check.
            if (check) {
                printf("Shouldn't have an Uncached block with set entries \n");
            }
            entry->set_dir_state(EM);
            entry->add_sharer_entry(processor_number);
            // No need to update the entry tag. That is how we found it in the
            // first place. Besides, you should never have reached this to begin
            // with.
        }
    } else {
        // No directory entry found, lets create a new one.
        dir_entry *newentry = directory->find_empty_line(requestedTag);
        // And another sanity check.
        bool check = entry->is_cached(num_processors);
        if (check) {
            printf("The newly requested line wasn't empty?\n");
        }
        // We don't have anyone to mess with since this line didn't exist in the
        // directory. Add this processor as the sharer
        newentry->add_sharer_entry(processor_number);
        // Set the directory's line to EM since we are the only ones who own it.
        newentry->set_dir_state(EM);
        // Set the tag
        newentry->set_dir_tag(requestedTag);
    }
}

void MESI::signalRdX(ulong addr, int processor_number) {
    // YOUR CODE HERE
    // Refer to signalRd description in the handout

    // Find the corresponding tag for the addres in question
    ulong requestedTag = tag_field(addr);
    // Find the corresponding directory entry for the given tag
    dir_entry *entry = directory->find_dir_line(requestedTag);

    if (entry != NULL) {
        // We found a tag in the directory
        // What is the state of the entry?
        dir_state requestedDirectoryEntryState = entry->get_state();
        if (requestedDirectoryEntryState == EM) {
            // Directory entry state was in EM which means someone either has it
            // in M or E state. Since I'm coming here to do a Write I need to
            // invalidate any other caches to I and call a cache2cache flush for
            // processors with state M

            // Someone has this in either E or M state. The directory doesn't
            // know so that processor should do a cache2cache transfer.
            cache2cache++;
            // Now we need to send an invalidation to the other caches to let
            // them know their value is no longer valid. This invalidation call
            // should also remove the processors bit from the vector/list.
            entry->sendInv_to_sharer(addr, processor_number, num_processors);
            // We now need to change the state of this directory entry to EM.
            // Since we are the only ones who hold a valid entry.
            entry->set_dir_state(EM);
            // We also now need to add our selves as a owner on the vector/list.
            entry->add_sharer_entry(processor_number);
        } else if (requestedDirectoryEntryState == S_) {
            // Directory entry state was in S_ which means we have 1 or more
            // processors with this cache in shared state.

            // Our state is shared, so 1 or more processors have this block in
            // shared state. Since no one is in M state a cache2cache is not
            // required. Just an invalidation is required.
            entry->sendInv_to_sharer(addr, processor_number, num_processors);
            // Now that everyone else is invalidated, and by operation should
            // have their bits turned to false. We are free to move this entry
            // into a EM state.
            entry->set_dir_state(EM);
        } else {
            // Directory state must be in Uncached state and is free to use
            // Surpised we got an Uncached state. This invoking processor should
            // have done that before calling us. But just in case the full moon
            // is out, fix it here.
            bool check = entry->is_cached(num_processors);
            // time to do that sanity check again
            if (check) {
                printf("Shouldn't have an uncached block with set entries\n");
            }
            entry->set_dir_state(EM);
            entry->add_sharer_entry(processor_number);
            // Again no need to update the tag since it is how we found
            // ourselves here in the first place.
        }
    } else {
        // No directory entry found, lets create a new one.
        dir_entry *newentry = directory->find_empty_line(requestedTag);
        // Do that sanity check thing.
        bool check = entry->is_cached(num_processors);
        if(check){
            printf("The newly requested line wasn't empty?\n");
        }
        // No one to invalidate on this go around since the entry was already in U state.
        // Add this processor as a sharer.
        newentry->add_sharer_entry(processor_number);
        // Set the directory's line to EM since we are the only ones who own it.
        newentry->set_dir_state(EM);
        // Set the tag
        newentry->set_dir_tag(requestedTag);
    }

//This code removal caused a regression in the output. It may not be an actually issue
//But I'm leaving this here until after I complete the refactor.
/*     
    cache_line *line = find_line(addr);

    if (line != NULL) {
        ulong currentTag = line->get_tag();
        dir_entry *entry = directory->find_dir_line(currentTag);
        // Directory doesn't contain an entry for this data.
        if (entry == NULL || entry->get_state() == U) {
            signal_rdxs--;
            // Find me an empty spot in the directory to place this new line
            entry = directory->find_empty_line(currentTag);
            // Set the entry tag so we can find it later
            entry->set_dir_tag(currentTag);
            // Set to exclusive since this is the first processor to ask for it
            entry->set_dir_state(EM);
            // Update vector bit to represent this processor's cache state
            entry->add_sharer_entry(processor_number);
        } else { We did find it, so we need to check some thing before we
                    proceed.
            switch (entry->get_state()) {
                // Directory has this block as Exclusive/Modify. We need to
                // invalidate it.
                case EM:
                    cache2cache++;
                    if (debug) {
                        printf("%d requested a RDx block that was in EM\n",
                               processor_number);
                    }
                    entry->sendInv_to_sharer(addr, processor_number,
                                             num_processors);
                    // Leaving state in E/M since this was a RDx
                    // Add requesting Processor to the vector as a sharer.
                    entry->add_sharer_entry(processor_number);
                    break;
                Directory has the Block as shared, there are likely other
                procs with the same state. No need to update directory state.
                case S_:
                    if (debug) {
                        printf("%d requested a Rdx block that was in S_\n",
                               processor_number);
                    }
                    // Add requesting processor to vector as a sharer.
                    entry->sendInv_to_sharer(addr, processor_number,
                                             num_processors);
                    entry->add_sharer_entry(processor_number);
                    entry->set_dir_state(EM);
                    break;
                default:
                    if (debug) {
                        printf("Default state%d\n", entry->get_state());
                    }
                    entry->add_sharer_entry(processor_number);
                    entry->set_dir_state(EM);
                    ;
            }
        }
    } 
    */
}

void MESI::signalUpgr(ulong addr, int processor_number) {
    // YOUR CODE HERE
    // Refer to signalUpgr description in the handout
    cache_line *line = find_line(addr);

    if (line != NULL) {
        ulong currentTag = line->get_tag();
        dir_entry *entry = directory->find_dir_line(currentTag);
        if (entry == NULL) {
            ;
        } else {
            switch (entry->get_state()) {
                // Directory has this block as Exclusive/Modify. We need to
                // invalidate it.
                case EM:
                    if (debug) {
                        printf("%d requested %ld a Upgr block that was in EM\n",
                               processor_number, currentTag);
                    }
                    if (entry->is_cached(num_processors) == 0) {
                        signal_upgrs--;
                    }
                    // sendInv(addr, processor_number);
                    // Leaving state in E/M since this was a RDx
                    // entry->set_dir_state(S_);
                    // Add requesting Processor to the vector as a sharer.
                    entry->add_sharer_entry(processor_number);
                    break;
                /*Directory has the Block as shared, there are likely other
                procs with the same state. No need to update directory state.*/
                case S_:
                    if (debug) {
                        printf("%d requested a Upgr block that was in S_\n",
                               processor_number);
                    }
                    // Add requesting processor to vector as a sharer.
                    // sendInv(addr,processor_number);
                    entry->sendInv_to_sharer(addr, processor_number,
                                             num_processors);
                    entry->add_sharer_entry(processor_number);
                    entry->set_dir_state(EM);
                    break;
                default:
                    /*noop*/;
            }
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
    if (line != NULL) {
        state = line->get_state();
        if (state == M || state == E) {
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

    cache_state state;
    cache_line *line = find_line(addr);
    if (line != NULL) {
        if (debug) {
            printf("%p I'm getting invalidated?\n", line);
        }
        state = line->get_state();
        if (debug) {
            printf("Invalidation state:%d\n", state);
        }
        switch (state) {
            case E:
                if (debug) {
                    printf("exclu state invalidated\n");
                }
                invalidations++;
                line->set_state(I);
                break;
            case M:
                if (debug) {
                    printf("mod state invalidated\n");
                }
                invalidations++;
                line->set_state(I);
                write_backs++;
                break;
            case S:
                if (debug) {
                    printf("shared state invalidated\n");
                }
                invalidations++;
                line->set_state(I);
                break;
            default:
                /*noop*/;
        }
        if (debug) {
            printf("%p Final state %d\n", line, line->get_state());
        }
    } else {
        // printf("Line not found\n");
    }
}