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
	
	int word = block_size/4;
	

	//off = addr & int(block_size/4);
	off = addr & (int)(word);
	ind = (addr >> off) & 0x3;

	int off_ind = off + ind;
	tag = (addr >> off_ind) & 0x26;
	// tag = (addr >> (off + ind)) addr & 0x26;


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

//We haven't actually used this yet, I'll have to see how we use it..
TransferUnit getByte() {
	//Convert block_size variable to Transfer unit enum
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
			//Check if addr is in cache
			for (int i = 0; i < assoc; i++) {
				if(cache[i].block[index].valid == VALID) { //Only Check valid blocks
					if (tag == cache[i].block[index].tag) { //Check to if tags match
						//hit
						hit = true;
						highlight_offset(i, index, offset, HIT);
						memcpy(data, cache[i].block[index].data + offset, 4); //Read data from block, 4 bytes = word
						//update LRU
						cache[i].block[index].accessCount += 1;
						break;
					} 
				}
			}

			if (!hit) { //If miss
				TransferUnit mode = WORD_SIZE; //We want to transfer a word's worth of data to the cache from DRAM
				accessDRAM(addr, data, mode, READ); //Read data from DRAM into data variable
				int replace = replacementPolicy(index); //return set number according to policy
				highlight_offset(replace, index, offset, MISS); //Highlight new block of data in cache
				memcpy(cache[replace].block[index].data + offset, data, 4); //Copy info in data variable to our block + offset

				//Update block to reflect that data is in it
				cache[replace].block[index].accessCount = 1; //Update LRU

				cache[replace].block[index].valid = VALID;
        
				cache[replace].block[index].tag = tag;
			}
			return; //end method

		// case WriteEnable.WRITE:
		case WRITE:
			//Check if addr is in cache and write to it first
			for (int i = 0; i < assoc; i++) {
				if(cache[i].block[index].valid == VALID) { //Only Check valid blocks
					if (tag == cache[i].block[index].tag) { //Check to if tags match
						//hit
						hit = true;
						highlight_offset(i, index, offset, HIT);
						memcpy(cache[i].block[index].data + offset, data, 4); //Write data into block/offset word
						//update LRU
						cache[i].block[index].accessCount = 1;
						break;
					} 
				}
			}

			if(!hit){ //If we missed
				int replace = replacementPolicy(index); //return set number according to policy
				highlight_offset(replace, index, offset, MISS); //Highlight new block of data in cache
				memcpy(cache[replace].block[index].data + offset, data, 4); //Place data in selected cache block

				//Update block to reflect that data is in it
				cache[replace].block[index].accessCount = 1; //Update LRU

				cache[replace].block[index].valid = VALID;

				cache[replace].block[index].tag = tag;
			}

			TransferUnit mode = WORD_SIZE; //Transfer word to our DRAM
			accessDRAM(addr, data, mode, WRITE); //Write data to memory normally now
			return; //end method
	}
}