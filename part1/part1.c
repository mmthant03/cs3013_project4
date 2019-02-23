#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

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

int map(unsigned char pid, unsigned char vaddr, unsigned char val);
int store(unsigned char pid, unsigned char vaddr, unsigned char val);
int load(unsigned char pid, unsigned char vaddr, unsigned char val);

typedef struct PageTableEntry
{
	int VPN;
	int PFN;
	int allocated;
	int protection;
} PageTableEntry;

typedef struct Process
{
	//int pid; is the index within the masterStruct
	PageTableEntry *PTE[4]; //this is page table
	int page[4][16];
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
	theMasterStruct = (masterStruct *)malloc(sizeof(masterStruct));
	for (int i = 0; i < 4; i++)
	{ //for each Process, malloc
		Process *tempProcess = (Process *)malloc(sizeof(Process));
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
	printf(" we reach PEE%d\n", theMasterStruct->isCreated[pid]);
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
int getNextFreePage()
{
	//example, if the nextFreePage is 2, return 2, but set the nextFreePage to three.
	if (nextFreePage == 3)
	{
		nextFreePage = 0;
		return 3;
	}
	else
	{
		nextFreePage++;
		return nextFreePage - 1;
	}
}

void printMemory()
{
	printf("starting print [memory]\n");
	for (int i = 0; i < 64; i++)
	{
		printf("%d %d\n", i, memory[i]);
	}
}

void printFileMemory() {
	printf("starting print [file]\n");
	for (int i = 0; i <340; i++) {
		printf("%d %d\n", i, fileMemory[i]);
	}
}
	
void printStruct()
{
	printf("printing struct start\n");
	for (int i = 0; i < 4; i++)
	{
		printf("	Printing process %d\n", i);
		printf("	isPageTableInMemory = %d\n", isPageTableInMemory[i]);
		for (int j = 0; j < 4; j++) {
			printf("		process %d, page %d is in memory %d\n", i, j, theMasterStruct->process[i]->PTE[j]->PFN);

		}
		for (int k = 0; k < 4; k++)
		{ //for each pte
			PageTableEntry *tempPTE = theMasterStruct->process[i]->PTE[k];
			printf("		VPN %d, PFN %d, allocated %d, protection %d\n", tempPTE->VPN, tempPTE->PFN, tempPTE->allocated, tempPTE->protection);
		}
		for (int l = 0; l < 4; l++) {
			printf ("			page %d frame %d - ", i, l);
			for (int ip = 0; ip < 16; ip++) {
				//print out every value within the table
				printf("%d,", theMasterStruct->process[i]->page[l][ip]);
			}
			printf("\n");
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

int storeFromStructToMemory()
{
	//theMasterStruct
	//unsigned char memory[MEMORY];
	//unsigned char fileMemory[1000];
	printf("We are storing to memory %d\n", theMasterStruct->process[0]->page[2][9]);

	for (int i = 0; i < 4; i++)
	{ //for each process
		int pid = i;
		Process *currentProcess = theMasterStruct->process[i];

		
		//copy page table over
		if (isPageTableInMemory[i] != -1) { //copy the page table to memory[] if needed
		

			//start by copying the page table over
			unsigned char tempPageTable[16];
			for (int g = 0; g < 4; g++)
			{ //for each PTE, copy it to the temppage
				PageTableEntry *localPTE = currentProcess->PTE[g];
				tempPageTable[4 * g + 0] = localPTE->VPN;
				tempPageTable[4 * g + 1] = localPTE->PFN;
				tempPageTable[4 * g + 2] = localPTE->allocated;
				tempPageTable[4 * g + 3] = localPTE->protection;
			}
			memcpy ( &(memory[16*isPageTableInMemory[i]]) , tempPageTable, 16);//currentProcess->PTE , 16 );

		}

		//pages
		for (int k = 0; k < 4; k++) {//load the pages
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
	//printFileMemory();
}

int storeFromMemorytoStruct() {
	return 0;


}



/**
 * takes the PPNum
 * kicks the page in that PPNum (prints the msg)
 * or nothing if there is no page in that num
 */
int kick( int PPNum ) {
	//check the page tables if they are PPNum
	for (int i = 0; i < 4; i ++) {
		int PTPNum = isPageTableInMemory[i];
		if (PTPNum == PPNum) { //if this page table is the page we are looking for
			printf("Swapped frame %d to disk at swap slot %d\n", PTPNum, 256/16+i);
			isPageTableInMemory[i] = -1;
			return 1;
		}
	}
	
	//check if a page is in memory
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			int PageFN = theMasterStruct->process[i]->PTE[j]->PFN;
			if (PageFN == PPNum) {
				printf("Swapped frame %d to disk at swap slot %d\n", PageFN, 4*i+j);
				theMasterStruct->process[i]->PTE[j]->PFN = -1;
				return 1;
			}
		}
	}
	
	
	printf("no switch required\n");
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

/**
 * takes pid and vaddr
 * calls kick (switchStore) on the page it wants
 * then takes the page you want to load and put it into slot
 */
int switchLoadPT( int pid, int vaddr ) {
	int isPT = 1; //WE ARE A PAGE TABLE
	
	int diskSlot;
	int VPN = vaddr/16;
	int PFN;
	
	int exist = isPageTableInMemory[pid];
	if (exist == -1) {//page table is not in memory, but in file memory
		int PPNum = getNextFreePage();
		//kick the process at our PPNum
		kick( PPNum );
	
		//grab the new process
		isPageTableInMemory[pid] = PPNum;
		printf("Swapped disk slot %d into frame %d\n", 256/16+pid, PPNum);
	
	}
	
	else {//page table is in memory
		; //maybe?
	}
	

}

/**
 * takes pid and vaddr
 * calls kick (switchStore) on the page it wants
 * then takes the page you want to load and put it into slot
 */
int switchLoadPage( int pid, int vaddr) {
	
	printf("checking if need a switch\n");
	int diskSlot;
	int VPN = vaddr/16;
	int PFN;
	
	//check if the PAGE exists in memory
	int existPage = theMasterStruct->process[pid]->PTE[VPN]->PFN;

	//check if page table exists yet
	int exist = isPageTableInMemory[pid];
	
	//page table in memory already
	if (exist != -1) {
		nextFreePage = exist;
		int PPNum = getNextFreePage();
		//kick the process at our PPNum
		kick( PPNum );
		
		theMasterStruct->process[pid]->PTE[VPN]->PFN = PPNum;
		printf("Swapped disk slot %d into frame %d\n", 4*pid+VPN, PPNum);
	}
	//page table has to be swapped
	else {
		int PPNum = getNextFreePage();
		int PPNum2 = getNextFreePage();
		//swap our page table in
		switchLoadPT( pid, vaddr, PPNum );
		//swap our page in
		theMasterStruct->process[pid]->PTE[VPN]->PFN = PPNum;
		printf("Swapped disk slot %d into frame %d\n", 4*pid+VPN, PPNum);	
	}
	
}

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
int map(unsigned char pidTemp, unsigned char vaddrTemp, unsigned char val)
{
	printf("Map begin\n");

	int pid = pidTemp;
	int vaddr = vaddrTemp;
	int tempVPN = vaddr / 16;
	int PPN = vaddr / 16;
	int offset = vaddr % 16;
	int value = val;
	int protection = val;
	int gottenPPage;
	int gottenPPage2;	

	//processExists returns 1, 2, 3 corresponding to the cases above
	int processExists1 = processExists(pid, vaddr, theMasterStruct); //run helper function to see if this process exists

	
	printf("		we are map with case %d\n", processExists1);
	
	switch(processExists1) {
		case 1 :; // 1) Process does not exist whatsoever- > get two pages
			//get a page
			gottenPPage = getNextFreePage(); //0
			//kick the old member
			kick( gottenPPage );
			//load the PT into the page slot
			isPageTableInMemory[pid] = gottenPPage;
			printf("page table at physical fr %d\n", isPageTableInMemory[pid]); 

			//get the page for datum
			//get page 2nd
			gottenPPage2 = getNextFreePage();
			//kick the old member
			kick ( gottenPPage2) ;
			//load the page into the page slot
			theMasterStruct->process[pid]->PTE[tempVPN] -> VPN =  tempVPN; //PTE's VPN is 2
			theMasterStruct->process[pid]->PTE[tempVPN] -> PFN = gottenPPage2; //PTE's PFN = 0
			theMasterStruct->process[pid]->PTE[tempVPN] -> allocated = 1;
			theMasterStruct->process[pid]->PTE[tempVPN] -> protection = protection;	

			return 0;

//int switchLoadPT( int pid, int vaddr)
//int switchLoadPage( int pid, int vaddr) {
//int switchStore( int PPNum )

		case 2  :; //Process exists, but the page we are looking for is not allocated - > check if you can get one page, store it in there
			//swap PT in, if needed
			switchLoadPT ( pid, vaddr);
			//make sure you do not overwrite the PT
			nextFreePage = isPageTableInMemory[pid];
			
			//the page table now exists in memory[64]

			//get the page for datum
			gottenPPage2 = getNextFreePage(); //1
			kick (gottenPPage2);
			theMasterStruct->process[pid]->PTE[tempVPN] -> VPN =  tempVPN; //PTE's VPN is 2
			theMasterStruct->process[pid]->PTE[tempVPN] -> PFN = gottenPPage2; //PTE's PFN = 0
			theMasterStruct->process[pid]->PTE[tempVPN] -> allocated = 1;
			theMasterStruct->process[pid]->PTE[tempVPN] -> protection = protection;	
			
			return 0;
			break;
		
		case 3 :; //3) Process exists, and the page we are looking for is allocated already - > update permissions
			
			gottenPPage = getNextFreePage(); //0
			isPageTableInMemory[pid] = gottenPPage; //page table goes in 1st gotten pg
			printf("page table at physical fr %d\n", isPageTableInMemory[pid]); 

			//get the page for datum
			gottenPPage2 = getNextFreePage(); //1
			theMasterStruct->process[pid]->PTE[tempVPN] -> VPN =  tempVPN; //PTE's VPN is 2
			theMasterStruct->process[pid]->PTE[tempVPN] -> PFN = gottenPPage2; //PTE's PFN = 0
			theMasterStruct->process[pid]->PTE[tempVPN] -> allocated = 1;
			theMasterStruct->process[pid]->PTE[tempVPN] -> protection = protection;	
			
			return 0;
			printf("case 3\n");
			break;
	}
	
		//declare process existence
	theMasterStruct->isCreated[pid] = 1;
}

int store(unsigned char pidTemp, unsigned char vaddrTemp, unsigned char val)
{
	int pid = pidTemp;
	int vaddr = vaddrTemp;
	int tempVPN = vaddr / 16;
	int PPN = vaddr / 16;
	int offset = vaddr % 16;
	int value = (int) val;
	int protection = val;

	//try storing a value
	theMasterStruct->process[pid]->page[tempVPN][offset] = value;
    return -1;
}

int load(unsigned char pidTemp, unsigned char vaddrTemp, unsigned char val)
{
	int pid = pidTemp;
	int vaddr = vaddrTemp;
	int tempVPN = vaddr / 16;
	int PPN = vaddr / 16;
	int offset = vaddr % 16;
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
	unsigned char pid, command = -1, vaddr, val = -1;
	char str[20];
	init();

	printf("starting main\n");
	printStruct();
	map(0, 7,1);
	printf("starting store\n");
	store(0, 14, 100);
	storeFromStructToMemory();
	//printFileMemory();
	printStruct();
	//map(0,18,1);
	printf("printing files fuckkk\n");
	
	return 0;
	

	
	while (1) //process commands
	{
		char *token;
		while (strlen(str) < 10)
		{
			printf("Instruction?: ");
			if (fgets(str, 20, stdin) == NULL)
			{
				printf("\n");
				return 0;
			}
		}

		// first argument is Process ID
		token = strtok(str, ",");
		pid = token[0];
		pid -= 48;

		// second argement is Instruction
		token = strtok(NULL, ",");
		if (token[0] == 'm')
		{
			command = 0;
		}
		else if (token[0] == 's')
		{
			command = 1;
		}
		else if (token[0] == 'l')
		{
			command = 2;
		}

		// check if pid is [0..3]
		if (pid > 3 || pid < 0)
		{
			printf("ERROR: Invalid Process ID! Process ID should be from 0, 1, 2, or 3.\n");
			command = 3;
		}

		// third argument is Virtual address
		token = strtok(NULL, ",");
		vaddr = atoi(token);

		// check if vaddr is [0..63]
		if (vaddr < 0 || vaddr > 63)
		{
			printf("ERROR: Invalid Virtual address! Virtual Address should be inclusively from 0 to 63.\n");
			command = 3;
		}

		// fourth argument is Value
		token = strtok(NULL, ",");
		int value = atoi(token);

		//check of val is [0..255] for store command
		if (command == 1)
		{
			if (value < 0 || value > 255)
			{
				printf("Invalid value for store command! Value should be inclusively from 0 to 255.\n");
				command = 3;
			}
			else
			{
				val = (unsigned char)value;
			}
		}
		// check if val is 0 or 1 for map command
		else if (command == 0)
		{
			if (value < 0 || value > 1)
			{
				printf("Invalid value for map command! Please provide 0 for read only OR 1 for read&write.\n");
				command = 3;
			}
			else
			{
				val = (unsigned char)value;
			}
		}
		else if (command == 2)
		{
			val = (unsigned char)value;
		}
		token = NULL;
		printf("\n");

		switch (command)
		{
		case 0: // map
			printf("reached MAP, %d, %d, %d\n", pid, vaddr, val);
			map(pid, vaddr, val);
			break;
		case 1: // store
			printf("reached STORE, %d, %d, %d\n", pid, vaddr, val);
			store(pid, vaddr, val);
			break;
		case 2: // load
			printf("reached LOAD, %d, %d, %d\n", pid, vaddr, val);
			load(pid, vaddr, val);
			break;
		case 3: // if invalid, do not exit the program but try again
			break;
		default:
			printf("Error input was not proper try again\n");
		}

		//===============================

		// example values
		/*
	int pid = 0;
	int vaddr = 41;
	int tempVPN = vaddr/16;
	int PPN = vaddr/16;
	int offset = vaddr%16;
	int value1 = 1;
	int value2 = 100;
	int value3 = 0;
	int protection = 1;
	
	map(pid, vaddr, value1);
	
	store(pid, vaddr, value2);
	
	load(pid, vaddr, value3);
	*/
		//printf("STOREING\n");
		//storeFromStructToMemory();
	}
	/**typedef struct PageTableEntry{
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
		int isCreated[4];
	} masterStruct; */
	return 0;
}