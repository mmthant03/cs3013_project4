#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

//MACROS
#define PAGE 16    // page of 16 bytes
#define VMEM 64    // virtual memory
#define WRITEBIT 1 // This will used if a page is writable
//#define PTE 4       //size of a page table entry
#define NUMPAGES 4 //total number of pages
//#define PFN 1       //PFN byte offset in a PTE
#define PROTECTION 2 //protection bits offset in a PTE
#define VALID 3      //VALID bits offset in a PTE

// 64 bytes of memory
unsigned char memory[64];
unsigned char fileMemory[1000];

// Search for free undeclared PFN, 0 means free unallocated, 1 means allocated
int pfnList[4];

/**Page table
 * The first 16 bytes or Memory are going to be Page Table Entries
 * Each 4 byte is a page table entry which contains 
 * VPN : 0 to 4, 
 * PFN : 16 + VPN, 
 * Valid : 2 for present, 1 for allocated and 0 for invalid
 * Protection bits : 0 for read, 1 for read&write
 * Each user given virtual address corresponds to VPN
 * Example: If user gives 18, VPN will be 1. If user gives 10, VPN will be 0.
 */

int map(unsigned char pid, unsigned char vaddr, unsigned char val);
int store(unsigned char pid, unsigned char vaddr, unsigned char val);
int load(unsigned char pid, unsigned char vaddr, unsigned char val);

typedef struct PageTableEntry{
	int VPN;
	int PFN;
	int allocated;
	int protection;
} PageTableEntry;

typedef struct Process{
	//int pid; is the index within the masterStruct
	int isPageTableInMemory; //PPN if page table is in memory -1 otherwise
	int isPageInMemory[4]; //PPN if page is in memory, -1 otherwise
	PageTableEntry *PTE[4]; //this is page table
	int page[16][4];
} Process;

typedef struct masterStruct
{
	Process *process[4];
} masterStruct;

masterStruct *theMasterStruct;
int nextFreePage;

void init()
{
	theMasterStruct = (masterStruct*) malloc(sizeof(masterStruct));
	for (int i = 0; i < 4; i++) { //for each Process, malloc
		Process *tempProcess = (Process*)malloc(sizeof(Process));
		(*theMasterStruct).process[i] = tempProcess;
		for (int j = 0; j < 4; j++) { //for each PageTableEntry of each process, malloc
			tempProcess->PTE[j] = malloc(sizeof(PageTableEntry));
			(*tempProcess).PTE[j]->VPN = -1;
			(*tempProcess).PTE[j]->PFN = -1;
			(*tempProcess).PTE[j]->allocated = -1;
			(*tempProcess).PTE[j]->protection = -1;
		}
		for (int j = 0; j < 4; j++) {//for each pageInMemory, set -1 for start
			tempProcess->isPageInMemory[j] = -1;
		}
		
	}
	nextFreePage = 0; //start off round robin PF0
	
}

// int locatePFN()
// {
//     for (int i = 1; i < 4; i++)
//     {
//         if (pfnList[i] == -1)
//         {
//             pfnList[i] = 1;
//             return i * 16;
//         }
//     }
//     return -1;
// }

int map(unsigned char pid, unsigned char vaddr, unsigned char val)
{
    return 0;
}

int store(unsigned char pid, unsigned char vaddr, unsigned char val)
{
    return -1;
}

int load(unsigned char pid, unsigned char vaddr, unsigned char val)
{
	return 0;
}

//returns the number of the next free page
//and moves the nextFreePage counter by 1
int getNextFreePage() {
	//example, if the nextFreePage is 2, return 2, but set the nextFreePage to three.
	if (nextFreePage == 3) {
		nextFreePage = 0;
		return 3;
	}
	else {
		nextFreePage++;
		return nextFreePage-1 ;
	}
}

void printMemory() {
	printf("starting print [memory]\n");
	for (int i = 0; i <64; i++) {
		printf("%d %d\n", i, memory[i]);
	}
}
	
void printStruct() {
	printf("printing struct start\n");
	for (int i = 0; i < 1; i++) {
		printf("	Printing process %d\n", i);
		printf("	isPageTableInMemory = %d\n", theMasterStruct->process[i]->isPageTableInMemory);
		for (int j = 0; j < 4; j++) {
			printf("		process %d, page %d is in memory %d\n", i, j, theMasterStruct->process[i]->isPageInMemory[j]);
		}
		for (int k = 0; k < 4; k++) {//for each pte
			PageTableEntry *tempPTE = theMasterStruct->process[i]->PTE[k];
			printf("		VPN %d, PFN %d, allocated %d, protection %d\n", tempPTE->VPN
			, tempPTE->PFN, tempPTE->allocated, tempPTE->protection);
		}
	}
}
	
	/**
	typedef struct PageTableEntry{
		int VPN;
		int PFN;
		int allocated;
		int protection;
	} PageTableEntry;

	typedef struct Process{
		//int pid; is the index within the masterStruct
		int isPageTableInMemory; //PPN if page table is in memory -1 otherwise
		int isPageInMemory[4]; //PPN if page is in memory, -1 otherwise
		PageTableEntry *PTE[4]; //this is page table
		int page[16][4];
	} Process;

	typedef struct masterStruct
	{
		Process *process[4];
	} masterStruct;
	*/

