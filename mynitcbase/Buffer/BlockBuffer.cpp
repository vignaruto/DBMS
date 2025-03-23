#include "BlockBuffer.h"

#include <cstdlib>
#include <cstring>

// the declarations for these functions can be found in "BlockBuffer.h"
BlockBuffer::BlockBuffer(int blockNum) {
    this->blockNum = blockNum;
    // initialise this.blockNum with the argument
}

BlockBuffer::BlockBuffer(char blockType){
    int blockTypeNum;
    if (blockType == 'R')
    {
        blockTypeNum = REC;
    }
    else if (blockType == 'I')
    {
        blockTypeNum = IND_INTERNAL;
    }
    else if (blockType == 'L')
    {
        blockTypeNum = IND_LEAF;
    }
    else
    {
        blockTypeNum = UNUSED_BLK;
    }
    int blockNum = getFreeBlock(blockTypeNum);

    this->blockNum = blockNum;
    if (blockNum < 0 || blockNum >= DISK_BLOCKS)
        return;
}

// calls the parent class constructor
RecBuffer::RecBuffer(int blockNum) : BlockBuffer(blockNum) {}

RecBuffer::RecBuffer() : BlockBuffer('R'){}
// call parent non-default constructor with 'R' denoting record block.

int BlockBuffer::getBlockNum(){
  return this->blockNum;
    //return corresponding block number.
}

// call the corresponding parent constructor
IndBuffer::IndBuffer(char blockType) : BlockBuffer(blockType) {}

// call the corresponding parent constructor
IndBuffer::IndBuffer(int blockNum) : BlockBuffer(blockNum) {}

IndInternal::IndInternal() : IndBuffer('I') {}
// call the corresponding parent constructor
// 'I' used to denote IndInternal.

IndInternal::IndInternal(int blockNum) : IndBuffer(blockNum) {}
// call the corresponding parent constructor

IndLeaf::IndLeaf() : IndBuffer('L') {} // this is the way to call parent non-default constructor.
                                       // 'L' used to denote IndLeaf.

// this is the way to call parent non-default constructor.
IndLeaf::IndLeaf(int blockNum) : IndBuffer(blockNum) {}

// stage-2

// load the block header into the argument pointer
// int BlockBuffer::getHeader(struct HeadInfo *head) {
//     unsigned char buffer[BLOCK_SIZE];

//     // read the block at this.blockNum into the buffer
//     Disk::readBlock(buffer, this->blockNum);  // Fixed argument order

//     // populate the numEntries, numAttrs and numSlots fields in *head
//     memcpy(&head->numSlots, buffer + 24, 4);
//     memcpy(&head->numEntries, buffer + 16, 4);
//     memcpy(&head->numAttrs, buffer + 20, 4);
//     memcpy(&head->rblock, buffer + 12, 4);
//     memcpy(&head->lblock, buffer + 8, 4);

//     return SUCCESS;
// }

// // load the record at slotNum into the argument pointer
// int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
//     struct HeadInfo head;

//     // get the header using this.getHeader() function
//     getHeader(&head);

//     int attrCount = head.numAttrs;
//     int slotCount = head.numSlots;

//     // read the block at this.blockNum into a buffer
//     unsigned char buffer[BLOCK_SIZE];
//     Disk::readBlock(buffer, this->blockNum);  // Fixed argument order

//     int slotMapSize = slotCount; // assuming 1 byte per slot
//     int recordSize = attrCount * ATTR_SIZE;
//     unsigned char *slotPointer = buffer + HEADER_SIZE + slotMapSize + (recordSize * slotNum);

//     // load the record into the rec data structure
//     memcpy(rec, slotPointer, recordSize);

//     return SUCCESS;
// }



//stage-3

/*
Used to get the header of the block into the location pointed to by `head`
NOTE: this function expects the caller to allocate memory for `head`
*/
int BlockBuffer::getHeader(struct HeadInfo *head) {

  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;   // return any errors that might have occured in the process
  }

  // ... (the rest of the logic is as in stage 2)

  memcpy(&head->numSlots, bufferPtr + 24, 4);
  memcpy(&head->numEntries,bufferPtr+16,4);
  memcpy(&head->numAttrs,bufferPtr+20,4);
  memcpy(&head->rblock,bufferPtr+12,4);
  memcpy(&head->lblock,bufferPtr+8,4);

  return SUCCESS;

}

