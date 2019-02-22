#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


// 64 bytes of memory
unsigned char memory[64];
unsigned char fileMemory[1000];
int map(unsigned char pid, unsigned char vaddr, unsigned char val);
int store(unsigned char pid, unsigned char vaddr, unsigned char val);
int load(unsigned char pid, unsigned char vaddr, unsigned char val);

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

typedef struct PageTableEntry{
	int VPN;
	int PFN;
	int allocated;
	int protection;
} PageTableEntry;

typedef struct Process{
	//int pid; is the index within the masterStruct
	PageTableEntry *PTE[4]; //this is page table
	int page[16][4];
} Process;

typedef struct masterStruct
{
	Process *process[4];
	int isCreated[4];
} masterStruct;

int isPageTableInMemory[4];//PPN if page is in memory, -1 otherwise


masterStruct *theMasterStruct;
int nextFreePage;

void init()
{
	theMasterStruct = (masterStruct*) malloc(sizeof(masterStruct));
	for (int i = 0; i < 4; i++) { //for each Process, malloc
		Process *tempProcess = (Process*)malloc(sizeof(Process));
		theMasterStruct->process[i] = tempProcess;
		theMasterStruct->isCreated[i] = -1;
		
		isPageTableInMemory[i] = -1;
		for (int j = 0; j < 4; j++) { //for each PageTableEntry of each process, malloc
			tempProcess->PTE[j] = malloc(sizeof(PageTableEntry));
			(*tempProcess).PTE[j]->VPN = -1;
			(*tempProcess).PTE[j]->PFN = -1;
			(*tempProcess).PTE[j]->allocated = -1;
			(*tempProcess).PTE[j]->protection = -1;
		}
	}
	nextFreePage = 0; //start off round robin PF0
	
}

/**
 * There are three cases here
 * 1) Process does not exist whatsoever
		- > check if you can get two pages, that is all we need
 * 2) Process exists, but the page we are looking for is not allocated
		- > check if you can get one page, store it in there
 * 3) Process exists, and the page we are looking for is allocated already
		- > update permissions
 * return the value appropriate
 */
