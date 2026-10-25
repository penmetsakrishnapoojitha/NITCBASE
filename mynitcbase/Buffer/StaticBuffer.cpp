#include "StaticBuffer.h"
#include <climits>
unsigned char StaticBuffer::blocks[BUFFER_CAPACITY][BLOCK_SIZE];
struct BufferMetaInfo StaticBuffer::metainfo[BUFFER_CAPACITY];
unsigned char StaticBuffer::blockAllocMap[DISK_BLOCKS];
StaticBuffer::StaticBuffer(){
    for (int blockIndex = 0, blockAllocMapSlot = 0; blockIndex < 4; blockIndex++) {
		unsigned char buffer [BLOCK_SIZE];
		Disk::readBlock(buffer, blockIndex);

		for (int slot = 0; slot < BLOCK_SIZE; slot++, blockAllocMapSlot++)
			StaticBuffer::blockAllocMap[blockAllocMapSlot] = buffer[slot];
	}
    for(int i=0;i<BUFFER_CAPACITY;i++){
        metainfo[i].free=true;
        metainfo[i].dirty=false;
        metainfo[i].timeStamp=-1;
        metainfo[i].blockNum=-1;
    }
}
StaticBuffer::~StaticBuffer(){
   for (int blockIndex = 0, blockAllocMapSlot = 0; blockIndex < 4; blockIndex++) {
		unsigned char buffer [BLOCK_SIZE];

		for (int slot = 0; slot < BLOCK_SIZE; slot++, blockAllocMapSlot++){
	    buffer[slot]=blockAllocMap[blockAllocMapSlot];
    }
      Disk::writeBlock(buffer, blockIndex);

	}
  for(int i=0;i<BUFFER_CAPACITY;i++){
        if(metainfo[i].free==false&&metainfo[i].dirty==true){
          Disk::writeBlock(blocks[i], metainfo[i].blockNum);
        }
        
  }
}
int StaticBuffer::getFreeBuffer(int blockNum){
    if(blockNum<0||blockNum>=DISK_BLOCKS){
        return E_OUTOFBOUND;
    }
    int allocatedbuffer;
    int i;
    for( i=0;i<BUFFER_CAPACITY;i++){
      if(metainfo[i].free==true){
        allocatedbuffer=i;
        break;
      }
    }
if(i == BUFFER_CAPACITY) {

    int firstTimestamp = INT_MAX;
    int bufferNum = -1;

    for (int bufferIndex = 0; bufferIndex < BUFFER_CAPACITY; bufferIndex++) {
        if (metainfo[bufferIndex].timeStamp < firstTimestamp) {
            firstTimestamp = metainfo[bufferIndex].timeStamp;
            bufferNum = bufferIndex;
        }
    }

    allocatedbuffer = bufferNum;

    if (metainfo[allocatedbuffer].dirty == true) {
        Disk::writeBlock(
            StaticBuffer::blocks[allocatedbuffer],
            metainfo[allocatedbuffer].blockNum
        );
    }
}
    	metainfo[allocatedbuffer].free = false, 
	metainfo[allocatedbuffer].dirty = false,
	metainfo[allocatedbuffer].timeStamp = 0, 
	metainfo[allocatedbuffer].blockNum = blockNum;

	return allocatedbuffer;

}
int StaticBuffer::getBufferNum(int blockNum) {
     if(blockNum<0||blockNum>=DISK_BLOCKS){
        return E_OUTOFBOUND;
    }
    for(int i=0;i<BUFFER_CAPACITY;i++){
      if(metainfo[i].blockNum==blockNum){
         return i;
      }
    }
    return E_BLOCKNOTINBUFFER;
}
int StaticBuffer::setDirtyBit(int blockNum){
	int bufferIndex = getBufferNum(blockNum);
	if (bufferIndex == E_BLOCKNOTINBUFFER)
        return E_BLOCKNOTINBUFFER;
	if (bufferIndex == E_OUTOFBOUND)
        return E_OUTOFBOUND;
	metainfo[bufferIndex].dirty = true;

    return SUCCESS;
}
int StaticBuffer::getStaticBlockType(int blockNum){
    // Check if blockNum is valid (non zero and less than number of disk blocks)
    // and return E_OUTOFBOUND if not valid.

	if (blockNum < 0 || blockNum >= DISK_BLOCKS) return E_OUTOFBOUND;

    // Access the entry in block allocation map corresponding to the blockNum argument
    // and return the block type after type casting to integer.
	return (int)blockAllocMap[blockNum];
}