#include "BlockAccess.h"
#include <cstring>

RecId BlockAccess::linearSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op) {
    // Get the previous search index of the relation relId from the relation cache
    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId, &prevRecId);

    int block, slot;

    // If the current search index record is invalid (both block and slot = -1)
    if (prevRecId.block == -1 && prevRecId.slot == -1) {
        RelCatEntry x;
        RelCacheTable::getRelCatEntry(relId, &x);
        block = x.firstBlk;
        slot = 0;
    } else {
        block = prevRecId.block;
        slot = prevRecId.slot + 1;
    }

    while (block != -1) {
        RecBuffer recbuffer(block);
        HeadInfo head;
        recbuffer.getHeader(&head);

        if (slot >= head.numSlots) {
            block = head.rblock;
            slot = 0;
            if (block < 0) break;  // Stop if invalid block
            continue;
        }

        unsigned char slotmap[head.numSlots];
        recbuffer.getSlotMap(slotmap);
        int numattrs = head.numAttrs;

        if (slot >= head.numSlots || slotmap[slot] == SLOT_UNOCCUPIED) {
            slot++;
            continue;
        }

        Attribute record[numattrs];
        recbuffer.getRecord(record, slot);

        // Get attribute cache entry
        AttrCatEntry attrcatentry;
        if (AttrCacheTable::getAttrCatEntry(relId, attrName, &attrcatentry) != SUCCESS) {
            return RecId{-1, -1};  // Attribute not found
        }

        int offset = attrcatentry.offset;
        if (offset < 0 || offset >= numattrs) {
            return RecId{-1, -1};  // Invalid offset
        }

        Attribute recordattrvalue = record[offset];

        int cmpVal;
        if(attrcatentry.attrType==STRING){
            cmpVal=compareAttrs(recordattrvalue,attrVal,STRING);
        }
        else{
            cmpVal=compareAttrs(recordattrvalue,attrVal,NUMBER);
        }

        if ((op == NE && cmpVal != 0) || (op == LT && cmpVal < 0) || 
            (op == LE && cmpVal <= 0) || (op == EQ && cmpVal == 0) || 
            (op == GT && cmpVal > 0) || (op == GE && cmpVal >= 0)) {

            prevRecId = RecId{block, slot};
            RelCacheTable::setSearchIndex(relId, &prevRecId);
            return RecId{block, slot};
        }

        slot++;
    }

    return RecId{-1, -1};
}

int BlockAccess::renameRelation(char oldName[ATTR_SIZE], char newName[ATTR_SIZE]){
    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute newRelationName;    // set newRelationName with newName
    strcpy(newRelationName.sVal,newName);
    // search the relation catalog for an entry with "RelName" = newRelationName
    RecId recid=linearSearch(RELCAT_RELID,(char *)RELCAT_ATTR_RELNAME,newRelationName,EQ);
    // If relation with name newName already exists (result of linearSearch
    //                                               is not {-1, -1})
    //    return E_RELEXIST;
    if(recid.block!=-1 && recid.slot!=-1){
        return E_RELEXIST;
    }


    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute oldRelationName;    // set oldRelationName with oldName
    strcpy(oldRelationName.sVal,oldName);
    // search the relation catalog for an entry with "RelName" = oldRelationName
    recid=linearSearch(RELCAT_RELID,(char *)RELCAT_ATTR_RELNAME,oldRelationName,EQ);
    // If relation with name oldName does not exist (result of linearSearch is {-1, -1})
    //    return E_RELNOTEXIST;
    if(recid.block==-1 && recid.slot==-1){
        return E_RELNOTEXIST;
    }
    /* get the relation catalog record of the relation to rename using a RecBuffer
       on the relation catalog [RELCAT_BLOCK] and RecBuffer.getRecord function
    */
   RecBuffer recbuffer(RELCAT_BLOCK);
   HeadInfo head;
   recbuffer.getHeader(&head);
   Attribute record[head.numAttrs];
   recbuffer.getRecord(record,recid.slot);
    /* update the relation name attribute in the record with newName.
       (use RELCAT_REL_NAME_INDEX) */
    // set back the record value using RecBuffer.setRecord
    strcpy(record[RELCAT_REL_NAME_INDEX].sVal,newName);
    recbuffer.setRecord(record,recid.slot);
    /*
    update all the attribute catalog entries in the attribute catalog corresponding
    to the relation with relation name oldName to the relation name newName
    */

    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    int numAttrs=record[RELCAT_NO_ATTRIBUTES_INDEX].nVal;
    //for i = 0 to numberOfAttributes :
    //    linearSearch on the attribute catalog for relName = oldRelationName
    //    get the record using RecBuffer.getRecord
    //
    //    update the relName field in the record to newName
    //    set back the record using RecBuffer.setRecord
    for(int i=0;i<numAttrs;i++){
        recid=linearSearch(ATTRCAT_RELID,(char *)ATTRCAT_ATTR_RELNAME,oldRelationName,EQ);
        RecBuffer recbuffer(recid.block);
        recbuffer.getRecord(record,recid.slot);
        strcpy(record[ATTRCAT_REL_NAME_INDEX].sVal,newName);
        recbuffer.setRecord(record,recid.slot);
    }

    return SUCCESS;
}

