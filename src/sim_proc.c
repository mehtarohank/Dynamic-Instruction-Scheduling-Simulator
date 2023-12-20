#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "sim_proc.h"

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim 256 32 4 gcc_trace.txt
    argc = 5
    argv[0] = "sim"
    argv[1] = "256"
    argv[2] = "32"
    ... and so on
*/

int inst_counter;
int dispatch_counter,iq_counter,read_counter,rob_counter,decode_counter,rename_counter;
proc_params temp_params;
struct pipe pipeline[15000];
int cyclic_counter=0;

int head_index=0;
int tail_index=0;

int Advance_Cycle(FILE *fp,bundle_params bundle[],rob_params rob[],rmt_params rmt[],iq_params issue_queue[],execute_list execute[],WB writeback[]);
void Fetch(FILE *FP,bundle_params bundel[],rob_params rob[],rmt_params rmt[]);
void Decode(bundle_params bundel[],rob_params rob[],rmt_params rmt[]);
void Rename(bundle_params bundle[],rob_params rob[],rmt_params rmt[]);
void RegRead(bundle_params bundel[],rob_params rob[],rmt_params rmt[]);
void Dispatch(bundle_params bundle[],rob_params rob[],rmt_params rmt[],iq_params issue_queue[]);
void Issue(bundle_params bundle[],rob_params rob[],rmt_params rmt[],iq_params issue_queue[],execute_list execute[]);
void Execute(bundle_params bundle[],iq_params issue_queue[],rob_params rob[],rmt_params rmt[],execute_list execute[],WB writeback[]);
void Writeback(bundle_params bundle[],rob_params rob[],rmt_params rmt[],WB writeback[]);
void Retire(bundle_params bundle[],rob_params rob[],rmt_params rmt[]);
void displayRmt(rmt_params rmt[]);
void displayRob(rob_params rob[]);
void displayIq(iq_params issue_queue[]);

int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params
    int i=0;

    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];
    // printf("rob_size:%lu " "iq_size:%lu " "width:%lu "  "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);

    temp_params = params;

    // Allocate size for ROB, bundle and RMT table
    rob_params rob[params.rob_size];
    bundle_params bundle[params.width];
    rmt_params rmt[67];
    iq_params issue_queue[params.iq_size];
    execute_list execute[params.width*5];
    WB writeback[params.width*5];
    
    // Creating Head and Tail for ROB Table Initialy they are pointing to 0th Index
    // rob_params *head=&rob[0];
    // rob_params *tail=&rob[0];

    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    
    for(i=0;i<params.width*5;i++)
    {
        execute[i].ex_valid=0;
        execute[i].ex_pc=0;
        execute[i].ex_type=0;
        execute[i].ex_dst=0;
        execute[i].ex_src1=0;
        execute[i].ex_src2=0;
        execute[i].ex_age=0;
        execute[i].ex_timer=0;
        execute[i].ex_src1_tag=0;
        execute[i].ex_src2_tag=0;
        execute[i].ex_dst_tag=0;
        execute[i].ex_src1_val_rob=0;
        execute[i].ex_src2_val_rob=0;

        writeback[i].wb_valid=0;
        writeback[i].wb_pc=0;
        writeback[i].wb_type=0;
        writeback[i].wb_dst=0;
        writeback[i].wb_src1=0;
        writeback[i].wb_src2=0;
        writeback[i].wb_age=0;
        writeback[i].wb_timer=0;
        writeback[i].wb_src1_tag = 0;
        writeback[i].wb_src2_tag=0;
        writeback[i].wb_dst_tag=0;
        writeback[i].wb_src1_val_rob=0;
        writeback[i].wb_src2_val_rob=0;
    }

    for(i=0;i<params.width;i++)
    {
        bundle[i].fe_valid=0;
        bundle[i].fe_age=0;

        bundle[i].de_valid=0;
        bundle[i].de_age=0;
        bundle[i].de_type=0;
        bundle[i].de_dst=0;
        bundle[i].de_src1=0;
        bundle[i].de_src2=0;

        bundle[i].rn_valid=0;
        bundle[i].rn_age=0;
        bundle[i].rn_type=0;
        bundle[i].rn_src1=0;
        bundle[i].rn_src2=0;
        bundle[i].rn_dst=0;
        bundle[i].rn_src1_rdy=0;
        bundle[i].rn_src2_rdy=0;
        bundle[i].rn_dst_rdy=0;
        bundle[i].rn_src1_tag=0;
        bundle[i].rn_src2_tag=0;
        bundle[i].rn_dst_tag=0;
        bundle[i].rn_src1_val_rob=0;
        bundle[i].rn_src2_val_rob=0;

        bundle[i].rr_valid=0;
        bundle[i].rr_age=0;
        bundle[i].rr_type=0;
        bundle[i].rr_src1=0;
        bundle[i].rr_src2=0;
        bundle[i].rr_dst=0;
        bundle[i].rr_src1_rdy=0;
        bundle[i].rr_src2_rdy=0;
        bundle[i].rr_dst_rdy=0;
        bundle[i].rr_src1_tag=0;
        bundle[i].rr_src2_tag=0;
        bundle[i].rr_dst_tag=0;
        bundle[i].rr_src1_val_rob=0;
        bundle[i].rr_src2_val_rob=0;
   
        bundle[i].di_valid=0;
        bundle[i].di_pc=0;
        bundle[i].di_type=0;
        bundle[i].di_dst=0;
        bundle[i].di_src1=0;
        bundle[i].di_src2=0;
        bundle[i].di_age=0;
        bundle[i].di_src1_rdy=0;
        bundle[i].di_src2_rdy=0;
        bundle[i].di_dst_rdy=0;
        bundle[i].di_src1_tag=0;
        bundle[i].di_src2_tag=0;
        bundle[i].di_dst_tag=0;
        bundle[i].di_src1_val_rob=0;
        bundle[i].di_src2_val_rob=0;
    }

    for(i=0;i<params.iq_size;i++)
    {
        issue_queue[i].iq_valid=0;
        issue_queue[i].iq_pc=0;
        issue_queue[i].iq_type=0;
        issue_queue[i].iq_age=0;
        issue_queue[i].iq_dst=0;
        issue_queue[i].iq_src1=0;
        issue_queue[i].iq_src2=0;
        issue_queue[i].iq_src1_rdy=0;
        issue_queue[i].iq_src2_rdy=0;
        issue_queue[i].iq_dst_rdy=0;
        issue_queue[i].iq_src1_tag=0;
        issue_queue[i].iq_src2_tag=0;
        issue_queue[i].iq_dst_tag=0;
        issue_queue[i].iq_src1_val_rob=0;
        issue_queue[i].iq_src2_val_rob=0;
    }

    for(i=0;i<temp_params.rob_size;i++)
    {
        rob[i].valid=0;
        rob[i].number=i;
        rob[i].dst=0;
        rob[i].rdy=0;
        rob[i].exc=0;
        rob[i].mis=0;
        rob[i].pc=0;
        rob[i].age=0;
    }

    for(i=0;i<67;i++)
    {
        rmt[i].valid=0;
        rmt[i].rob_tag=0;
    }
    do 
    {
        Retire(bundle,rob,rmt);
        Writeback(bundle,rob,rmt,writeback);
        Execute(bundle,issue_queue,rob,rmt,execute,writeback); 
        Issue(bundle,rob,rmt,issue_queue,execute); 
        Dispatch(bundle,rob,rmt,issue_queue); 
        RegRead(bundle,rob,rmt); 
        Rename(bundle,rob,rmt); 
        Decode(bundle,rob,rmt); 
        Fetch(FP, bundle,rob,rmt);
       // printf("%d cyclic_counter =%d\n",inst_counter,cyclic_counter);

    } while(Advance_Cycle(FP, bundle,rob,rmt,issue_queue,execute,writeback));

    fclose(FP);

  
    for(i=1;i<inst_counter;i++)
    {
        printf("%d fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} RR{%d,%d} DI{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d} RT{%d,%d}\n",(pipeline[i].seq)-1,pipeline[i].type,pipeline[i].src1,pipeline[i].src2,pipeline[i].dst,pipeline[i].fe,(pipeline[i].de-pipeline[i].fe),pipeline[i].de,(pipeline[i].rn-pipeline[i].de),pipeline[i].rn,(pipeline[i].rr-pipeline[i].rn),pipeline[i].rr,(pipeline[i].di-pipeline[i].rr),pipeline[i].di,(pipeline[i].iq-pipeline[i].di),pipeline[i].iq,(pipeline[i].ex-pipeline[i].iq),pipeline[i].ex,(pipeline[i].wb-pipeline[i].ex),pipeline[i].wb,(pipeline[i].rt_start-pipeline[i].wb),pipeline[i].rt_start,(pipeline[i].rt_end-pipeline[i].rt_start));
    }

    printf("# === Simulator Command =========\n");
    printf("# ./sim %s %s %s %s\n",argv[1],argv[2],argv[3],argv[4]);
    printf("# === Processor Configuration ===\n");
    printf("# ROB_SIZE 	= %s\n",argv[1]);
    printf("# IQ_SIZE  	= %s\n",argv[2]);
    printf("# WIDTH    	= %s\n",argv[3]);
    printf("# === Simulation Results ========\n");
    printf("# Dynamic Instruction Count      = %d\n",inst_counter-1);

    if(params.width == 1)
    {
        printf("# Cycles                         = %d\n",cyclic_counter-1);
    }
    else if(params.width > 1)
    {
        printf("# Cycles                         = %d\n",cyclic_counter);
    }
    printf("# Instructions Per Cycle (IPC)   = %.2f\n",((float)inst_counter)/((float)cyclic_counter));
    return 0;
}

