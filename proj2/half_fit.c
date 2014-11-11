#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "half_fit.h"
#define NUM_BUCKETS 11

unsigned char memory_init[32768] __attribute__ ((section(".ARM.__at_0x10000000"), zero_init));

typedef struct half_struct {
	unsigned int *memory;
	unsigned int buckets[NUM_BUCKETS];
	unsigned int bucketSize[NUM_BUCKETS];
} half_struct_t;

half_struct_t half_str;

// Get the previous address of the chunk at the specified index
unsigned int getPrev(int index) {
	return (half_str.memory[index] >> 22) << 3;
}

// Get the previous address of the chunk at the specified pointer
unsigned int getPrevPointer(unsigned int* pointer) {
	return ((*pointer) >> 22) << 3;
}

// Set the previous address of the chunk at the specified index
void setPrev(int index, unsigned int address) {
	address /= 8;
	half_str.memory[index] = (half_str.memory[index] & 4194303u) + (address << 22);
}

// Get the next address of the chunk at the specified index
unsigned int getNext(int index) {
	return ((half_str.memory[index] & 4190208u) >> 12) << 3;
}

// Set the next address of the chunk at the specified index
void setNext(int index, unsigned int address) {
	address /= 8;
	half_str.memory[index] = (half_str.memory[index] & 4290777087u) + (address << 12);
}

// Get the size of the chunk at the specified index
unsigned int getSize(int index) {
	return ((half_str.memory[index] & 4092) >> 2) + 1;
}

// Set the size of the chunk at the specified index
void setSize(int index, unsigned int address) {
	half_str.memory[index] = (half_str.memory[index] & 4294963203u) + ((address - 1) << 2);
}

// Returns whether the chunk at the specified index is allocated
bool isAllocated(int index) {
	return (half_str.memory[index] & 2) == 2;
}

// Sets whether the chunk at the specified index is allocated
void setAllocated(int index, bool allocated) {
	half_str.memory[index] = (half_str.memory[index] & 4294967293u) + ((allocated ? 1 : 0) << 1);
}

// Used to find the bucket index when trying to find a chunk in a bucket of at least the size specified
unsigned int half_bucket_request(int size) {
	unsigned int amountOfOnes = 0;
	unsigned int index = 0;
	while(size > 1) {
		if((size & 1) != 0) {
			amountOfOnes++;
		}
		size = size >> 1;
		index++;
	}
	if(amountOfOnes > 1) {
		index++;
	}
	return index;
}

// Used to find the bucket index when trying to figure out which bucket a chunk of the specified size belongs to
unsigned int half_bucket(int size) {
	unsigned int index = 0;
	while(size > 1) {
		size = size >> 1;
		index++;
	}
	return index;
}

void  half_init( void ) {
	int i;
	// Set all bucket sizes to 0 except the last one
	for(i = 0; i < NUM_BUCKETS - 1; i++) {
		half_str.bucketSize[i] = 0;
	}
	
	// Finds the pointer for the allocated memory chunk and treats it as 4 byte (unsigned integer) chunks
	half_str.memory = (unsigned int*)memory_init;
	
	// Sets the header data for the first chunk
	half_str.memory[0] = 1023 << 2;
	half_str.memory[1] = 0;
	half_str.bucketSize[NUM_BUCKETS-1] = 1;
}

