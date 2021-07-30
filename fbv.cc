/************************************************************
                        Course		: 	CSC/ECE506
                        Source 		:	fbv.cc
                        Owner		:	Ed Gehringer
                        Email Id	:	efg@ncsu.edu
*************************************************************/
#include "fbv.h"

static const bool debug = false;
void FBV::print_vector() {
    printf("TAG:%ld\nState:%d\n", get_dir_tag(), get_state());
    for (int i = 0; i < 16; i++) {
        printf("[%d]", bit[i]);
    }
    printf("\n");
}

void FBV::add_sharer_entry(int proc_num) {
    if (debug) {
        if (proc_num == 4 && get_dir_tag() == 5342208) {
            print_vector();
        }
    }
    bit[proc_num] = true;
}

void FBV::remove_sharer_entry(int proc_num) {
    if (isValid(proc_num)) {
        bit[proc_num] = false;
    }
    // YOUR CODE HERE
    // Reset the bit vector entry
    // Done. (MA)
}

int FBV::is_cached(int num_proc) {
    // YOUR CODE HERE
    // Check bit vector for any set bit.
    // If set, return 1, else send 0
    // Done. (MA)
    if (isValid(num_proc)) {
        for (int i = 0; i < num_proc; i++) {
            if (bit[i] == 1) {
                return true;
            }
        }
    }
    return false;
}

int FBV::others_are_sharing(int proc_num, int num_proc) {
    for (int p = 0; p < num_proc; p++) {
        if ((p != proc_num) && bit[p]) {
            return true;
        }
    }

    return false;
}

void FBV::sendInt_to_sharer(ulong addr, int specific_proc, int proc_count) {
    // YOUR CODE HERE
    //
    // Invoke the sendInt function defined in main
    // for all the processors except for proc_num
    // Make sure that you check the FBV to see if the
    // bit is set

    for (int p = 0; p < proc_count; p++) {
        if ((p != specific_proc) && bit[p]) {
            sendInt(addr, p);
        }
    }
}

void FBV::sendInv_to_sharer(ulong addr, int specific_proc, int proc_count) {
    // YOUR CODE HERE
    //
    // Invoke the sendInv function defined in main
    // for all the processors except for proc_num
    // Make sure that you check the FBV to see if the
    // bit is set
    for (int p = 0; p < proc_count; p++) {
        if ((p != specific_proc) && bit[p]) {
            sendInv(addr, p);
            bit[p] = false;
        }
    }
}
bool FBV::isValid(int index) {
    if (index > -1 && index <= MAX_PROCS) {
        return true;
    } else {
        return false;
        // Ignore any processor counts larger than the hardcoded 16.
        // This is just a safegaurd against out-of-bound conditions.
    }
}