int Advance_Cycle(FILE *fp,bundle_params bundle[],rob_params rob[],rmt_params rmt[],iq_params issue_queue[],execute_list execute[],WB writeback[])
{
    int temp=0;
    cyclic_counter+=1;
    
    int i=0;

    if(feof(fp))    //checks eof
    {
        for(i=0;i<temp_params.width;i++)
        {
            //checks whether pipeline is empty
            if(bundle[i].de_valid==1 || bundle[i].rn_valid==1 || bundle[i].rr_valid==1 || bundle[i].di_valid==1)
            {
                temp=1;
                break;
            }
        }

        if(temp==0)
	    {
            for(i=0;i<temp_params.iq_size;i++)
            {
                if(issue_queue[i].iq_valid==1)  //checks whether pipeline is empty in issue queue
                {
                    temp=1;  //if pipeline is not empty,then come out of the loop
                    break;
                }
            }

            if(temp==0)
            {
                for(i=0;i<temp_params.width*5;i++)
                {
                    if(execute[i].ex_valid==1)    //checks whether pipeline is empty in execute queue
                    {
                        temp=1;
                        break;
                    }
                }
     
                for(i=0;i<temp_params.width;i++)
                {
                    if(writeback[i].wb_valid==1)  //checks whether pipeline is empty in writeback queue
                    {
                        temp=1;
                        break;
                    }
                }

                if(temp==0)
                {
                    for(i=0;i<temp_params.rob_size;i++)
                    {
                        if(rob[i].rdy==1)  //checks whether pipeline is empty
                        {
                            temp=1;
                            break;
                        }
                    }
                }

		        if(temp==0)
		        {
		            if(!(head_index==tail_index && cyclic_counter!=0))
		            {
			            temp=1;
		            }
                }
            }
        }
    
        if(temp==0)
            return 0; //since pipeline is empty and eof,therefore return false
        else
        {
            return 1;   //continue to run the simulator
        }
    }
    else
    {
        return 1;   //continue to run the simulator
    }
}

void Fetch(FILE *FP,bundle_params bundle[],rob_params rob[],rmt_params rmt[])
{
    int i=0;
    decode_counter = 0;
    for(i=0;i<temp_params.width;i++)
    {
         if(bundle[i].de_valid==0)
            decode_counter+=1;  
    }

    if(feof(FP) || decode_counter!=temp_params.width)
    {
    //     if(decode_counter!=temp_params.width)
    //        printf("nothing\n");
    //    if(feof(FP))
    //        printf("End of file\n");
    }
    else if(decode_counter == temp_params.width && (!feof(FP)) )
    {   
        for(i=0;i<temp_params.width;i++) 
        {          
            if(fscanf(FP, "%x %d %d %d %d", &bundle[i].de_pc, &bundle[i].de_type, &bundle[i].de_dst, &bundle[i].de_src1, &bundle[i].de_src2))
            {
                inst_counter+=1;
                             
                bundle[i].de_age = inst_counter;
                bundle[i].de_valid = 1;

                pipeline[inst_counter].seq = bundle[i].de_age;
                pipeline[inst_counter].type = bundle[i].de_type;
                pipeline[inst_counter].dst = bundle[i].de_dst;
                pipeline[inst_counter].src1 = bundle[i].de_src1;
                pipeline[inst_counter].src2 = bundle[i].de_src2;
                pipeline[inst_counter].fe = cyclic_counter;
                pipeline[inst_counter].de = cyclic_counter+1;
            }

            //if no more instructions are present in trace file,then get out the loop
            if(feof(FP))    
            {
                //printf("No more instruction\n");
                break;
            }
        }
    }

    #if 0
    if(cyclic_counter > 65 && cyclic_counter < 75)
    {
        printf("2 Fetch de_pc = %x type =%d src1=%d src2=%d dest=%d\n",bundle[0].de_pc,bundle[0].de_type,bundle[0].de_src1,bundle[0].de_src2,bundle[0].de_dst);
        displayRmt(rmt);
        displayRob(rob);
    }
    #endif
}

