/*
Author: Keaton Shelton
File: kernel.c
Desc: Primary C driver for HW2 along with supporting structures
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>


//Defined Max Constants
#define P_MAX 6
#define STACK_SIZE 1024
//#define NULL ((void*)0)


//Structures
//IDT Structs
struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t always0;
    uint8_t access;
    uint16_t base_high;
}__attribute__((packed));
typedef struct idt_entry idt_entry_t;

struct idtr {
    uint16_t limit;
    uint32_t base;
}__attribute__((packed));
typedef struct idtr idtr_t;

//PCB Struct
struct pcb {
    uint32_t* esp;
    uint32_t pid;
    uint32_t priority;
    struct pcb* next;
}__attribute__((packed));
typedef struct pcb pcb_t;

//Queue Struct
struct pcbq {
    struct pcb* head;
    struct pcb* tail;
    uint32_t q_total;
}__attribute__((packed));
typedef struct pcbq pcbq_t;

//Declare Stack / Loaded Stacks / Loaded Processes
uint32_t pStack[P_MAX][STACK_SIZE];
uint32_t loaded_stacks = 0;
uint32_t loaded_proc = 0;


//Declare Main Queue
pcbq_t queue;
//Declare Intermediate Queues
pcbq_t lowQueue;
pcbq_t medQueue;
pcbq_t highQueue;


//Declare PCB Array and current PCB
pcb_t PCB_A[P_MAX];
pcb_t* Running;

//Declare IDT Array and IDTR
idt_entry_t IDTa[256];
idtr_t idtr;


//Messages For Screen Functions
char *message = "Running...";
char *topbotom = "+------------------------------------------------------------------------------+";
char *middle = "|                                                                              |";
char *blank = "                                                                                ";
char *errormsg = "ERROR";


//Function Prototypes
//Screen Function Prototypes
void k_print(char *string, int string_length, int row, int column);
void k_clearscr();
void print_border(int start_row, int start_column, int end_row, int end_column);


//PIC Processes
void setup_PIC();
void outportb(uint16_t port, uint8_t value);

//Interrupt Timer Value Programmer
void init_timer_dev(uint8_t value);


//Process Creation Function Prototypes
uint32_t* allocate_stack();
pcb_t* allocatePCB();
int create_process(uint32_t processEntry, uint32_t priority);
void enqueue(pcbq_t *q, pcb_t *pcb);
void enqueue_priority(pcbq_t *q, pcb_t *pcb);
//void enqueue();
pcb_t *dequeue();
void initQue();


//IDT Creation Function Prototypes
void default_handler();
void init_idt();
void init_idt_entry(idt_entry_t *entry, uint32_t base, uint16_t selector, uint8_t access);
void lidtr(idtr_t* help);
void dispatch();


//Starter
void go();


//Process Function Prototypes
void p1();
void p2();
void p3();
void p4();
void p5();
void idle();





//Start of Primary Function
void main() {
    int retval = 0;

    k_clearscr();
    init_idt();
    initQue();
    init_timer_dev(50);
    setup_PIC();

    /*
    retval = create_process((uint32_t)idle,5);
    if(retval == 1) {
        default_handler();
    }
    retval = create_process((uint32_t)p1, 10);
    if(retval == 1) {
        default_handler();
    }
    retval = create_process((uint32_t)p2, 10);
    if(retval == 1) {
        default_handler();
    }
    retval = create_process((uint32_t)p3, 12);
    if(retval == 1) {
        default_handler();
    }
    */
    retval = create_process((uint32_t)p3, 12);
    if(retval == 1) {
        default_handler();
    }
    retval = create_process((uint32_t)p1, 10);
    if(retval == 1) {
        default_handler();
    }
    retval = create_process((uint32_t)p2, 10);
    if(retval == 1) {
        default_handler();
    }
    retval = create_process((uint32_t)idle,5);
    if(retval == 1) {
        default_handler();
    }
    go();
}


//Function Definitions
//Screen Function Definitions
void k_clearscr() {
    for(int i = 0; i < 25;i++) {
            k_print(blank, 80,i,0);
    }
}