int BlockAccess::renameAttribute(char relName[ATTR_SIZE], char oldName[ATTR_SIZE], char newName[ATTR_SIZE]) {

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute relNameAttr;    // set relNameAttr to relName
    strcpy(relNameAttr.sVal,relName);
    // Search for the relation with name relName in relation catalog using linearSearch()
    RecId recid=linearSearch(RELCAT_RELID,(char *)RELCAT_ATTR_RELNAME,relNameAttr,EQ);
    // If relation with name relName does not exist (search returns {-1,-1})
    //    return E_RELNOTEXIST;
    if(recid.block==-1 && recid.slot==-1){
        return E_RELNOTEXIST;
    }

    /* reset the searchIndex of the attribute catalog using
       RelCacheTable::resetSearchIndex() */
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    /* declare variable attrToRenameRecId used to store the attr-cat recId
    of the attribute to rename */
    RecId attrToRenameRecId{-1, -1};
    Attribute attrCatEntryRecord[ATTRCAT_NO_ATTRS];

    /* iterate over all Attribute Catalog Entry record corresponding to the
       relation to find the required attribute */
    while (true) {
        // linear search on the attribute catalog for RelName = relNameAttr
        RecId recid=linearSearch(ATTRCAT_RELID,(char*)ATTRCAT_ATTR_RELNAME,relNameAttr,EQ);

        // if there are no more attributes left to check (linearSearch returned {-1,-1})
        //     break;
        if(recid.block==-1 && recid.slot==-1){
            break;
        }

        /* Get the record from the attribute catalog using RecBuffer.getRecord
          into attrCatEntryRecord */
        RecBuffer recbuffer(recid.block);
        recbuffer.getRecord(attrCatEntryRecord,recid.slot);

        // if attrCatEntryRecord.attrName = oldName
        //     attrToRenameRecId = block and slot of this record
        if(strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,oldName)==0){
            attrToRenameRecId=recid;
        }

        // if attrCatEntryRecord.attrName = newName
        //     return E_ATTREXIST;
        if(strcmp(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,newName)==0){
            return E_ATTREXIST;
        }
    }

    // if attrToRenameRecId == {-1, -1}
    //     return E_ATTRNOTEXIST;
    if(attrToRenameRecId.block==-1 && attrToRenameRecId.slot==-1){
        return E_ATTRNOTEXIST;
    }


    // Update the entry corresponding to the attribute in the Attribute Catalog Relation.
    /*   declare a RecBuffer for attrToRenameRecId.block and get the record at
         attrToRenameRecId.slot */
    //   update the AttrName of the record with newName
    //   set back the record with RecBuffer.setRecord
    RecBuffer recbuffer(attrToRenameRecId.block);
    recbuffer.getRecord(attrCatEntryRecord,attrToRenameRecId.slot);
    strcpy(attrCatEntryRecord[ATTRCAT_ATTR_NAME_INDEX].sVal,newName);
    recbuffer.setRecord(attrCatEntryRecord,attrToRenameRecId.slot);

    return SUCCESS;
}