void Decode(bundle_params bundle[],rob_params rob[],rmt_params rmt[])
{
    #if 0
    if(cyclic_counter > 65 && cyclic_counter < 75)
    {
        printf("1 Decode rn_pc = %x type =%d src1=%d src2=%d dest=%d\n",bundle[0].rn_pc,bundle[0].rn_type,bundle[0].rn_src1,bundle[0].rn_src2,bundle[0].rn_dst);
    }
    #endif
    
    int i = 0;
    decode_counter=0;
    rename_counter=0;

    for(i=0;i<temp_params.width;i++)    //count the number of instructions in decode counter
    {
        if(bundle[i].de_valid==1)
            decode_counter+=1;
    }

    if(decode_counter>0)   //proceed only if DE is filled with instructions
    {
        for(i=0;i<temp_params.width;i++)    //count the number of empty slots in RN bundle
        {
              if(bundle[i].rn_valid==0)
                rename_counter+=1;
        }
        if(rename_counter==temp_params.width)   //proceed only if RN bundle is fully empty
        {
            for(i=0;i<temp_params.width;i++)    
            {
                 if(bundle[i].de_valid==1)
                 {
                    if(bundle[i].rn_valid==0)
                    {
                        bundle[i].de_valid = 0;   //removing from pipeline
                        bundle[i].rn_valid = 1;  // Setting rename valid value so it uses in rename stage 
                        
                        bundle[i].rn_pc = bundle[i].de_pc;
                        bundle[i].rn_type = bundle[i].de_type;
                        bundle[i].rn_src1 = bundle[i].de_src1;
                        bundle[i].rn_src2 = bundle[i].de_src2;
                        bundle[i].rn_dst = bundle[i].de_dst;
                        bundle[i].rn_age = bundle[i].de_age;

                        pipeline[bundle[i].rn_age].rn=cyclic_counter+1;

                        #if 0
                        if(cyclic_counter > 65 && cyclic_counter < 75)
                        {
                            printf("In Decode bundle[i].rn_pc = %x\n",bundle[i].rn_pc);
                            printf("rn = %d\n",pipeline[bundle[i].rn_age].rn);
                        }
                        #endif
                     }
                 }
            }
        }
    }

    #if 0
    if(cyclic_counter > 65 && cyclic_counter < 75)
    {
        printf("2 Decode rn_pc = %x type =%d src1=%d src2=%d dest=%d\n",bundle[0].rn_pc,bundle[0].rn_type,bundle[0].rn_src1,bundle[0].rn_src2,bundle[0].rn_dst);
        displayRmt(rmt);
        displayRob(rob);
    }
    #endif
}

void Rename(bundle_params bundle[],rob_params rob[],rmt_params rmt[])
{
    #if 0
    // printf("In Rename bundle[0].rn_valid=%d\n",bundle[0].rn_valid);
    // printf("In Rename bundle[0].rr_valid=%d\n",bundle[0].rr_valid);


    // if(cyclic_counter > 65 && cyclic_counter < 75)
    {
        printf("1 Rename rr_pc = %x type =%d src1=%d src2=%d dest=%d\n",bundle[0].rr_pc,bundle[0].rr_type,bundle[0].rr_src1,bundle[0].rr_src2,bundle[0].rr_dst);
        printf("1 Rename rr_pc = %x type =%d src1=%d src2=%d dest=%d\n",bundle[1].rr_pc,bundle[1].rr_type,bundle[1].rr_src1,bundle[1].rr_src2,bundle[1].rr_dst);
    }
    #endif
     
    int i = 0;
    rename_counter=0;
    read_counter = 0;
    rob_counter = 0;

    for(i=0;i<temp_params.width;i++)    //count the number of instructions in RN counter
    {
        if(bundle[i].rn_valid==1)
            rename_counter+=1;
    }

    if(rename_counter>0)   //proceed only if DE is filled with instructions
    {
        for(i=0;i<temp_params.width;i++)    //count the number of empty slots in RN bundle
        {
              if(bundle[i].rr_valid==0)
                read_counter+=1;
        }

        for(i=0;i<temp_params.rob_size;i++) //count the number of instructions present in ROB
        {
            if(rob[i].valid==0)
                rob_counter+=1;
        }
    
        if(read_counter == temp_params.width && rob_counter>=temp_params.width)   //proceed only if RN bundle is fully empty and also ROB has enough space
        {
            for(i=0;i<temp_params.width;i++)    
            {
                if(bundle[i].rn_valid==1)
                {
                    if(bundle[i].rr_valid==0)
                    {
                        //renaming source register 1 checking particular index in RMT and check that index value is 0 or 1
                        if (bundle[i].rn_src1 != -1)
                        {
                            if(rmt[bundle[i].rn_src1].valid==1)
                            {
                                bundle[i].rn_src1_tag = rmt[bundle[i].rn_src1].rob_tag;
                                bundle[i].rn_src1_val_rob = 1;   // This flag we use to check value modified with rob tag
                                bundle[i].rn_src1_rdy = 0;
                            }
                            else
                            {
                                bundle[i].rn_src1_val_rob = 0;
                                bundle[i].rn_src1_rdy = 1;
                            }
                        }
                        else
                        {
                            bundle[i].rn_src1_rdy = 1;
                        }

                        //renaming source register 1 checking particular index in RMT and check that index value is 0 or 1
                        if (bundle[i].rn_src2 != -1)
                        {
                            if(rmt[bundle[i].rn_src2].valid==1)
                            {
                                bundle[i].rn_src2_tag = rmt[bundle[i].rn_src2].rob_tag;
                                bundle[i].rn_src2_val_rob = 1;   // This flag we use to check value modified with rob tag
                                bundle[i].rn_src2_rdy = 0;
                            }
                            else
                            {
                                bundle[i].rn_src2_val_rob = 0;
                                bundle[i].rn_src2_rdy = 1;
                            }
                        } 
                        else
                        {
                                bundle[i].rn_src2_rdy = 1;
                        }

                        //put entry for dst in ROB
                        rob[tail_index].valid = 1;
                        rob[tail_index].dst = bundle[i].rn_dst;
                        rob[tail_index].rdy = 0;
                        rob[tail_index].exc = 0;
                        rob[tail_index].mis = 0;

                        //update rmt
                        if(bundle[i].rn_dst != -1)
                        {
                            rmt[rob[tail_index].dst].rob_tag=rob[tail_index].number;   // Assigned rob index or Number in RMT and also set valid bit for the same index
                            rmt[rob[tail_index].dst].valid=1;
                        }
                        //renaming destination in RR so that it points to ROB
                        bundle[i].rn_dst_tag = rob[tail_index].number;

                        // Increment Tail in ROB table
                        if(tail_index == temp_params.rob_size-1)    //if tail points to last entry in ROB then point to start
                            tail_index = 0;
                        else
                            tail_index++;                     //point to next index

                        bundle[i].rn_valid=0;
                        bundle[i].rr_valid=1;

                        bundle[i].rr_pc = bundle[i].rn_pc;
                        bundle[i].rr_age = bundle[i].rn_age;
                        bundle[i].rr_type = bundle[i].rn_type;

                        bundle[i].rr_src1 = bundle[i].rn_src1;
                        bundle[i].rr_src2 = bundle[i].rn_src2;
                        bundle[i].rr_dst = bundle[i].rn_dst;
                        
                        bundle[i].rr_src1_tag = bundle[i].rn_src1_tag;
                        bundle[i].rr_src2_tag = bundle[i].rn_src2_tag;
                        bundle[i].rr_dst_tag = bundle[i].rn_dst_tag;
                        
                        bundle[i].rr_src1_val_rob = bundle[i].rn_src1_val_rob;
                        bundle[i].rr_src1_val_rob = bundle[i].rn_src2_val_rob;    
                        
                        bundle[i].rr_src1_rdy = bundle[i].rn_src1_rdy;
                        bundle[i].rr_src2_rdy = bundle[i].rn_src2_rdy;
 
                        pipeline[bundle[i].rr_age].rr = cyclic_counter+1;

                        #if 0
                        if(cyclic_counter > 65 && cyclic_counter < 75)
                        {
                            printf("In Rename rr_pc = %x ,rr_src1 =%d ,rr_src2=%d rr_src1_tag =%d , rr_src2_tag=%d rr_src1_rdy=%d rr_src2_rdy=%d\n",bundle[i].rr_pc,bundle[i].rr_src1,bundle[i].rr_src2,bundle[i].rr_src1_tag,bundle[i].rr_src2_tag,bundle[i].rr_src1_rdy,bundle[i].rr_src2_rdy);
                            printf("rr = %d\n",pipeline[bundle[i].rr_age].rr);
                        }
                        #endif
                    }
                }
            }
        }
   }

   #if 0
  // if(cyclic_counter > 65 && cyclic_counter < 75)
   {
        printf("2 Rename rr_pc = %x type =%d src1=%d src2=%d dest=%d src1_rdy=%d src2_rdy =%d\n",bundle[0].rr_pc,bundle[0].rr_type,bundle[0].rr_src1,bundle[0].rr_src2,bundle[0].rr_dst,bundle[0].rr_src1_rdy,bundle[0].rr_src2_rdy);
        printf("2 Rename rr_pc = %x type =%d src1=%d src2=%d dest=%d src1_rdy=%d src2_rdy =%d\n",bundle[1].rr_pc,bundle[1].rr_type,bundle[1].rr_src1,bundle[1].rr_src2,bundle[1].rr_dst,bundle[1].rr_src1_rdy,bundle[1].rr_src2_rdy);

        displayRmt(rmt);
        displayRob(rob);
   }
   #endif
}

