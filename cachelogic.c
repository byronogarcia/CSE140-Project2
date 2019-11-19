#include "tips.h"
#include <math.h>
#include <stdbool.h>

/* The following two functions are defined in util.c */

/* finds the highest 1 bit, and returns its position, else 0xFFFFFFFF */
unsigned int uint_log2(word w);

/* return random int from 0..x-1 */
int randomint( int x );

/*
 This function allows the lfu information to be displayed
 assoc_index - the cache unit that contains the block to be modified
 block_index - the index of the block to be modified
 returns a string representation of the lfu information
 */
char* lfu_to_string(int assoc_index, int block_index)
{
    /* Buffer to print lfu information -- increase size as needed. */
    static char buffer[9];
    sprintf(buffer, "%u", cache[assoc_index].block[block_index].accessCount);
    
    return buffer;
}

/*
 This function allows the lru information to be displayed
 assoc_index - the cache unit that contains the block to be modified
 block_index - the index of the block to be modified
 returns a string representation of the lru information
 */
char* lru_to_string(int assoc_index, int block_index)
{
    /* Buffer to print lru information -- increase size as needed. */
    static char buffer[9];
    sprintf(buffer, "%u", cache[assoc_index].block[block_index].lru.value);
    
    return buffer;
}

/*
 This function initializes the lfu information
 assoc_index - the cache unit that contains the block to be modified
 block_number - the index of the block to be modified
 */
void init_lfu(int assoc_index, int block_index)
{
    cache[assoc_index].block[block_index].accessCount = 0;
}

/*
 This function initializes the lru information
 assoc_index - the cache unit that contains the block to be modified
 block_number - the index of the block to be modified
 */
void init_lru(int assoc_index, int block_index)
{
    cache[assoc_index].block[block_index].lru.value = 0;
}

/*
 This is the primary function you are filling out,
 You are free to add helper functions if you need them
 @param addr 32-bit byte address
 @param data a pointer to a SINGLE word (32-bits of data)
 @param we   if we == READ, then data used to return
 information back to CPU
 if we == WRITE, then data used to
 update Cache/DRAM
 */

//Helper structs
struct addy {
    unsigned int offset;
    unsigned int index;
    unsigned int tag;
};

//Helper functions
struct addy getBytes(address addr) {
    //Convert address to our addy struct
    unsigned int off = 0;
    unsigned int ind = 0;
    unsigned int tag = 0;
    
    // //offset will contain 3 bits.... last 3 bits
    // off = addr & (0x7);
    
    // //offset will contain the next 4 bits
    // ind = (addr >> 3) & 0xF;
    
    // //Tag will be the upper 25 bits
    // tag = (addr >> 7) & 0x1FFFFFF;
    
    if (block_size == 4) {
        off = 0x3 & addr;
        addr = addr >> 2;
    }
    else if (block_size == 8) {
        off = 0x7 & addr;
        addr = addr >> 3;
    }
    else if (block_size == 16) {
        off = 0xF & addr;
        addr = addr >> 4;
    }
    else if (block_size == 32) {
        off = 0x1F & addr;
        addr = addr >> 5;
    }
    
    // INDEX
    if (set_count == 1) {
        ind = 0;
    }
    else if (set_count == 2) {
        ind = 0x1 & addr;
        addr = addr >> 1;
    }
    else if (set_count == 4) {
        ind = 0x3 & addr;
        addr = addr >> 2;
    }
    else if (set_count == 8) {
        ind = 0x7 & addr;
        addr = addr >> 3;
    }
    else if (set_count == 16) {
        ind = 0xF & addr;
        addr = addr >> 4;
    }
    
    // TAG
    tag = addr;
    
    struct addy holder = {off, ind, tag};
    return holder;
}

int replacementPolicy(unsigned int index) {
    switch(policy){
        case 1: //LRU
        {
            // case ReplacementPolicy.LRU:
            int replace = 0;
            unsigned min_count = cache[0].block[index].accessCount;
            
            for(int i = 1; i < assoc; i++){
                if(min_count > cache[i].block[index].accessCount){
                    min_count = cache[i].block[index].accessCount;
                    replace = i;
                }
            }
            
            return replace;
        }
            
        case 0://RANDOM
        {
            // case ReplacementPolicy.RANDOM:
            int replace = randomint(assoc); //Generate a random number between 0 and set_count -1
            
            return replace;
        }
            
        case 2: {
            
        }
    }
}