int BlockAccess::insert(int relId, Attribute *record) {
    RelCatEntry relCatBuf;
    int ret = RelCacheTable::getRelCatEntry(relId, &relCatBuf);

    if (ret != SUCCESS)
    {
        return ret;
    }

    int blockNum = relCatBuf.firstBlk;

    RecId recId = {-1, -1};

    int numSlots = relCatBuf.numSlotsPerBlk;
    int numAttrs = relCatBuf.numAttrs;

    int prevBlockNum = -1;

    while (blockNum != -1)
    {
        RecBuffer currentBlock(blockNum);

        HeadInfo currentHeader;
        currentBlock.getHeader(&currentHeader);

        unsigned char slotMap[numSlots];
        currentBlock.getSlotMap(slotMap);

        int freeSlot = -1;
        for (int i = 0; i < numSlots; i++)
        {
            if (slotMap[i] == SLOT_UNOCCUPIED)
            {
                freeSlot = i;
                break;
            }
        }

        if (freeSlot != -1)
        {
            recId.block = blockNum;
            recId.slot = freeSlot;
            break;
        }

        prevBlockNum = blockNum;
        blockNum = currentHeader.rblock;
    }

    if (recId.block == -1 || recId.slot == -1)
    {
        if (relId == RELCAT_RELID)
        {
            return E_MAXRELATIONS;
        }

        RecBuffer newBlock;

        int newBlockNum = newBlock.getBlockNum();

        if (newBlockNum == E_DISKFULL)
        {
            return E_DISKFULL;
        }

        recId.block = newBlockNum;
        recId.slot = 0;

        HeadInfo newBlockHeader;
        newBlock.getHeader(&newBlockHeader);
        newBlockHeader.lblock = prevBlockNum;
        newBlockHeader.numAttrs = numAttrs;
        newBlockHeader.numSlots = numSlots;
        newBlock.setHeader(&newBlockHeader);

        unsigned char newBlockSlotMap[numSlots];
        newBlock.getSlotMap(newBlockSlotMap);
        for (int i = 0; i < numSlots; i++)
            newBlockSlotMap[i] = SLOT_UNOCCUPIED;
        newBlock.setSlotMap(newBlockSlotMap);

        if (prevBlockNum != -1)
        {
            RecBuffer prevBlock(prevBlockNum);

            HeadInfo prevBlockHeader;
            prevBlock.getHeader(&prevBlockHeader);
            prevBlockHeader.rblock = recId.block;
            prevBlock.setHeader(&prevBlockHeader);
        }
        else
        {
            relCatBuf.firstBlk = recId.block;
            RelCacheTable::setRelCatEntry(relId, &relCatBuf);
        }
        relCatBuf.lastBlk = recId.block;
    }

    RecBuffer blockToInsert(recId.block);
    blockToInsert.setRecord(record, recId.slot);

    unsigned char slotMapToInsert[numSlots];
    blockToInsert.getSlotMap(slotMapToInsert);
    slotMapToInsert[recId.slot] = SLOT_OCCUPIED;
    blockToInsert.setSlotMap(slotMapToInsert);

    HeadInfo headerToInsert;
    blockToInsert.getHeader(&headerToInsert);
    headerToInsert.numEntries++;
    blockToInsert.setHeader(&headerToInsert);

    relCatBuf.numRecs++;
    RelCacheTable::setRelCatEntry(relId, &relCatBuf);
    
    return SUCCESS;
}

/*
NOTE: This function will copy the result of the search to the `record` argument.
      The caller should ensure that space is allocated for `record` array
      based on the number of attributes in the relation.
*/
// int BlockAccess::search(int relId, Attribute *record, char attrName[ATTR_SIZE], Attribute attrVal, int op) {
//     // Declare a variable called recid to store the searched record
//     RecId recId;

//     /* search for the record id (recid) corresponding to the attribute with
//     attribute name attrName, with value attrval and satisfying the condition op
//     using linearSearch() */
//     recId = linearSearch(relId, attrName, attrVal, op);

//     // if there's no record satisfying the given condition (recId = {-1, -1})
//     //    return E_NOTFOUND;
//     if (recId.block == -1 && recId.slot == -1)
//     {
//         return E_NOTFOUND;
//     }


//     /* Copy the record with record id (recId) to the record buffer (record)
//        For this Instantiate a RecBuffer class object using recId and
//        call the appropriate method to fetch the record
//     */
//     RecBuffer recBuffer(recId.block);
//     recBuffer.getRecord(record, recId.slot);

//     return SUCCESS;
// }