void print_border(int start_row, int start_column, int end_row, int end_column) {
    int startR = start_row + 1;
    k_print(topbotom, 80, start_row, start_column);
    for(startR; startR < 24; startR++) {
        k_print(middle, 80, startR, 0);
    }
    k_print(topbotom, 80, end_row, start_column);
}

//Process Creation Definitions
pcb_t* allocatePCB() {
    pcb_t* temp = &PCB_A[loaded_proc];
    loaded_proc++;
    return temp;
}

uint32_t* allocate_stack() {
    uint32_t* temp = pStack[loaded_stacks];
    temp += STACK_SIZE;
    loaded_stacks++;
    return temp;
}

int create_process(uint32_t processEntry, uint32_t priority) {
    if(loaded_stacks == P_MAX || loaded_proc == P_MAX) {
        return 1;
    }
    uint32_t ds = 8, es = 8, fs = 8, gs = 8;
    uint32_t cs = 16;
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0, esi = 0, edi = 0, esp = 0, ebp = 0;
    //Set st to Stack Size + stackptr
    uint32_t* st = allocate_stack();

    st--;
    *st = (uint32_t)go;
    st--;
    *st = (uint32_t)0x200; //set 32-bit word pointed at by st to 0 (Eflags with interrupts Enabled!)
    st--;
    *st = cs; //set 32-bit word pointed at by st to cs
    st--;
    *st = (uint32_t)processEntry; //set to address of first process
    st--;
    *st = ebp;
    st--;
    *st = esp;
    st--;
    *st = edi;
    st--;
    *st = esi;
    st--;
    *st = edx;
    st--;
    *st = ecx;
    st--;
    *st = ebx;
    st--;
    *st = eax;
    st--;
    *st = ds;
    st--;
    *st = es;
    st--;
    *st = fs;
    st--;
    *st = gs;

    Running = allocatePCB();
    Running->esp = st;
    Running->pid = loaded_proc;
    Running->priority = priority;
    enqueue_priority(&queue, Running);
    return 0;
}

void enqueue(pcbq_t *q, pcb_t *pcb) {
    if(q->head == NULL) {
        q->head = pcb;
        q->head->next = NULL;
    }
    else {
        q->tail->next = pcb;
    }
    q->tail = pcb;
    q->tail->next = NULL;
    q->q_total++;

}

void enqueue_priority(pcbq_t *q, pcb_t *pcb) {
 /*   pcb_t* current;
    if (q->head == NULL || q->head->priority < pcb->priority) {
        pcb->next = q->head;
        q->head = pcb;
    }
    else {
        current = q->head;
        while(current->next != NULL && current->next->priority <= pcb->priority) {
            current = current->next;
        }
        pcb->next = current->next;
        current->next = pcb;
    }
    */
   if(q->head == NULL) {
       q->head = pcb;
       q->head->next = NULL;
       q->tail = pcb;
   }
   else {
       pcb_t* currPCB = q->head;
       pcb_t* prevPCB = NULL;
       while (currPCB != NULL && currPCB->priority >= pcb->priority) {
           prevPCB = currPCB;
           currPCB = currPCB->next;
       }
       if(prevPCB == NULL) {
           q->head = pcb;
           pcb->next = currPCB;
       }
       else {
           prevPCB->next = pcb;
           pcb->next = currPCB;
           if(pcb->next == NULL) {
               q->tail = pcb;
           }
       }
   }
}

pcb_t *dequeue(pcbq_t *q) {
    if(q->head == NULL) {
        return NULL;
    }
    else {
        Running = q->head;
        q->head = (q->head)->next;
        if(q->head == NULL) {
            q->tail = NULL;
        }
        q->q_total--;
    }
    return Running;
}

void initQue() {
    queue.head = NULL;
    queue.tail = NULL;
    queue.q_total = 0;
}


//IDT Function Defintions
void default_handler() {
    k_print(errormsg,5,2,1);
    while(1) {}
}

void init_idt_entry(idt_entry_t *entry, uint32_t base, uint16_t selector, uint8_t access) {
    entry->base_high = ((uint32_t)base & 0xFFFF0000) >> 16;
    entry->base_low = ((uint32_t)base & 0x0000FFFF);
    entry->always0 = 0;
    entry->access = access;
    entry->selector = selector;
}

