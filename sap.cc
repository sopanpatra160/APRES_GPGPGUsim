#include "sap.h"
//*****************************************************************
//Function Name :- Constuctore of Laws Module
//Function Description :- Constructor sets the verbosity to 0. 
//			  Verbosity=0 disables the prints
//*****************************************************************
SAP::SAP()
{
 verbosity=0;
}

//*****************************************************************
//Function Name :- search_pc
//Function Description :- This function searches the PC of demand 
//			  request in PT. If found it returns the 
//			  struct of type ptable_entry 
//*****************************************************************
struct ptable_entry SAP::search_pc(unsigned long long int pc_request)
{
	struct ptable_entry temp={};
	if (verbosity == 1)
	{	
		printf("-----------PC Search-------------\n");
		printf("[CHECKPOINT]search_pc accessed\n");
		printf("[CHECKPOINT]pc requested:%llu\n",pc_request);
	}
	//Iterating the prefetch table to see if PC is found
	for (std::vector<ptable_entry>::iterator it =prefetch_table.begin() ; it != prefetch_table.end(); ++it)
	{
		//it->pc is pc in PT, pc_request is the pc present in demand request
		if( it->pc == pc_request )
		{
			if (verbosity == 1)
			{	
				printf("[CHECKPOINT]pc found!\n");	
			}	
			//return the whole row present in the vector prefetch_table
			return *it;
		}
	}
	if (verbosity == 1)
	{	
		printf("[CHECKPOINT]pc not found!\n");		
	}
	return temp;
}

//*****************************************************************
//Function Name :- update_prefetch_table
//Function Description :- If pc of demand request is found in the 
//			  PT, the corresponding row of PT is updates
//			  Else a new row is pushed into PT 
//*****************************************************************
void SAP::update_prefetch_table(unsigned long long int pc_request,int warp_id, unsigned long long int address, signed long long int new_stride)
{
	if (verbosity == 1)
	{	
		printf("-----------Prefetch Table Update-----------\n");
		printf("[CHECKPOINT]update_prefetch_table  accessed\n");
	}
	for (std::vector<ptable_entry>::iterator it =prefetch_table.begin() ; it != prefetch_table.end(); ++it)
	{
		//if pc of demand request is found in the PT, then update the corresponding row in PT
		if(it->pc == pc_request)
		{
			it->pc 		= pc_request;
			it->wid 	= warp_id;
			it->addr 	= address;
			it->stride 	= new_stride;
			if (verbosity == 1)
			{	
				printf("[CHECKPOINT] pc found in PT\n");			
				printf("[CHECKPOINT] pc to be updated :%llu\n",pc_request);
				printf("[CHECKPOINT] wid to be updated :%d\n",warp_id);
				printf("[CHECKPOINT] address to be updated :%llu\n",address);
				printf("[CHECKPOINT] stride to be updated : %lli \n",new_stride);
				printf("[CHECKPOINT] Prefetch Table updated\n");
				return;
			}
		}
	
	} 
	//if pc of demand request is not found in PT, store the demand request fields in a struct of type prefetch_table_entry

	PT.pc	 =pc_request;
	PT.wid	 =warp_id;
	PT.addr	 =address;
	PT.stride=new_stride;

	if (verbosity == 1)
	{	
		printf("[CHECKPOINT] pc not found in PT\n");			
		printf("[CHECKPOINT] pc to be updated :%llu\n",pc_request);
		printf("[CHECKPOINT] wid to be updated :%d\n",warp_id);
		printf("[CHECKPOINT] address to be updated :%llu\n",address);
		printf("[CHECKPOINT] stride to be updated :%lli\n",new_stride);
	
	}

	//push the struct into the prefetch_table vector
	prefetch_table.push_back(PT);	
	if (verbosity == 1)
	{	
		printf("[CHECKPOINT]New row pushed into Prefetch Table\n");
	}
	return; 


}

//*****************************************************************
//Function Name :- calculate_stride
//Function Description :- This function calculates a stride based on 
//			  the address present in the demand request
//			  and the address present in PT 
//*****************************************************************

signed long long int SAP::calculate_stride(int old_wid, int curr_wid, unsigned long long int old_addr, unsigned long long int curr_addr)
{

	if (verbosity == 1)
	{	
		printf("-----------Stride Calculate-----------\n");
		printf("[CHECKPOINT] wid in Ptable :%d\n",old_wid);
		printf("[CHECKPOINT] wid in demand request :%d\n",curr_wid);
		printf("[CHECKPOINT] address in Ptable :%llu\n",old_addr);
		printf("[CHECKPOINT] addr in demand request :%llu\n",curr_addr);
	}
	signed long long int  addr_diff =  (old_addr-curr_addr)  ;
	if (verbosity == 1)
	{	
		printf("[CHECKPOINT] Address diff :%lli\n",addr_diff);
	}

	int wid_diff = (old_wid - curr_wid);
	if (verbosity == 1)
	{	
		printf("[CHECKPOINT] wid diff :%d\n",wid_diff);
	}

	signed long long int stride_new = addr_diff / wid_diff;
	
	if (verbosity == 1)
	{	
		printf("[CHECKPOINT] stride calculated :%lli\n",stride_new);
	}
	return stride_new;
	

}

//*****************************************************************
//Function Name :- compare_stride
//Function Description :- This function is used for comparing stride 
//			  just calculated in calculate_stride() function
//			  and stride present in PT
//*****************************************************************

bool SAP::compare_stride(signed long long int  old_stride, signed long long int  current_stride)
{

	if (verbosity == 1)
	{	
		printf("-----------Stride Compare-----------\n");
		printf("[CHECKPOINT] Stride in Ptable :%lli\n",old_stride);
		printf("[CHECKPOINT] Stride in demand request :%lli\n",current_stride);
	}
	
	bool stride_match = false;

	if(old_stride==current_stride){
		stride_match=true;
		if (verbosity == 1)
		{	
			printf("[CHECKPOINT] Stride match!Go for prefetching !\n");
		}
	}
	else
	{
		if (verbosity == 1)
		{	
			printf("[CHECKPOINT] Stride did not match!Just update prefetch table !\n");
		}

	}
	return stride_match;

}

//*****************************************************************
//Function Name :- generate_prefetch_request()
//Function Description :- This function is used to generate prefetch
//			  request  based on the address present in 
//			demand request, stride calculated , Wid in 
//			warp queue and wid in demand request
//*****************************************************************

unsigned long long int 	SAP::generate_prefetch_request(unsigned long long int base_address, int wid_wq, int wid_dq, signed long long int current_stride)
{
	if (verbosity == 1)
	{	
		printf("-----------Prefetch Address Gen-----------\n");
	}
	unsigned long long int prefetch_addr;

	if (verbosity == 1)
	{	
		printf("[CHECKPOINT] Wid to be prefetched :%d",wid_wq);
	}
	prefetch_addr = base_address + (wid_wq - wid_dq)*current_stride;
	
	if (verbosity == 1)
	{	
		printf("Prefetch addr :%llu\n",prefetch_addr);
	}
	return prefetch_addr;
	
}