// stage-10
int BlockAccess::search(int relId, Attribute *record, char attrName[ATTR_SIZE], Attribute attrVal, int op)
{
    // Declare a variable called recid to store the searched record
    RecId recId;

    /* get the attribute catalog entry from the attribute cache corresponding
    to the relation with Id=relid and with attribute_name=attrName  */
    AttrCatEntry attrCatEntry;
    int ret = AttrCacheTable::getAttrCatEntry(relId, attrName, &attrCatEntry);

    // if this call returns an error, return the appropriate error code
    if (ret != SUCCESS)
    {
        return ret;
    }

    // get rootBlock from the attribute catalog entry
    int rootBlock = attrCatEntry.rootBlock;
    /* if Index does not exist for the attribute (check rootBlock == -1) */
    if (rootBlock == -1)
    {

        /* search for the record id (recid) corresponding to the attribute with
           attribute name attrName, with value attrval and satisfying the
           condition op using linearSearch()
        */
        recId = linearSearch(relId, attrName, attrVal, op);
        // resetting the search index will be handled by linear search
    }

    /* else */
    else
    {
        // (index exists for the attribute)

        /* search for the record id (recid) correspoding to the attribute with
        attribute name attrName and with value attrval and satisfying the
        condition op using BPlusTree::bPlusSearch() */
        recId = BPlusTree::bPlusSearch(relId, attrName, attrVal, op);
    }

    // if there's no record satisfying the given condition (recId = {-1, -1})
    //     return E_NOTFOUND;
    if (recId.block == -1 && recId.slot == -1)
    {
        return E_NOTFOUND;
    }

    /* Copy the record with record id (recId) to the record buffer (record).
       For this, instantiate a RecBuffer class object by passing the recId and
       call the appropriate method to fetch the record
    */
    RecBuffer recBuffer(recId.block);
    recBuffer.getRecord(record, recId.slot);

    return SUCCESS;
}

int BlockAccess::deleteRelation(char relName[ATTR_SIZE])
{
    if (
        strcmp(relName, (char *)RELCAT_RELNAME) == 0 ||
        strcmp(relName, (char *)ATTRCAT_RELNAME) == 0)
    {
        return E_NOTPERMITTED;
    }

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute relNameAttribute;
    strcpy(relNameAttribute.sVal, relName);

    RecId recId = linearSearch(RELCAT_RELID, (char *)RELCAT_ATTR_RELNAME, relNameAttribute, EQ);

    if (recId.block == -1 || recId.slot == -1)
    {
        return E_RELNOTEXIST;
    }

    Attribute relCatEntryRecord[RELCAT_NO_ATTRS];

    RecBuffer recBuffer(recId.block);
    recBuffer.getRecord(relCatEntryRecord, recId.slot);

    int currentBlock = relCatEntryRecord[RELCAT_FIRST_BLOCK_INDEX].nVal;

    while (currentBlock != -1)
    {
        RecBuffer currentBlockBuffer(currentBlock);
        HeadInfo currentBlockHeader;
        currentBlockBuffer.getHeader(&currentBlockHeader);

        int nextBlock = currentBlockHeader.rblock;

        currentBlockBuffer.releaseBlock();
        currentBlock = nextBlock;
    }

    int numAttrsDeleted = 0;
    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    while (true)
    {
        RecId attrCatRecId = linearSearch(ATTRCAT_RELID, (char *)ATTRCAT_ATTR_RELNAME, relNameAttribute, EQ);

        if (attrCatRecId.slot == -1 || attrCatRecId.block == -1)
        {
            break;
        }

        numAttrsDeleted++;

        RecBuffer currentBlock(attrCatRecId.block);

        HeadInfo currentBlockHeader;
        currentBlock.getHeader(&currentBlockHeader);

        Attribute record[ATTRCAT_NO_ATTRS];
        currentBlock.getRecord(record, attrCatRecId.slot);

        int rootBlock = record[ATTRCAT_ROOT_BLOCK_INDEX].nVal;

        unsigned char slotMap[currentBlockHeader.numSlots];

        currentBlock.getSlotMap(slotMap);
        slotMap[attrCatRecId.slot] = SLOT_UNOCCUPIED;
        currentBlock.setSlotMap(slotMap);

        currentBlockHeader.numEntries--;
        currentBlock.setHeader(&currentBlockHeader);

        if (currentBlockHeader.numEntries == 0)
        {
            int leftBlock = currentBlockHeader.lblock;
            int rightBlock = currentBlockHeader.rblock;

            if (leftBlock != -1)
            {
                RecBuffer prevBlock(leftBlock);
                HeadInfo prevBlockHeader;

                prevBlock.getHeader(&prevBlockHeader);
                prevBlockHeader.rblock = rightBlock;
                prevBlock.setHeader(&prevBlockHeader);
            }

            if (rightBlock != -1)
            {
                RecBuffer nextBlock(rightBlock);
                HeadInfo nextBlockHeader;

                nextBlock.getHeader(&nextBlockHeader);
                nextBlockHeader.lblock = leftBlock;
                nextBlock.setHeader(&nextBlockHeader);
            }

            currentBlock.releaseBlock();
        }
    }

    HeadInfo relCatHeader;
    recBuffer.getHeader(&relCatHeader);

    unsigned char recSlotMap[relCatHeader.numSlots];

    recBuffer.getSlotMap(recSlotMap);
    recSlotMap[recId.slot] = SLOT_UNOCCUPIED;
    recBuffer.setSlotMap(recSlotMap);

    relCatHeader.numEntries--;
    recBuffer.setHeader(&relCatHeader);

    RelCatEntry relCatBuf;
    RelCacheTable::getRelCatEntry(RELCAT_RELID, &relCatBuf);
    relCatBuf.numRecs--;
    RelCacheTable::setRelCatEntry(RELCAT_RELID, &relCatBuf);

    RelCatEntry attrCatBuf;
    RelCacheTable::getRelCatEntry(ATTRCAT_RELID, &attrCatBuf);
    attrCatBuf.numRecs -= numAttrsDeleted;
    RelCacheTable::setRelCatEntry(ATTRCAT_RELID, &attrCatBuf);

    return SUCCESS;
}