void RegRead(bundle_params bundle[],rob_params rob[],rmt_params rmt[])
{
    #if 0
        printf("In Register bundle[0].rr_valid=%d\n",bundle[0].rr_valid);
        printf("In Register bundle[0].di_valid=%d\n",bundle[0].di_valid);

        if(cyclic_counter > 65 && cyclic_counter < 75)
        {
        printf("1 RegRead di_pc = %x type =%d src1=%d src2=%d dest=%d src1_rdy=%d src2_rdy =%d\n",bundle[0].rr_pc,bundle[0].rr_type,bundle[0].rr_src1,bundle[0].rr_src2,bundle[0].rr_dst,bundle[0].rr_src1_rdy,bundle[0].rr_src1_rdy);
        printf("1 RegRead di_pc = %x type =%d src1=%d src2=%d dest=%d src1_rdy=%d src2_rdy =%d\n",bundle[1].rr_pc,bundle[1].rr_type,bundle[1].rr_src1,bundle[1].rr_src2,bundle[1].rr_dst,bundle[1].rr_src1_rdy,bundle[1].rr_src1_rdy);

        displayRob(rob);
        displayRmt(rmt);
        }
    #endif

    int i = 0 , j = 0,count =0 ,count1 = 0;
    read_counter = 0;
    dispatch_counter = 0;

    for(i=0;i<temp_params.width;i++)    //count the number of instructions in RR counter
    {
        if(bundle[i].rr_valid==1)
            read_counter+=1;
    }


    if(read_counter>0)   //proceed only if rr is filled with instructions
    {
        for(i=0;i<temp_params.width;i++)    //count the number of instructions in di counter
        {
            if(bundle[i].di_valid == 0)
                dispatch_counter+=1;
        }
        if(dispatch_counter == temp_params.width)
        {
            for(i=0;i<temp_params.width;i++)
            {
                if(bundle[i].rr_valid == 1)
                {
                    if(bundle[i].rr_src1 != -1)
                    {
                       // printf("In RR1 pc =%x rmt.valid=%d , rob.valid=%d and rob.rdy = %d\n",bundle[i].rr_pc,rmt[bundle[i].rr_src1].valid,rob[rmt[bundle[i].rr_src1].rob_tag].valid,rob[rmt[bundle[i].rr_src1].rob_tag].rdy);
                        if(rmt[bundle[i].rr_src1].valid == 1)
                        {
                            //check through ROB entry in table 
                            for(j=0;j<temp_params.rob_size;j++)
                            { 
                                if(rob[j].dst == bundle[i].rr_src1 && rob[j].rdy == 1 && rob[bundle[i].rr_src1_tag].rdy == 1)
                                {                                  
                                    count = 1;
                                    break;
                                }
                                if(rob[bundle[i].rr_src1_tag].valid==0)
                                {
                                    count=1;
                                    break;
                                }
                            }
                            if(count == 1)
                            {
                                bundle[i].rr_src1_rdy = 1;  
                                count = 0;
                            }
                        }
                        else
                        {
                          bundle[i].rr_src1_rdy = 1;
                        }
                    }

                    if(bundle[i].rr_src2 != -1)
                    {
                        if(rmt[bundle[i].rr_src2].valid == 1)
                        {
                          // printf("In RR2 pc =%x rmt.valid=%d , rob.valid=%d and rob.rdy = %d and rmt[bundle[i].rr_src2].rob_tag=%d\n",bundle[i].rr_pc,rmt[bundle[i].rr_src2].valid,rob[rmt[bundle[i].rr_src2].rob_tag].valid,rob[rmt[bundle[i].rr_src2].rob_tag].rdy,rmt[bundle[i].rr_src2].rob_tag);

                            //check through ROB entry in table 
                            for(j=0;j<temp_params.rob_size;j++)
                            {
                              //  printf("RR2 rob[j].dst =%d ,bundle[i].rr_src2 =%d and rob[j].rdy=%d and rob[bundle[i].rr_src2_tag].valid=%d\n",rob[j].dst,bundle[i].rr_src2,rob[j].rdy,rob[bundle[i].rr_src2_tag].valid);
                                if(rob[j].dst == bundle[i].rr_src2 && rob[j].rdy == 1 && rob[bundle[i].rr_src2_tag].rdy == 1)
                                {
                                    count1 = 1;
                                    break;
                                }
                                if(rob[bundle[i].rr_src2_tag].valid==0)
                                {
                                    count1 = 1;
                                    break;
                                }
                            }
                            if(count1 == 1)
                            {
                                bundle[i].rr_src2_rdy = 1;
                                count1 = 0;
                            }
                        }
                        else
                        {
                          bundle[i].rr_src2_rdy = 1;
                        }
                
                    }

                    if(bundle[i].rr_src1==-1)     //if no source register,then that instruction is ready partly
						bundle[i].rr_src1_rdy=1;
					if(bundle[i].rr_src2==-1)     //if no source register,then that instruction is ready partly
						bundle[i].rr_src2_rdy=1;

                }

                if(bundle[i].rr_valid == 1 && bundle[i].di_valid == 0)
                {
                 //   printf("In Register Read 3\n");
                 //  printf("1 RegRead di_pc = %x type =%d src1=%d src2=%d dest=%d src1_rdy=%d src2_rdy =%d\n",bundle[i].rr_pc,bundle[i].rr_type,bundle[i].rr_src1,bundle[i].rr_src2,bundle[i].rr_dst,bundle[i].rr_src1_rdy,bundle[i].rr_src1_rdy);

                    bundle[i].rr_valid = 0;
                    bundle[i].di_valid = 1;

                    bundle[i].di_age = bundle[i].rr_age;
                    bundle[i].di_pc = bundle[i].rr_pc;
                    bundle[i].di_type = bundle[i].rr_type;

                    bundle[i].di_src1 = bundle[i].rr_src1;
                    bundle[i].di_src2 = bundle[i].rr_src2;
                    bundle[i].di_dst = bundle[i].rr_dst;
                   

                    bundle[i].di_src1_tag = bundle[i].rr_src1_tag;
                    bundle[i].di_src2_tag = bundle[i].rr_src2_tag;
                    bundle[i].di_dst_tag = bundle[i].rr_dst_tag;
                        
                    bundle[i].di_src1_val_rob = bundle[i].rr_src1_val_rob;
                    bundle[i].di_src2_val_rob = bundle[i].rr_src2_val_rob;

                    bundle[i].di_src1_rdy = bundle[i].rr_src1_rdy;
                    bundle[i].di_src2_rdy = bundle[i].rr_src2_rdy;

                    pipeline[bundle[i].di_age].di = cyclic_counter+1;

                    #if 0
                    if(cyclic_counter > 65 && cyclic_counter < 75)
                    {
                        printf("In Register Read bundle[i].di_pc = %x\n",bundle[i].di_pc);
                        printf("In RR di_pc = %x ,di_src1 =%d ,di_src2=%d di_src1_tag =%d , di_src2_tag=%d di_src1_rdy=%d di_src2_rdy=%d\n",bundle[i].di_pc,bundle[i].di_src1,bundle[i].di_src2,bundle[i].di_src1_tag,bundle[i].di_src2_tag,bundle[i].di_src1_rdy,bundle[i].di_src2_rdy);
                        printf("di = %d\n",pipeline[bundle[i].di_age].di);
                    }
                    #endif
                }
            }
        }
    }

    #if 0
 //   if(cyclic_counter > 65 && cyclic_counter < 75)
    {
        printf("2 RegRead di_pc = %x type =%d src1=%d src2=%d dest=%d src1_rdy=%d src2_rdy =%d\n",bundle[0].di_pc,bundle[0].di_type,bundle[0].di_src1,bundle[0].di_src2,bundle[0].di_dst,bundle[0].di_src1_rdy,bundle[0].di_src2_rdy);
        printf("2 RegRead di_pc = %x type =%d src1=%d src2=%d dest=%d src1_rdy=%d src2_rdy =%d\n",bundle[1].di_pc,bundle[1].di_type,bundle[1].di_src1,bundle[1].di_src2,bundle[1].di_dst,bundle[1].di_src1_rdy,bundle[1].di_src2_rdy);
        displayRmt(rmt);
        displayRob(rob);
    }
    #endif
}

