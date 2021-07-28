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
    if (debug) {
        printf("A(%p) P%d is making a rd for A%ld\n", line, processor_number,
               addr);
        print_cache_states(addr);
    }
    if ((line == NULL) || (line->get_state() == I)) {
        if (debug) {
            printf("P:%d | A:%ld\n", processor_number, addr);
        }
        cache_line *newline = allocate_line(addr);
        read_misses++;
        if (sharers_exclude(addr, processor_number) > 0) {
            // printf("P%d found shareres of %ld\n",processor_number,addr);
            // cache2cache++;
            newline->set_state(S);
            // signal_rds++;
            // signalRd(addr, processor_number);
        } else {
            // printf("P%d found no shareres of %ld\n",processor_number,addr);
            memory_transactions++;
            newline->set_state(E);
        }

        signal_rds++;

        signalRd(addr, processor_number);
    } else {
        if (debug) {
            print_cache_states(addr);
        }
        state = line->get_state();
        if (debug) {
            printf("A(%p)_P%d found %ld in state %d\n", line, processor_number,
                   addr, state);
        }
        if (state == E || state == M || state == S) {
            update_LRU(line);
        }
    }
}

void MESI::PrWr(ulong addr, int processor_number) {
    // YOUR CODE HERE
    // Refer comments for PrRd

    // printf("write\n");
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
            // cache2cache++;
            // printf("PrWr from P%d\n",processor_number);
            if (debug) {
                print_cache_states(addr);
            }
            // newline->set_state(M);
            // signal_rdxs++;

            // signalRdX(addr, processor_number);
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
    state = victim->get_state();
    if (state == M) {
        write_back(addr);
    }

    ulong victim_tag = victim->get_tag();
    dir_entry *dir_line = directory->find_dir_line(victim_tag);
    if (dir_line != NULL) {
        //////printf("%ld Line wasn't null, removing state\n",victim_tag);
        dir_line->remove_sharer_entry(cache_num);
        int present = 0;
        present = dir_line->is_cached(num_processors);
        if (!present) dir_line->state = U;
    }

    tag = tag_field(addr);
    victim->set_tag(tag);
    // dir_line->set_dir_tag(tag);
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
    /*
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
    */
    cache_line *line = find_line(addr);

    if (line != NULL) {
        ulong currentTag = line->get_tag();
        dir_entry *entry = directory->find_dir_line(currentTag);
        // Directory doesn't contain an entry for this data.
        if (entry == NULL|| entry->get_state()==U) {
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
                    // sendInv(addr, processor_number);
                    entry->sendInv_to_sharer(addr, processor_number,
                                             num_processors);
                    // Leaving state in E/M since this was a RDx
                    // entry->set_dir_state(S_);
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
                    // sendInv(addr,processor_number);
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
                    // cache2cache++;
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