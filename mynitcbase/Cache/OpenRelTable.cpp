#include "OpenRelTable.h"
#include "bits/stdc++.h"

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

AttrCacheEntry *createLinkedList(int length)
{
    AttrCacheEntry *head = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    AttrCacheEntry *tail = head;
    for (int i = 1; i < length; i++)
    {
        tail->next = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
        tail = tail->next;
    }
    tail->next = nullptr;
    return head;
}

void freeLinkedList(AttrCacheEntry *head)
{
    for (AttrCacheEntry *it = head, *next; it != nullptr; it = next)
    {
        next = it->next;
        free(it);
    }
}


OpenRelTable::OpenRelTable() {
    // Initialize relCache and attrCache with nullptr
    for (int i = 0; i < MAX_OPEN; ++i) {
        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
        tableMetaInfo[i].free=true;
        
    }

    /************ Setting up Relation Cache entries ************/
    for (int i = 0; i < 2; i++) {
      RecBuffer relCatBlock(RELCAT_BLOCK);

     Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBlock.getRecord(relCatRecord, i);

    RelCacheEntry relCacheEntry;
    RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
    relCacheEntry.dirty = false;
    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = i;

    RelCacheTable::relCache[i] = new RelCacheEntry(relCacheEntry);
 }

 RecBuffer attrCatBlock(ATTRCAT_BLOCK);
for (int i = 0; i < 2 * RELCAT_NO_ATTRS; i++) {
Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrCatBlock.getRecord(attrCatRecord, i);

        AttrCacheEntry attrcacheEntry;
         AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &attrcacheEntry.attrCatEntry);
         attrcacheEntry.dirty = false;
         attrcacheEntry.recId.block = ATTRCAT_BLOCK;
         attrcacheEntry.recId.slot = i;
         attrcacheEntry.next = nullptr;

         AttrCacheEntry* attrcacheEntry2 = new AttrCacheEntry(attrcacheEntry);
        if (i < RELCAT_NO_ATTRS) {
            attrcacheEntry2->next = AttrCacheTable::attrCache[RELCAT_RELID];
            AttrCacheTable::attrCache[RELCAT_RELID] = attrcacheEntry2;
        } else {
            attrcacheEntry2->next = AttrCacheTable::attrCache[ATTRCAT_RELID];
            AttrCacheTable::attrCache[ATTRCAT_RELID] = attrcacheEntry2;
        }
    }
    for(int i=0;i<2;i++){
        if(i==0){
            tableMetaInfo[i].free=false;
            strcpy(tableMetaInfo[i].relName,RELCAT_RELNAME);
        }
        else if(i==1){
            tableMetaInfo[i].free=false;
            strcpy(tableMetaInfo[i].relName,ATTRCAT_RELNAME);
        }
    }
    /*exercise*/
    // int j=2;
    // RecBuffer attrCatBuffer(ATTRCAT_BLOCK); // Correctly initialize with ATTRCAT_BLOCK
    // HeadInfo attrCatHeader;
    // attrCatBuffer.getHeader(&attrCatHeader);
    // int b = ATTRCAT_BLOCK;

    // for (int i = 0; i < attrCatHeader.numEntries; i++) {
    //     Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
    //     attrCatBuffer.getRecord(attrCatRecord, i);

    //     if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, "Students") == 0) {
    //         AttrCacheEntry attrCacheentry;
    //         AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &attrCacheentry.attrCatEntry);
    //         attrCacheentry.dirty = false;
    //         attrCacheentry.recId.block = b; // Use current block ID
    //         attrCacheentry.recId.slot = i;

    //         AttrCacheEntry *attrCacheentry2 = new AttrCacheEntry(attrCacheentry);
    //         attrCacheentry2->next = AttrCacheTable::attrCache[j];
    //         AttrCacheTable::attrCache[j] = attrCacheentry2;
    //     }

    //     // Check if the current block is exhausted and move to the next block
    //     if (i == attrCatHeader.numEntries - 1 && attrCatHeader.rblock != -1) {
    //         // Update to the next block
    //         b = attrCatHeader.rblock; // Move to the next block
    //         attrCatBuffer = RecBuffer(b);
    //         attrCatBuffer.getHeader(&attrCatHeader);
    //         i = -1; // Reset the loop to process records in the new block
    //     }
    // }
}