void Dispatch(bundle_params bundle[],rob_params rob[],rmt_params rmt[],iq_params issue_queue[])
{
    #if 0
    printf("In Dispatch bundle[0].di_valid=%d\n",bundle[0].di_valid);
    printf("In Dispatch issue_queue[0].iq_valid=%d\n",issue_queue[0].iq_valid);

    printf("1 Dispatch di_pc = %x type =%d src1=%d src2=%d dest=%d src1_rdy=%d src2_rdy =%d\n",bundle[0].di_pc,bundle[0].di_type,bundle[0].di_src1,bundle[0].di_src2,bundle[0].di_dst,bundle[0].di_src1_rdy,bundle[0].di_src2_rdy);
    printf("1 Dispatch di_pc = %x type =%d src1=%d src2=%d dest=%d src1_rdy=%d src2_rdy =%d\n",bundle[1].di_pc,bundle[1].di_type,bundle[1].di_src1,bundle[1].di_src2,bundle[1].di_dst,bundle[1].di_src1_rdy,bundle[1].di_src2_rdy);

    displayRob(rob);
    displayRmt(rmt);
    #endif

    int i = 0,j =0;
    dispatch_counter = 0;
    iq_counter = 0;

    for(i=0;i<temp_params.width;i++)    //count the number of instructions in DI counter
    {
        if(bundle[i].di_valid==1)
            dispatch_counter+=1;
    }

    for(i=0;i<temp_params.iq_size;i++)  //counts the number of instructions in IQ
    {
        if(issue_queue[i].iq_valid==0)
            iq_counter+=1;
    }
    //printf("In Dispatch1 dispatch_counter =%d and iq_counter =%d\n",dispatch_counter,iq_counter);
  
    for(i=0;i<temp_params.width;i++)    
    {
        if(rmt[bundle[i].di_src1].valid == 1 && bundle[i].di_src1_val_rob && bundle[i].di_valid == 1)  // Checking rediness of Source 1 from rob
        {
            //check through ROB entry in table
            if(rob[bundle[i].di_src1_tag].rdy == 1)
            {
                bundle[i].di_src1_rdy = 1;                           
            }
        }
        else if(rmt[bundle[i].di_src1].valid == 0) //(bundle[i].di_valid == 1)
        {
            bundle[i].di_src1_rdy = 1;
        }
    }

    for(i=0;i<temp_params.width;i++)    
    {  
        if(rmt[bundle[i].di_src2].valid == 1 && bundle[i].di_src2_val_rob && bundle[i].di_valid == 1)  // Checking rediness of Source 2 from rob
        {
            //check through ROB entry in table 
         
            if(rob[bundle[i].di_src2_tag].rdy == 1)
            {               
                bundle[i].di_src2_rdy = 1;                            
            }
        }
        else if(rmt[bundle[i].di_src2].valid == 0) 
        {
            bundle[i].di_src2_rdy = 1;
        }
    }
    //check dispatch is not empty then proceed
    if(dispatch_counter > 0)
    {
        if(iq_counter>=dispatch_counter)//move the entries only if vacancy in IQ is greater than or equal to number of instructions in DI bundle
        {
            for(i=0;i<temp_params.width;i++)
            {
                if(bundle[i].di_valid == 1)
                {
                    for(j=0;j<temp_params.iq_size;j++)  //finding a free place in IQ
                    {
                        if(issue_queue[j].iq_valid == 0)   //move only at a free place
                        {
                            #if 0
                            if(cyclic_counter > 65 && cyclic_counter < 75)
                            {
                                printf("In Dispatch di_pc = %x src1_rdy =%d and src2_rdy=%d\n",bundle[i].di_pc,bundle[i].di_src1_rdy,bundle[i].di_src2_rdy);
                            }
                            #endif

                            bundle[i].di_valid = 0;
                            issue_queue[j].iq_valid = 1;

                            issue_queue[j].iq_pc = bundle[i].di_pc;
                            issue_queue[j].iq_age = bundle[i].di_age;
                            issue_queue[j].iq_type = bundle[i].di_type;

                            issue_queue[j].iq_src1 = bundle[i].di_src1;
                            issue_queue[j].iq_src2 = bundle[i].di_src2;
                            issue_queue[j].iq_dst = bundle[i].di_dst;
                            
                            issue_queue[j].iq_src1_rdy = bundle[i].di_src1_rdy;
                            issue_queue[j].iq_src2_rdy = bundle[i].di_src2_rdy;

                            issue_queue[j].iq_src1_tag = bundle[i].di_src1_tag;
                            issue_queue[j].iq_src2_tag = bundle[i].di_src2_tag;
                            issue_queue[j].iq_dst_tag = bundle[i].di_dst_tag;
                        
                            issue_queue[j].iq_src1_val_rob = bundle[i].di_src1_val_rob;
                            issue_queue[j].iq_src2_val_rob = bundle[i].di_src2_val_rob;

                            pipeline[issue_queue[j].iq_age].iq = cyclic_counter+1;

                            #if 0
                            if(cyclic_counter > 65 && cyclic_counter < 75)
                            {
                                printf("In Dispatch issue_queue[i].iq_pc = %x src1_rdy =%d and src2_rdy=%d\n",issue_queue[j].iq_pc,issue_queue[j].iq_src1_rdy,issue_queue[j].iq_src2_rdy);
                                printf("In Dispatch issue_queue[i].iq_pc = %x src1_tag =%d and src2_tag=%d and dst_tag=%d\n",issue_queue[j].iq_pc,issue_queue[j].iq_src1_tag,issue_queue[j].iq_src2_tag,issue_queue[j].iq_dst_tag);                 
                                printf("IQ = %d\n",pipeline[issue_queue[j].iq_age].iq);
                            }
                            #endif

                            break;
                        }
                    }
                }
            }
        }
    }

    #if 0
    if(cyclic_counter > 65 && cyclic_counter < 75)
    {
     printf("In Dispatch After\n");
     displayRmt(rmt);
     displayRob(rob);
     displayIq(issue_queue);
    }
    #endif
}

