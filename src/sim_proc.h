#ifndef SIM_PROC_H
#define SIM_PROC_H

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

typedef struct rob_params{
    int valid;
    int number;
    int dst;
    int rdy;
    int exc;
    int mis;
    int pc;
    int age;
}rob_params;

typedef struct iq_params{
    int iq_valid;
    int iq_pc;
    int iq_type;
    int iq_age;

    int iq_dst;
    int iq_src1;
    int iq_src2;

    int iq_src1_rdy;
    int iq_src2_rdy;
    int iq_dst_rdy;

    int iq_src1_tag;
    int iq_src2_tag;
    int iq_dst_tag;
    int iq_src1_val_rob;
    int iq_src2_val_rob;


}iq_params;

typedef struct rmt_params{
    int valid;
    int rob_tag;
}rmt_params;

typedef struct bundle_params{
    int fe_valid;
    int fe_age;

    int de_valid;
    int de_pc;
    int de_type;
    int de_dst;
    int de_src1;
    int de_src2;
    int de_age;

    int rn_valid;
    int rn_pc;
    int rn_type;
    int rn_dst;
    int rn_src1;
    int rn_src2;
    int rn_age;
    int rn_src1_rdy;
    int rn_src2_rdy;
    int rn_dst_rdy;
    int rn_src1_tag;
    int rn_src2_tag;
    int rn_dst_tag;
    int rn_src1_val_rob;
    int rn_src2_val_rob;


    int rr_valid;
    int rr_pc;
    int rr_type;
    int rr_dst;
    int rr_src1;
    int rr_src2;
    int rr_age;
    int rr_src1_rdy;
    int rr_src2_rdy;
    int rr_dst_rdy;
    int rr_src1_tag;
    int rr_src2_tag;
    int rr_dst_tag;
    int rr_src1_val_rob;
    int rr_src2_val_rob;

    int di_valid;
    int di_pc;
    int di_type;
    int di_dst;
    int di_src1;
    int di_src2;
    int di_age;
    int di_src1_rdy;
    int di_src2_rdy;
    int di_dst_rdy;
    int di_src1_tag;
    int di_src2_tag;
    int di_dst_tag;
    int di_src1_val_rob;
    int di_src2_val_rob;
}bundle_params;

typedef struct execute_list
{
    int ex_valid;
    int ex_pc;
    int ex_type;
    int ex_dst;
    int ex_src1;
    int ex_src2;
    int ex_age;
    int ex_timer;
    int ex_src1_tag;
    int ex_src2_tag;
    int ex_dst_tag;
    int ex_src1_val_rob;
    int ex_src2_val_rob;
}execute_list;

typedef struct WB
{
    int wb_valid;
    int wb_pc;
    int wb_type;
    int wb_dst;
    int wb_src1;
    int wb_src2;
    int wb_age;
    int wb_timer;
    int wb_src1_tag;
    int wb_src2_tag;
    int wb_dst_tag;
    int wb_src1_val_rob;
    int wb_src2_val_rob;
}WB;

struct pipe
{
    int seq,type,src1,src2,dst,fe,de,rn,rr,di,iq,ex,wb,rt_start,rt_end;
};
#endif
