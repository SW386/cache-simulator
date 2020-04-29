#include <stdio.h>
#include <stdlib.h>

int log2(int n);
int bitRemover(int n);

int main(int argc, char* argv[]) {
    // Get inputs
    FILE* pageTable = fopen(argv[1], "r");
    char* virtualAddress = argv[2];

    // Get address size and page size
    int addressBits, pageSize;
    fscanf(pageTable, "%d %d", &addressBits, &pageSize);

    // Convert virtual address to integer
    int vAddressInt;
    sscanf(virtualAddress, "%x", &vAddressInt);

    // Get virtual page number from virtual address
    int offsetSize = log2(pageSize);
    int vPageNumber = vAddressInt >> offsetSize;
    
    // Get offset from virtual address
    int temp = bitRemover(offsetSize);
    int offset = temp&vAddressInt; 

    // Get physical address from page number
    int pPageNumber;
    int currPage = 0;
    while(fscanf(pageTable, "%d",&pPageNumber) != EOF) {
        if (currPage == vPageNumber) {
            if(pPageNumber == -1) {
                printf("PAGEFAULT\n");
            } else {
                int physicalAddress = (pPageNumber << offsetSize) + offset;
                printf("%x\n", physicalAddress);
                break;
            }
        }
        currPage += 1;
    }
    fclose(pageTable);
    return EXIT_SUCCESS;
}

//Performs simple logarithms to base 2
int log2(int n) {
    int r=0;
    while (n>>=1) r++;
    return r;
}

//Creates a value to preserve the offset bits but remove the virtual page number
int bitRemover(int n) {
    int start = 1;
    int ret = 0;
    for (int i = 0; i < n; i++) {
        ret += start;
        start = start*2;
    }
    return ret;
}