void Issue(bundle_params bundle[],rob_params rob[],rmt_params rmt[],iq_params issue_queue[],execute_list execute[])
{
    #if 0
    if(cyclic_counter > 65 && cyclic_counter < 75)
    {
        printf("In Isuue issue_queue[i].iq_pc = %x\n",issue_queue[0].iq_pc);
        printf("In Issue temp_params.iq_size=%ld\n",temp_params.iq_size);
        displayIq(issue_queue);
    }
    #endif

    int i = 0,j = 0, k = 0;
    int ready_count = 0;

    for(i=0;i<temp_params.iq_size;i++)    
    {
        if(rmt[issue_queue[i].iq_src1].valid == 1 && issue_queue[i].iq_src1_val_rob && issue_queue[i].iq_valid == 1)  // Checking rediness of Source 1 from rob
        {
            //check through ROB entry in table 
            if(rob[issue_queue[i].iq_src1_tag].rdy == 1)
            {           
                issue_queue[i].iq_src1_rdy = 1;                      
            }
        }
        else if (issue_queue[i].iq_valid==1 && rmt[issue_queue[i].iq_src1].valid == 0)
        {           
            issue_queue[i].iq_src1_rdy = 1; 
        }   
    }
 
    for(i=0;i<temp_params.iq_size;i++)    
    {
        // printf("22 iq_pc =%x rmt[issue_queue[i].iq_src2].valid =%d issue_queue[i].iq_src2 =%d ,issue_queue[i].iq_src2_val_rob=%d and  issue_queue[i].iq_valid=%d\n",issue_queue[i].iq_pc,rmt[issue_queue[i].iq_src2].valid,issue_queue[i].iq_src2,issue_queue[i].iq_src2_val_rob,issue_queue[i].iq_valid);
        if(rmt[issue_queue[i].iq_src2].valid == 1 && issue_queue[i].iq_src2_val_rob && issue_queue[i].iq_valid == 1)  // Checking rediness of Source 1 from rob
        {
            //check through ROB entry in table 
            if(rob[issue_queue[i].iq_src2_tag].rdy == 1)
            {
                issue_queue[i].iq_src2_rdy = 1;                      
            }
        }
        else if(issue_queue[i].iq_valid==1 && rmt[issue_queue[i].iq_src2].valid==0)
        {
            issue_queue[i].iq_src2_rdy = 1; 
        }
    }
    //Finding valid and ready instruction count from Issue queue
    for(i=0;i<temp_params.iq_size;i++)
    {
        if(issue_queue[i].iq_src1_rdy && issue_queue[i].iq_src2_rdy && issue_queue[i].iq_valid ==1)
        {
            ready_count +=1;
        }
    }

    // Creating array to store age
    int ready_queue[ready_count],temp;

    // Storing all ready and valid instructions in ready queue
    for(i=0;i<temp_params.iq_size;i++)
    {
        if(issue_queue[i].iq_src1_rdy && issue_queue[i].iq_src2_rdy && issue_queue[i].iq_valid ==1)
        {
            ready_queue[j++] = issue_queue[i].iq_age;
        }
    }

    //Sort ready queue array
    for(i=0;i<ready_count;i++)
    {
        for(j=0;j<ready_count-1;j++)
        {
            if(ready_queue[j]>ready_queue[j+1])
            {
                temp=ready_queue[j];
                ready_queue[j]=ready_queue[j+1];
                ready_queue[j+1]=temp;
            }
        }
    }

    //if array size is greater than or equal to width,then issue width number of corresponding instruction from IQ
    if(ready_count>= temp_params.width)
    {
        for(i=0;i<temp_params.width;i++)
        {
            for(j=0;j<temp_params.iq_size;j++)  //finding the corresponding element in IQ
            {
                if(ready_queue[i]==issue_queue[j].iq_age && issue_queue[j].iq_valid==1)
                {
                    break;
                }
            }
            issue_queue[j].iq_valid =0;   //removing instruction form issue queue

            // forward instruction in Execute list
            for(k=0;k<temp_params.width*5;k++)
            {
                if(execute[k].ex_valid == 0)
                {
                    execute[k].ex_valid = 1;

                    execute[k].ex_pc = issue_queue[j].iq_pc;
                    execute[k].ex_age = issue_queue[j].iq_age;
                    execute[k].ex_type = issue_queue[j].iq_type;

                    execute[k].ex_src1 = issue_queue[j].iq_src1;
                    execute[k].ex_src2 = issue_queue[j].iq_src2;
                    execute[k].ex_dst = issue_queue[j].iq_dst;
                            
                    execute[k].ex_src1_tag = issue_queue[j].iq_src1_tag;
                    execute[k].ex_src2_tag = issue_queue[j].iq_src2_tag;
                    execute[k].ex_dst_tag = issue_queue[j].iq_dst_tag;
                        
                    execute[k].ex_src1_val_rob = issue_queue[j].iq_src1_val_rob;
                    execute[k].ex_src2_val_rob = issue_queue[j].iq_src2_val_rob;

                    //setting timer according to the latency of operation
                    if(execute[k].ex_type==0)
                    execute[k].ex_timer=1;
                    else if(execute[k].ex_type==1)
                    execute[k].ex_timer=2;
                    else if(execute[k].ex_type==2)
                    execute[k].ex_timer=5;

                    pipeline[execute[k].ex_age].ex = cyclic_counter+1;

                  //  printf("EX = %d\n",pipeline[execute[k].ex_age].ex);
                  //  printf("In Issue execute[k].ex_pc = %x src1_rdy =%d and src2_rdy=%d\n",execute[j].ex_pc,issue_queue[j].iq_src1_rdy,issue_queue[j].iq_src2_rdy);
                    break;
                }
            }
        }
    }
    else  //else array size is less than 'width', then issue all corresponding instructions from the issue queue bundle
    {
        for(i=0;i<ready_count;i++)
        {
            for(j=0;j<temp_params.iq_size;j++)  //finding the corresponding element in IQ
            {
                if(ready_queue[i]==issue_queue[j].iq_age && issue_queue[j].iq_valid==1)
                {
                    break;
                }
            }
            issue_queue[j].iq_valid =0;   //removing instruction form issue queue

            // forward instruction in Execute list
            for(k=0;k<temp_params.width*5;k++)
            {
                if(execute[k].ex_valid == 0)
                {
                    execute[k].ex_valid = 1;

                    execute[k].ex_pc = issue_queue[j].iq_pc;
                    execute[k].ex_age = issue_queue[j].iq_age;
                    execute[k].ex_type = issue_queue[j].iq_type;

                    execute[k].ex_src1 = issue_queue[j].iq_src1;
                    execute[k].ex_src2 = issue_queue[j].iq_src2;
                    execute[k].ex_dst = issue_queue[j].iq_dst;

                    execute[k].ex_src1_tag = issue_queue[j].iq_src1_tag;
                    execute[k].ex_src2_tag = issue_queue[j].iq_src2_tag;
                    execute[k].ex_dst_tag = issue_queue[j].iq_dst_tag;
                        
                    execute[k].ex_src1_val_rob = issue_queue[j].iq_src1_val_rob;
                    execute[k].ex_src2_val_rob = issue_queue[j].iq_src2_val_rob;
                            
                    //setting timer according to the latency of operation
                    if(execute[k].ex_type==0)
                    execute[k].ex_timer=1;
                    else if(execute[k].ex_type==1)
                    execute[k].ex_timer=2;
                    else if(execute[k].ex_type==2)
                    execute[k].ex_timer=5;

                    pipeline[execute[k].ex_age].ex = cyclic_counter+1;

                    //printf("EX = %d\n",pipeline[execute[k].ex_age].ex);
                    //printf("In Issue execute[k].ex_pc = %x src1_rdy =%d and src2_rdy=%d\n",execute[j].ex_pc,issue_queue[j].iq_src1_rdy,issue_queue[j].iq_src2_rdy);
                    break;
                }
            }
        }
    }

    #if 0
    if(cyclic_counter > 65 && cyclic_counter < 75)
    {
       displayRmt(rmt);
       displayRob(rob);
       printf("Issue End\n");
       displayIq(issue_queue);
    }
    #endif
}