int processExists(int pid, int vaddr, masterStruct* theMasterStruct) {
		
	int VPN = vaddr/16;
	int isCreated = theMasterStruct->isCreated[pid];
	int isAllocated = theMasterStruct->process[pid]->PTE[VPN]->allocated;		
	
	if (isCreated == -1) {
		return 1; //process does not exist
	}
	else {
		if (isAllocated == -1) {
			return 2; //process exists but this page is not allocated
		}
		else {
			return 3; //process exists but this page is allocated
		}
		
	}
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

void printFileMemory() {
	printf("starting print [file]\n");
	for (int i = 0; i <340; i++) {
		printf("%d %d\n", i, fileMemory[i]);
	}
}
	
void printStruct() {
	printf("printing struct start\n");
	for (int i = 0; i < 4; i++) {
		printf("	Printing process %d\n", i);
		printf("	isPageTableInMemory = %d\n", isPageTableInMemory[i]);
		for (int j = 0; j < 4; j++) {
			printf("		process %d, page %d is in memory %d\n", i, j, theMasterStruct->process[i]->PTE[j]->PFN);
		}
		for (int k = 0; k < 4; k++) {//for each pte
			PageTableEntry *tempPTE = theMasterStruct->process[i]->PTE[k];
			printf("		VPN %d, PFN %d, allocated %d, protection %d\n", tempPTE->VPN
			, tempPTE->PFN, tempPTE->allocated, tempPTE->protection);
		}
		for (int l = 0; l < 4; l++) {
			
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
		PageTableEntry *PTE[4]; //this is page table
		int page[16][4];
	} Process;

	typedef struct masterStruct
	{
		Process *process[4];
		int isCreated[4];
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
		
		//copy page table over
		if (isPageTableInMemory[i] != -1) { //copy the page table to memory[] if needed
		
			//start by copying the page table over
			unsigned char tempPageTable[16];
			for (int g = 0; g < 4; g++) {//for each PTE, copy it to the temppage
				PageTableEntry *localPTE = currentProcess->PTE[g];
				tempPageTable[4*g+0] = localPTE->VPN;
				printf("%d just checking\n", localPTE->VPN);
				tempPageTable[4*g+1] = localPTE->PFN;
				tempPageTable[4*g+2] = localPTE->allocated;
				tempPageTable[4*g+3] = localPTE->protection;
			}
			memcpy ( &(memory[16*isPageTableInMemory[i]]) , tempPageTable, 16);//currentProcess->PTE , 16 );
		}
		
		//pages
		for (int k = 0; k < 4; k++) {//load the pages
			printf("%d WTF IS THE PFMN\n", currentProcess->PTE[k]->PFN);
			if (currentProcess->PTE[k]->PFN != -1) {//ifpage is in memory
				printf("got to er for %d %d\n", k, currentProcess->PTE[k]->PFN);
				printf("%d\n", currentProcess->page[k][9]);
				printf("%d\n", memory[16*(currentProcess->PTE[k]->PFN)] );

				
				//convert to unsigned cha
				unsigned char tempCharArray[16];
				for (int v = 0; v < 16; v++) { //convert the ints to unsigned chars
					tempCharArray[v] = currentProcess->page[k][v];
				}
				
				//copy the array to memory
				memcpy ( &(memory[16*currentProcess->PTE[k]->PFN]) , tempCharArray , 16);
				printf("%d\n", memory[16*(currentProcess->PTE[k]->PFN)] );
				
			}
		}
		

		/* 	file Memory is fix-mapped as follows
			0-63 is the memory for process 0
			64-127 is the memory for process 1
			128-191 is the memory for process 2
			192-255 is the memory for process 3
			256-319 is the memory for page tables
			320 - 335 is for the isCreated 4 int
		*/					
		//file memory

		/**typedef struct PageTableEntry{
				int VPN;
				int PFN;
				int allocated;
				int protection;
			} PageTableEntry;

			typedef struct Process{
				//int pid; is the index within the masterStruct
				PageTableEntry *PTE[4]; //this is page table
				int page[4][16]
			} Process;

			typedef struct masterStruct
			{
				Process *process[4];
				int isCreated[4];
			} masterStruct; */
			
		currentProcess = theMasterStruct->process[i];
		unsigned char tempCharArray[64];
		
		//store each page all 16
		for (int k = 0; k < 4; k++) {//for each page
			//convert to unsigned cha
			for (int v = 0; v < 16; v++) { //convert the ints to unsigned chars
				tempCharArray[16*k+v] = currentProcess->page[k][v];
			}
		}
		memcpy ( &(fileMemory[64*i]) , tempCharArray , 16); 	//copy the array/page to file
		
		//store each page table
		unsigned char tempPageTable[16];
		for (int g = 0; g < 4; g++) {//for each PTE, copy it to the temppage
			PageTableEntry *localPTE = currentProcess->PTE[g];
			tempPageTable[4*g+0] = localPTE->VPN;
			printf("%d just checking\n", localPTE->VPN);
			tempPageTable[4*g+1] = localPTE->PFN;
			tempPageTable[4*g+2] = localPTE->allocated;
			tempPageTable[4*g+3] = localPTE->protection;
		}
		memcpy ( &(fileMemory[256+16*i]) , tempPageTable, 16);
		
		//store
		//theMasterStruct->isCreated[i]
		unsigned char tempCreatedArray;
		tempCreatedArray = theMasterStruct->isCreated[i];
		memcpy( &(fileMemory[320+i]), &tempCreatedArray , 1);

	}
	//printMemory();
	//printStruct();
	printf("about to print file rwer\n");
	printFileMemory();
}

int storeFromMemorytoStruct() {
	return 0;


}



/* switch for map a new page
*/
	/**
	typedef struct PageTableEntry{
		int VPN;
		int PFN;
		int allocated;
		int protection;
	} PageTableEntry;

	typedef struct Process{
		//int pid; is the index within the masterStruct
		PageTableEntry *PTE[4]; //this is page table
		int page[16][4];
	} Process;

	typedef struct masterStruct
	{
		Process *process[4];
		int isCreated[4];
	} masterStruct;
	*/
int switchStore( int PPNum ) {
	theMasterStruct = (masterStruct*) malloc(sizeof(masterStruct));
	for (int i = 0; i < 4; i++) { //for each Process, malloc
		Process *tempProcess = (Process*)malloc(sizeof(Process));
		theMasterStruct->process[i] = tempProcess;
		theMasterStruct->isCreated[i] = -1;
		
		isPageTableInMemory[i] = -1;
		for (int j = 0; j < 4; j++) {//for each pageInMemory, set -1 for start
			tempProcess->PTE[j]->PFN = -1;
		}
		for (int j = 0; j < 4; j++) { //for each PageTableEntry of each process, malloc
			tempProcess->PTE[j] = malloc(sizeof(PageTableEntry));
			(*tempProcess).PTE[j]->VPN = -1;
			(*tempProcess).PTE[j]->PFN = -1;
			(*tempProcess).PTE[j]->allocated = -1;
			(*tempProcess).PTE[j]->protection = -1;
		}
	}
	nextFreePage = 0; //start off round robin PF0
	
}

int switchLoad( int PPNum ) {
	return 0;
}

int map(unsigned char pidTemp, unsigned char vaddrTemp, unsigned char val)
{
	printf("Finding valid process\n");
	/**
	 * There are three cases here
	 * 1) Process does not exist whatsoever
			- > check if you can get two pages, that is all we need
	 * 2) Process exists, but the page we are looking for is not allocated
			- > check if you can get one page, store it in there
	 * 3) Process exists, and the page we are looking for is allocated already
			- > update permissions
	 *
	*/
	int pid = pidTemp;
	int vaddr = vaddrTemp;
	int tempVPN = vaddr/16;
	int PPN = vaddr/16;
	int offset = vaddr%16;
	int value = val;
	int protection = val;
	
	
	theMasterStruct->isCreated[pid] = 1;
	
	//processExists returns 1, 2, 3 corresponding to the cases above
	int processExists1 = processExists(pid, vaddr, theMasterStruct); //run helper function to see if this process exists

		//declare process existence
	
	//try mapping a page for test
	//start with page table
	int gottenPPage = getNextFreePage(); //0
	isPageTableInMemory[pid] = gottenPPage; //page table goes in 1st gotten pg
	printf("page table at physical fr %d\n", isPageTableInMemory[pid]); 

	//get the page for datum
	int gottenPPage2 = getNextFreePage(); //1
	theMasterStruct->process[pid]->PTE[tempVPN] -> VPN =  tempVPN; //PTE's VPN is 2
	theMasterStruct->process[pid]->PTE[tempVPN] -> PFN = gottenPPage2; //PTE's PFN = 0
	theMasterStruct->process[pid]->PTE[tempVPN] -> allocated = 1;
	theMasterStruct->process[pid]->PTE[tempVPN] -> protection = protection;	
	
	return 0;
			
			/*
	
	switch(processExists1) {
		case 1 :; // 1) Process does not exist whatsoever- > get two pages


		case 2  :;
			printf("case 2 not ready\n");
			break;
		
		case 3 :;
			printf("case 3\n");
			break;
	}*/
	

}

int store(unsigned char pidTemp, unsigned char vaddrTemp, unsigned char val)
{
	int pid = pidTemp;
	int vaddr = vaddrTemp;
	int tempVPN = vaddr/16;
	int PPN = vaddr/16;
	int offset = vaddr%16;
	int value = val;
	int protection = val;
	
	//try storing a value
	theMasterStruct->process[pid]->page[tempVPN][offset] = value;
    return -1;
}

int load(unsigned char pidTemp, unsigned char vaddrTemp, unsigned char val)
{
	int pid = pidTemp;
	int vaddr = vaddrTemp;
	int tempVPN = vaddr/16;
	int PPN = vaddr/16;
	int offset = vaddr%16;
	int value = val;
	int protection = val;
	
		//try loading a value
	int load = theMasterStruct->process[pid]->page[tempVPN][offset];
	printf("%d hehehehe\n", load);
	
	
	return 0;
}

int main()
{
//unsigned char pid, choice = -1, vaddr, val = -1;
	unsigned char pid, choice = -1, vaddr, val = -1;
    char str[20];
    init();

	map(0, 7,1);
	store(0, 14, 100);
	//map(0,18,1);
	printf("printing files fuckkk\n");
	
	storeFromStructToMemory();
	
	return 0;
	
    while (1) //process commands
    {
        char *token;
        while ( strlen(str) < 10 )
        {
            printf("Instruction?: ");
            if (fgets(str, 20, stdin) == NULL)
            {
                printf("\n");
                return 0;
            }
        }
        token = strtok(str, ",");
        pid = token[0];
        pid -= 48;
        token = strtok(NULL, ",");
        if (token[0] == 'm')
        {
            choice = 0;
        }
        else if (token[0] == 's')
        {
            choice = 1;
        }
        else if (token[0] == 'l')
        {
            choice = 2;
        }
        token = strtok(NULL, ",");
        vaddr = atoi(token);
        token = strtok(NULL, ",");
        val = atoi(token);
        token = NULL;
        printf("\n");
        if (pid > 3 || pid < 0)
        {
            printf("Error process id is not valid, try again \n");
            choice = 3;
        }
        if (vaddr < 0 || vaddr > 63)
        {
            printf("Virtual address is not valid, try again \n");
            choice = 3;
        }
        if (val < 0 || val > 255)
        {
            printf("Incorrect value, try again");
            choice = 3;
        }
        switch (choice)
        {
        case 0:
			printf("reached MAP, %d, %d, %d\n", pid, vaddr, val);
            map(pid, vaddr, val);
            //print out
            break;
        case 1:
			printf("reached STORE, %d, %d, %d\n", pid, vaddr, val);
            store(pid, vaddr, val);
            //print out
            break;
        case 2:
			printf("reached LOAD, %d, %d, %d\n", pid, vaddr, val);
            load(pid, vaddr, val);
            break;
        // case 3:
        //     break;
        default:
            printf("Error input was not proper try again\n");
        }
		
		
		printf("STOREING\n");
		storeFromStructToMemory();
	}
	
	return 0;
}