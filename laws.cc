#include "laws.h"
//*****************************************************************
//Function Name :- Constuctore of Laws Module
//Function Description :- Constructor creates the LLT & WGT tables
//*****************************************************************
LAWS::LAWS(unsigned int num_entry_llt, unsigned int num_entry_wgt){
  llt_size=num_entry_llt;
  wgt_size=num_entry_wgt;
  LLT = new llt_entry [num_entry_llt];  // Creating the last load table
  WGT = new wgt_entry [num_entry_wgt];  // Creating the warp group table
  
  for(unsigned int i=0;i<num_entry_llt;i++){
    LLT[i].llpc=0; 
    LLT[i].valid=false;
  }
  
  for(unsigned int i=0;i<num_entry_wgt;i++){
    WGT[i].valid=false;
    WGT[i].bit_vector=0; 
  }

  priority_entry p_entry;    // Priority Queue
  
  for(unsigned int i=0;i<num_entry_llt;i++){
    p_entry.warp_id=i;
    p_entry.is_Bundle=false;
    priority_queue.push_back(p_entry);      // Creating the priority queue 
  }
  print_priority_q();    // printing the priority queue
  free_group=0;          // Initialising the free group ID to zero
}

//*****************************************************************
//Function Name :- Print_Prority_queue()
//Function Description :- to print the contents of the priority queue
//*****************************************************************

void LAWS::print_priority_q(){
  //printf("__debug Priority Queue:\n");
  //printf("__debug ");
  for(std::list<priority_entry>::iterator it=priority_queue.begin();it!=priority_queue.end();it++){
    //printf("%d(%d) ",it->warp_id,it->is_Bundle);
  }
  //printf("\n");
}


//*****************************************************************
//Function Name :- set_llpc()
//Function Description :- Function to store the PC of a load for a
// corresponding warp ID
//*****************************************************************
void LAWS::set_llpc(unsigned int llpc, unsigned int wid){
  ////printf("pushing %x at %d\n", llpc, wid);
  LLT[wid].llpc=llpc;
  LLT[wid].valid=true;
}

//*****************************************************************
//Function Name :- get_llpc()
//Function Description :- function to return the PC of a warp in the 
// last load table
//*****************************************************************
unsigned int LAWS::get_llpc(unsigned int wid){
  return LLT[wid].llpc;
}

//*****************************************************************
//Function Name :-form_group()
//Function Description :- This function forms and returns the group_ID
// to the WGT table. The requesting warp flushes its entry throughout
// the last load table and all the matches are stored into a 64 bit
// unsigned bit vector.
//*****************************************************************
unsigned int LAWS::form_group(unsigned int wid){
  unsigned int last_load_pc;
  bool group_formed;
  group_formed=false;
 // WGT[free_group].bit_vector=0; //to be removed later and done in the execute stage when the group has been used
  if(LLT[wid].valid){
    last_load_pc=LLT[wid].llpc;
    for(unsigned int i=0;i<(llt_size);i++){
      if(LLT[i].valid && i != wid){
        if(LLT[i].llpc==last_load_pc){
	  WGT[free_group].valid=true;
	  WGT[free_group].bit_vector=(1<<i)|WGT[free_group].bit_vector;
	  group_formed=true;
	}
      }
    }
  }

  unsigned int ret_free_group=0;
  ret_free_group = free_group;
  if(group_formed){
    if((free_group==wgt_size-1)/*&&(group_formed)*/)
      free_group=0;
    else
      free_group++;
  }
	
 return ret_free_group;
}

//*****************************************************************
//Function Name :- Wrapper_shader
//Function Description :- This function is just a top level wrapper 
// to the laws module from the shader core
//*****************************************************************
unsigned int LAWS::wrapper_shader(unsigned int wid,unsigned int llpc)
{
	unsigned int group_id=0;
	group_id = form_group(wid);
	set_llpc(llpc,wid);		
	return group_id;
}

//*****************************************************************
//Function Name :- Update_priority_issue
//Function Description :- This function follows the round robin policy 
// already present and moves the issued warp to the tail of the priority 
// queue
//*****************************************************************
void LAWS::update_priority_issue(unsigned int wid)
{
  if(!DEBUG1)
  {
  	//printf("__debug Priority Before Updation of Issue of WID= %d\n",wid);
	print_priority_q();
  }
  std::list<priority_entry>::iterator headIter = priority_queue.begin();
  headIter->is_Bundle=false;
  priority_queue.push_back(*headIter);
  priority_queue.pop_front();
  if(!DEBUG1)
  {
  	//printf("__debug Priority After Updation of Issue of WID= %d\n",wid);
	print_priority_q();
  }
}