/*
Used to get the record at slot `slotNum` into the array `rec`
NOTE: this function expects the caller to allocate memory for `rec`
*/
int RecBuffer::getRecord(union Attribute *rec, int slotNum) {
  struct HeadInfo head;

  this->getHeader(&head);

  int attrCount = head.numAttrs;
  int slotCount = head.numSlots;

  unsigned char *bufferPtr;
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }
  // ... (the rest of the logic is as in stage 2
  int recordSize = attrCount * ATTR_SIZE;
  unsigned char *slotPointer = bufferPtr+HEADER_SIZE+slotCount+(recordSize*slotNum)/* calculate buffer + offset */;

  // load the record into the rec data structure
  memcpy(rec, slotPointer, recordSize);

  return SUCCESS;

}

/*
Used to load a block to the buffer and get a pointer to it.
NOTE: this function expects the caller to allocate memory for the argument
*/
// stage-3
// int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char **buffPtr) {
//   // check whether the block is already present in the buffer using StaticBuffer.getBufferNum()
//   int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

//   if (bufferNum == E_BLOCKNOTINBUFFER) {
//     bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

//     if (bufferNum == E_OUTOFBOUND) {
//       return E_OUTOFBOUND;
//     }

//     Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
//   }

//   // store the pointer to this buffer (blocks[bufferNum]) in *buffPtr
//   *buffPtr = StaticBuffer::blocks[bufferNum];

//   return SUCCESS;
// }

//stage-6

/* NOTE: This function will NOT check if the block has been initialised as a
   record or an index block. It will copy whatever content is there in that
   disk block to the buffer.
   Also ensure that all the methods accessing and updating the block's data
   should call the loadBlockAndGetBufferPtr() function before the access or
   update is done. This is because the block might not be present in the
   buffer due to LRU buffer replacement. So, it will need to be bought back
   to the buffer before any operations can be done.
 */
int BlockBuffer::loadBlockAndGetBufferPtr(unsigned char ** buffPtr) {
    int bufferNum = StaticBuffer::getBufferNum(this->blockNum);

    if (bufferNum == E_BLOCKNOTINBUFFER)
    {
        bufferNum = StaticBuffer::getFreeBuffer(this->blockNum);

        if (bufferNum == E_OUTOFBOUND)
            return E_OUTOFBOUND;

        Disk::readBlock(StaticBuffer::blocks[bufferNum], this->blockNum);
    }
    else
    {
        for (int i = 0; i < BUFFER_CAPACITY; i++)
        {
            if (!StaticBuffer::metainfo[i].free)
                StaticBuffer::metainfo[i].timeStamp++;
        }

        StaticBuffer::metainfo[bufferNum].timeStamp = 0;
    }

    *buffPtr = StaticBuffer::blocks[bufferNum];

    return SUCCESS;
}

int RecBuffer::setRecord(union Attribute *rec, int slotNum) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    // return the value returned by the call.
    if (ret != SUCCESS)
    {
        return ret;
    }

    /* get the header of the block using the getHeader() function */
    struct HeadInfo head;
    this->getHeader(&head);

    // get number of attributes in the block.
    int numAttrs = head.numAttrs;

    // get the number of slots in the block.
    int numSlots = head.numSlots;

    // if input slotNum is not in the permitted range return E_OUTOFBOUND.
    if (slotNum >= numSlots || slotNum < 0)
    {
        return E_OUTOFBOUND;
    }

    /* offset bufferPtr to point to the beginning of the record at required
       slot. the block contains the header, the slotmap, followed by all
       the records. so, for example,
       record at slot x will be at bufferPtr + HEADER_SIZE + (x*recordSize)
       copy the record from `rec` to buffer using memcpy
       (hint: a record will be of size ATTR_SIZE * numAttrs)
    */
    int recordSize = numAttrs * ATTR_SIZE;
    int offset = HEADER_SIZE + numSlots + (recordSize * slotNum);
    memcpy(bufferPtr + offset, rec, recordSize);

    // update dirty bit using setDirtyBit()
    StaticBuffer::setDirtyBit(this->blockNum);

    /* (the above function call should not fail since the block is already
       in buffer and the blockNum is valid. If the call does fail, there
       exists some other issue in the code) */

    // return SUCCESS
    return SUCCESS;
}

