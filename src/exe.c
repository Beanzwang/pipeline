/* execution */
/* 1. Add using ALU
   2. Read control signals to decide which action to take.
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "struct.h"
#include "dataHazard.h"


extern int jump;
extern int clear_id;

void exe_print(struct EX_MEM* ex_mem) {
    printf("\nEX/MEM :\n");
    printf("ALUout\t\t%d\n", ex_mem->ALUOut);
    printf("WriteData\t%d\n", ex_mem->WriteData);
    printf("Rt/Rd\t\t%d\n", ex_mem->rt_rd);
    printf("Control signals\t%s\n", ex_mem->control_signal);
}

/* funct: */
const int add_funct = 32;
const int sub_funct = 34;
const int and_funct = 36;
const int or_funct = 37;
const int slt_funct = 42;

/* control signals */
/* RegDst ALUOp1 ALUOp2 ALUSrc */

struct EX_MEM* execution(struct ID_EX* id_ex, struct MEM_WB* mem_wb) {
    /* pass in ex_mem -> checking hazard */
    char ALUOp[3];
    char *control_bits;
    struct EX_MEM *ex_mem = (struct EX_MEM*)malloc(sizeof(struct EX_MEM));
    ex_mem->control_signal = (char *)malloc(sizeof(char)*6);
    strncpy(ex_mem->control_signal, id_ex->control_signal + 4, 5);
    ex_mem->control_signal[5] = '\0';
    ALUOp[0] = id_ex->control_signal[1];
    ALUOp[1] = id_ex->control_signal[2];
    ALUOp[2] = '\0';
    /* check if bubble */
    int condition_1 = strcmp(id_ex->control_signal, "000000000") == 0;
    /* check if the instruction is all 0s (all 32 bits) */
    int condition_2 = (id_ex->rs == 0) && (id_ex->rt == 0) && (id_ex->rd == 0)
                        && (id_ex->addr == 0);
    if (condition_1 && !condition_2) {
        if (mem_wb->rt_rd == id_ex->rs)
            id_ex->rs_v = mem_wb->ALUOut;
        else
            id_ex->rt_v = mem_wb->ALUOut;
    }

    if (strcmp(ALUOp, "10") == 0) {
        /* r-type, see func */
        switch(id_ex->funct) {
            case add_funct:
                ex_mem->ALUOut = id_ex->rs_v + id_ex->rt_v;
                break;
            case sub_funct:
                ex_mem->ALUOut = id_ex->rs_v - id_ex->rt_v;
                break;
            case and_funct:
                ex_mem->ALUOut = id_ex->rs_v & id_ex->rt_v;
                break;
            case or_funct:
                ex_mem->ALUOut = id_ex->rs_v | id_ex->rt_v;
                break;
            case slt_funct:
                if (id_ex->rs_v < id_ex->rt_v) ex_mem->ALUOut = 1;
                else ex_mem->ALUOut = 0;
                break;
            default:
                /* cannot recognize the value */
                break;
        }
    } else {
        if (strcmp(ALUOp, "00") == 0) {
            /* do add */
            /* lw or sw or addi -> rs + addr*/
            /* control signal[3] = ALUSrc */
            if (id_ex->control_signal[3] == '1') {
                ex_mem->ALUOut = id_ex->rs_v + id_ex->addr;
            } else {
                ex_mem->ALUOut = id_ex->rs_v + id_ex->rt_v;
            }
        } else if (strcmp(ALUOp, "11") == 0) {
            /* andi */
            ex_mem->ALUOut = id_ex->rs_v & id_ex->addr;
        } else if (strcmp(ALUOp, "01") == 0) {
            /* branch not equal */
            ex_mem->ALUOut = id_ex->rs_v - id_ex->rt_v;
            if (ex_mem->ALUOut != 0) {
                /* the number of instructions to jump */
                jump = id_ex->addr + 1;
            }
        }
    }
    if (id_ex->control_signal[0] == '1') {
        /* RegDst comes from rd */
        ex_mem->rt_rd = id_ex->rd;
    } else {
        /* RegDst comes from rt */
        ex_mem->rt_rd = id_ex->rt;
    }
    ex_mem->WriteData = id_ex->rt_v;
    
    /* if jump, change PC and clear ID_EX instruction */
    if (jump != 1) {
        clear_id = 1;
    }

    return ex_mem;
}