OpenRelTable::~OpenRelTable()
{

    for (int i = 2; i < MAX_OPEN; i++)
    {
        if (!tableMetaInfo[i].free)
            OpenRelTable::closeRel(i);
    }

    for (int i = 2; i < MAX_OPEN; i++)
    {
        free(RelCacheTable::relCache[i]);
        freeLinkedList(AttrCacheTable::attrCache[i]);

        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
    }

    if (RelCacheTable::relCache[ATTRCAT_RELID]->dirty)
    {
        Attribute relCatRecord[RELCAT_NO_ATTRS];

        RelCatEntry relCatEntry = RelCacheTable::relCache[ATTRCAT_RELID]->relCatEntry;
        RecId recId = RelCacheTable::relCache[ATTRCAT_RELID]->recId;

        RelCacheTable::relCatEntryToRecord(&relCatEntry, relCatRecord);

        RecBuffer relCatBlock(recId.block);
        relCatBlock.setRecord(relCatRecord, recId.slot);

        free(RelCacheTable::relCache[ATTRCAT_RELID]);
    }

    if (RelCacheTable::relCache[RELCAT_RELID]->dirty)
    {
        Attribute relCatRecord[RELCAT_NO_ATTRS];

        RelCatEntry relCatEntry = RelCacheTable::relCache[RELCAT_RELID]->relCatEntry;
        RecId recId = RelCacheTable::relCache[RELCAT_RELID]->recId;

        RelCacheTable::relCatEntryToRecord(&relCatEntry, relCatRecord);

        RecBuffer relCatBlock(recId.block);
        relCatBlock.setRecord(relCatRecord, recId.slot);

        free(RelCacheTable::relCache[RELCAT_RELID]);
    }

    freeLinkedList(AttrCacheTable::attrCache[RELCAT_RELID]);
    freeLinkedList(AttrCacheTable::attrCache[ATTRCAT_RELID]);
}

// /* This function will open a relation having name `relName`.
// Since we are currently only working with the relation and attribute catalog, we
// will just hardcode it. In subsequent stages, we will loop through all the relations
// and open the appropriate one.
// */
int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {
    for(int i=0;i<MAX_OPEN;i++){
        if(tableMetaInfo[i].free==false && (strcmp(tableMetaInfo[i].relName,relName)==0)){
            return i;
        }
    }
    return E_RELNOTOPEN; // Return error if relation is not open
}

int OpenRelTable::getFreeOpenRelTableEntry() {

  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/
    for(int i=0;i<MAX_OPEN;i++){
        if(tableMetaInfo[i].free==true){
            return i;
        }
    }
    return E_CACHEFULL;

  // if found return the relation id, else return E_CACHEFULL.
}

int OpenRelTable::openRel(char relName[ATTR_SIZE]) {
  int relId = OpenRelTable::getRelId(relName);
  if (relId != E_RELNOTOPEN) { 
    return relId;
  }


  /* find a free slot in the Open Relation Table
     using OpenRelTable::getFreeOpenRelTableEntry(). */
  int relID=getFreeOpenRelTableEntry();
  if (relID==E_CACHEFULL){
    return E_CACHEFULL;
  }


  /****** Setting up Relation Cache entry for the relation ******/

  /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/
    Attribute relnameattribute;
    memcpy(relnameattribute.sVal,relName,ATTR_SIZE);
    RelCacheTable::resetSearchIndex(RELCAT_RELID);
  // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
  RecId relcatRecId=BlockAccess::linearSearch(RELCAT_RELID,(char *)RELCAT_ATTR_RELNAME,relnameattribute,EQ);

  if (relcatRecId.block==-1 || relcatRecId.slot==-1) {
    // (the relation is not found in the Relation Catalog.)
    return E_RELNOTEXIST;
  }

  /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      update the recId field of this Relation Cache entry to relcatRecId.
      use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  */
   RecBuffer recbuffer(relcatRecId.block);
   RelCatEntry relcatentry;
   Attribute record[RELCAT_NO_ATTRS];
   recbuffer.getRecord(record,relcatRecId.slot);
   RelCacheTable::recordToRelCatEntry(record,&relcatentry);
   RelCacheTable::relCache[relID]=(RelCacheEntry *)malloc(sizeof(RelCacheEntry));
   RelCacheTable::relCache[relID]->recId=relcatRecId;
   RelCacheTable::relCache[relID]->relCatEntry=relcatentry;

  /****** Setting up Attribute Cache entry for the relation ******/

  // let listHead be used to hold the head of the linked list of attrCache entries.
  int numattrs=relcatentry.numAttrs;
  AttrCacheEntry* listHead=createLinkedList(numattrs);
  AttrCacheEntry*node=listHead;
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
  /*iterate over all the entries in the Attribute Catalog corresponding to each
  attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
  care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
  corresponding to Attribute Catalog before the first call to linearSearch().*/
  while(true)
  {
      /* let attrcatRecId store a valid record id an entry of the relation, relName,
      in the Attribute Catalog.*/
      RecId attrcatRecId=BlockAccess::linearSearch(ATTRCAT_RELID,(char *)ATTRCAT_ATTR_RELNAME,relnameattribute,EQ);

      if(attrcatRecId.block!=-1 && attrcatRecId.slot!=-1){
        Attribute attrcatrecord[ATTRCAT_NO_ATTRS];
        RecBuffer attrrecbuffer(attrcatRecId.block);
        attrrecbuffer.getRecord(attrcatrecord,attrcatRecId.slot);

        AttrCatEntry attrcatentry;
        AttrCacheTable::recordToAttrCatEntry(attrcatrecord,&attrcatentry);

        node->recId=attrcatRecId;
        node->attrCatEntry=attrcatentry;
        node=node->next;
      }
      else{
        break;
      }
  }

  // set the relIdth entry of the AttrCacheTable to listHead.
  AttrCacheTable::attrCache[relID]=listHead;

  /****** Setting up metadata in the Open Relation Table for the relation******/

  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.
  OpenRelTable::tableMetaInfo[relID].free=false;
  memcpy(OpenRelTable::tableMetaInfo[relID].relName,relcatentry.relName,ATTR_SIZE);

  return relID;
}