//*****************************************************************
//Function Name :- get_group_ID
//Function Description :- This function just returns the bit vector
// associated with an entry in the warp group table
//*****************************************************************
unsigned int LAWS::get_group_ID(unsigned int gid)
{
	return WGT[gid].bit_vector;
}


//*****************************************************************
//Function Name :- update_priority_execute()
//Function Description :- This function is a call to the LAWS module 
//when a certain warp executing a load instruction reaches the execute stage
// There are two scenarios :-
// 1. Cache HIT 
//    In this case all the associated entries in the bit vector are move to the 
//    head of the priority queue and a is_Bundle bit is set associated with them 
//    so that a proritized warp stays at head until issued
// 2. Cache Miss
//    All the warp in the group of that particular warp are moved to the 
//    tail of the priority queue.
//*****************************************************************

void LAWS::update_priority_execute(bool cache_hit, unsigned int group_ID){
  if(!DEBUG1){
    //printf("__debug Priority Before Updation of Execute\n");
    print_priority_q();
  }
  std::list<priority_entry>::iterator it;
  if(cache_hit){ 			//CASE 1. HIT case move all the warps to Head
    unsigned int bv = WGT[group_ID].bit_vector;
    for(unsigned int i=0;i<llt_size;i++){
	if(!DEBUG1)
	{
		print_priority_q();	
		//printf("\nBV = %d\n",bv);
	}
      if(bv & 0x01){ 		// Condition to find the warps present in that group
        priority_entry temp_search;
        temp_search.warp_id = i;
        temp_search.is_Bundle = false;
        std::list<priority_entry>::iterator findIter = std::find(priority_queue.begin(),priority_queue.end(),temp_search);
        if(findIter!= priority_queue.end()){
          priority_entry temp_entry;
          temp_entry.warp_id = i;
          temp_entry.is_Bundle = true;
	 // Finding an entry for insertion into the queue
   	 for(it=priority_queue.begin();it!=priority_queue.end() && (it->is_Bundle==true);it++){
    	  }
          priority_queue.insert(it,temp_entry); // Inserting it into the queue
          priority_queue.erase(findIter); // Easing the old entry
        }
      }
      bv = bv>>1; // traversing through the bit vector
    }	 
    if(!DEBUG1){
      //printf("__debug Priority After Hit in Execution\n");
      //printf("__debug Group ID:-%d\n",group_ID);
      print_wgt();
      print_priority_q();
    }
  }
  else{          			//CASE 2. Miss Case move all the warps to tail 
    unsigned int bv = WGT[group_ID].bit_vector;
    for(unsigned int i=0;i<llt_size;i++){
      if(bv & 0x01){
        priority_entry temp_search1;
        temp_search1.warp_id = i;
        temp_search1.is_Bundle = false;
        std::list<priority_entry>::iterator findIter = std::find(priority_queue.begin(),priority_queue.end(),temp_search1);
        if(!findIter->is_Bundle){
         priority_queue.push_back(*findIter);
         priority_queue.erase(findIter);
        }
      }
      bv = bv>>1;
    }	
    if(!DEBUG1){
      //printf("__debug Priority After Miss in Execution\n");
      //printf("__debug Group ID:-%d\n",group_ID);
      print_wgt();
      print_priority_q();
    }
  }
 // WGT[group_ID].bit_vector=0;
}
void LAWS::clear_group(unsigned int group_id)
{
 WGT[group_id].bit_vector = 0; //Resetting the used group vector to zero
}

void LAWS::print_llt(){
  //printf("__debug LLT:\n");
  //printf("__debug Entry\tValid\tLLPC\t\n");
  //printf("__debug ");
  for(unsigned int i=0;i<llt_size;i++);
    ////printf("%d"LLT[i].valid);
      //printf("%d\t%d\t%d\t\n",i,LLT[i].valid,LLT[i].llpc);
  ////printf("\n");
}

void LAWS::print_wgt(){
  //printf("__debug WGT(%d):\n",wgt_size);
  for(unsigned int i=0;i<wgt_size;i++);
    //if(WGT[i].valid)
      //printf("__debug %d %x\n",i,WGT[i].bit_vector);
  ////printf("\n");
}