void Execute(bundle_params bundle[],iq_params issue_queue[],rob_params rob[],rmt_params rmt[],execute_list execute[],WB writeback[])
{
    int i=0,j=0;
    for(i=0;i<temp_params.width*5;i++)
    {
        if(execute[i].ex_valid == 1)
        {
            #if 0
            if(cyclic_counter > 65 && cyclic_counter < 75)
            {
                printf("In Execute1 execute[%d].ex_timer=%d and pc = %x\n" ,i,execute[i].ex_timer,execute[i].ex_pc);
            }
            #endif

            execute[i].ex_timer -= 1;  //if valid instruction present in execute, decrease the ttl counter

            if(execute[i].ex_timer == 0) //checking if instruction in pipeline has finished executing
            {
                //adding the instruction to WB Bundle to an empty location
                for(j=0;j<temp_params.width*5;j++)
                {
                    if(writeback[j].wb_valid==0)
                    {                         
                        writeback[j].wb_valid = 1;
                        execute[i].ex_valid = 0;

                        writeback[j].wb_pc = execute[i].ex_pc;
                        writeback[j].wb_age = execute[i].ex_age;
                        writeback[j].wb_type = execute[i].ex_type;

                        writeback[j].wb_src1 = execute[i].ex_src1;
                        writeback[j].wb_src2 = execute[i].ex_src2;
                        writeback[j].wb_dst = execute[i].ex_dst;

                        writeback[j].wb_src1_tag = execute[i].ex_src1_tag;
                        writeback[j].wb_src2_tag = execute[i].ex_src2_tag;
                        writeback[j].wb_dst_tag = execute[i].ex_dst_tag;
                        
                        writeback[j].wb_src1_val_rob = execute[i].ex_src1_val_rob;
                        writeback[j].wb_src2_val_rob = execute[i].ex_src2_val_rob;

                       
                        pipeline[writeback[j].wb_age].wb = cyclic_counter+1;
                       
                        #if 0
                        if(cyclic_counter > 65 && cyclic_counter < 75)
                        {
                            printf("wb_pc =%x , src1=%d,src2=%d,dst =%d,src1_tag =%d , src2_tag =%d\n",writeback[j].wb_pc,writeback[j].wb_src1,writeback[j].wb_src2,writeback[j].wb_dst,writeback[j].wb_src1_tag,writeback[j].wb_src2_tag);
                            printf("WB = %d\n",pipeline[writeback[j].wb_age].wb);
                        }
                        #endif
                        break;
                    }
                }
                // waking up the dependent instruction in Issue Queue
                for(j=0;j<temp_params.iq_size;j++)
                {
                    if(issue_queue[j].iq_src1_tag == execute[i].ex_dst_tag && issue_queue[j].iq_src1_rdy == 0 && issue_queue[j].iq_valid==1)
                    {
                        issue_queue[j].iq_src1_rdy = 1;
                    }

                    if(issue_queue[j].iq_src2_tag == execute[i].ex_dst_tag && issue_queue[j].iq_src2_rdy == 0 && issue_queue[j].iq_valid==1)
                    {
                        issue_queue[j].iq_src2_rdy = 1;
                    }
                }

                // waking up the dependent instruction in Dispatch
                for(j=0;j<temp_params.width;j++)
                {
                    if(bundle[j].di_src1_tag == execute[i].ex_dst_tag && bundle[j].di_src1_rdy == 0 && bundle[j].di_valid==1)
                    {
                        bundle[j].di_src1_rdy = 1;
                    }

                    if(bundle[j].di_src2_tag == execute[i].ex_dst_tag && bundle[j].di_src2_rdy == 0 && bundle[j].di_valid==1)
                    {
                        bundle[j].di_src2_rdy = 1;
                    }
                }

                // waking up the dependent instruction in Register Read
                for(j=0;j<temp_params.width;j++)
                {
                    if(bundle[j].rr_src1_tag == execute[i].ex_dst_tag && bundle[j].rr_src1_rdy == 0 && bundle[j].rr_valid==1)
                    {
                        bundle[j].rr_src1_rdy = 1;
                    }

                    if(bundle[j].rr_src2_tag == execute[i].ex_dst_tag && bundle[j].rr_src2_rdy == 0 && bundle[j].rr_valid==1)
                    {
                        bundle[j].rr_src2_rdy = 1;
                    }
                }
            }
        }
    }

    #if 0
    if(cyclic_counter > 65 && cyclic_counter < 75)
    {
        printf("Rohan Here is Not fault");
        displayRmt(rmt);
        displayRob(rob);
        displayIq(issue_queue);
        printf("Rohan Here is fault");
    }
    #endif
}

