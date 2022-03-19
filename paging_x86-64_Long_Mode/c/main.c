#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


// declarations
typedef struct {
    long unsigned paddr;
    long unsigned value;
} PhysicMemoryItem;
typedef struct {
    unsigned offset        : 12;
    unsigned table         : 9;
    unsigned directory     : 9;
    unsigned directory_ptr : 9;
    unsigned pml4          : 9;
    unsigned space         : 16;
} SegmentDescriptorProtectedMode;


// data
char *input_file_path = "test_dataset.txt";
//char *input_file_path = "dataset_44327_15.txt";
//char *input_file_path = "new_dataset.txt";

char *output_file_path = "output.txt";
FILE *fp_input, *fp_output;
PhysicMemoryItem *phisic_mem;
SegmentDescriptorProtectedMode *logical_addrs;
unsigned long i, m, q, r;



unsigned long alignLittle12bit(unsigned long number) {
    return (number >> 12) << 12;
}

// ret value or -1
unsigned long findPhysicValueByAddr(unsigned long addr) {
    unsigned long findValue = 0;
    for(int i = 0; i < m; i++) {
        if((addr) == (phisic_mem[i].paddr)) {
            findValue = (phisic_mem[i].value);
            break;
        }
    }
    return findValue;
}


void request(SegmentDescriptorProtectedMode descriptor) {
    //printf("Logical addr: %lu\n", descriptor);
    printf("space=%d|", descriptor.space);
    printf("PML4=%d|", descriptor.pml4);
    printf("Directory Ptr=%d|", descriptor.directory_ptr);
    printf("Directory=%d|", descriptor.directory);
    printf("Table=%d|", descriptor.table);
    printf("Offset=%d\n", descriptor.offset);


    unsigned long pml4e = (r + 8 * descriptor.pml4);
    unsigned long lvl3_root = findPhysicValueByAddr(pml4e);
    if(lvl3_root == 0 || (lvl3_root & 1) == 0) {
        fprintf(fp_output, "fault\n");
        printf("fault\n\n");
        return;
    }
    lvl3_root = alignLittle12bit(lvl3_root);
    printf("Finded lvl3 value = %lu\n", lvl3_root);


    unsigned long pdpte = (lvl3_root + 8 * descriptor.directory_ptr);
    unsigned long lvl2_root = findPhysicValueByAddr(pdpte);
    if(lvl2_root == 0 || (lvl2_root & 1) == 0) {
        fprintf(fp_output, "fault\n");
        printf("fault\n\n");
        return;
    }
    lvl2_root = alignLittle12bit(lvl2_root);
    printf("Finded lvl2 value = %lu\n", lvl2_root);


    unsigned long pde = (lvl2_root + 8 * descriptor.directory);
    unsigned long lvl1_root = findPhysicValueByAddr(pde);
    if(lvl1_root == 0 || (lvl1_root & 1) == 0) {
        fprintf(fp_output, "fault\n");
        printf("fault\n\n");
        return;
    }
    lvl1_root = alignLittle12bit(lvl1_root);
    printf("Finded lvl1 value = %lu\n", lvl1_root);


    unsigned long pte = (lvl1_root + 8 * descriptor.table);
    unsigned long lvl0_root = findPhysicValueByAddr(pte);
    if(lvl0_root == 0 || (lvl0_root & 1) == 0) {
        fprintf(fp_output, "fault\n");
        printf("fault\n\n");
        return;
    }
    lvl0_root = alignLittle12bit(lvl0_root);
    printf("Finded lvl0 value = %lu\n", lvl0_root);


    unsigned long physic_addr = (lvl0_root + descriptor.offset);
    fprintf(fp_output, "%lu\n", physic_addr);
    printf("Final addres=%lu\n\n", physic_addr);
}


int main(void) {
    // open and read first line
    fp_input = fopen(input_file_path, "r");
    fp_output = fopen(output_file_path, "w");
    fscanf(fp_input, "%lu %lu %lu\n", &m, &q, &r);

    // allocation mem for dinamics array
    phisic_mem = malloc(m * sizeof(PhysicMemoryItem));
    logical_addrs = malloc(q * sizeof(SegmentDescriptorProtectedMode));

    // read other data
    for(i = 0; i < m; i++)
        fscanf(fp_input, "%lu %lu\n", &phisic_mem[i].paddr, &phisic_mem[i].value);
    for(i = 0; i < q; i++)
        fscanf(fp_input, "%lu\n", (long unsigned *)(&logical_addrs[i]));

    // proccessing requests
    for(i = 0; i < q; i++)
        request((SegmentDescriptorProtectedMode)logical_addrs[i]);

    return 0;
}