/*
NOTE: the caller is expected to allocate space for the argument `record` based
      on the size of the relation. This function will only copy the result of
      the projection onto the array pointed to by the argument.
*/
int BlockAccess::project(int relId, Attribute *record)
{
    // get the previous search index of the relation relId from the relation
    // cache (use RelCacheTable::getSearchIndex() function)
    RecId prevRecId;
    RelCacheTable::getSearchIndex(relId, &prevRecId);

    // declare block and slot which will be used to store the record id of the
    // slot we need to check.
    int block, slot;

    /* if the current search index record is invalid(i.e. = {-1, -1})
       (this only happens when the caller reset the search index)
    */
    if (prevRecId.block == -1 && prevRecId.slot == -1)
    {
        // (new project operation. start from beginning)

        // get the first record block of the relation from the relation cache
        // (use RelCacheTable::getRelCatEntry() function of Cache Layer)
        RelCatEntry relCatEntry;
        RelCacheTable::getRelCatEntry(relId, &relCatEntry);

        // block = first record block of the relation
        // slot = 0
        block = relCatEntry.firstBlk;
        slot = 0;
    }
    else
    {
        // (a project/search operation is already in progress)

        // block = previous search index's block
        // slot = previous search index's slot + 1
        block = prevRecId.block;
        slot = prevRecId.slot + 1;
    }

    // The following code finds the next record of the relation
    /* Start from the record id (block, slot) and iterate over the remaining
       records of the relation */
    while (block != -1)
    {
        // create a RecBuffer object for block (using appropriate constructor!)
        RecBuffer recBlock(block);

        // get header of the block using RecBuffer::getHeader() function
        // get slot map of the block using RecBuffer::getSlotMap() function
        struct HeadInfo header;
        recBlock.getHeader(&header);
        unsigned char slotMap[header.numSlots];
        recBlock.getSlotMap(slotMap);

        if (slot >= header.numSlots)
        {
            // (no more slots in this block)
            // update block = right block of block
            // update slot = 0
            // (NOTE: if this is the last block, rblock would be -1. this would
            //        set block = -1 and fail the loop condition )
            block = header.rblock;
            slot = 0;
            continue; // continue to the beginning of this while loop
        }
        else if (slotMap[slot] == SLOT_UNOCCUPIED)
        {
            // (slot is free)
            // increment slot and continue to the next record slot
            slot++;
            continue;
        }
        else
        {
            // (the next occupied slot / record has been found)
            // declare nextRecId to store the RecId of the record found
            break;
        }
    }

    if (block == -1)
    {
        // (a record was not found. all records exhausted)
        return E_NOTFOUND;
    }

    // declare nextRecId to store the RecId of the record found
    RecId nextRecId{block, slot};

    // set the search index to nextRecId using RelCacheTable::setSearchIndex
    RelCacheTable::setSearchIndex(relId, &nextRecId);

    /* Copy the record with record id (nextRecId) to the record buffer (record)
       For this Instantiate a RecBuffer class object by passing the recId and
       call the appropriate method to fetch the record
    */
    RecBuffer recBuffer(nextRecId.block);
    recBuffer.getRecord(record, nextRecId.slot);

    return SUCCESS;
}