// /* used to get the slotmap from a record block
// NOTE: this function expects the caller to allocate memory for `*slotMap`
// */
int RecBuffer::getSlotMap(unsigned char *slotMap) {
  unsigned char *bufferPtr;

  // get the starting address of the buffer containing the block using loadBlockAndGetBufferPtr().
  int ret = loadBlockAndGetBufferPtr(&bufferPtr);
  if (ret != SUCCESS) {
    return ret;
  }

  struct HeadInfo head;
  // get the header of the block using getHeader() function
  getHeader(&head);

  int slotCount = head.numSlots/* number of slots in block from header */;

  // get a pointer to the beginning of the slotmap in memory by offsetting HEADER_SIZE
  unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;

  // copy the values from `slotMapInBuffer` to `slotMap` (size is `slotCount`)
  memcpy(slotMap,slotMapInBuffer,slotCount);

  return SUCCESS;
}

int RecBuffer::setSlotMap(unsigned char *slotMap) {
    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block using
       loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret=loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    if(ret!=SUCCESS){
      return ret;
    }

    // get the header of the block using the getHeader() function
    HeadInfo head;
    this->getHeader(&head);

    int numSlots = head.numSlots;

    // the slotmap starts at bufferPtr + HEADER_SIZE. Copy the contents of the
    // argument `slotMap` to the buffer replacing the existing slotmap.
    // Note that size of slotmap is `numSlots`
    unsigned char *slotMapInBuffer = bufferPtr + HEADER_SIZE;
    memcpy(slotMapInBuffer, slotMap, numSlots);

    // update dirty bit using StaticBuffer::setDirtyBit
    // if setDirtyBit failed, return the value returned by the call
    return StaticBuffer::setDirtyBit(this->blockNum);
}

int compareAttrs(union Attribute attr1, union Attribute attr2, int attrType) {
    
    StaticBuffer::cmpattrs++;
    if (attrType == STRING) {
        return strcmp(attr1.sVal, attr2.sVal);
    } else {
        double diff = attr1.nVal - attr2.nVal;
        if (diff > 0) return 1;
        if (diff < 0) return -1;
        return 0;
    }
}

int BlockBuffer::setHeader(struct HeadInfo *head){

    unsigned char *bufferPtr;
    // get the starting address of the buffer containing the block using
    // loadBlockAndGetBufferPtr(&bufferPtr).
    int ret=loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
        if(ret!=SUCCESS){
          return ret;
        }

    // cast bufferPtr to type HeadInfo*
    struct HeadInfo *bufferHeader = (struct HeadInfo *)bufferPtr;

    // copy the fields of the HeadInfo pointed to by head (except reserved) to
    // the header of the block (pointed to by bufferHeader)
    //(hint: bufferHeader->numSlots = head->numSlots )
    bufferHeader->numEntries=head->numEntries;
    bufferHeader->numAttrs=head->numAttrs;
    bufferHeader->pblock=head->pblock;
    bufferHeader->numSlots=head->numSlots;
    bufferHeader->rblock=head->rblock;
    bufferHeader->lblock=head->lblock;
   
    // update dirty bit by calling StaticBuffer::setDirtyBit()
    // if setDirtyBit() failed, return the error code

    // return SUCCESS;
    return StaticBuffer::setDirtyBit(this->blockNum);
}

int BlockBuffer::setBlockType(int blockType){

    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret=loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
        // return the value returned by the call.
    if(ret!=SUCCESS){
      return ret;
    }

    // store the input block type in the first 4 bytes of the buffer.
    // (hint: cast bufferPtr to int32_t* and then assign it)
    // *((int32_t *)bufferPtr) = blockType;
    *((int32_t *)bufferPtr) = blockType;
    // update the StaticBuffer::blockAllocMap entry corresponding to the
    // object's block number to `blockType`.
    StaticBuffer::blockAllocMap[this->blockNum]=blockType;
    // update dirty bit by calling StaticBuffer::setDirtyBit()
    // if setDirtyBit() failed
        // return the returned value from the call

    // return SUCCESS
    return StaticBuffer::setDirtyBit(this->blockNum);
}