void half_free_index(int index) {
	unsigned int prev;
	unsigned int next;
	unsigned int bucketIndex;
	
	//Merge with neighbour chunks if possible
	//If there is a previous unallocated chunck
	prev = getPrev(index);
	if(prev != index && !isAllocated(prev)) {
		unsigned int nextB;
		unsigned int prevB;
		//Remove prev from bucket
		prevB = getPrev(prev + 1);
		nextB = getNext(prev + 1);
		bucketIndex = half_bucket(getSize(prevB));
		if(prevB == prev) { //If previous in bucket does not exist
			half_str.buckets[bucketIndex] = nextB;
			setPrev(nextB + 1, nextB);
		}
		else { //If previous in bucket exists
			if(nextB == prev) {//If next in bucket does not exist
				setNext(prevB + 1, prevB);
			}
			else {//If next in bucket exists
				setNext(prevB + 1, nextB);
				setPrev(nextB + 1, prevB);
			}
		}
		half_str.bucketSize[bucketIndex]--;
		
		//Merge with previous chunck
		next = getNext(index);
		if(next != index) {
			setPrev(next, prev);
			setNext(prev, next);
		}
		else {
			setNext(prev, prev);
		}
		setSize(prev, getSize(prev) + getSize(index));
		index = prev;
	}
	
	//If there is a next unallocated chunk
	next = getNext(index);
	if(next != index && !isAllocated(next)) {
		unsigned int nextB;
		unsigned int prevB;
		unsigned int afterNext;
		//Remove next from bucket
		prevB = getPrev(next + 1);
		nextB = getNext(next + 1);
		bucketIndex = half_bucket(getSize(prevB));
		if(prevB == next) { //If previous in bucket does not exist
			half_str.buckets[bucketIndex] = nextB;
			setPrev(nextB + 1, nextB);
		}
		else { //If previous in bucket exists
			if(nextB == next) { //If next in bucket does not exist
				setNext(prevB + 1, prevB);
			}
			else { //If next in bucket exists
				setNext(prevB + 1, nextB);
				setPrev(nextB + 1, prevB);
			}
		}
		half_str.bucketSize[bucketIndex]--;
		
		//Merge with next chunck
		afterNext = getNext(next);
		if(afterNext != next) {
			setPrev(afterNext, index);
			setNext(index, afterNext);
		}
		else {
			setNext(index, index);
		}
		setSize(index, getSize(index) + getSize(next));
	}
	setAllocated(index, false);
	
	//Put into correct bucket
	bucketIndex = half_bucket(getSize(index));
	if(half_str.bucketSize[bucketIndex] == 0) {
		setNext(index + 1, index);
	}
	else {
		next = half_str.buckets[bucketIndex];
		setPrev(next + 1, index);
		setNext(index + 1, next);
	}
	half_str.buckets[bucketIndex] = index;
	setPrev(index + 1, index);
	half_str.bucketSize[bucketIndex]++;
}

void *half_alloc( int size ) {
	unsigned int index;
	unsigned int next;
	unsigned int leftOverSize;
	int i;
	
	if(size > 32764) {
		return NULL;
	}
	
	// Adds 4 bytes to the size and finds the amount of 32 byte chunks needed
	size += 4;
	size = size/32 + (size % 32 == 0 ? 0 : 1);
	//size now contains the number of 32 byte blocks needed
	index = half_bucket_request(size);
	
	//Finds the index in memory of the available block
	for(i = index; i < NUM_BUCKETS; i++) {
		if(half_str.bucketSize[i] > 0) {
			index = half_str.buckets[i];
			break;
		}
		else if(i == NUM_BUCKETS - 1) {
			return NULL;
		}
	}
	setAllocated(index, true);
	next = getNext(index + 1);
	//If there is a next block in the bucket
	if(next != index) {
		setPrev(next + 1, next);
	}
	half_str.bucketSize[i]--;
	if(half_str.bucketSize[i] > 0) {
		half_str.buckets[i] = next;
	}
	leftOverSize = getSize(index) - size;
	//If there is enough leftover space to split into two chunks
	if(leftOverSize > 0) {
		unsigned int leftOver = 0;
		setSize(index, size);
		leftOver = index + size*8;
		setSize(leftOver, leftOverSize);
		setPrev(leftOver, index);
		next = getNext(index);
		//if there is a next element
		if(next != index) {
			setNext(leftOver, next);
			setPrev(next, leftOver);
		}
		else {
			setNext(leftOver, leftOver);
		}
		setNext(index, leftOver);
		half_free_index(leftOver);
	}
	return (void *)(&half_str.memory[index + 1]);
}

// Finds the index for the given pointer by looking at its previous address
// Then frees the memory by calling half_free_index
void  half_free( void * pointer ) {
	unsigned int prev;
	unsigned int current;
	unsigned int *toFree;
	if(pointer == NULL) {
		return;
	}
	toFree = (unsigned int*)pointer;
	toFree--;
	prev = getPrevPointer(toFree);
	if(&(half_str.memory[prev]) == toFree) {
		current = prev;
	}
	else {
		current = getNext(prev);
	}
	half_free_index(current);
}