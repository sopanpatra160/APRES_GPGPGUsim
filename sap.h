#ifndef SAP_H
#define SAP_H
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <list>
#define DEBUG 1

//=======================================
//DEMAND REQUEST QUEUE ENTRY
//=======================================
//struct dqueue_entry{
//	unsigned long long int 	pc;
//	unsigned long long int	addr;
//	unsigned int	wid;
//};

//=======================================
//PREFETCH TABLE ENTRY
//=======================================
struct ptable_entry{
        unsigned long long int 	pc;
        int	wid;
        unsigned long long int 	addr;
        signed long long int 	stride;

};

//=======================================
//WARP QUEUE ENTRY
//=======================================
//struct wqueue_entry{
//	int	wid;
//        bool 		end;
//
//};


//=======================================
//SAP class
//=======================================
class SAP{
      public:
        //dqueue_entry *DREQ;
        ptable_entry PT;
        //wqueue_entry WQ;
	int verbosity;        	
	//unsigned int dqueue_size;
	//unsigned int ptable_size;
	//unsigned int wqueue_size;
 	//unsigned int prefetch_addr[48];

	//lists
        //std::list <dqueue_entry>	drequest_queue;
        std::vector <ptable_entry>	prefetch_table;
        std::vector <int>		warp_queue;
	
	unsigned int prefetch_count;
        //functions
        SAP();
        ptable_entry		search_pc(unsigned long long int pc);
        signed long long int	calculate_stride(int old_wid, int curr_wid, unsigned long long int old_addr, unsigned long long int curr_addr);
        bool 			compare_stride(signed long long int old_stride, signed long long int current_stride);
        unsigned long long int 	generate_prefetch_request(unsigned long long int base_address, int wid_wq, int wid_req, signed long long int current_stride);
        bool 			detect_wqueue_end(int wq_end);
        void 			update_prefetch_table(unsigned long long int pc,int wid, unsigned long long int address, signed long long int new_stride);
	
};

#endif


















