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
            // have it. Go check. We will be
            if (entry->get_state() == U) {
                // We found a line, but it isn't owned by anyone. So we will
                // take it over. We are now exclusive
                newline->set_state(E);
                // We will turn out bit on in the directory line
                entry->add_sharer_entry(processor_number);
                // We will now set the directory state to EM
                entry->set_dir_state(EM);
                // we will now assign out tag for the directory line.
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
    cache_line *line = find_line(addr);
    if (debug) {
        print_cache_states(addr);
    }
    if ((line == NULL) || (line->get_state() == I)) {
        write_misses++;
        cache_line *newline = allocate_line(addr);
        if (sharers_exclude(addr, processor_number) > 0) {
            if (debug) {
                print_cache_states(addr);
            }
        } else {
            memory_transactions++;
        }
        newline->set_state(M);
        signal_rdxs++;
        signalRdX(addr, processor_number);
    } else {
        if (debug) {
            print_cache_states(addr);
        }
        state = line->get_state();
        if (debug) {
            printf("P%d write state in state %d\n", processor_number, state);
        }
        if (state == E) {
            line->set_state(M);
            update_LRU(line);
        }

        else if (state == M) {
            update_LRU(line);
        }

        else if (state == S) {
            if (debug) {
                printf("PrWr with upgrade from P%d\n", processor_number);
            }
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
    if (debug) {
        printf("Found victim to replace for A:%ld\n", addr);
    }
    state = victim->get_state();
    if (debug) {
        printf("Victims state is %d\n", state);
    }
    if (state == M) {
        write_back(addr);
    }

    ulong victim_tag = victim->get_tag();
    if (debug) {
        printf("Victim's tag is %ld\n", victim_tag);
    }
    dir_entry *dir_line = directory->find_dir_line(victim_tag);
    if (dir_line != NULL) {
        dir_line->remove_sharer_entry(cache_num);
        int present = 0;
        if (debug) {
            printf("-----\n");
        }
        present = dir_line->is_cached(num_processors);
        if (!present) {
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

    cache_line *line = find_line(addr);

    if (line != NULL) {
        ulong currentTag = line->get_tag();

        dir_entry *entry = directory->find_dir_line(currentTag);
        // Directory doesn't contain an entry for this data.
        if (entry == NULL) {
            if (debug) {
                printf("*\n");
                printf("P%d didn't find A(%ld) in directory\n",
                       processor_number, addr);
            }
            signal_rds--;
            // Find me an empty spot in the directory to place this new line
            entry = directory->find_empty_line(currentTag);
            // Set the entry tag so we can find it later
            entry->set_dir_tag(currentTag);
            // Set to exclusive since this is the first processor to ask for it
            entry->set_dir_state(EM);
            // Update vector bit to represent this processor's cache state
            entry->add_sharer_entry(processor_number);
        } else { /*We did find it, so we need to check some thing before we
                    proceed.*/
            switch (entry->get_state()) {
                // Directory has this block as Exclusive/Modify. We need to
                // downgrade the owner cache to S.
                case EM:
                    cache2cache++;
                    if (debug) {
                        printf("%d requested RD a block that was in EM\n",
                               processor_number);
                    }
                    entry->sendInt_to_sharer(addr, processor_number,
                                             num_processors);
                    // sendInt(addr, processor_number);
                    // Set the directory state to Shared.
                    entry->set_dir_state(S_);
                    // Add requesting Processor to the vector as a sharer.
                    entry->add_sharer_entry(processor_number);
                    break;
                /*Directory has the Block as shared, there are likely other
                procs with the same state. No need to update directory state.*/
                case S_:
                    if (debug) {
                        printf("%d requested a Rd block that was in S_\n",
                               processor_number);
                    }
                    // Add requesting processor to vector as a sharer.
                    entry->add_sharer_entry(processor_number);
                    break;
                default:
                    /*noop*/;
            }
        }
    }
}

void MESI::signalRdX(ulong addr, int processor_number) {
    // YOUR CODE HERE
    // Refer to signalRd description in the handout

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
        } else { /*We did find it, so we need to check some thing before we
                    proceed.*/
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
                /*Directory has the Block as shared, there are likely other
                procs with the same state. No need to update directory state.*/
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
                    /*noop*/;
            }
        }
    }
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