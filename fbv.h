/************************************************************
			Course		: 	CSC/ECE506
			Source 		:	fbv.h
			Owner		:	Ed Gehringer
			Email Id	:	efg@ncsu.edu
*************************************************************/
#include "directory.h"
class FBV: public dir_entry {
 private:
	static const int MAX_PROCS=16;
	bool bit[MAX_PROCS];
 
 public:
	unsigned long get_dir_tag() { return dir_entry::tag; }
	dir_state get_state() { return dir_entry::state;}
	void set_dir_state(dir_state d_state) { dir_entry::state = d_state;}
	void set_dir_tag(unsigned long a) { /*printf("Old tag:%ld, new tag:%ld\n",get_dir_tag(),a);*/tag = a; }
	void add_sharer_entry(int proc_num);
	void remove_sharer_entry(int proc_num);
	int is_cached(int np);
    int others_are_sharing(int proc_num, int num_proc);
	//num_proc - number of processors in the DSM
	//proc_num - processor number invoking the function
	void sendInv_to_sharer(ulong addr, int num_proc, int proc_num);
	void sendInt_to_sharer(ulong addr, int num_proc, int proc_num);
	bool isValid(int);
 	void print_vector();
};
