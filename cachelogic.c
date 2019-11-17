#include "tips.h"
#include <math.h>

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
}

//Helper functions
struct addy getBytes(address addr) { 
	//Convert address to our addy struct

}

int replacementPolicy(unsigned int index) { //return set number
	//Create replacement policy based on lru, lfu & rand

}

void lruUpdate(int index, int block) {
	//

}

TransferUnit getByte() {
	//Convert block_size variable to Transfer unit enum
	switch(block_size){
		case 4: //1 word
			return TransferUnit.WORD_SIZE;

		case 8:
			return TransferUnit.DOUBLEWORD_SIZE;

		case 16:
			return TransferUnit.QUADWORD_SIZE;

		case 32:
			return TransferUnit.OCTOWORD_SIZE;
	}

	//default
	return TransferUnit.WORD_SIZE;
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
	unsigned int offset = theAddr.offset; //Address struct to simplify code
	unsigned int index = theAddr.index;
	unsigned int tag = theAddr.tag;
	bool hit = false; 
	TransferUnit b = getByte(); //helper function used to...

	switch (we) {
		//Read
		case WriteEnable.READ:
			//Cache[index] -> block[association] -> cacheBlock

			for (int i = 0; i < assoc; i++) {
				//Read HIT
				//Valid bit to see if data is valid (Not valid of start up)
				//Note: Dirty bit is used for write back. 
				if(cache[i].block[index].valid == VALID) {
					//The tags match
					if (tag == cache[i].block[index].tag) {
						//Find correct Block(Use offset)
						//change to memcpy
						highlight_offset(i, index, offset, HIT);
						memcpy((void*)data, (void*)cache[i].block[index].data + offset, 4);
						cache[i].block[index].accessCount += 1;
						lruUpdate(i, index);
						hit = true; 
					} 
				}
			}
			//Read MISS
			//Need to go to main memory to cache
			if (!hit) {
				int replace = replacementPolicy(index); //return set number
				highlight_offset(replace, index, offset, MISS);
				//MAKE INTO A FUNCTION WILL USE FOR WRITE
				//Decide how much memory to get from the physical memory (Block size)
				//Search for an invalid bit in the set to physical memory to
				cache[replace].block[index].accessCount += 1;
				lruUpdate(replace, index);
				cache[replace].block[index].valid = 1;
				cache[replace].block[index].tag = tag;
				//Access main memory 
				accessDRAM(addr, (byte*) cache[replace].block[index].data, b, READ);
				highlight_block(replace, index);
				//Then read cache 
				memcpy((void*) data, (void*) cache[replace].block[index].data + offset, 4);
			}
			break;

		case WriteEnable.WRITE:
			//Write code

			break;
	}
	/* This call to accessDRAM occurs when you modify any of the
		 cache parameters. It is provided as a stop gap solution.
		 At some point, ONCE YOU HAVE MORE OF YOUR CACHELOGIC IN PLACE,
		 THIS LINE SHOULD BE REMOVED.
	*/
	accessDRAM(addr, (byte*)data, WORD_SIZE, we);
}