int BlockBuffer::getFreeBlock(int blockType){

    // iterate through the StaticBuffer::blockAllocMap and find the block number
    // of a free block in the disk.
    int freeblock=-1;
    for(int i=0;i<DISK_BLOCKS;i++){
      if(StaticBuffer::blockAllocMap[i]==UNUSED_BLK){
        freeblock=i;
        break;
      }
    }

    // if no block is free, return E_DISKFULL.
    if(freeblock==-1){
      return E_DISKFULL;
    }

    // set the object's blockNum to the block number of the free block.
    this->blockNum=freeblock;

    // find a free buffer using StaticBuffer::getFreeBuffer() .
    int freebuffer=StaticBuffer::getFreeBuffer(freeblock);

    // initialize the header of the block passing a struct HeadInfo with values
    // pblock: -1, lblock: -1, rblock: -1, numEntries: 0, numAttrs: 0, numSlots: 0
    // to the setHeader() function.
    HeadInfo header;
    header.pblock = -1;
    header.lblock = -1;
    header.rblock = -1;
    header.numEntries = 0;
    header.numAttrs = 0;
    header.numSlots = 0;

    this->setHeader(&header);
    this->setBlockType(blockType);

    // update the block type of the block to the input block type using setBlockType().

    // return block number of the free block.
    return freeblock;
}

void BlockBuffer::releaseBlock()
{

    if (blockNum < 0 || blockNum >= DISK_BLOCKS || StaticBuffer::blockAllocMap[blockNum] == UNUSED_BLK)
    {
        return;
    }

    int bufferNum = StaticBuffer::getBufferNum(blockNum);

    if (bufferNum == E_BLOCKNOTINBUFFER)
    {
        return;
    }

    StaticBuffer::metainfo[bufferNum].free = true;

    StaticBuffer::blockAllocMap[blockNum] = UNUSED_BLK;

    this->blockNum = INVALID_BLOCKNUM;
}

int IndInternal::getEntry(void *ptr, int indexNum)
{
    // if the indexNum is not in the valid range of [0, MAX_KEYS_INTERNAL-1]
    //     return E_OUTOFBOUND.
    if (indexNum < 0 || indexNum >= MAX_KEYS_INTERNAL)
    {
        return E_OUTOFBOUND;
    }

    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    //     return the value returned by the call.
    if (ret != SUCCESS)
    {
        return ret;
    }

    // typecast the void pointer to an internal entry pointer
    struct InternalEntry *internalEntry = (struct InternalEntry *)ptr;

    /*
    - copy the entries from the indexNum`th entry to *internalEntry
    - make sure that each field is copied individually as in the following code
    - the lChild and rChild fields of InternalEntry are of type int32_t
    - int32_t is a type of int that is guaranteed to be 4 bytes across every
      C++ implementation. sizeof(int32_t) = 4
    */

    /* the indexNum'th entry will begin at an offset of
       HEADER_SIZE + (indexNum * (sizeof(int) + ATTR_SIZE) )         [why?]
       from bufferPtr */
    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * 20);

    memcpy(&(internalEntry->lChild), entryPtr, sizeof(int32_t));
    memcpy(&(internalEntry->attrVal), entryPtr + 4, sizeof(Attribute));
    memcpy(&(internalEntry->rChild), entryPtr + 20, sizeof(int32_t));

    // return SUCCESS.
    return SUCCESS;
}

int IndLeaf::getEntry(void *ptr, int indexNum)
{

    // if the indexNum is not in the valid range of [0, MAX_KEYS_LEAF-1]
    //     return E_OUTOFBOUND.
    if (indexNum < 0 || indexNum >= MAX_KEYS_LEAF)
    {
        return E_OUTOFBOUND;
    }

    unsigned char *bufferPtr;
    /* get the starting address of the buffer containing the block
       using loadBlockAndGetBufferPtr(&bufferPtr). */
    int ret = loadBlockAndGetBufferPtr(&bufferPtr);

    // if loadBlockAndGetBufferPtr(&bufferPtr) != SUCCESS
    //     return the value returned by the call.
    if (ret != SUCCESS)
    {
        return ret;
    }

    // copy the indexNum'th Index entry in buffer to memory ptr using memcpy

    /* the indexNum'th entry will begin at an offset of
       HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE)  from bufferPtr */
    unsigned char *entryPtr = bufferPtr + HEADER_SIZE + (indexNum * LEAF_ENTRY_SIZE);
    // memcpy((struct Index *)ptr, entryPtr, LEAF_ENTRY_SIZE);
    struct Index *index = (struct Index *)ptr;
    memcpy(&(index->attrVal), entryPtr, sizeof(Attribute));
    memcpy(&(index->block), entryPtr + 16, 4);
    memcpy(&(index->slot), entryPtr + 20, 4);

    // return SUCCESS
    return SUCCESS;
}

int IndInternal::setEntry(void *ptr, int indexNum) {
  return 0;
}

int IndLeaf::setEntry(void *ptr, int indexNum) {
  return 0;
}