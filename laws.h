#ifndef LAWS_H
#define LAWS_H
#include <stdio.h>
#include <list>
#include <algorithm>

#define DEBUG1 0

struct llt_entry{   // Structure defining 1 LLT entry
  unsigned int llpc;
  bool valid;
};

struct wgt_entry{  // Structure defining 1 WGT entry
  unsigned long int bit_vector;
  bool valid;
};

struct priority_entry  // Structure defining 1 priority queue entry
{
  unsigned int warp_id;
  bool is_Bundle;
  bool operator==(const priority_entry& rhs){
    return (warp_id==rhs.warp_id);
  } 
};


//***********+************+***********+*************+*********//
// 			LAWS Module
// The laws module contains the WGT, LLT tables and the priority 
// queue. Its wrapper is called from the shader module and also
// the SAP module interacts with it to get the bit vector of a warp
//***********+************+***********+*************+*********//
class LAWS{
  public:
  //private:
    llt_entry *LLT;
    wgt_entry *WGT;
    std::list<priority_entry> priority_queue;
    unsigned int llt_size;
    unsigned int wgt_size;
    unsigned int free_group;
    unsigned int last_priority;
  
  //public:
    LAWS(unsigned int num_entry_llt, unsigned int num_entry_wgt);
    void set_llpc(unsigned int llpc,unsigned int index);
    unsigned int form_group(unsigned int wid);
    unsigned int get_llpc(unsigned int wid);
    void update_priority_execute(bool cache_hit, unsigned int gid);
    void update_priority_issue(unsigned warp_ID);
    unsigned int wrapper_shader(unsigned int,unsigned int); // returning the group ID of the corresponsing warp 
    void print_priority_q();
    void print_llt();
    void print_wgt();
    unsigned int get_group_ID(unsigned int); // SOPI needs GID 
    void clear_group(unsigned int); 
};
#endif /* LAWS_H */