int storeFromStructToMemory() {
	//theMasterStruct
	//unsigned char memory[MEMORY];
	//unsigned char fileMemory[1000];
	printf("WE CHECKING IN STOREEEEE %d\n", theMasterStruct->process[0]->page[2][9]);
	
	printf("got tothe func\n");
	for (int i = 0; i < 4; i++) { //for each process
		printf("got to the loop %d\n", i);
		int pid = i;
		Process *currentProcess = theMasterStruct->process[i];
		int isPageTableInMemory = currentProcess->isPageTableInMemory;
		
		if (isPageTableInMemory != -1) { //copy the page table to memory[] if needed
			memcpy ( &(memory[16*isPageTableInMemory]) , currentProcess->PTE , 16 );
			//memcpy ( currentProcess->PTE , &(memory[isPageTableInMemory]) , 16 );
			//printf ("%s\n", memory[isPageTableInMemory]);
		}
		
		for (int k = 0; k < 4; k++) {//load the pages
			if (currentProcess->isPageInMemory[k] != -1) {//if page is in memory
				printf("got to er for %d %d\n", k, currentProcess->isPageInMemory[k]);
				printf("%d\n", currentProcess->page[k][9]);
				printf("%d\n", memory[16*(currentProcess->isPageInMemory[k])] );
				
				int *tempInt = malloc (sizeof(int));
				*tempInt = 100;
				memcpy ( &(memory[16*currentProcess->isPageInMemory[k]]) , tempInt , 16);
				printf("%d\n", memory[16*(currentProcess->isPageInMemory[k])] );
				
				int *tempInt2 = memcpy( tempInt , &(memory[16*currentProcess->isPageInMemory[k]]) , 16);
				printf("TEMP 22ER2 %d\n", *tempInt2);
			}
		}
		
		
		for (int j = 0; j < 4; j++) { //for each page
			//printf("%d\n", currentProcess->PTE[j]->VPN);
		}
	}
	printStruct();
	printMemory();
	/**typedef struct PageTableEntry{
		int VPN;
		int PFN;
		int allocated;
		int protection;
	} PageTableEntry;

	typedef struct Process{
		//int pid; is the index within the masterStruct
		int isPageTableInMemory; //PPN if page table is in memory -1 otherwise
		int isPageInMemory[4]; //PPN if page is in memory, -1 otherwise
		PageTableEntry *PTE[4]; //this is page table
		int page[4][16]
	} Process;

	typedef struct masterStruct
	{
		Process *process[4];
	} masterStruct; */
}

int main()
{
//unsigned char pid, choice = -1, vaddr, val = -1;
    char str[20];
    init();
    
	
	// example values
	int pid = 0;
	int vaddr = 41;
	int tempVPN = vaddr/16;
	int PPN = vaddr/16;
	int offset = vaddr%16;
	int value = 100;
	int protection = 1;
	
	//try mapping a page for test
	//start with page table
	int gottenPPage = getNextFreePage(); //0
	theMasterStruct->process[0]->isPageTableInMemory = gottenPPage; //page table goes in 1st gotten pg
	printf("page table at physical fr %d\n", theMasterStruct->process[0]->isPageTableInMemory); 

	//get the page for datum
	int gottenPPage2 = getNextFreePage(); //1
	theMasterStruct->process[pid]->isPageInMemory[PPN] = gottenPPage2;
	theMasterStruct->process[pid]->PTE[tempVPN] -> VPN =  tempVPN; //PTE's VPN is 2
	theMasterStruct->process[pid]->PTE[tempVPN] -> PFN = gottenPPage2; //PTE's PFN = 0
	theMasterStruct->process[pid]->PTE[tempVPN] -> allocated = 1;
	theMasterStruct->process[pid]->PTE[tempVPN] -> protection = 1;	
	
	
	//try storing a value
	theMasterStruct->process[pid]->page[tempVPN][offset] = value;
	//theMasterStruct->process[pid]->isPageInMemory[PPN] = gottenPPage;
	
	//try loading a value
	int load = theMasterStruct->process[pid]->page[tempVPN][offset];
	printf("%d hehehehe\n", load);
	
	storeFromStructToMemory();
	
	/*typedef struct PageTableEntry{
		int VPN;
		int PFN;
		int allocated;
		int protection;
	} PageTableEntry;

	typedef struct Process{
		//int pid; is the index within the masterStruct
		int isPageTableInMemory; //PPN if page table is in memory -1 otherwise
		int *isPageInMemory[4]; //PPN if page is in memory, -1 otherwise
		PageTableEntry *PTE[4]; //this is page table
		int page[4][16];
	} Process;

	typedef struct masterStruct
	{
		Process *process[4];
	} masterStruct; */
	
	return 0;
}