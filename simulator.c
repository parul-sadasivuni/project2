#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

int main (int argc, char** argv) {

    //union for floats
    union Float {
        float flt;
        int32_t sign;
    };

    union Double {
        double dbl;
        int64_t sign;
    };

    //memory array with max size of 512*1024
    uint32_t memSize = 0x80000;
    int8_t memory[memSize];

    //validating input file
    char fileName[strlen(argv[1]) + 2];
    strcpy(fileName, argv[1]);
    if(fileName[strlen(fileName) - 4] != 's' || fileName[strlen(fileName) - 3] != 'l' || fileName[strlen(fileName) - 2] != 'k' || fileName[strlen(fileName) - 1] != 'o') {
        fprintf(stderr, "Invalid slinker filepath\n");
        exit(1);
    }

    FILE* binary = fopen(argv[1], "r");
    if(binary == NULL) {
        fprintf(stderr, "Invalid slinker filepath\n");
        exit(1);
    }


    //figure out data and instruction offsets
    uint8_t offset[4];
    fread(offset, sizeof(uint8_t), 4, binary);
    uint32_t instrOff = ((uint8_t)offset[0] << 24) | ((uint8_t)offset[1] << 16) | ((uint8_t)offset[2] << 8) | ((uint8_t)offset[3]);

    fread(offset, sizeof(uint8_t), 4, binary);
    uint32_t dataOff = ((uint8_t)offset[0] << 24) | ((uint8_t)offset[1] << 16) | ((uint8_t)offset[2] << 8) | ((uint8_t)offset[3]);

    //put instructions into memory array, put the data into the memory array starting at the heap
    int index = 8;
    int i = 0;
    int data = 0x10000;
    uint8_t byte;
    long temp;
    char input[100];
    int8_t out;
    while (fread(&byte, sizeof(uint8_t), 1, binary) == 1) {
        if(index >= 8 && index < dataOff) {
            //these are the instructions, so saving to memory array
            memory[i] = byte;
            i++;
        }
        else {
            //data, so saving starting at heap index
            memory[data] = byte;
            data++;
        }
        index++;
    }

    int pc = 0; //indicates instruction
    int sp = 0x40000; //indicates position in memory array
    uint32_t address;
    int8_t tmp;
    int8_t items;
    //iterate through memory until end of instructions
    bool halt = false;
    uint8_t currentByte;
    while(pc < (dataOff-instrOff) && !halt) {
        currentByte = (uint8_t) memory[pc];
        //printf("currentByte: %d ", currentByte);
        //switch case based on the opcode
        switch (currentByte) {
        case 0: //pushb value 00
            memory[sp] = memory[pc + 1];
            sp += 1;
            pc += 2;
            break;
        case 1: //pushs value 01
            memory[sp] = memory[pc + 1];
            memory[sp + 1] = memory[pc + 2];
            sp += 2;
            pc += 3;
            break;
        case 2: //pushi value 02
            memory[sp] = memory[pc + 1];
            memory[sp + 1] = memory[pc + 2];
            memory[sp + 2] = memory[pc + 3];
            memory[sp + 3] = memory[pc + 4];
            sp += 4;
            pc += 5;
            //printf("called pushi pc: %d; ", pc);
            break;
        case 3: //pushl value 03
            memory[sp] = memory[pc + 1];
            memory[sp + 1] = memory[pc + 2];
            memory[sp + 2] = memory[pc + 3];
            memory[sp + 3] = memory[pc + 4];
            memory[sp + 4] = memory[pc + 5];
            memory[sp + 5] = memory[pc + 6];
            memory[sp + 6] = memory[pc + 7];
            memory[sp + 7] = memory[pc + 8];
            sp += 8;
            pc += 9;
            break;
        case 4: //pushf value 04
            memory[sp] = memory[pc + 1];
            memory[sp + 1] = memory[pc + 2];
            memory[sp + 2] = memory[pc + 3];
            memory[sp + 3] = memory[pc + 4];
            sp += 4;
            pc += 5;
            break;
        case 5: //pushd value 05
            memory[sp] = memory[pc + 1];
            memory[sp + 1] = memory[pc + 2];
            memory[sp + 2] = memory[pc + 3];
            memory[sp + 3] = memory[pc + 4];
            memory[sp + 4] = memory[pc + 5];
            memory[sp + 5] = memory[pc + 6];
            memory[sp + 6] = memory[pc + 7];
            memory[sp + 7] = memory[pc + 8];
            sp += 8;
            pc += 9;
            break;
        case 6: //pushbm address 06
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            memory[sp] = memory[address];
            sp += 1;
            pc += 4;
            break;
        case 7: //pushsm address 07
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            memory[sp] = memory[address];
            memory[sp + 1] = memory[address + 1]; 
            sp += 2;
            pc += 4;
            break;
        case 8: //pushim address 08
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            memory[sp] = memory[address];
            memory[sp + 1] = memory[address + 1];
            memory[sp + 2] = memory[address + 2];
            memory[sp + 3] = memory[address + 3];
            sp += 4;
            pc += 4;
            break;
        case 9: //pushlm address 09
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            memory[sp] = memory[address];
            memory[sp + 1] = memory[address + 1];
            memory[sp + 2] = memory[address + 2];
            memory[sp + 3] = memory[address + 3];
            memory[sp + 4] = memory[address + 4];
            memory[sp + 5] = memory[address + 5];
            memory[sp + 6] = memory[address + 6];
            memory[sp + 7] = memory[address + 7];
            sp += 8;
            pc += 4;
            break;
        case 10: //pushfm address 0a
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            memory[sp] = memory[address];
            memory[sp + 1] = memory[address + 1];
            memory[sp + 2] = memory[address + 2];
            memory[sp + 3] = memory[address + 3];
            sp += 4;
            pc += 4;
            break;
        case 11: {//pushdm 0b
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            memory[sp] = memory[address];
            memory[sp + 1] = memory[address + 1];
            memory[sp + 2] = memory[address + 2];
            memory[sp + 3] = memory[address + 3];
            memory[sp + 4] = memory[address + 4];
            memory[sp + 5] = memory[address + 5];
            memory[sp + 6] = memory[address + 6];
            memory[sp + 7] = memory[address + 7];
            sp += 8;
            pc += 4;
            break;
        }
        case 12: {//pushmm address items 0c
            items = memory[pc + 4];
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            for(int i = 0; i < items; i++) {
                memory[sp + i] = memory[address + i];
            }
            sp += items;
            pc += 5;
            break;
        }
        case 13: //dupb 0d
            memory[sp] = memory[sp - 1];
            sp += 1;
            pc += 1;
            break;
        case 14: //dups 0e
            memory[sp] = memory[sp - 2];
            memory[sp + 1] = memory[sp - 1];
            sp += 2;
            pc += 1;
            break;
        case 15: //dupi 0f
            memory[sp] = memory[sp - 4];
            // printf("memory %d: %d ", sp, memory[sp]);
            memory[sp + 1] = memory[sp - 3];
            // printf("memory %d: %d ", (sp + 1), memory[sp + 1]);
            memory[sp + 2] = memory[sp - 2];
            // printf("memory %d: %d ", (sp + 2), memory[sp + 2]);
            memory[sp + 3] = memory[sp - 1];
            // printf("memory %d: %d\n", (sp + 3), memory[sp + 3]);
            sp += 4;
            pc += 1;
            break;
        case 16: //dupl 10
            memory[sp] = memory[sp - 8];
            memory[sp + 1] = memory[sp - 7];
            memory[sp + 2] = memory[sp - 6];
            memory[sp + 3] = memory[sp - 5];
            memory[sp + 4] = memory[sp - 4];
            memory[sp + 5] = memory[sp - 3];
            memory[sp + 6] = memory[sp - 2];
            memory[sp + 7] = memory[sp - 1];
            sp += 8;
            pc += 1;
            break;
        case 17: //dupf 11
            memory[sp] = memory[sp - 4];
            memory[sp + 1] = memory[sp - 3];
            memory[sp + 2] = memory[sp - 2];
            memory[sp + 3] = memory[sp - 1];
            sp += 4;
            pc += 1;
            break;
        case 18: //dupd 12
            memory[sp] = memory[sp - 8];
            memory[sp + 1] = memory[sp - 7];
            memory[sp + 2] = memory[sp - 6];
            memory[sp + 3] = memory[sp - 5];
            memory[sp + 4] = memory[sp - 4];
            memory[sp + 5] = memory[sp - 3];
            memory[sp + 6] = memory[sp - 2];
            memory[sp + 7] = memory[sp - 1];
            sp += 8;
            pc += 1;
            break;
        case 19: //popb 13
            sp -= 1;
            pc += 1;
            break;
        case 20: //pops 14
            sp -= 2;
            pc += 1;
            break;
        case 21: //popi 15
            sp -= 4;
            pc += 1;
            break;
        case 22: //popl 16
            sp -= 8;
            pc += 1;
            break;
        case 23: //popf 17
            sp -= 4;
            pc += 1;
            break;
        case 24: //popd 18
            sp -= 8;
            pc += 1;
            break;
        case 25: //popbm address 19
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            memory[address] = memory[sp - 1]; 
            sp -= 1;
            pc += 4;
            break;
        case 26: //popsm address 1a
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            sp -= 2;
            memory[address] = memory[sp]; 
            memory[address + 1] = memory[sp + 1]; 
            pc += 4;
            break;
        case 27: //popim address
            sp -= 4;
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            memory[address] = memory[sp]; 
            memory[address + 1] = memory[sp + 1];
            memory[address + 2] = memory[sp + 2];
            memory[address + 3] = memory[sp + 3];
            pc += 4;
            break;
        case 28: //poplm address
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            sp -= 8;
            memory[address] = memory[sp]; 
            memory[address + 1] = memory[sp + 1];
            memory[address + 2] = memory[sp + 2];
            memory[address + 3] = memory[sp + 3];
            memory[address + 4] = memory[sp + 4]; 
            memory[address + 5] = memory[sp + 5];
            memory[address + 6] = memory[sp + 6];
            memory[address + 7] = memory[sp + 7];
            pc += 4;
            break;
        case 29: //popfm address 1d
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            sp -= 4;
            memory[address] = memory[sp]; 
            memory[address + 1] = memory[sp + 1];
            memory[address + 2] = memory[sp + 2];
            memory[address + 3] = memory[sp + 3];
            pc += 4;
            break;
        case 30: //popdm address 1e
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            sp -= 8;
            memory[address] = memory[sp]; 
            memory[address + 1] = memory[sp + 1];
            memory[address + 2] = memory[sp + 2];
            memory[address + 3] = memory[sp + 3];
            memory[address + 4] = memory[sp + 4]; 
            memory[address + 5] = memory[sp + 5];
            memory[address + 6] = memory[sp + 6];
            memory[address + 7] = memory[sp + 7];
            pc += 4;
            break;
        case 31: {//popmm address items 1f
            items = memory[pc + 4];
            sp -= items;
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            for(int i = 0; i < items; i++) {
                memory[address + i] = memory[sp + i];
            }
            pc += 5;
            break;
        }
        case 32: {//swapb 20
            tmp = memory[sp - 1];
            memory[sp - 1] = memory[sp - 2];
            memory[sp - 2] = tmp;
            pc += 1;
            break;
        }
        case 33: {//swaps 21 
            tmp = memory[sp - 2];
            memory[sp - 2] = memory[sp - 4];
            memory[sp - 4] = tmp;

            tmp = memory[sp - 1];
            memory[sp - 1] = memory[sp - 3];
            memory[sp - 3] = tmp;
            pc += 1;
            break;
        }
        case 34: {//swapi 22
            tmp = memory[sp - 4];
            memory[sp - 4] = memory[sp - 8];
            memory[sp - 8] = tmp;

            tmp = memory[sp - 3];
            memory[sp - 3] = memory[sp - 7];
            memory[sp - 7] = tmp;

            tmp = memory[sp - 2];
            memory[sp - 6] = memory[sp - 6];
            memory[sp - 6] = tmp;

            tmp = memory[sp - 1];
            memory[sp - 1] = memory[sp - 5];
            memory[sp - 1] = tmp;
            pc += 1;
            break;
        }
        case 35: {//swapl 23
            tmp = memory[sp - 8];
            memory[sp - 8] = memory[sp - 16];
            memory[sp - 16] = tmp;

            tmp = memory[sp - 7];
            memory[sp - 7] = memory[sp - 15];
            memory[sp - 15] = tmp;

            tmp = memory[sp - 6];
            memory[sp - 6] = memory[sp - 14];
            memory[sp - 14] = tmp;

            tmp = memory[sp - 5];
            memory[sp - 5] = memory[sp - 13];
            memory[sp - 13] = tmp;

            tmp = memory[sp - 4];
            memory[sp - 4] = memory[sp - 12];
            memory[sp - 12] = tmp;

            tmp = memory[sp - 3];
            memory[sp - 3] = memory[sp - 11];
            memory[sp - 11] = tmp;

            tmp = memory[sp - 2];
            memory[sp - 2] = memory[sp - 10];
            memory[sp - 10] = tmp;

            tmp = memory[sp - 1];
            memory[sp - 1] = memory[sp - 9];
            memory[sp - 9] = tmp;
            pc += 1;
            break;
        }
        case 36: //swapf 24
            tmp = memory[sp - 4];
            memory[sp - 4] = memory[sp - 8];
            memory[sp - 8] = tmp;

            tmp = memory[sp - 3];
            memory[sp - 3] = memory[sp - 7];
            memory[sp - 7] = tmp;

            tmp = memory[sp - 2];
            memory[sp - 6] = memory[sp - 6];
            memory[sp - 6] = tmp;

            tmp = memory[sp - 1];
            memory[sp - 1] = memory[sp - 5];
            memory[sp - 1] = tmp;
            pc += 1;
            break;
        case 37: //swapd 25
            tmp = memory[sp - 8];
            memory[sp - 8] = memory[sp - 16];
            memory[sp - 16] = tmp;

            tmp = memory[sp - 7];
            memory[sp - 7] = memory[sp - 15];
            memory[sp - 15] = tmp;

            tmp = memory[sp - 6];
            memory[sp - 6] = memory[sp - 14];
            memory[sp - 14] = tmp;

            tmp = memory[sp - 5];
            memory[sp - 5] = memory[sp - 13];
            memory[sp - 13] = tmp;

            tmp = memory[sp - 4];
            memory[sp - 4] = memory[sp - 12];
            memory[sp - 12] = tmp;

            tmp = memory[sp - 3];
            memory[sp - 3] = memory[sp - 11];
            memory[sp - 11] = tmp;

            tmp = memory[sp - 2];
            memory[sp - 2] = memory[sp - 10];
            memory[sp - 10] = tmp;

            tmp = memory[sp - 1];
            memory[sp - 1] = memory[sp - 9];
            memory[sp - 9] = tmp;
            pc += 1;
            break;
        case 38: {//convbs 26
            int8_t bs = memory[sp - 1];
            memory[sp - 1] = (bs >> 8) & 0xFF;
            memory[sp] = bs & 0xFF;
            sp += 1;
            pc += 1;
            break;
        }
        case 39: {//convbi 27
            int8_t bi = memory[sp - 1];
            int bii = (int) bi;
            //uint32_t biiu = memcpy(&biiu, &bii, sizeof(int));
            memory[sp - 1] = (bii >> 24) & 0xFF;
            memory[sp] = (bii >> 16) & 0xFF;
            memory[sp + 1] = (bii >> 8) & 0xFF;
            memory[sp + 2] = (bii) & 0xFF;
            sp += 3;
            pc += 1;
            break;
        }
        case 40: {//convbl 28
            int8_t bl = memory[sp - 1];
            int64_t bll = (int64_t) bl & 0xFF;
            // int64_t bllu;
            // memcpy(&bllu, &bll, sizeof(int64_t));
            memory[sp - 1] = (bll >> 56) & 0xFF;
            memory[sp] = (bll >> 48) & 0xFF;
            memory[sp + 1] = (bll >> 40) & 0xFF;
            memory[sp + 2] = (bll >> 32) & 0xFF;
            memory[sp + 3] = (bll >> 24) & 0xFF;
            memory[sp + 4] = (bll >> 16) & 0xFF;
            memory[sp + 5] = (bll >> 8) & 0xFF;
            memory[sp + 6] = (bll) & 0xFF;
            sp += 7;
            pc += 1;
            break;
        }
        case 41: {//convbf 29
            int8_t bf = memory[sp - 1];
            float bff = (float) bf;
            int32_t bffu;
            memcpy(&bffu, &bff, sizeof(float));
            memory[sp - 1] = (bffu >> 24) & 0xFF;
            memory[sp] = (bffu >> 16) & 0xFF;
            memory[sp + 1] = (bffu >> 8) & 0xFF;
            memory[sp + 2] = (bffu) & 0xFF;
            sp += 3;
            pc += 1;
            break;
        }
        case 42: {//convbd 2a
            int8_t bd = memory[sp - 1];
            double bdd = (double) bd;
            int64_t bddu;
            memcpy(&bddu, &bdd, sizeof(double));
            memory[sp - 1] = (bddu >> 56) & 0xFF;
            memory[sp] = (bddu >> 48) & 0xFF;
            memory[sp + 1] = (bddu >> 40) & 0xFF;
            memory[sp + 2] = (bddu >> 32) & 0xFF;
            memory[sp + 3] = (bddu >> 24) & 0xFF;
            memory[sp + 4] = (bddu >> 16) & 0xFF;
            memory[sp + 5] = (bddu >> 8) & 0xFF;
            memory[sp + 6] = (bddu) & 0xFF;
            sp += 7;
            pc += 1;
            break;
            memory[sp - 1] = (double)memory[sp - 1];
            sp += 7;
            pc += 1;
            break;
        }
        case 43: {//convsb 2b
            short sb = ((short)memory[sp - 2] << 8 | (short)memory[sp - 1]);
            memory[sp - 2] = (int8_t)sb;
            sp -= 1;
            pc += 1;
            break;
        }
        case 44: {//convsi 2c
            short si = ((short)memory[sp - 2] << 8 | (short)memory[sp - 1]);
            int32_t sii = (int32_t) si;
            memory[sp - 2] = (sii >> 24) & 0xFF;
            memory[sp - 1] = (sii >> 16) & 0xFF;
            memory[sp] = (sii >> 8) & 0xFF;
            memory[sp + 1] = (sii) & 0xFF;
            sp += 2;
            pc += 1;
            break;
        }
        case 45: {//convsl 2d
            short sl = ((short)memory[sp - 2] << 8 | (short)memory[sp - 1]);
            int64_t sll = (int64_t) sl & 0xFF;
            memory[sp - 2] = (sll >> 56) & 0xFF;
            memory[sp - 1] = (sll >> 48) & 0xFF;
            memory[sp] = (sll >> 40) & 0xFF;
            memory[sp + 1] = (sll >> 32) & 0xFF;
            memory[sp + 2] = (sll >> 24) & 0xFF;
            memory[sp + 3] = (sll >> 16) & 0xFF;
            memory[sp + 4] = (sll >> 8) & 0xFF;
            memory[sp + 5] = (sll) & 0xFF;
            sp += 6;
            pc += 1;
            break;
        }
        case 46: {//convsf 2e
            short sf = ((short)memory[sp - 2] << 8 | (short)memory[sp - 1]);
            float sff = (float) sf;
            int32_t sffu;
            memcpy(&sffu, &sff, sizeof(float));
            memory[sp - 2] = (sffu >> 24) & 0xFF;
            memory[sp - 1] = (sffu >> 16) & 0xFF;
            memory[sp] = (sffu >> 8) & 0xFF;
            memory[sp + 1] = (sffu) & 0xFF;
            sp += 2;
            pc += 1;
            break;
        }
        case 47: {//convsd 2f
            short sd = ((short)memory[sp - 2] << 8 | (short)memory[sp - 1]);
            double sdd = (double) sd;
            int64_t sddu;
            memcpy(&sddu, &sdd, sizeof(double));
            memory[sp - 2] = (sddu >> 56) & 0xFF;
            memory[sp - 1] = (sddu >> 48) & 0xFF;
            memory[sp] = (sddu >> 40) & 0xFF;
            memory[sp + 1] = (sddu >> 32) & 0xFF;
            memory[sp + 2] = (sddu >> 24) & 0xFF;
            memory[sp + 3] = (sddu >> 16) & 0xFF;
            memory[sp + 4] = (sddu >> 8) & 0xFF;
            memory[sp + 5] = (sddu) & 0xFF;
            sp += 6;
            pc += 1;
            break;
        }
        case 48: {//convib 30
            int ib = ((int)memory[sp - 4] << 24 | (int)memory[sp - 3] << 16 | (int)memory[sp - 2] << 8 | (int)memory[sp - 1]);
            memory[sp - 4] = (int8_t)ib;
            sp -= 3;
            pc += 1;
            break;
        }
        case 49: {//convis 31
            int is = ((int)memory[sp - 4] << 24 | (int)memory[sp - 3] << 16 | (int)memory[sp - 2] << 8 | (int)memory[sp - 1]);
            short iss = (short)is;
            memory[sp - 4] = (iss >> 8 & 0xFF);
            memory[sp - 3] = iss & 0xFF;
            sp -= 2;
            pc += 1;
            break;
        }
        case 50: {//convil 32
            int il = ((int)memory[sp - 4] << 24 | (int)memory[sp - 3] << 16 | (int)memory[sp - 2] << 8 | (int)memory[sp - 1]);
            int64_t ill = (int64_t) il;
            memory[sp - 4] = (ill >> 56) & 0xFF;
            memory[sp - 3] = (ill >> 48) & 0xFF;
            memory[sp - 2] = (ill >> 40) & 0xFF;
            memory[sp - 1] = (ill >> 32) & 0xFF;
            memory[sp] = (ill >> 24) & 0xFF;
            memory[sp + 1] = (ill >> 16) & 0xFF;
            memory[sp + 2] = (ill >> 8) & 0xFF;
            memory[sp + 3] = (ill) & 0xFF;
            sp += 4;
            pc += 1;
            break;
        }
        case 51: {//convif 33
            int iph = ((int)memory[sp - 4] << 24 | (int)memory[sp - 3] << 16 | (int)memory[sp - 2] << 8 | (int)memory[sp - 1]);
            float iff = (float) iph;
            int32_t iffu;
            memcpy(&iffu, &iff, sizeof(float));
            memory[sp - 4] = (iffu >> 24) & 0xFF;
            memory[sp - 3] = (iffu >> 16) & 0xFF;
            memory[sp - 2] = (iffu >> 8) & 0xFF;
            memory[sp - 1] = (iffu) & 0xFF;
            pc += 1;
            break;
        }
        case 52: {//convid 34
            int id = ((int)memory[sp - 4] << 24 | (int)memory[sp - 3] << 16 | (int)memory[sp - 2] << 8 | (int)memory[sp - 1]);
            double idd = (double) id;
            int64_t iddu;
            memcpy(&iddu, &idd, sizeof(double));
            memory[sp - 4] = (iddu >> 56) & 0xFF;
            memory[sp - 3] = (iddu >> 48) & 0xFF;
            memory[sp - 2] = (iddu >> 40) & 0xFF;
            memory[sp - 1] = (iddu >> 32) & 0xFF;
            memory[sp] = (iddu >> 24) & 0xFF;
            memory[sp + 1] = (iddu >> 16) & 0xFF;
            memory[sp + 2] = (iddu >> 8) & 0xFF;
            memory[sp + 3] = (iddu) & 0xFF;
            sp += 4;
            pc += 1;
            break;
        }
        case 53: {//convlb 35
            int64_t lb = ((int64_t)(uint8_t)memory[sp - 8] << 56 | (int64_t)(uint8_t)memory[sp - 7] << 48 | (int64_t)(uint8_t)memory[sp - 6] << 40 | (int64_t)(uint8_t)(uint8_t)memory[sp + 3] << 32 | (int64_t)(uint8_t)(uint8_t)memory[sp - 5] << 24 | (int64_t)(uint8_t)(uint8_t)memory[sp - 4] << 16 | (int64_t)(uint8_t)(uint8_t)memory[sp - 3] << 8 | (int64_t)(uint8_t)(uint8_t)memory[sp - 1]);
            memory[sp - 8] = (int8_t)lb;
            sp -= 7;
            pc += 1;
            break;
        }
        case 54: {//convls 36
            int64_t ls = ((int64_t)(uint8_t)memory[sp - 8] << 56 | (int64_t)(uint8_t)memory[sp - 7] << 48 | (int64_t)(uint8_t)memory[sp - 6] << 40 | (int64_t)(uint8_t)memory[sp + 3] << 32 | (int64_t)(uint8_t)memory[sp - 5] << 24 | (int64_t)(uint8_t)memory[sp - 4] << 16 | (int64_t)(uint8_t)memory[sp - 3] << 8 | (int64_t)(uint8_t)memory[sp - 1]);
            short lss = (short)ls;
            memory[sp - 8] = (lss >> 8 & 0xFF);
            memory[sp - 7] = lss & 0xFF;
            sp -= 6;
            pc += 1;
            break;
        }
        case 55: {//convli 37
            int64_t li = ((int64_t)(uint8_t)memory[sp - 8] << 56 | (int64_t)(uint8_t)memory[sp - 7] << 48 | (int64_t)(uint8_t)memory[sp - 6] << 40 | (int64_t)(uint8_t)memory[sp + 3] << 32 | (int64_t)(uint8_t)memory[sp - 5] << 24 | (int64_t)(uint8_t)memory[sp - 4] << 16 | (int64_t)(uint8_t)memory[sp - 3] << 8 | (int64_t)(uint8_t)memory[sp - 1]);
            int lii = (int) li;
            memory[sp - 8] = (lii >> 24) & 0xFF;
            memory[sp - 7] = (lii >> 16) & 0xFF;
            memory[sp - 6] = (lii >> 8) & 0xFF;
            memory[sp - 5] = (lii) & 0xFF;
            sp -= 4;
            pc += 1;
            break;
        }
        case 57: {//convlf 39
            int64_t lf = ((int64_t)(uint8_t)memory[sp - 8] << 56 | (int64_t)(uint8_t)memory[sp - 7] << 48 | (int64_t)(uint8_t)memory[sp - 6] << 40 | (int64_t)(uint8_t)memory[sp + 3] << 32 | (int64_t)(uint8_t)memory[sp - 5] << 24 | (int64_t)(uint8_t)memory[sp - 4] << 16 | (int64_t)(uint8_t)memory[sp - 3] << 8 | (int64_t)(uint8_t)memory[sp - 1]);
            float lff = (float) lf;
            int32_t lffu;
            memcpy(&lffu, &lff, sizeof(float));
            memory[sp - 8] = (lffu >> 24) & 0xFF;
            memory[sp - 7] = (lffu >> 16) & 0xFF;
            memory[sp - 6] = (lffu >> 8) & 0xFF;
            memory[sp - 5] = (lffu) & 0xFF;
            sp -= 4;
            pc += 1;
            break;
        }
        case 58: {//convld 3a
            int64_t ld = ((int64_t)(uint8_t)memory[sp - 8] << 56 | (int64_t)(uint8_t)memory[sp - 7] << 48 | (int64_t)(uint8_t)memory[sp - 6] << 40 | (int64_t)(uint8_t)memory[sp + 3] << 32 | (int64_t)(uint8_t)memory[sp - 5] << 24 | (int64_t)(uint8_t)memory[sp - 4] << 16 | (int64_t)(uint8_t)memory[sp - 3] << 8 | (int64_t)(uint8_t)memory[sp - 1]);
            double ldd = (double) ld;
            uint64_t lddu;
            memcpy(&lddu, &ldd, sizeof(double));
            memory[sp - 8] = (lddu >> 56) & 0xFF;
            memory[sp - 7] = (lddu >> 48) & 0xFF;
            memory[sp - 6] = (lddu >> 40) & 0xFF;
            memory[sp - 5] = (lddu >> 32) & 0xFF;
            memory[sp - 4] = (lddu >> 24) & 0xFF;
            memory[sp - 3] = (lddu >> 16) & 0xFF;
            memory[sp - 2] = (lddu >> 8) & 0xFF;
            memory[sp - 1] = (lddu) & 0xFF;
            pc += 1;
            break;
        }
        case 59: {//convfb 3b
            int8_t fb[4];
            for (int i = 0; i < 4; i++) {
                fb[i] = memory[sp - 1 - i];
            }
            float fbf = *((float*)fb);
            memory[sp - 4] = (int8_t)fbf;
            sp -= 3;
            pc += 1;
            break;
        }
        case 60: {//convfs 3c
            int8_t fs[4];
            for (int i = 0; i < 4; i++) {
                fs[i] = memory[sp - 1 - i];
            }
            float fsf = *((float*)fs);
            short fss = (short) fsf;
            memory[sp - 4] = (fss >> 8 & 0xFF);
            memory[sp - 3] = fss & 0xFF;
            sp -= 2;
            pc += 1;
            break;
        }
        case 61: {//convfi 3d
            int8_t fi[4];
            for (int i = 0; i < 4; i++) {
                fi[i] = memory[sp - 1 - i];
            }
            float fif = *((float*)fi);
            int fii = (int) fif;
            memory[sp - 4] = (fii >> 24) & 0xFF;
            memory[sp - 3] = (fii >> 16) & 0xFF;
            memory[sp - 2] = (fii >> 8) & 0xFF;
            memory[sp - 1] = (fii) & 0xFF;
            pc += 1;
            break;
        }
        case 62: {//convfl 3e
            int8_t fl[4];
            for (int i = 0; i < 4; i++) {
                fl[i] = memory[sp - 1 - i];
            }
            float flf = *((float*)fl);
            int64_t fll = (int64_t)(uint8_t) flf;
            memory[sp - 4] = (fll >> 56) & 0xFF;
            memory[sp - 3] = (fll >> 48) & 0xFF;
            memory[sp - 2] = (fll >> 40) & 0xFF;
            memory[sp - 1] = (fll >> 32) & 0xFF;
            memory[sp] = (fll >> 24) & 0xFF;
            memory[sp + 1] = (fll >> 16) & 0xFF;
            memory[sp + 2] = (fll >> 8) & 0xFF;
            memory[sp + 3] = (fll) & 0xFF;
            sp += 4;
            pc += 1;
            break;
        }
        case 63: {//convfd 3f
            int8_t fd[4];
            for (int i = 0; i < 4; i++) {
                fd[i] = memory[sp - 1 - i];
            }
            float fdf = *((float*)fd);
            double fdd = (double) fdf;
            int64_t fddu;
            memcpy(&fddu, &fdd, sizeof(double));
            memory[sp - 4] = (fddu >> 56) & 0xFF;
            memory[sp - 3] = (fddu >> 48) & 0xFF;
            memory[sp - 2] = (fddu >> 40) & 0xFF;
            memory[sp - 1] = (fddu >> 32) & 0xFF;
            memory[sp] = (fddu >> 24) & 0xFF;
            memory[sp + 1] = (fddu >> 16) & 0xFF;
            memory[sp + 2] = (fddu >> 8) & 0xFF;
            memory[sp + 3] = (fddu) & 0xFF;
            sp += 4;
            pc += 1;
            break;
        }
        case 64: {//convdb 40
            int8_t db[8];
            for (int i = 0; i < 8; i++) {
                db[i] = memory[sp - 1 - i];
            }
            double dbd = *((double*)db);
            memory[sp - 8] = (int8_t)dbd;
            sp -= 7;
            pc += 1;
            break;
        }
        case 65: {//convds 41
            int8_t ds[8];
            for (int i = 0; i < 8; i++) {
                ds[i] = memory[sp - 1 - i];
            }
            double dsd = *((double*)ds);
            short dss = (short)dsd;
            memory[sp - 8] = (dss >> 8 & 0xFF);
            memory[sp - 7] = dss & 0xFF;
            sp -= 6;
            pc += 1;
            break;
        }
        case 66: {//convdi 42
            int8_t di[8];
            for (int i = 0; i < 8; i++) {
                di[i] = memory[sp - 1 - i];
            }
            double did = *((double*)di);
            int dii = (int) did;
            memory[sp - 8] = (dii >> 24) & 0xFF;
            memory[sp - 7] = (dii >> 16) & 0xFF;
            memory[sp - 6] = (dii >> 8) & 0xFF;
            memory[sp - 5] = (dii) & 0xFF;
            sp -= 4;
            pc += 1;
            break;
        }
        case 67: {//convdl 43
            int8_t dl[8];
            for (int i = 0; i < 8; i++) {
                dl[i] = memory[sp - 1 - i];
            }
            double dld = *((double*)dl);
            int64_t dll = (int64_t)(uint8_t)dld;
            memory[sp - 8] = (dll >> 56) & 0xFF;
            memory[sp - 7] = (dll >> 48) & 0xFF;
            memory[sp - 6] = (dll >> 40) & 0xFF;
            memory[sp - 5] = (dll >> 32) & 0xFF;
            memory[sp - 4] = (dll >> 24) & 0xFF;
            memory[sp - 3] = (dll >> 16) & 0xFF;
            memory[sp - 2] = (dll >> 8) & 0xFF;
            memory[sp - 1] = (dll) & 0xFF;
            pc += 1;
            break;
        }
        case 68: {//convdf 44
            int8_t df[8];
            for (int i = 0; i < 8; i++) {
                df[i] = memory[sp - 1 - i];
            }
            double dfd = *((double*)df);
            float dff = (float) dfd;
            int dffu;
            memcpy(&dffu, &dff, sizeof(float));
            memory[sp - 8] = (dffu >> 24) & 0xFF;
            memory[sp - 7] = (dffu >> 16) & 0xFF;
            memory[sp - 6] = (dffu >> 8) & 0xFF;
            memory[sp - 5] = (dffu) & 0xFF;
            sp -= 4;
            pc += 1;
            break;
        }
        case 0x45: {//inch
            char ch;
            if (scanf(" %c", &ch) != 1) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            char remaining;
            if (scanf("%c", &remaining) == 1 && remaining != '\n') {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            else {
                int8_t byte = (int8_t)ch;
            }
            pc += 1;
            break;
        }
        case 0x46: //inb
            scanf("%99[^\n]", input);
            if(sscanf(input, "%ld", &temp) == 1 && temp >= INT8_MIN && temp <= INT8_MAX) {
                int8_t toInput = (int8_t) temp;
                memory[sp] = toInput;
                sp += 1;
                pc += 1;
            }
            else {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            break;
        case 0x47: //ins
            scanf("%99[^\n]", input);
            if(sscanf(input, "%ld", &temp) == 1 && temp >= INT16_MIN && temp <= INT16_MAX) {
                int16_t toInput = (int16_t) temp;
                memory[sp] = toInput;
                sp += 1;
                pc += 1;
            }
            else {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            break;
        case 0x48: //ini
            scanf("%99[^\n]", input);
            if(sscanf(input, "%ld", &temp) == 1 && temp >= INT32_MIN && temp <= INT32_MAX) {
                int toInput = (int) temp;
                memory[sp] = toInput;
                sp += 1;
                pc += 1;
            }
            else {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            break;
        case 0x49: //inl
            scanf("%99[^\n]", input);
            if(sscanf(input, "%ld", &temp) == 1) {
                memory[sp] = (int8_t) input;
                sp += 1;
                pc += 1;
            }
            else {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            break;
        case 0x4a: //inf
            scanf("%99[^\n]", input);
            if(sscanf(input, "%ld", &temp) == 1 && temp >= -__FLT_MAX__ && temp <= __FLT_MAX__) {
                float toInput = (float) temp;
                memory[sp] = toInput;
                sp += 1;
                pc += 1;
            }
            else {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            break;
        case 0x4b: {//ind
            scanf("%99[^\n]", input);
            if(sscanf(input, "%ld", &temp) == 1 && temp >= -__DBL_MAX__ && temp <= __DBL_MAX__) {
                double toInput = (double) temp;
                memory[sp] = toInput;
                sp += 1;
                pc += 1;
            }
            else {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            break;
        }
        case 0x4c: //outch
            sp -= 1;
            char outch = (char) memory[sp];
            printf("%c", outch);
            pc += 1;
            break;
        case 0x4d: //outb
            sp -= 1;
            out = memory[sp];
            printf("%d\n", out);
            pc += 1;
            break;
        case 0x4e: //outs
            sp -= 2;
            int16_t outs = ((int16_t)memory[sp] << 8 | (int16_t)memory[sp + 1]);
            printf("%d\n", outs);
            pc += 1;
            break;
        case 0x4f: //outi
            sp -= 4;
            // int32_t outi = ((int32_t)memory[sp] << 24 | (int32_t)memory[sp + 1] << 16 | (int32_t)memory[sp + 2] << 8 | (int32_t)memory[sp + 3]);
            int8_t intBytes[8];
            for (int i = 0; i < 8; i++) {
                intBytes[i] = memory[sp + 3 - i];
            }
            int outi = *((int*)intBytes);
            printf("%d\n", outi);
            pc += 1;
            break;
        case 0x50: //outl
            sp -= 8;
            int64_t outl = ((int64_t)(uint8_t)memory[sp] << 56 | (int64_t)(uint8_t)memory[sp + 1] << 48 | (int64_t)(uint8_t)memory[sp + 2] << 40 | (int64_t)(uint8_t)memory[sp + 3] << 32 | (int64_t)(uint8_t)memory[sp + 4] << 24 | (int64_t)(uint8_t)memory[sp + 5] << 16 | (int64_t)(uint8_t)memory[sp + 6] << 8 | (int64_t)(uint8_t)memory[sp + 7]);
            printf("%ld\n", outl);
            pc += 1;
            break;
        case 0x51: //outf
            sp -= 4;
            int8_t floBytes[4];
            for (int i = 0; i < 4; i++) {
                floBytes[i] = memory[sp + 3 - i];
            }
            float outf = *((float*)floBytes);
            printf("%f\n", outf);
            pc += 1;
            break;
        case 0x52: //outd
            sp -= 8;
            int8_t douBytes[8];
            for (int i = 0; i < 8; i++) {
                douBytes[i] = memory[sp + 7 - i];
            }
            double outd = *((double*)douBytes);
            printf("%lf\n", outd);
            pc += 1;
            break;
        case 0x53: //addb
            memory[sp - 2] = memory[sp - 2] + memory[sp - 1];
            sp -= 1;
            pc += 1;
            break;
        case 0x54: { //adds
            short one = ((int16_t)memory[sp - 2] << 8 | (int16_t)memory[sp - 1]);
            short two = ((int16_t)memory[sp - 4] << 8 | (int16_t)memory[sp - 3]);
            short sum = (short)(one + two);
            memory[sp - 3] = sum & 0xFF; // least significant
            memory[sp - 4] = (sum >> 8) & 0xFF; //most significant
            sp -= 2;
            pc += 1;
            break;
        }
        case 0x55: { //addi
            int32_t on = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);

            int32_t tw = ((uint8_t)memory[sp - 8] << 24 | (uint8_t)memory[sp - 7] << 16 | (uint8_t)memory[sp - 6] << 8 | (uint8_t)memory[sp - 5]);

            int32_t sum1 = on + tw;

            memory[sp - 8] =  (sum1 >> 24) & 0xFF; //most significant
            // printf("memory %d: %d ", (sp - 8), memory[sp - 8]);
            memory[sp - 7] = (sum1 >> 16) & 0xFF; 
            // printf("memory %d: %d ", (sp - 7), memory[sp - 7]);
            memory[sp - 6] = (sum1 >> 8) & 0xFF;
            // printf("memory %d: %d ", (sp - 6), memory[sp - 6]);
            memory[sp - 5] = sum1 & 0xFF; //least significant
            // printf("memory %d: %d\n", (sp - 5), memory[sp - 5]);
            sp -= 4;
            pc += 1;
            break;
        }
        case 0x56: {//addl
            int64_t one1 = ((int64_t)(uint8_t)memory[sp - 8] << 56 | (int64_t)(uint8_t)memory[sp - 7] << 48 | (int64_t)(uint8_t)memory[sp - 6] << 40 | (int64_t)(uint8_t)memory[sp - 5] << 32 | (int64_t)(uint8_t)memory[sp - 4] << 24 | (int64_t)(uint8_t)memory[sp - 3] << 16 | (int64_t)(uint8_t)memory[sp - 2] << 8 | (int64_t)(uint8_t)memory[sp - 1]);

            int64_t two1 = ((int64_t)(uint8_t)memory[sp - 16] << 56 | (int64_t)(uint8_t)memory[sp - 15] << 48 | (int64_t)(uint8_t)memory[sp - 14] << 40 | (int64_t)(uint8_t)memory[sp - 13] << 32 | (int64_t)(uint8_t)memory[sp - 12] << 24 | (int64_t)(uint8_t)memory[sp - 11] << 16 | (int64_t)(uint8_t)memory[sp - 10] << 8 | (int64_t)(uint8_t)memory[sp - 9]);

            int64_t sum2 = one1 + two1;

            memory[sp - 9] = sum2 & 0xFF; //least significant
            memory[sp - 10] = (sum2 >> 8) & 0xFF;
            memory[sp - 11] = (sum2 >> 16) & 0xFF; 
            memory[sp - 12] = (sum2 >> 24) & 0xFF;
            memory[sp - 13] = (sum2 >> 32) & 0xFF; 
            memory[sp - 14] = (sum2 >> 40) & 0xFF;
            memory[sp - 15] = (sum2 >> 48) & 0xFF;
            memory[sp - 16] = (sum2 >> 56) & 0xFF; //most significant
            sp -= 8;
            pc += 1;
            break;
        }
        case 0x57: { //addf
            // int32_t one2 = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);
            int8_t addf1[4];
            for (int i = 0; i < 4; i++) {
                addf1[i] = memory[sp - 1 - i];
            }
            float af = *((float*)addf1);
            
            int8_t addf2[4];
            for (int i = 5; i < 8; i++) {
                addf2[i] = memory[sp - 1 - i];
            }
            float af2 = *((float*)addf2);
            float sumfloat = (float) (af + af2);
            int32_t sumfl;
            memcpy(&sumfl, &sumfloat, sizeof(float));
            
            // int32_t two2 = ((int32_t)memory[sp - 8] << 24 | (int32_t)memory[sp - 7] << 16 | (int32_t)memory[sp - 6] << 8 | (int32_t)memory[sp - 5]);

            // int32_t sum3 = one2 + two2;
            memory[sp - 5] = sumfl & 0xFF; //least significant
            memory[sp - 6] = (sumfl >> 8) & 0xFF;
            memory[sp - 7] = (sumfl >> 16) & 0xFF; 
            memory[sp - 8] = (sumfl >> 24) & 0xFF; //most significant
            sp -= 4;
            pc += 1;
            break;
        }
        case 0x58: { //addd
            int64_t one3 = ((int64_t)(uint8_t)memory[sp - 8] << 56 | (int64_t)(uint8_t)memory[sp - 7] << 48 | (int64_t)(uint8_t)memory[sp - 6] << 40 | (int64_t)(uint8_t)memory[sp - 5] << 32 | (int64_t)(uint8_t)memory[sp - 4] << 24 | (int64_t)(uint8_t)memory[sp - 3] << 16 | (int64_t)(uint8_t)memory[sp - 2] << 8 | (int64_t)(uint8_t)memory[sp - 1]);

            int64_t two3 = ((int64_t)(uint8_t)memory[sp - 16] << 56 | (int64_t)(uint8_t)memory[sp - 15] << 48 | (int64_t)(uint8_t)memory[sp - 14] << 40 | (int64_t)(uint8_t)memory[sp - 13] << 32 | (int64_t)(uint8_t)memory[sp - 12] << 24 | (int64_t)(uint8_t)memory[sp - 11] << 16 | (int64_t)(uint8_t)memory[sp - 10] << 8 | (int64_t)(uint8_t)memory[sp - 9]);

            int64_t sum4 = one3 + two3;

            memory[sp - 9] = sum4 & 0xFF; //least significant
            memory[sp - 10] = (sum4 >> 8) & 0xFF;
            memory[sp - 11] = (sum4 >> 16) & 0xFF; 
            memory[sp - 12] = (sum4 >> 24) & 0xFF;
            memory[sp - 13] = (sum4 >> 32) & 0xFF; 
            memory[sp - 14] = (sum4 >> 40) & 0xFF;
            memory[sp - 15] = (sum4 >> 48) & 0xFF;
            memory[sp - 16] = (sum4 >> 56) & 0xFF; //most significant
            sp -= 8;
            pc += 1;
            break;
        }
        case 0x59: //subb
            memory[sp - 2] = memory[sp - 2] - memory[sp - 1];
            sp -= 1;
            pc += 1;
            break;
        case 0x5a: {//subs
            short one5 = ((int16_t)memory[sp - 2] << 8 | (int16_t)memory[sp - 1]);
            short two5 = ((int16_t)memory[sp - 4] << 8 | (int16_t)memory[sp - 3]);
            short sum5 = two5 - one5;
            memory[sp - 3] = sum5 & 0xFF; // least significant
            memory[sp - 4] = (sum5 >> 8) & 0xFF; //most significant
            sp -= 2;
            pc += 1;
        }
        case 0x5b: {//subi
            int32_t one6 = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);

            int32_t two6 = ((uint8_t)memory[sp - 8] << 24 | (uint8_t)memory[sp - 7] << 16 | (uint8_t)memory[sp - 6] << 8 | (uint8_t)memory[sp - 5]);

            int32_t sum6 = two6 - one6;

            memory[sp - 5] = sum6 & 0xFF; //least significant
            memory[sp - 6] = (sum6 >> 8) & 0xFF;
            memory[sp - 7] = (sum6 >> 16) & 0xFF; 
            memory[sp - 8] = (sum6 >> 24) & 0xFF; //most significant
            sp -= 4;
            pc += 1;
            break;
        }
        case 0x5c:{ //subl
            int64_t one7 = ((int64_t)(uint8_t)memory[sp - 8] << 56 | (int64_t)(uint8_t)memory[sp - 7] << 48 | (int64_t)(uint8_t)memory[sp - 6] << 40 | (int64_t)(uint8_t)memory[sp - 5] << 32 | (int64_t)(uint8_t)memory[sp - 4] << 24 | (int64_t)(uint8_t)memory[sp - 3] << 16 | (int64_t)(uint8_t)memory[sp - 2] << 8 | (int64_t)(uint8_t)memory[sp - 1]);

            int64_t two7 = ((int64_t)(uint8_t)memory[sp - 16] << 56 | (int64_t)(uint8_t)memory[sp - 15] << 48 | (int64_t)(uint8_t)memory[sp - 14] << 40 | (int64_t)(uint8_t)memory[sp - 13] << 32 | (int64_t)(uint8_t)memory[sp - 12] << 24 | (int64_t)(uint8_t)memory[sp - 11] << 16 | (int64_t)(uint8_t)memory[sp - 10] << 8 | (int64_t)(uint8_t)memory[sp - 9]);

            int64_t sum7 = two7 - one7;

            memory[sp - 9] = sum7 & 0xFF; //least significant
            memory[sp - 10] = (sum7 >> 8) & 0xFF;
            memory[sp - 11] = (sum7 >> 16) & 0xFF; 
            memory[sp - 12] = (sum7 >> 24) & 0xFF;
            memory[sp - 13] = (sum7 >> 32) & 0xFF; 
            memory[sp - 14] = (sum7 >> 40) & 0xFF;
            memory[sp - 15] = (sum7 >> 48) & 0xFF;
            memory[sp - 16] = (sum7 >> 56) & 0xFF; //most significant
            sp -= 8;
            pc += 1;
            break;
        }
        case 0x5d: {//subf
            int32_t one8 = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);

            int32_t two8 = ((uint8_t)memory[sp - 8] << 24 | (uint8_t)memory[sp - 7] << 16 | (uint8_t)memory[sp - 6] << 8 | (uint8_t)memory[sp - 5]);

            int32_t sum8 = two8 - one8;

            memory[sp - 5] = sum8 & 0xFF; //least significant
            memory[sp - 6] = (sum8 >> 8) & 0xFF;
            memory[sp - 7] = (sum8 >> 16) & 0xFF; 
            memory[sp - 8] = (sum8 >> 24) & 0xFF; //most significant
            sp -= 4;
            pc += 1;
            break;
        }
        case 0x5e: {//subd
            int64_t one9 = ((int64_t)(uint8_t)memory[sp - 8] << 56 | (int64_t)(uint8_t)memory[sp - 7] << 48 | (int64_t)(uint8_t)memory[sp - 6] << 40 | (int64_t)(uint8_t)memory[sp - 5] << 32 | (int64_t)(uint8_t)memory[sp - 4] << 24 | (int64_t)(uint8_t)memory[sp - 3] << 16 | (int64_t)(uint8_t)memory[sp - 2] << 8 | (int64_t)(uint8_t)memory[sp - 1]);

            int64_t two9 = ((int64_t)(uint8_t)memory[sp - 16] << 56 | (int64_t)(uint8_t)memory[sp - 15] << 48 | (int64_t)(uint8_t)memory[sp - 14] << 40 | (int64_t)(uint8_t)memory[sp - 13] << 32 | (int64_t)(uint8_t)memory[sp - 12] << 24 | (int64_t)(uint8_t)memory[sp - 11] << 16 | (int64_t)(uint8_t)memory[sp - 10] << 8 | (int64_t)(uint8_t)memory[sp - 9]);

            int64_t sum9 = two9 - one9;

            memory[sp - 9] = sum9 & 0xFF; //least significant
            memory[sp - 10] = (sum9 >> 8) & 0xFF;
            memory[sp - 11] = (sum9 >> 16) & 0xFF; 
            memory[sp - 12] = (sum9 >> 24) & 0xFF;
            memory[sp - 13] = (sum9 >> 32) & 0xFF; 
            memory[sp - 14] = (sum9 >> 40) & 0xFF;
            memory[sp - 15] = (sum9 >> 48) & 0xFF;
            memory[sp - 16] = (sum9 >> 56) & 0xFF; //most significant
            sp -= 8;
            pc += 1;
            break;
        }
        case 0x5f: //mulb
            memory[sp - 2] = memory[sp - 2] * memory[sp - 1];
            sp -= 1;
            pc += 1;
            break;
        case 0x60: {//muls
            short onea = ((int16_t)memory[sp - 2] << 8 | (int16_t)memory[sp - 1]);
            short twoa = ((int16_t)memory[sp - 4] << 8 | (int16_t)memory[sp - 3]);
            short suma = twoa * onea;
            memory[sp - 3] = suma & 0xFF; // least significant
            memory[sp - 4] = (suma >> 8) & 0xFF; //most significant
            sp -= 2;
            pc += 1;
        }
        case 0x61: {//muli
            int32_t oneb = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);

            int32_t twob = ((uint8_t)memory[sp - 8] << 24 | (uint8_t)memory[sp - 7] << 16 | (uint8_t)memory[sp - 6] << 8 | (uint8_t)memory[sp - 5]);

            int32_t sumb = twob * oneb;

            memory[sp - 5] = sumb & 0xFF; //least significant
            memory[sp - 6] = (sumb >> 8) & 0xFF;
            memory[sp - 7] = (sumb >> 16) & 0xFF; 
            memory[sp - 8] = (sumb >> 24) & 0xFF; //most significant
            sp -= 4;
            pc += 1;
            break;
        }
        case 0x62: {//mull
            int64_t onec = ((int64_t)(uint8_t)memory[sp - 8] << 56 | (int64_t)(uint8_t)memory[sp - 7] << 48 | (int64_t)(uint8_t)memory[sp - 6] << 40 | (int64_t)(uint8_t)memory[sp - 5] << 32 | (int64_t)(uint8_t)memory[sp - 4] << 24 | (int64_t)(uint8_t)memory[sp - 3] << 16 | (int64_t)(uint8_t)memory[sp - 2] << 8 | (int64_t)(uint8_t)memory[sp - 1]);

            int64_t twoc = ((int64_t)(uint8_t)memory[sp - 16] << 56 | (int64_t)(uint8_t)memory[sp - 15] << 48 | (int64_t)(uint8_t)memory[sp - 14] << 40 | (int64_t)(uint8_t)memory[sp - 13] << 32 | (int64_t)(uint8_t)memory[sp - 12] << 24 | (int64_t)(uint8_t)memory[sp - 11] << 16 | (int64_t)(uint8_t)memory[sp - 10] << 8 | (int64_t)(uint8_t)memory[sp - 9]);

            int64_t sumc = twoc * onec;

            memory[sp - 9] = sumc & 0xFF; //least significant
            memory[sp - 10] = (sumc >> 8) & 0xFF;
            memory[sp - 11] = (sumc >> 16) & 0xFF; 
            memory[sp - 12] = (sumc >> 24) & 0xFF;
            memory[sp - 13] = (sumc >> 32) & 0xFF; 
            memory[sp - 14] = (sumc >> 40) & 0xFF;
            memory[sp - 15] = (sumc >> 48) & 0xFF;
            memory[sp - 16] = (sumc >> 56) & 0xFF; //most significant
            sp -= 8;
            pc += 1;
            break;
        }
        case 0x63: {//mulf
            int32_t oned = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);

            int32_t twod = ((uint8_t)memory[sp - 8] << 24 | (uint8_t)memory[sp - 7] << 16 | (uint8_t)memory[sp - 6] << 8 | (uint8_t)memory[sp - 5]);

            int32_t sumd = twod * oned;

            memory[sp - 5] = sumd & 0xFF; //least significant
            memory[sp - 6] = (sumd >> 8) & 0xFF;
            memory[sp - 7] = (sumd >> 16) & 0xFF; 
            memory[sp - 8] = (sumd >> 24) & 0xFF; //most significant
            sp -= 4;
            pc += 1;
            break;
        }
        case 0x64: {//muld
            int64_t onee = ((int64_t)(uint8_t)memory[sp - 8] << 56 | (int64_t)(uint8_t)memory[sp - 7] << 48 | (int64_t)(uint8_t)memory[sp - 6] << 40 | (int64_t)(uint8_t)memory[sp - 5] << 32 | (int64_t)(uint8_t)memory[sp - 4] << 24 | (int64_t)(uint8_t)memory[sp - 3] << 16 | (int64_t)(uint8_t)memory[sp - 2] << 8 | (int64_t)(uint8_t)memory[sp - 1]);

            int64_t twoe = ((int64_t)(uint8_t)memory[sp - 16] << 56 | (int64_t)(uint8_t)memory[sp - 15] << 48 | (int64_t)(uint8_t)memory[sp - 14] << 40 | (int64_t)(uint8_t)memory[sp - 13] << 32 | (int64_t)(uint8_t)memory[sp - 12] << 24 | (int64_t)(uint8_t)memory[sp - 11] << 16 | (int64_t)(uint8_t)memory[sp - 10] << 8 | (int64_t)(uint8_t)memory[sp - 9]);

            int64_t sume = twoe * onee;

            memory[sp - 9] = sume & 0xFF; //least significant
            memory[sp - 10] = (sume >> 8) & 0xFF;
            memory[sp - 11] = (sume >> 16) & 0xFF; 
            memory[sp - 12] = (sume >> 24) & 0xFF;
            memory[sp - 13] = (sume >> 32) & 0xFF; 
            memory[sp - 14] = (sume >> 40) & 0xFF;
            memory[sp - 15] = (sume >> 48) & 0xFF;
            memory[sp - 16] = (sume >> 56) & 0xFF; //most significant
            sp -= 8;
            pc += 1;
            break;
        }
        case 0x65: //divb
            memory[sp - 2] = memory[sp - 2] / memory[sp - 1];
            sp -= 1;
            pc += 1;
            break;
        case 0x66: {//divs
            short onef = ((int16_t)memory[sp - 2] << 8 | (int16_t)memory[sp - 1]);
            if(onef == 0) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            short twof = ((int16_t)memory[sp - 4] << 8 | (int16_t)memory[sp - 3]);
            short sumf = twof / onef;
            memory[sp - 4] = sumf & 0xFF; // least significant
            memory[sp - 3] = (sumf >> 8) & 0xFF; //most significant
            sp -= 2;
            pc += 1;
        }
        case 0x67: {//divi
            int32_t oneg = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);
            if(oneg == 0) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            int32_t twog = ((uint8_t)memory[sp - 8] << 24 | (uint8_t)memory[sp - 7] << 16 | (uint8_t)memory[sp - 6] << 8 | (uint8_t)memory[sp - 5]);

            int32_t sumg = twog / oneg;

            memory[sp - 5] = sumg & 0xFF; //least significant
            memory[sp - 6] = (sumg >> 8) & 0xFF;
            memory[sp - 7] = (sumg >> 16) & 0xFF; 
            memory[sp - 8] = (sumg >> 24) & 0xFF; //most significant
            sp -= 4;
            pc += 1;
            break;
        }
        case 0x68: {//divl
            int64_t oneh = ((int64_t)(uint8_t)memory[sp - 8] << 56 | (int64_t)(uint8_t)memory[sp - 7] << 48 | (int64_t)(uint8_t)memory[sp - 6] << 40 | (int64_t)(uint8_t)memory[sp - 5] << 32 | (int64_t)(uint8_t)memory[sp - 4] << 24 | (int64_t)(uint8_t)memory[sp - 3] << 16 | (int64_t)(uint8_t)memory[sp - 2] << 8 | (int64_t)(uint8_t)memory[sp - 1]);
            if(oneh == 0) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            int64_t twoh = ((int64_t)(uint8_t)memory[sp - 16] << 56 | (int64_t)(uint8_t)memory[sp - 15] << 48 | (int64_t)(uint8_t)memory[sp - 14] << 40 | (int64_t)(uint8_t)memory[sp - 13] << 32 | (int64_t)(uint8_t)memory[sp - 12] << 24 | (int64_t)(uint8_t)memory[sp - 11] << 16 | (int64_t)(uint8_t)memory[sp - 10] << 8 | (int64_t)(uint8_t)memory[sp - 9]);

            int64_t sumh = twoh / oneh;

            memory[sp - 9] = sumh & 0xFF; //least significant
            memory[sp - 10] = (sumh >> 8) & 0xFF;
            memory[sp - 11] = (sumh >> 16) & 0xFF; 
            memory[sp - 12] = (sumh >> 24) & 0xFF;
            memory[sp - 13] = (sumh >> 32) & 0xFF; 
            memory[sp - 14] = (sumh >> 40) & 0xFF;
            memory[sp - 15] = (sumh >> 48) & 0xFF;
            memory[sp - 16] = (sumh >> 56) & 0xFF; //most significant
            sp -= 8;
            pc += 1;
            break;
        }
        case 0x69: {//divf 
            int32_t onej = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);
            if(onej == 0) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            int32_t twoj = ((uint8_t)memory[sp - 8] << 24 | (uint8_t)memory[sp - 7] << 16 | (uint8_t)memory[sp - 6] << 8 | (uint8_t)memory[sp - 5]);

            int32_t sumj = twoj / onej;

            memory[sp - 7] = sumj & 0xFF; //least significant
            memory[sp - 6] = (sumj >> 8) & 0xFF;
            memory[sp - 5] = (sumj >> 16) & 0xFF; 
            memory[sp - 8] = (sumj >> 24) & 0xFF; //most significant
            sp -= 4;
            pc += 1;
            break;
        }
        case 0x6a: {//divd
            int64_t onez = ((int64_t)(uint8_t)memory[sp - 8] << 56 | (int64_t)(uint8_t)memory[sp - 7] << 48 | (int64_t)(uint8_t)memory[sp - 6] << 40 | (int64_t)(uint8_t)memory[sp - 5] << 32 | (int64_t)(uint8_t)memory[sp - 4] << 24 | (int64_t)(uint8_t)memory[sp - 3] << 16 | (int64_t)(uint8_t)memory[sp - 2] << 8 | (int64_t)(uint8_t)memory[sp - 1]);
            if(onez == 0) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            int64_t twoz = ((int64_t)(uint8_t)memory[sp - 16] << 56 | (int64_t)(uint8_t)memory[sp - 15] << 48 | (int64_t)(uint8_t)memory[sp - 14] << 40 | (int64_t)(uint8_t)memory[sp - 13] << 32 | (int64_t)(uint8_t)memory[sp - 12] << 24 | (int64_t)(uint8_t)memory[sp - 11] << 16 | (int64_t)(uint8_t)memory[sp - 10] << 8 | (int64_t)(uint8_t)memory[sp - 9]);

            int64_t sumz = twoz / onez;

            memory[sp - 9] = sumz & 0xFF; //least significant
            memory[sp - 10] = (sumz >> 8) & 0xFF;
            memory[sp - 11] = (sumz >> 16) & 0xFF; 
            memory[sp - 12] = (sumz >> 24) & 0xFF;
            memory[sp - 13] = (sumz >> 32) & 0xFF; 
            memory[sp - 14] = (sumz >> 40) & 0xFF;
            memory[sp - 15] = (sumz >> 48) & 0xFF;
            memory[sp - 16] = (sumz >> 56) & 0xFF; //most significant
            sp -= 8;
            pc += 1;
            break;
        }
        case 0x6b: //and8
            memory[sp - 2] = memory[sp - 2] & memory[sp - 1];
            sp -= 1;
            pc += 1;
            break;
        case 0x6c: //and16
            memory[sp - 4] = memory[sp - 4] & memory[sp - 2];
            memory[sp - 3] = memory[sp - 3] & memory[sp - 1];
            sp -= 2;
            pc += 1;
            break;
        case 0x6d: //and32
            memory[sp - 8] = memory[sp - 8] & memory[sp - 4];
            memory[sp - 7] = memory[sp - 7] & memory[sp - 3];
            memory[sp - 6] = memory[sp - 6] & memory[sp - 2];
            memory[sp - 5] = memory[sp - 5] & memory[sp - 1];
            sp -= 4;
            pc += 1;
            break;
        case 0x6e: //and64
            memory[sp - 16] = memory[sp - 16] & memory[sp - 8];
            memory[sp - 15] = memory[sp - 15] & memory[sp - 7];
            memory[sp - 14] = memory[sp - 14] & memory[sp - 6];
            memory[sp - 13] = memory[sp - 13] & memory[sp - 5];
            memory[sp - 12] = memory[sp - 12] & memory[sp - 4];
            memory[sp - 11] = memory[sp - 11] & memory[sp - 3];
            memory[sp - 10] = memory[sp - 10] & memory[sp - 2];
            memory[sp - 9] = memory[sp - 9] & memory[sp - 1];
            sp -= 8;
            pc += 1;
            break;
        case 0x6f: //or8
            memory[sp - 2] = memory[sp - 2] | memory[sp - 1];
            sp -= 1;
            pc += 1;
            break;
        case 0x70: //or16
            memory[sp - 4] = memory[sp - 4] | memory[sp - 2];
            memory[sp - 3] = memory[sp - 3] | memory[sp - 1];
            sp -= 2;
            pc += 1;
            break;
        case 0x71: //or32
            memory[sp - 8] = memory[sp - 8] | memory[sp - 4];
            memory[sp - 7] = memory[sp - 7] | memory[sp - 3];
            memory[sp - 6] = memory[sp - 6] | memory[sp - 2];
            memory[sp - 5] = memory[sp - 5] | memory[sp - 1];
            sp -= 4;
            pc += 1;
            break;
        case 0x72: //or64
            memory[sp - 16] = memory[sp - 16] | memory[sp - 8];
            memory[sp - 15] = memory[sp - 15] | memory[sp - 7];
            memory[sp - 14] = memory[sp - 14] | memory[sp - 6];
            memory[sp - 13] = memory[sp - 13] | memory[sp - 5];
            memory[sp - 12] = memory[sp - 12] | memory[sp - 4];
            memory[sp - 11] = memory[sp - 11] | memory[sp - 3];
            memory[sp - 10] = memory[sp - 10] | memory[sp - 2];
            memory[sp - 9] = memory[sp - 9] | memory[sp - 1];
            sp -= 8;
            pc += 1;
            break;
        case 0x73: //xor8
            memory[sp - 2] = memory[sp - 2] ^ memory[sp - 1];
            sp -= 1;
            pc += 1;
            break;
        case 0x74: //xor16
            memory[sp - 4] = memory[sp - 4] ^ memory[sp - 2];
            memory[sp - 3] = memory[sp - 3] ^ memory[sp - 1];
            sp -= 2;
            pc += 1;
            break;
        case 0x75: //xor32
            memory[sp - 8] = memory[sp - 8] ^ memory[sp - 4];
            memory[sp - 7] = memory[sp - 7] ^ memory[sp - 3];
            memory[sp - 6] = memory[sp - 6] ^ memory[sp - 2];
            memory[sp - 5] = memory[sp - 5] ^ memory[sp - 1];
            sp -= 4;
            pc += 1;
            break;
        case 0x76: //xor64
            memory[sp - 16] = memory[sp - 16] ^ memory[sp - 8];
            memory[sp - 15] = memory[sp - 15] ^ memory[sp - 7];
            memory[sp - 14] = memory[sp - 14] ^ memory[sp - 6];
            memory[sp - 13] = memory[sp - 13] ^ memory[sp - 5];
            memory[sp - 12] = memory[sp - 12] ^ memory[sp - 4];
            memory[sp - 11] = memory[sp - 11] ^ memory[sp - 3];
            memory[sp - 10] = memory[sp - 10] ^ memory[sp - 2];
            memory[sp - 9] = memory[sp - 9] ^ memory[sp - 1];
            sp -= 8;
            pc += 1;
            break;
        case 0x77: //not8
            memory[sp - 1] = ~memory[sp - 1];
            pc += 1;
            break;
        case 0x78: //not16
            memory[sp - 1] = ~memory[sp - 1];
            memory[sp - 2] = ~memory[sp - 2];
            pc += 1;
            break;
        case 0x79: //not32
            memory[sp - 1] = ~memory[sp - 1];
            memory[sp - 2] = ~memory[sp - 2];
            memory[sp - 3] = ~memory[sp - 3];
            memory[sp - 4] = ~memory[sp - 4];
            pc += 1;
            break;
        case 0x7a: //not64
            memory[sp - 1] = ~memory[sp - 1];
            memory[sp - 2] = ~memory[sp - 2];
            memory[sp - 3] = ~memory[sp - 3];
            memory[sp - 4] = ~memory[sp - 4];
            memory[sp - 5] = ~memory[sp - 5];
            memory[sp - 6] = ~memory[sp - 6];
            memory[sp - 7] = ~memory[sp - 7];
            memory[sp - 8] = ~memory[sp - 8];
            pc += 1;
            break;
        case 0x7b: {//shftrb value
            int8_t toShift = memory[sp - 1];
            toShift = toShift >> memory[pc + 1];
            memory[sp - 1] = toShift;
            pc += 2;
            break;
        }
        case 0x7c: {//shftrs value
            int16_t toShift = ((uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);
            toShift = toShift >> memory[pc + 1];
            memory[sp - 1] = toShift & 0xFF; // least significant
            memory[sp - 2] = (toShift >> 8) & 0xFF; //most significant
            pc += 2;
            break;
        }
        case 0x7d: {//shftri valu
            int32_t toShift = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);
            toShift = toShift >> memory[pc + 1]; 
            memory[sp - 1] = toShift & 0xFF; //least significant
            memory[sp - 2] = (toShift >> 8) & 0xFF;
            memory[sp - 3] = (toShift >> 16) & 0xFF; 
            memory[sp - 4] = (toShift >> 24) & 0xFF; //most significant
            pc += 2;
            break;
        }
        case 0x7e: {//shftrl vlaue
            int64_t toShift1 = ((int64_t)(uint8_t)memory[sp - 8] << 56 | (int64_t)(uint8_t)memory[sp - 7] << 48 | (int64_t)(uint8_t)memory[sp - 6] << 40 | (int64_t)(uint8_t)memory[sp - 5] << 32 | (int64_t)(uint8_t)memory[sp - 4] << 24 | (int64_t)(uint8_t)memory[sp - 3] << 16 | (int64_t)(uint8_t)memory[sp - 2] << 8 | (int64_t)(uint8_t)memory[sp - 1]);
            toShift1 = toShift1 >> memory[pc + 1]; 
            memory[sp - 1] = toShift1 & 0xFF; //least significant
            memory[sp - 2] = (toShift1 >> 8) & 0xFF;
            memory[sp - 3] = (toShift1 >> 16) & 0xFF; 
            memory[sp - 4] = (toShift1 >> 24) & 0xFF;
            memory[sp - 5] = (toShift1 >> 32) & 0xFF; 
            memory[sp - 6] = (toShift1 >> 40) & 0xFF;
            memory[sp - 7] = (toShift1 >> 48) & 0xFF;
            memory[sp - 8] = (toShift1 >> 56) & 0xFF; //most significant
            pc += 2;
            break;
        }
        case 0x7f: {//shftlb value
            int8_t toShift2 = memory[sp - 1];
            toShift2 = toShift2 << memory[pc + 1];
            memory[sp - 1] = toShift2;
            pc += 2;
            break;
        }
        case 0x80: {//shftls value
            int16_t toShift3 = ((uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);
            toShift3 = toShift3 << memory[pc + 1];
            memory[sp - 1] = toShift3 & 0xFF; // least significant
            memory[sp - 2] = (toShift3 >> 8) & 0xFF; //most significant
            pc += 2;
            break;
        }
        case 0x81: {//shftli value
            int32_t toShift4 = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);
            toShift4 = toShift4 >> memory[pc + 1]; 
            memory[sp - 1] = toShift4 & 0xFF; //least significant
            memory[sp - 2] = (toShift4 >> 8) & 0xFF;
            memory[sp - 3] = (toShift4 >> 16) & 0xFF; 
            memory[sp - 4] = (toShift4 >> 24) & 0xFF; //most significant
            pc += 2;
            break;
        }
        case 0x82: {//shftll vlaue
            int64_t toShift5 = ((int64_t)(uint8_t)memory[sp - 8] << 56 | (int64_t)(uint8_t)memory[sp - 7] << 48 | (int64_t)(uint8_t)memory[sp - 6] << 40 | (int64_t)(uint8_t)memory[sp - 5] << 32 | (int64_t)(uint8_t)memory[sp - 4] << 24 | (int64_t)(uint8_t)memory[sp - 3] << 16 | (int64_t)(uint8_t)memory[sp - 2] << 8 | (int64_t)(uint8_t)memory[sp - 1]);
            toShift5 = toShift5 << memory[pc + 1]; 
            memory[sp - 1] = toShift5 & 0xFF; //least significant
            memory[sp - 2] = (toShift5 >> 8) & 0xFF;
            memory[sp - 3] = (toShift5 >> 16) & 0xFF; 
            memory[sp - 4] = (toShift5 >> 24) & 0xFF;
            memory[sp - 5] = (toShift5 >> 32) & 0xFF; 
            memory[sp - 6] = (toShift5 >> 40) & 0xFF;
            memory[sp - 7] = (toShift5 >> 48) & 0xFF;
            memory[sp - 8] = (toShift5 >> 56) & 0xFF; //most significant
            pc += 2;
            break;
        }
        case 0x83: //jmp address
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            pc = address;
            break;
        case 0x84: //jrpc offset
            pc += memory[pc + 1];
            break;
        case 0x85: {//jind
            int32_t jind = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);
            pc = jind;
            break;
        }
        case 0x86: //jz
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | (uint8_t)memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            int32_t jz = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);
            if(jz == 0) {
                pc = address;
            }
            else {
                pc += 4;
            }
            break;
        case 0x87: //jnz
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            int32_t jnz = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);
            if(jnz != 0) {
                pc = address;
            }
            else {
                pc += 4;
            }
            break;
        case 0x88: //jgt
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            int32_t jgt = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);
            if(jgt > 0) {
                pc = address;
            }
            else {
                pc += 4;
            }
            break;
        case 0x89: //jlt
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            int32_t jlt = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);
            if(jlt < 0) {
                pc = address;
            }
            else {
                pc += 4;
            }
            break;
        case 0x8a: //jge
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            int32_t jge = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);
            if(jge >= 0) {
                pc = address;
            }
            else {
                pc += 4;
            }
            break;
        case 0x8b: //jle
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            int32_t jle = ((uint8_t)memory[sp - 4] << 24 | (uint8_t)memory[sp - 3] << 16 | (uint8_t)memory[sp - 2] << 8 | (uint8_t)memory[sp - 1]);
            if(jle <= 0) {
                pc = address;
            }
            else {
                pc += 4;
            }
            break;
        case 0x8c: //call
            address = ((uint8_t)memory[pc + 1] << 16) | ((uint8_t)memory[pc + 2] << 8) | memory[pc + 3];
            if(!(address >= 0 && address <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            pc += 4;
            memory[sp] = (pc >> 24) & 0xFF;
            memory[sp + 1] = (pc >> 16) & 0xFF;
            memory[sp + 2] = (pc >> 8) & 0xFF;
            memory[sp + 3] = (pc) & 0xFF;
            sp += 4;
            pc = address;
            break;
        case 0x8d: //return
            sp -= 4;
            int32_t ret = ((uint8_t)memory[sp] << 24 | (uint8_t)memory[sp + 1] << 16 | (uint8_t)memory[sp + 2] << 8 | (int32_t)memory[sp + 3]);
            if(!(ret >= 0 && ret <= memSize)) {
                fprintf(stderr, "Simulation error\n");
                exit(1);
            }
            pc = ret;
            break;
        case 0x8e: //halt
            halt = true;
            break;
        default: 
            fprintf(stderr, "Simulation error\n");
            exit(1);
            break;
        }

        //printf("pc %d, sp %d\n", pc, sp);
    }
    if(!halt) {
        fprintf(stderr, "Simulation error\n");
        exit(1);
    }
    //printf("memory[0x40000]: %d\n", memory[0x40000]);
}