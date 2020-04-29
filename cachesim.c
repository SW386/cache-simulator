#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MEM_SIZE 16777216
unsigned char physicalMemory[MEM_SIZE];

//Struct for a block
typedef struct Block {
    int tag;
	int useNum; //Number of Set access where a block isn't used
	int valid;
    struct Block *next; 
} Block;

Block** cache;
void insert(int index, int tag);
int cacheLookup(int index, int tag);
void breakAddress(int* store, int address, int cacheSize, int associativity, int blockSize);
void deallocCache(int cacheSize, int associativity, int blockSize);
void initCache (int cacheSize, int associativity, int blockSize);
int log2(int n);
int bitRemover(int n);


int main (int argc, char* argv[]) {
	int cacheSize, associativity, blockSize;

    // Buffer to store instruction (i.e. "load" or "store")
	char instruction_buffer[6];

    // Open the trace file in read mode
	FILE* myFile = fopen(argv[1], "r");

    // Read in the command line arguments
	sscanf(argv[2], "%d", &cacheSize);
	sscanf(argv[3], "%d", &associativity);
	sscanf(argv[4], "%d", &blockSize);

	initCache(cacheSize, associativity, blockSize);

    // Keep reading the instruction until end of file
	while(fscanf(myFile,"%s", &instruction_buffer)!=EOF) {
		int currAddress, accessSize;

        // Read the address and access size info
		fscanf(myFile, "%x", &currAddress);
		fscanf(myFile, "%d", &accessSize);

		int address_component[3];
		breakAddress(address_component, currAddress, cacheSize, associativity, blockSize);
		int index = address_component[1];
		int tag = address_component[2];
		int hit = cacheLookup(index,tag);
		
		if (instruction_buffer[0]=='l'){    // If load
			//Print if it is a hit or miss
			if (hit == 1) {
				printf("load 0x%x hit ", currAddress);
			} else {
				insert(index, tag);
				printf("load 0x%x miss ", currAddress);
			}
			for(int i = 0; i < accessSize; i++) {
				printf("%02hhx", physicalMemory[currAddress+i]);
			}
			printf("\n");
		}
        else {                              // Else store
			// Store the data into memory
			for(int i = 0; i < accessSize; i ++){
    			fscanf(myFile, "%2hhx", &physicalMemory[currAddress+i]);
			} 		

			// Print if it is a hit or miss
			if (hit == 1) {
				printf("store 0x%x hit\n", currAddress);
			} else {
				insert(index, tag);
				printf("store 0x%x miss\n", currAddress);
			}
		}
	}

	deallocCache(cacheSize, associativity, blockSize);
	return EXIT_SUCCESS;
}

void insert(int index, int tag) {
	Block* curr = cache[index];
	int lru = -1;
	int lruTag = 0;
	int insertSuccess = 0;

	//Insert if empty and get the least recently used block
	while (curr != NULL) {
		//Insert data the valid bit is 0
		if (curr->valid == 0 && insertSuccess == 0) {
			curr->tag = tag;
			curr->useNum = 0;
			curr->valid = 1;
			insertSuccess = 1;
		} else  if (curr->valid == 1) {
			curr->useNum += 1;
		}

		//Find the id of the of the least recently used;
		if (curr->useNum > lru) {
			lru = curr->useNum;
			lruTag = curr->tag;
		}

		curr = curr->next;
	}

	//Insert if there is no empty block
	if(insertSuccess == 0) {
		curr = cache[index];
		while (curr != NULL) {
			if(curr->tag == lruTag) {
				curr->tag = tag;
				curr->useNum = 0;
				curr->valid = 1;
			}
			curr = curr->next;
		}	
	}
}

//Return the value of lookup and hit or miss
int cacheLookup(int index, int tag) {
	Block* curr = cache[index];
	int hit = 0;
	while (curr != NULL) {
		if (curr->tag == tag) {
			curr->useNum = 0;
			hit = 1;
		} else {
			if (curr->valid == 1) {
				curr->useNum += 1;
			}
		}
		curr = curr->next;
	}
	return hit;
}

//Calculates the Block Offset, Index, and Tag
void breakAddress(int* store, int address, int cacheSize, int associativity, int blockSize) {
	// Get Block Offset
	int offsetSize = log2(blockSize);
	int temp = bitRemover(offsetSize);
	int offset = address&temp;
	store[0] = offset;
	
	//Get Index
	int sets = cacheSize*1024/(associativity*blockSize);
	int indexSize = log2(sets);
	int tempAddress = address >> offsetSize;
	temp = bitRemover(indexSize);
	int index = tempAddress&temp;
	store[1] = index;

	//Get Tag
	int tag = address >> (offsetSize + indexSize);
	store[2] = tag;
}

//Deallocates the cache, removing all blocks
void deallocCache(int cacheSize, int associativity, int blockSize) {
	int sets = cacheSize*1024/(associativity*blockSize);
	for (int i = 0; i < sets; i++) {
		Block* curr = cache[i];
		Block* temp;
		while(curr != NULL) {
			temp = curr;
			curr = curr->next;
			free(temp);
		}
	}
}

//Initializes the cache, calculating rows and creating an array of linked lists. 
void initCache (int cacheSize, int associativity, int blockSize) {
	int sets = cacheSize*1024/(associativity*blockSize);
	cache = (Block**)malloc(sets*sizeof(Block*));
	Block* temp;
	for (int i = 0; i < sets; i++) {
		for(int j = 0; j < associativity; j++) {
			Block* block = (Block*) malloc(sizeof(Block));
			block->tag = -1;
			block->useNum = 0;
			block->valid = 0;
			if(j == 0) {
				cache[i] = block;
				temp = block;
			} else {
				temp->next = block;
				temp = temp->next;
			}
		}
	}
}

//Performs simple logarithms to base 2
int log2(int n) {
    int r=0;
    while (n>>=1) r++;
    return r;
}

//Creates a value to preserve the last n bits
int bitRemover(int n) {
    int start = 1;
    int ret = 0;
    for (int i = 0; i < n; i++) {
        ret += start;
        start = start*2;
    }
    return ret;
}