void Writeback(bundle_params bundle[],rob_params rob[],rmt_params rmt[],WB writeback[])
{
    int i=0,j=0;
    for(i=0;i<temp_params.width*5;i++)
    {
        #if 0
        if(cyclic_counter > 65 && cyclic_counter < 75)
        {
            printf("Writeback Bundle: pc = %x %d\t%d\t%d\t%d\t%d\t%d\n",writeback[i].wb_pc,writeback[i].wb_type,writeback[i].wb_dst,writeback[i].wb_src1,writeback[i].wb_src2,writeback[i].wb_valid,writeback[i].wb_dst_tag);
        }
        #endif
        
        if(writeback[i].wb_valid==1)   //if instruction is present in the pipeline
        {
            for(j=0;j<temp_params.rob_size;j++)  //finding the corresponding element in IQ
            {
                if(writeback[i].wb_dst_tag == rob[j].number && rob[j].valid==1 && rob[j].rdy==0)
                {
                   
                    rob[j].rdy = 1;        //makes the corresponding ready bit 1
                    writeback[i].wb_valid = 0;   //removes the instruction from the pipeline
                    rob[j].age = writeback[i].wb_age;
                    pipeline[rob[j].age].rt_start=cyclic_counter+1;

                    #if 0
                    if(cyclic_counter > 65 && cyclic_counter < 75)
                    {
                        printf("In Wb2 pipeline[rob[j].age].rt_start =%d\n",pipeline[rob[j].age].rt_start);
                        printf("Inside WB and head_index =%d and j =%d and tail_index =%d\n",head_index,j, tail_index);
                    }
                    #endif

                    break;
                }
            }
        }
    }
}

void Retire(bundle_params bundle[],rob_params rob[],rmt_params rmt[])
{
  //  printf("In Retire \n");
    int i = 0;

    //for fetching upto width instructions at a time from the header in ROB
    for(i=0;i<temp_params.width;i++)
    {
        if((rob[head_index].rdy)==1 && (rob[head_index].dst)!=-1 && (rob[head_index].valid)==1)   //if rdy bit is 1 and it contains a destinations register
        {
            if((rmt[(rob[head_index].dst)].rob_tag == (rob[head_index].number)) && (rmt[(rob[head_index].dst)].valid==1))    //if rmt rob tag matches the rob number, then clear the rmt entry and rob entry
            {
                rmt[(rob[head_index].dst)].valid=0;
                rmt[(rob[head_index].dst)].rob_tag=0;
            }

            rob[head_index].exc=0;
            rob[head_index].mis=0;
            rob[head_index].rdy=0;
            rob[head_index].valid=0;
           
            pipeline[rob[head_index].age].rt_end=cyclic_counter+1;

            if((rob[head_index].number)==temp_params.rob_size-1)   //check if head pointer reaches the end of ROB
                head_index=0;                        //if yes, then point to start else point to next
            else
                head_index++;    
        }
        else if((rob[head_index].dst)==-1 && (rob[head_index].rdy)==1 && (rob[head_index].valid)==1)    //condition if instruction does not have a destination register no need to clear RMT because No data in RMT for dst = -1.
        {
            rob[head_index].exc=0;
            rob[head_index].mis=0;
            rob[head_index].rdy=0;
            rob[head_index].valid=0;

            pipeline[rob[head_index].age].rt_end=cyclic_counter+1;

            if((rob[head_index].number)==temp_params.rob_size-1)   //check if head pointer reaches the end of ROB
                head_index=0;                        //if yes, then point to start else point to next
            else
                head_index++;  
        }
        else
        {
            break;
        }
    }
}

void displayRob(rob_params rob[])
{
    #if 0
    int i;
    for(i=0;i<temp_params.rob_size;i++)
    {
          printf("Rob number = %d\t\t dst = %d\t\t rdy = %d\t\t valid = %d\n",rob[i].number,rob[i].dst,rob[i].rdy,rob[i].valid);
    }
    printf("Head:%d\n",head_index);
    printf("Tail:%d\n",tail_index);
    #endif
}

void displayRmt(rmt_params rmt[])
{
    #if 0
    int i;
    for(i=0;i<31;i++)
    {
          printf("rmt  valid = %d\t\t rob_tag=%d\n",rmt[i].valid,rmt[i].rob_tag);
    }
    #endif
}

void displayIq(iq_params issue_queue[])
{
    #if 0
    int i;
    for(i=0;i<temp_params.iq_size;i++)
    {
          printf("issue_queue  pc =%x valid = %d type = %d src1 = %d  src1_rdy = %d src2 = %d src2_rdy = %d\n",issue_queue[i].iq_pc,issue_queue[i].iq_valid,issue_queue[i].iq_type,issue_queue[i].iq_src1,issue_queue[i].iq_src1_rdy,issue_queue[i].iq_src2,issue_queue[i].iq_src2_rdy);
    }
    #endif
}