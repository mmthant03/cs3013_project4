#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

//MACROS
#define MEMORY 64  // memory of 64 bytes
#define PAGE 16    // page of 16 bytes
#define VMEM 64    // virtual memory
#define WRITEBIT 1 // This will used if a page is writable
//#define PTE 4       //size of a page table entry
#define NUMPAGES 4 //total number of pages
#define VPN 0      //virtual page number byte offset in a PTE
//#define PFN 1       //PFN byte offset in a PTE
#define PROTECTION 2 //protection bits offset in a PTE
#define VALID 3      //VALID bits offset in a PTE

// 64 bytes of memory
unsigned char memory[MEMORY];

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

typedef struct process
{
    int pid;
    int address;
    int allocated;
} process;

//store the list of processes
process *processes[4];
//store the list of where the page tables are so we can actually find them
int *pageTableLocations;
//count the process
int processCount = 0;

int map(unsigned char pid, unsigned char vaddr, unsigned char val);
int store(unsigned char pid, unsigned char vaddr, unsigned char val);
int load(unsigned char pid, unsigned char vaddr, unsigned char val);

void init()
{
    for (int i = 0; i < 4; i++)
    {
        if (processes[i] == NULL)
        {
            process *proc = (process *)malloc(sizeof(process));
            proc->pid = -1;
            proc->address = 0;
            proc->allocated = 0;
            processes[i] = proc;
            pfnList[i] = -1;
        }
    }
}

/**
 * this claims a physical page as the processes only if free
 * if none are free, return -1 for now (until swapping)
 */
int locatePFN(unsigned char vpn)
{
	//return vpn+1;
    for (int i = 1; i < 4; i++)
    {
        if (pfnList[i] == -1)
        {
            pfnList[i] = 1;
            return i;
        }
    }
    return -1;
}

int locatePFN(unsigned char vpn) {
    return vpn+1;
}

int addPTE(unsigned char vpn, unsigned char pfn, unsigned char prot)
{
    if (processCount == 1)
    {
        for (int i = 0; i < 16; i = i + 4)
        {
            if (memory[i + 2] == 0)
            {
                memory[i] = vpn;
                memory[i + 1] = pfn;
                memory[i + 2] = 1;
                memory[i + 3] = prot;
                return 0;
            }
        }
    }
    else if (processCount == 2)
    {
        for (int i = 32; i < 48; i = i + 4)
        {
            if (memory[i + 2] == 0)
            {
                memory[i] = vpn;
                memory[i + 1] = pfn;
                memory[i + 2] = 1;
                memory[i + 3] = prot;
                return 0;
            }
        }
    }
    return -1;
}

int findVPN(unsigned char vpn,int offset, unsigned char value)
{
    int paddr = 0;
    unsigned char pfn = 0;
    if (processCount == 1)
    {
        for (int i = 0; i < 16; i = i + 4)
        {
            if (memory[i] == vpn && memory[i + 3] == 1) // if vpn is there and is writable
            {
                pfn = memory[i + 1];
                paddr = pfn*16+offset;
                memory[paddr] = value;
                return paddr;
            }
        }
    }
    else if (processCount == 2)
    {
        for (int i = 32; i < 48; i = i + 4)
        {
            if (memory[i] == vpn && memory[i + 3] == 1)
            {
                pfn = memory[i + 1];
                paddr = pfn + 48 + offset;
                memory[paddr] = value;
                return paddr;
            }
        }
    }
    return -1;
}

/**
 * helper function to determine if a process exists, returns the index of processes[4] if exists -1 if not
 */
int checkProcessExists(unsigned char pid) {
	for (int i = 0; i < 4; i++) //for each processes stored in our array
    {
        if(processes[i]->pid == pid) {
			//if the process already exists
            return i;
        }
	}
	return -1;
}
/**
 * helper function to locate the Page Table
 * will return the physical page number of the page table [0, 3] or -1 if lookup failed
 *
 */


int map(unsigned char pid, unsigned char vaddr, unsigned char val)
{
    unsigned char vpn = 0;
    unsigned char pfn = 0;
    unsigned char prot = val;
    unsigned char v = vaddr;
    int validProcess = 0;
    printf("Finding valid process\n");
	int processExists = checkProcessExists(pid); //run helper function to see if this process exists
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
	
	if (processExists == -1) { //Case 1 if the process does NOT exist, store it as a new entry
		
	
	}
	
	else { //else the process exists already; check if it needs a new mapping
	
	
	}
	
    for (int i = 0; i < 4; i++) //for each page
    {

        if (processes[i]->allocated == 0 && processes[i]->pid != pid) //if process is not allocated and not same process
        {
			printf("WE SHOULD NEVER HIT HERE TWICE\n");
            processes[i]->pid = pid;
            processes[i]->address = vaddr;
            processes[i]->allocated = 1;
            processCount++;
            break;
        }
        else
        {
            validProcess++;
        }
    }
    if (validProcess == 4)
    {
        return -1;
    }
    while (vaddr > 16)
    {
        vaddr = vaddr - 16;
        vpn++;
    }

    if (vaddr == 16)
    {
        vpn++;
    }
    printf("Locating valid PFN\n");
    pfn = locatePFN(vpn);
    int added = addPTE(vpn, pfn, prot);

    if (added == 0)
    {
        printf("Put page table for PID %d into physical frame %d\n", pid, pfn);
        printf("Mapped virtual address %d (page %d) into physical frame %d\n", v, vpn, pfn);
    }
    else
    {
        return -1;
    }

    return 0;
}

int findOffset(unsigned char vaddr) {
    if(vaddr < 16) {
        return vaddr;
    } else if(vaddr >= 16 && vaddr < 32) {
        return vaddr-16;
    } else if(vaddr >= 32 && vaddr < 48) {
        return vaddr-32;
    } else {
        return vaddr-48;
    }
}

int store(unsigned char pid, unsigned char vaddr, unsigned char val)
{
    unsigned char vpn = 0;
    unsigned char v = vaddr;
    int offset = findOffset(v);
    while (vaddr > 16)
    {
        vaddr = vaddr - 16;
        vpn++;
    }

    if (vaddr == 16)
    {
        vpn++;
    }
    int stored = findVPN(vpn,offset,val);
    if (stored > -1)
    {
        printf("Stored value %d at virtual address %d (physical address %d)\n", val, v, stored);
    }
    else
    {
        return -1;
    }
}

int load(unsigned char pid, unsigned char vaddr, unsigned char val)
{
	int tempPage = vaddr/16;
	int tempOffset = vaddr%16;
	int calculatedPhysAddr = (1+tempPage)*16+tempOffset; //currently just assumes these are in order
	int returnVal = memory[calculatedPhysAddr];
	printf("The value %d is virtual address %d (physical address %d)\n", returnVal, vaddr, calculatedPhysAddr);
	return returnVal;
}


int main()
{
unsigned char pid, choice = -1, vaddr, val = -1;
    char str[20];
    init();
    while (1)
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
            map(pid, vaddr, val);
            //print out
            break;
        case 1:
            store(pid, vaddr, val);
            //print out
            break;
        case 2:
            load(pid, vaddr, val);
            break;
        // case 3:
        //     break;
        default:
            printf("Error input was not proper try again\n");
        }
        /*for(int i =0; i < MAXMEM;i++){
			if(i%16==0 && i>0){
				printf("| ");
			}
			printf("%d ",mem[i]);
		}*/
        printf("\n");
    }
    return 0;
}