TransferUnit getBlock() {
    //This helper function turns our block_size global variable to TransferUnit enum
    switch(block_size){
        case 4: //1 word
            // return TransferUnit.WORD_SIZE;
            return WORD_SIZE;
            
        case 8:
            // return TransferUnit.DOUBLEWORD_SIZE;
            return DOUBLEWORD_SIZE;
            
        case 16:
            // return TransferUnit.QUADWORD_SIZE;
            return QUADWORD_SIZE;
            
        case 32:
            // return TransferUnit.OCTWORD_SIZE;
            return OCTWORD_SIZE;
    }
    
    //default
    // return TransferUnit.WORD_SIZE;
    return WORD_SIZE;
}

void accessMemory(address addr, word* data, WriteEnable we){
    /* Declare variables here */
    
    /* handle the case of no cache at all - leave this in */
    if(assoc == 0) {
        accessDRAM(addr, (byte*)data, WORD_SIZE, we);
        return;
    }
    /*
     You need to read/write between memory (via the accessDRAM() function) and
     the cache (via the cache[] global structure defined in tips.h)
     Remember to read tips.h for all the global variables that tell you the
     cache parameters
     The same code should handle random, LFU, and LRU policies. Test the policy
     variable (see tips.h) to decide which policy to execute. The LRU policy
     should be written such that no two blocks (when their valid bit is VALID)
     will ever be a candidate for replacement. In the case of a tie in the
     least number of accesses for LFU, you use the LRU information to determine
     which block to replace.
     Your cache should be able to support write-through mode (any writes to
     the cache get immediately copied to main memory also) and write-back mode
     (and writes to the cache only gets copied to main memory when the block
     is kicked out of the cache.
     Also, cache should do allocate-on-write. This means, a write operation
     will bring in an entire block if the block is not already in the cache.
     To properly work with the GUI, the code needs to tell the GUI code
     when to redraw and when to flash things. Descriptions of the animation
     functions can be found in tips.h
     */
    
    /* Start adding code here */
    struct addy theAddr = getBytes(addr); //Convert address into struct to simplify code
    unsigned int offset = theAddr.offset;
    unsigned int index = theAddr.index;
    unsigned int tag = theAddr.tag;
    bool hit = false;
    
    switch (we) {
            // case WriteEnable.READ:
        case READ:
            if(memory_sync_policy == WRITE_THROUGH){
                //Check if addr is in cache
                for (int i = 0; i < assoc; i++) {
                    if(cache[index].block[i].valid == VALID) { //Only Check valid blocks
                        if (tag == cache[index].block[i].tag) { //Check to if tags match
                            //hit
                            hit = true;
                            highlight_offset(index, i, offset, HIT);
                            memcpy(data, cache[index].block[i].data + offset, 4); //Read data from block, 4 bytes = word
                            //update LRU
                            cache[index].block[i].accessCount += 1;
                            cache[index].block[i].lru.value += 1;
                            break;
                        }
                    }
                }
                
                if (!hit) { //If miss
                    TransferUnit mode = getBlock(); //We want to transfer a block's worth of data to the cache from DRAM
                    accessDRAM(addr, data, mode, READ); //Read data from DRAM into data variable
                    int replace = replacementPolicy(index); //return assoc according to policy
                    highlight_offset(index, replace, offset, MISS); //Highlight new block of data in cache
                    memcpy(cache[index].block[replace].data, data, block_size); //Copy info in data variable to our block + offset
                    
                    //Update block to reflect that data is in it
                    cache[index].block[replace].accessCount = 1; //Update LRU
                    cache[index].block[replace].lru.value = 1;
                    
                    cache[index].block[replace].valid = VALID;
                    cache[index].block[replace].tag = tag;
                    
                }
                return; //end method
            } else if(memory_sync_policy == WRITE_BACK){
                //Check if addr is in cache
                for (int i = 0; i < assoc; i++) {
                    if(cache[index].block[i].valid == VALID) { //Only Check valid blocks
                        if (tag == cache[index].block[i].tag) { //Check to if tags match
                            //hit
                            hit = true;
                            highlight_offset(index, i, offset, HIT);
                            memcpy(data, cache[index].block[i].data + offset, 4); //Read data from block, 4 bytes = word
                            //update LRU
                            cache[index].block[i].accessCount += 1;
                            cache[index].block[i].lru.value += 1;
                            break;
                        }
                    }
                }
                
                if (!hit) { //If miss
                    int replace = replacementPolicy(index); //return set number according to policy
                    if(cache[index].block[replace].dirty == DIRTY){ //If the data we are to replace is dirty, upload it to DRAM
                        //Get block data
                        byte* block_data;
                        memcpy(block_data, cache[index].block[replace].data, block_size);
                        
                        TransferUnit mode = getBlock(); //Transfer entire block to DRAM
                        accessDRAM(addr, block_data, mode, WRITE); //Write data to memory normally now
                    }
                    TransferUnit mode = getBlock(); //We want to transfer a block's worth of data to the cache from DRAM
                    accessDRAM(addr, data, mode, READ); //Read data from DRAM into data variable
                    
                    highlight_offset(index, replace, offset, MISS); //Highlight new block of data in cache
                    memcpy(cache[index].block[replace].data, data, block_size); //Copy info in data variable to our block
                    
                    //Update block to reflect that data is in it
                    cache[index].block[replace].accessCount = 1; //Update LRU
                    cache[index].block[replace].lru.value = 1;
                    
                    cache[index].block[replace].valid = VALID;
                    cache[index].block[replace].tag = tag;
                    cache[index].block[replace].dirty = VIRGIN; //The data is no longer different than the DRAM's
                    
                }
                return; //end method
            }
            
            // case WriteEnable.WRITE:
        case WRITE:
            if(memory_sync_policy == WRITE_THROUGH){
                //Write Through policy
                //Check if addr is in cache and write to it first
                for (int i = 0; i < assoc; i++) {
                    if(cache[index].block[i].valid == VALID) { //Only Check valid blocks
                        if (tag == cache[index].block[i].tag) { //Check to if tags match
                            //hit
                            hit = true;
                            highlight_offset(index, i, offset, HIT);
                            memcpy(cache[index].block[i].data + offset, data, 4); //Write data into block/offset word
                            //update LRU
                            cache[index].block[i].accessCount = 1;
                            cache[index].block[i].lru.value = 1;
                            
                            //data should now hold block data for Write Through policy
                            memcpy(data, cache[index].block[i].data, block_size);
                            break;
                        }
                    }
                }
                
                //If we missed we write the data straight to DRAM, no cache operations needed
                TransferUnit mode = getBlock(); //Transfer entire block to DRAM
                accessDRAM(addr, data, mode, WRITE); //Write data to memory normally now
                return; //end method
                
            } else if(memory_sync_policy == WRITE_BACK){
                //Write Back policy
                //Check if addr is in cache and write to it
                for (int i = 0; i < assoc; i++) {
                    if(cache[index].block[i].valid == VALID) { //Only Check valid blocks
                        if (tag == cache[index].block[i].tag) { //Check if tags match
                            //hit
                            hit = true;
                            highlight_offset(index, i, offset, HIT);
                            memcpy(cache[index].block[i].data + offset, data, 4); //Write data into block/offset word
                            //update LRU
                            cache[index].block[i].accessCount = 1;
                            cache[index].block[i].lru.value = 1;
                            //Our data is now dirty!
                            cache[index].block[i].dirty = DIRTY;
                            break;
                        }
                    }
                }
                
                if(!hit){ //If we missed
                    int replace = replacementPolicy(index); //return set number according to policy
                    highlight_offset(index, replace, offset, MISS); //Highlight new block of data in cache
                    if(cache[index].block[replace].dirty == DIRTY){ //If the data we are to replace is dirty, upload it to DRAM
                        //Get block data
                        byte* block_data;
                        memcpy(block_data, cache[index].block[replace].data, block_size);
                        
                        TransferUnit mode = getBlock(); //Transfer entire block to DRAM
                        accessDRAM(addr, block_data, mode, WRITE); //Write data to memory normally now
                    }
                    
                    
                    memcpy(cache[index].block[replace].data + offset, data, 4); //Place data in selected cache block
                    
                    //Update block to reflect that data is in it
                    cache[index].block[replace].accessCount = 1; //Update LRU
                    cache[index].block[replace].lru.value = 1;
                    cache[index].block[replace].valid = VALID;
                    cache[index].block[replace].tag = tag;
                    cache[index].block[replace].dirty = DIRTY;
                }
                
                return; //end method
            }
    }
}