void init_idt() {
    //Call init_idt_entry() for entries 0 to 31 setting these entries to point to the default handler
    for(int i = 0; i <= 31;i++) {
        init_idt_entry((IDTa + i),(uint32_t)default_handler,16,0x8e);
    }
    init_idt_entry((IDTa + 32),(uint32_t)dispatch,16,0x8e);
    for(int i = 33; i <= 255;i++) {
        init_idt_entry((IDTa + i),(uint32_t)0,0,0);
    }
    idtr.limit = (sizeof(IDTa) - 1);
    idtr.base = (uint32_t)IDTa;
    lidtr(&idtr);
}


//Processes Definitions
void p1() {
    int i = 0;
    char msg1[] = "Process P1: ";
    int length = sizeof(msg1) / sizeof(msg1[0]);
    char* p = &msg1[0];
    for(int x = 0; x <= (INT_MAX/1000);x++) {
        char num = i + '0';
        char* numP = &num;
        k_print(p,length,5,0);
        k_print(numP,1,5,length+1);
        i = ((i+1) % 10); //Wanted to do 500 but cant figure it out 
    }
}

void p2() {
   int i = 0;
    char msg1[] = "Process P2: ";
    int length = sizeof(msg1) / sizeof(msg1[0]);
    char* p = &msg1[0];
    for(int x = 0; x <= (INT_MAX/1000);x++) {
        char num = i + '0';
        char* numP = &num;
        k_print(p,length,6,0);
        k_print(numP,1,6,length+1);
        i = ((i+1) % 10); //Wanted to do 500 but cant figure it out 
   } 
}

void p3() {
   int i = 0;
    char msg1[] = "Process P3: ";
    int length = sizeof(msg1) / sizeof(msg1[0]);
    char* p = &msg1[0];
    for(int x = 0; x <= (INT_MAX/1000);x++) {
        char num = i + '0';
        char* numP = &num;
        k_print(p,length,7,0);
        k_print(numP,1,7,length+1);
        i = ((i+1) % 10); //Wanted to do 500 but cant figure it out 
    } 
}

void p4() {
   int i = 0;
    char msg1[] = "Process P4: ";
    int length = sizeof(msg1) / sizeof(msg1[0]);
    char* p = &msg1[0];
    while(1) {
        char num = i + '0';
        char* numP = &num;
        k_print(p,length,8,0);
        k_print(numP,1,8,length+1);
        i = ((i+1) % 10); //Wanted to do 500 but cant figure it out 
    } 
}

void p5() {
   int i = 0;
    char msg1[] = "Process P5: ";
    int length = sizeof(msg1) / sizeof(msg1[0]);
    char* p = &msg1[0];
    while(1) {
        char num = i + '0';
        char* numP = &num;
        k_print(p,length,9,0);
        k_print(numP,1,9,length+1);
        i = ((i+1) % 10); //Wanted to do 500 but cant figure it out 
    } 
}

//Idle Function
void idle() {
    char msg1[] = "Process Idle Running...";
    int length = sizeof(msg1) / sizeof(msg1[0]);
    char* p = &msg1[0];
    char* move1 = "/";
    char* move2 = "\\";
    k_print(p,length,24,0);
    while (1) {
        k_print(move1,1,24,22);
        k_print(move2,1,24,22);
    }
}

//PIC Setup
void setup_PIC() {
// set up cascading mode:
    outportb(0x20, 0x11); // start 8259 master initialization
    outportb(0xA0, 0x11); // start 8259 slave initialization
    outportb(0x21, 0x20); // set master base interrupt vector (idt 32-38)
    outportb(0xA1, 0x28); // set slave base interrupt vector (idt 39-45)
    // Tell the master that he has a slave:
    outportb(0x21, 0x04); // set cascade ...
    outportb(0xA1, 0x02); // on IRQ2
    // Enabled 8086 mode:
    outportb(0x21, 0x01); // finish 8259 initialization
    outportb(0xA1, 0x01);
    // Reset the IRQ masks
    outportb(0x21, 0x0);
    outportb(0xA1, 0x0);
    // Now, enable the clock IRQ only 
    outportb(0x21, 0xfe); // Turn on the clock IRQ
    outportb(0xA1, 0xff); // Turn off all others
}