// stage-5

// int OpenRelTable::closeRel(int relId) {
  // if (relId==RELCAT_RELID || relId==ATTRCAT_RELID) {
  //   return E_NOTPERMITTED;
  // }

  // if (relId<0 || relId>=MAX_OPEN) {
  //   return E_OUTOFBOUND;
  // }

  // if (tableMetaInfo[relId].free==true) {
  //   return E_RELNOTOPEN;
  // }

  // // free the memory allocated in the relation and attribute caches which was
  // // allocated in the OpenRelTable::openRel() function
  // delete(RelCacheTable::relCache[relId]);
  // RelCacheTable::relCache[relId]=nullptr;
//   // update `tableMetaInfo` to set `relId` as a free slot
//   tableMetaInfo[relId].free=true;
//   // update `relCache` and `attrCache` to set the entry at `relId` to nullptr
//   AttrCacheEntry *next;
//   for(AttrCacheEntry *entry=AttrCacheTable::attrCache[relId];entry!=nullptr;entry=next){
//     next=entry->next;
//     delete(next);
//   }
//   AttrCacheTable::attrCache[relId]=nullptr;
//   return SUCCESS;
// }

int OpenRelTable::closeRel(int relId) {
    if (relId==RELCAT_RELID || relId==ATTRCAT_RELID) {
    return E_NOTPERMITTED;
  }

  if (relId<0 || relId>=MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  if (tableMetaInfo[relId].free==true) {
    return E_RELNOTOPEN;
  }

  /****** Releasing the Relation Cache entry of the relation ******/

  RelCacheEntry *relCacheEntry = RelCacheTable::relCache[relId];

    if (relCacheEntry && relCacheEntry->dirty == true)
    {
        RecBuffer relCatBlock((relCacheEntry->recId).block);

        RelCatEntry relCatEntry = relCacheEntry->relCatEntry;
        Attribute record[RELCAT_NO_ATTRS];

        RelCacheTable::relCatEntryToRecord(&relCatEntry, record);

        relCatBlock.setRecord(record, (relCacheEntry->recId).slot);
    }

  /****** Releasing the Attribute Cache entry of the relation ******/

  // free the memory allocated in the relation and attribute caches which was
  // allocated in the OpenRelTable::openRel() function
  delete(RelCacheTable::relCache[relId]);
  RelCacheTable::relCache[relId]=nullptr;

  // (because we are not modifying the attribute cache at this stage,
  // write-back is not required. We will do it in subsequent
  // stages when it becomes needed)


  /****** Set the Open Relation Table entry of the relation as free ******/

  // update `metainfo` to set `relId` as a free slot
  tableMetaInfo[relId].free=true;

  return SUCCESS;
}

