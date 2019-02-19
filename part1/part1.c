#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

//MACROS
#define MEMORY 64   // memory of 64 bytes
#define PAGE 16     // page of 16 bytes
#define VMEM 64     // virtual memory
#define WRITEBIT 1  // This will used if a page is writable
//#define PTE 4       //size of a page table entry
#define NUMPAGES 4  //total number of pages
#define VPN 0       //virtual page number byte offset in a PTE
//#define PFN 1       //PFN byte offset in a PTE
#define PROTECTION 2//protection bits offset in a PTE
#define VALID 3       //VALID bits offset in a PTE

typedef struct PageTableEntry {
    int valid;  // 0 or 1, 0 means valid & 1 means not valid
    int prot;   // 0 or 1, 0 means read only & 1 means read&write
    int PFN;    // Physical Frame Number or Virtual Page Number (we assuming the same here in this project)
    int value;  // Value store inside the PFN offset
} PageTableEntry;

typedef struct Page {
    PageTableEntry* pte[16];
} Page;

typedef struct Memory {
    Page* page[4];
} Memory;

int map(unsigned char pid,unsigned char vaddr,unsigned char val);
int store(unsigned char pid,unsigned char vaddr,unsigned char val);
int load(unsigned char pid,unsigned char vaddr,unsigned char val);

int map(unsigned char pid,unsigned char vaddr,unsigned char val){

    return 1;
}
