#include <arch/process.h>
#include <arch/64bit.h>
#include <arch/mem/liballoc.h>
#include <arch/gdt.h>
#include <com.h>
#pragma GCC optimize ("-O0")
extern "C" void irq0_first_jump();
process* process_array;
process kernel_process;
process* current_process = 0x0;
bool process_locked = true;
bool process_loaded = false;
void lock_process(){
    process_locked = true;
}
void unlock_process(){
    process_locked = false;
}
void main_process_1(){
    while(true){
        com_write_str("hello 1");
    }
}
void main_process_2(){
    while(true){
        com_write_str("hello 2");
    }
}

void init_multi_process(){
    com_write_str("loading process");
    process_array = (process*)malloc(sizeof (process)* MAX_PROCESS);
    com_write_str("loading process 0 ");
    for (int i = 0; i < MAX_PROCESS; i++){
           process_array[i].pid = i;
             process_array[i].current_process_state = process_state::PROCESS_AVAILABLE;
           for(int j = 0; j < PROCESS_STACK_SIZE; j++){
               process_array[i].stack[j] = 0;
           }
    }
    com_write_str("loading process 1");
    init_process(main_process_1);
    com_write_str("loading process 2");
    init_process(main_process_2);
    process_loaded = true;


    asm volatile("jmp irq0_first_jump");
}

process* init_process(func entry_point){

    for (int i = 0; i < MAX_PROCESS; i++){

        if(process_array[i].current_process_state == process_state::PROCESS_AVAILABLE){
            process_array[i].current_process_state = process_state::PROCESS_WAITING;
            process_array[i].entry_point = (uint64_t)entry_point;
            process_array[i].rsp = ((uint64_t)process_array[i].stack) + PROCESS_STACK_SIZE ;

            uint64_t* rsp =(uint64_t*) process_array[i].rsp;
            rsp--;
            uint64_t crsp = (uint64_t)rsp;
             *rsp-- = SLTR_KERNEL_DATA;  // SS
             *rsp-- = crsp;       // RSP
             *rsp-- = 0x286;              // RFLAGS
             *rsp-- = SLTR_KERNEL_CODE;  // CS
             *rsp-- = (uint64_t)entry_point;        // RIP
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp-- = 0;
            *rsp = 0;
            process_array[i].rsp = (uint64_t)rsp;
            if(current_process == 0x0){
                current_process = &process_array[i];
            }
            return &process_array[i];
        }
    }
    com_write_str("no free process found");
    return 0x0;
}



void switch_context(InterruptStackFrame* current_Isf, process* next){
    if(next == 0){
        return ; // early return
     }
     if(current_process == 0x0){
         current_Isf = (InterruptStackFrame*)next->rsp;
        next->current_process_state = process_state::PROCESS_RUNNING;
        current_process = next;
     }else{
        current_process->current_process_state = PROCESS_WAITING;
        current_process->rsp = current_Isf->rsp;

        current_Isf = (InterruptStackFrame*)next->rsp;
        next->current_process_state = process_state::PROCESS_RUNNING;
        current_process = next;
     }
}

process* get_next_process(uint64_t current_id){
    for(uint64_t i = current_id; i < MAX_PROCESS; i ++){
        if(process_array[i].current_process_state == process_state::PROCESS_AVAILABLE){
            break;
        }else if(process_array[i].current_process_state == process_state::PROCESS_WAITING){
            return &process_array[i];
        }
    }for(uint64_t i = 0; i < current_id; i ++){ // we do a loop
        if(process_array[i].current_process_state == process_state::PROCESS_WAITING){
            return &process_array[i];
        }
    }
    com_write_str("no process found");
    return 0x0;
}

void irq_0_process_handler(InterruptStackFrame* isf){
    if(process_locked){
        return;
    }
    if(current_process == 0x0){
        process* i = get_next_process(0);
        if(i == 0){
            return;
        }
       switch_context(isf, i);
    }else{
        process* i = get_next_process(current_process->pid);
        if(i == 0){
            return;
        }
       switch_context(isf, i);
    }
}
char temp_esp[8192];
extern "C" uint64_t get_current_esp(){

    if(current_process == 0x0){
        return (uint64_t)temp_esp + 4096;
    }else{
        return (uint64_t)current_process->rsp;
    }
}extern "C" uint64_t get_next_esp(){
    if(current_process == 0){
        return
              (uint64_t)  get_next_process(0)->rsp;
    }else{
        return
               (uint64_t) get_next_process(current_process->pid)->rsp;

    }
}
extern "C" void task_update_switch(process* next){
    tss_set_rsp0(next->rsp);
    current_process->current_process_state = process_state::PROCESS_WAITING;
    current_process = next;
    next->current_process_state = process_state::PROCESS_RUNNING;
}
