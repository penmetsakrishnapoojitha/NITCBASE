#include "BlockAccess.h"
#include <cstring>
#include <iostream>

using namespace std;

inline bool operator == (RecId lhs, RecId rhs) {
    return (lhs.block == rhs.block && lhs.slot == rhs.slot);
}

inline bool operator != (RecId lhs, RecId rhs) {
    return (lhs.block != rhs.block || lhs.slot != rhs.slot);
}


/*---------------------------------------------------------
    LINEAR SEARCH
---------------------------------------------------------*/

RecId BlockAccess::linearSearch(
    int relId,
    char attrName[ATTR_SIZE],
    union Attribute attrVal,
    int op
) {
    // Get previous search index
    RecId prevRecId;

    int ret = RelCacheTable::getSearchIndex(relId, &prevRecId);

    if (ret != SUCCESS) {
        return RecId{-1, -1};
    }

    int block = -1;
    int slot = -1;

    /*
        If there was no previous search,
        start from the first block.
    */
    if (prevRecId.block == -1 && prevRecId.slot == -1) {

        RelCatEntry relCatBuffer;

        ret = RelCacheTable::getRelCatEntry(
            relId,
            &relCatBuffer
        );

        if (ret != SUCCESS) {
            return RecId{-1, -1};
        }

        block = relCatBuffer.firstBlk;
        slot = 0;
    }
    else {

        // Continue from next slot
        block = prevRecId.block;
        slot = prevRecId.slot + 1;
    }


    RelCatEntry relCatBuffer;

    ret = RelCacheTable::getRelCatEntry(
        relId,
        &relCatBuffer
    );

    if (ret != SUCCESS) {
        return RecId{-1, -1};
    }


    while (block != -1) {

        RecBuffer blockBuffer(block);

        HeadInfo blockHeader;

        ret = blockBuffer.getHeader(&blockHeader);

        if (ret != SUCCESS) {
            return RecId{-1, -1};
        }


        unsigned char slotMap[blockHeader.numSlots];

        ret = blockBuffer.getSlotMap(slotMap);

        if (ret != SUCCESS) {
            return RecId{-1, -1};
        }


    


        /*
            If current slot is beyond the slots
            in this block, move to next block.
        */
        if (slot >= relCatBuffer.numSlotsPerBlk) {

            block = blockHeader.rblock;
            slot = 0;

            continue;
        }


        /*
            Skip free slots
        */
        if (slotMap[slot] == SLOT_UNOCCUPIED) {

            slot++;

            continue;
        }


        /*
            Read record
        */
        Attribute record[blockHeader.numAttrs];

        ret = blockBuffer.getRecord(
            record,
            slot
        );

        if (ret != SUCCESS) {
            return RecId{-1, -1};
        }


        /*
            Get attribute information
        */
        AttrCatEntry attrCatBuffer;

        ret = AttrCacheTable::getAttrCatEntry(
            relId,
            attrName,
            &attrCatBuffer
        );

        if (ret != SUCCESS) {
            return RecId{-1, -1};
        }


        int attrOffset = attrCatBuffer.offset;


        /*
            Compare attribute
        */
        int cmpVal = compareAttrs(
            record[attrOffset],
            attrVal,
            attrCatBuffer.attrType
        );


        bool condition = false;

        if (op == NE && cmpVal != 0)
            condition = true;

        else if (op == LT && cmpVal < 0)
            condition = true;

        else if (op == LE && cmpVal <= 0)
            condition = true;

        else if (op == EQ && cmpVal == 0)
            condition = true;

        else if (op == GT && cmpVal > 0)
            condition = true;

        else if (op == GE && cmpVal >= 0)
            condition = true;


        if (condition) {

            RecId newRecId = {
                block,
                slot
            };

            RelCacheTable::setSearchIndex(
                relId,
                &newRecId
            );

            return newRecId;
        }


        slot++;
    }


    return RecId{-1, -1};
}


/*---------------------------------------------------------
    RENAME RELATION
---------------------------------------------------------*/

int BlockAccess::renameRelation(
    char oldName[ATTR_SIZE],
    char newName[ATTR_SIZE]
) {

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute newRelationName;

    strcpy(
        newRelationName.sVal,
        newName
    );

    RecId searchIndex =
        BlockAccess::linearSearch(
            RELCAT_RELID,
            RELCAT_ATTR_RELNAME,
            newRelationName,
            EQ
        );

    if (searchIndex != RecId{-1, -1})
        return E_RELEXIST;


    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute oldRelationName;

    strcpy(
        oldRelationName.sVal,
        oldName
    );

    searchIndex =
        BlockAccess::linearSearch(
            RELCAT_RELID,
            RELCAT_ATTR_RELNAME,
            oldRelationName,
            EQ
        );

    if (searchIndex == RecId{-1, -1})
        return E_RELNOTEXIST;


    RecBuffer relCatBlock(RELCAT_BLOCK);

    Attribute relCatRecord[RELCAT_NO_ATTRS];

    relCatBlock.getRecord(
        relCatRecord,
        searchIndex.slot
    );


    strcpy(
        relCatRecord[RELCAT_REL_NAME_INDEX].sVal,
        newName
    );


    relCatBlock.setRecord(
        relCatRecord,
        searchIndex.slot
    );


    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);


    for (
        int attrIndex = 0;
        attrIndex < relCatRecord[
            RELCAT_NO_ATTRIBUTES_INDEX
        ].nVal;
        attrIndex++
    ) {

        searchIndex =
            BlockAccess::linearSearch(
                ATTRCAT_RELID,
                ATTRCAT_ATTR_RELNAME,
                oldRelationName,
                EQ
            );


        RecBuffer attrCatBlock(
            searchIndex.block
        );

        Attribute attrCatRecord[
            ATTRCAT_NO_ATTRS
        ];

        attrCatBlock.getRecord(
            attrCatRecord,
            searchIndex.slot
        );


        strcpy(
            attrCatRecord[
                ATTRCAT_REL_NAME_INDEX
            ].sVal,
            newName
        );


        attrCatBlock.setRecord(
            attrCatRecord,
            searchIndex.slot
        );
    }


    return SUCCESS;
}


/*---------------------------------------------------------
    RENAME ATTRIBUTE
---------------------------------------------------------*/

int BlockAccess::renameAttribute(
    char relName[ATTR_SIZE],
    char oldName[ATTR_SIZE],
    char newName[ATTR_SIZE]
) {

    RelCacheTable::resetSearchIndex(RELCAT_RELID);


    Attribute relNameAttr;

    strcpy(
        relNameAttr.sVal,
        relName
    );


    RecId searchIndex =
        BlockAccess::linearSearch(
            RELCAT_RELID,
            RELCAT_ATTR_RELNAME,
            relNameAttr,
            EQ
        );


    if (searchIndex == RecId{-1, -1})
        return E_RELNOTEXIST;


    RelCacheTable::resetSearchIndex(
        ATTRCAT_RELID
    );


    RecId attrToRenameRecId{-1, -1};


    while (true) {

        searchIndex =
            BlockAccess::linearSearch(
                ATTRCAT_RELID,
                ATTRCAT_ATTR_RELNAME,
                relNameAttr,
                EQ
            );


        if (searchIndex == RecId{-1, -1})
            break;


        RecBuffer attrCatBlock(
            searchIndex.block
        );

        Attribute attrCatRecord[
            ATTRCAT_NO_ATTRS
        ];


        attrCatBlock.getRecord(
            attrCatRecord,
            searchIndex.slot
        );


        if (
            strcmp(
                attrCatRecord[
                    ATTRCAT_ATTR_NAME_INDEX
                ].sVal,
                oldName
            ) == 0
        ) {

            attrToRenameRecId =
                searchIndex;

            break;
        }


        if (
            strcmp(
                attrCatRecord[
                    ATTRCAT_ATTR_NAME_INDEX
                ].sVal,
                newName
            ) == 0
        ) {

            return E_ATTREXIST;
        }
    }


    if (attrToRenameRecId == RecId{-1, -1})
        return E_ATTRNOTEXIST;


    RecBuffer attrCatBlock(
        attrToRenameRecId.block
    );

    Attribute attrCatRecord[
        ATTRCAT_NO_ATTRS
    ];


    attrCatBlock.getRecord(
        attrCatRecord,
        attrToRenameRecId.slot
    );


    strcpy(
        attrCatRecord[
            ATTRCAT_ATTR_NAME_INDEX
        ].sVal,
        newName
    );


    attrCatBlock.setRecord(
        attrCatRecord,
        attrToRenameRecId.slot
    );


    return SUCCESS;
}


/*---------------------------------------------------------
    INSERT
---------------------------------------------------------*/

int BlockAccess::insert(
    int relId,
    Attribute *record
) {

    /*
        Get relation catalog entry
    */
    RelCatEntry relCatEntry;

    int ret =
        RelCacheTable::getRelCatEntry(
            relId,
            &relCatEntry
        );

    if (ret != SUCCESS) {
        cout << "ERROR: getRelCatEntry failed: "
             << ret << endl;

        return ret;
    }


    int blockNum = relCatEntry.firstBlk;

    RecId rec_id = {-1, -1};

    int numOfSlots =
        relCatEntry.numSlotsPerBlk;

    int numOfAttributes =
        relCatEntry.numAttrs;

    int prevBlockNum = -1;


    /*
        Search existing blocks for a free slot
    */
    while (blockNum != -1) {

        RecBuffer blockBuffer(blockNum);

        HeadInfo blockHeader;

        ret = blockBuffer.getHeader(
            &blockHeader
        );

        if (ret != SUCCESS) {
            return ret;
        }


        int numSlots =
            blockHeader.numSlots;

        unsigned char slotMap[numSlots];


        ret =
            blockBuffer.getSlotMap(
                slotMap
            );

        if (ret != SUCCESS) {
            return ret;
        }


        int slotIndex = 0;


        for (
            ;
            slotIndex < numSlots;
            slotIndex++
        ) {

            if (
                slotMap[slotIndex]
                == SLOT_UNOCCUPIED
            ) {

                rec_id =
                    RecId{
                        blockNum,
                        slotIndex
                    };

                break;
            }
        }


        if (rec_id != RecId{-1, -1})
            break;


        prevBlockNum = blockNum;

        blockNum =
            blockHeader.rblock;
    }


    /*
        No free slot:
        allocate a new record block
    */
    if (rec_id == RecId{-1, -1}) {

        /*
            RELCAT cannot grow
        */
        if (relId == RELCAT_RELID)
            return E_MAXRELATIONS;


        RecBuffer newBlockBuffer;

        blockNum =
            newBlockBuffer.getBlockNum();


        if (blockNum == E_DISKFULL)
            return E_DISKFULL;


        rec_id =
            RecId{
                blockNum,
                0
            };


        /*
            Initialize new block header
        */
        HeadInfo blockHeader;

        blockHeader.blockType = REC;
        blockHeader.pblock = -1;
        blockHeader.lblock = prevBlockNum;
        blockHeader.rblock = -1;
        blockHeader.numEntries = 0;
        blockHeader.numAttrs =
            numOfAttributes;
        blockHeader.numSlots =
            numOfSlots;


        ret =
            newBlockBuffer.setHeader(
                &blockHeader
            );

        if (ret != SUCCESS) {
            return ret;
        }


        /*
            Initialize slot map
        */
        unsigned char slotMap[numOfSlots];

        for (
            int slotIndex = 0;
            slotIndex < numOfSlots;
            slotIndex++
        ) {
            slotMap[slotIndex] =
                SLOT_UNOCCUPIED;
        }


        ret =
            newBlockBuffer.setSlotMap(
                slotMap
            );

        if (ret != SUCCESS) {
            return ret;
        }


        /*
            Link previous block to new block
        */
        if (prevBlockNum != -1) {

            RecBuffer prevBlockBuffer(
                prevBlockNum
            );

            HeadInfo prevBlockHeader;

            ret =
                prevBlockBuffer.getHeader(
                    &prevBlockHeader
                );

            if (ret != SUCCESS) {
                return ret;
            }


            prevBlockHeader.rblock =
                blockNum;


            ret =
                prevBlockBuffer.setHeader(
                    &prevBlockHeader
                );

            if (ret != SUCCESS) {
                return ret;
            }
        }
        else {

            /*
                This is the first block
            */
            relCatEntry.firstBlk =
                blockNum;

            ret =
                RelCacheTable::setRelCatEntry(
                    relId,
                    &relCatEntry
                );

            if (ret != SUCCESS) {
                return ret;
            }
        }


        /*
            Update last block
        */
        relCatEntry.lastBlk =
            blockNum;


        ret =
            RelCacheTable::setRelCatEntry(
                relId,
                &relCatEntry
            );

        if (ret != SUCCESS) {
            return ret;
        }
    }


    /*
        Insert record
    */

 


    RecBuffer blockBuffer(
        rec_id.block
    );


    ret =
        blockBuffer.setRecord(
            record,
            rec_id.slot
        );



    if (ret != SUCCESS)
        return ret;


    /*
        Update slot map
    */
    unsigned char slotmap[numOfSlots];

    ret =
        blockBuffer.getSlotMap(
            slotmap
        );

    if (ret != SUCCESS)
        return ret;


    slotmap[rec_id.slot] =
        SLOT_OCCUPIED;


    ret =
        blockBuffer.setSlotMap(
            slotmap
        );


    if (ret != SUCCESS)
        return ret;


    /*
        Increment block's number of entries
    */
    HeadInfo blockHeader;

    ret =
        blockBuffer.getHeader(
            &blockHeader
        );

    if (ret != SUCCESS)
        return ret;


    blockHeader.numEntries++;


    ret =
        blockBuffer.setHeader(
            &blockHeader
        );

    if (ret != SUCCESS)
        return ret;


    /*
        Increment relation's number of records
    */
    relCatEntry.numRecs++;


    ret =
        RelCacheTable::setRelCatEntry(
            relId,
            &relCatEntry
        );



    if (ret != SUCCESS)
        return ret;


    return SUCCESS;
}
int BlockAccess::search(int relId, Attribute *record, char attrName[ATTR_SIZE], Attribute attrVal, int op) {
    // Declare a variable called recid to store the searched record
    RecId recId;

    /* search for the record id (recid) corresponding to the attribute with
    attribute name attrName, with value attrval and satisfying the condition op
    using linearSearch() */
	recId = BlockAccess::linearSearch(relId, attrName, attrVal, op);

    // if there's no record satisfying the given condition (recId = {-1, -1})
	if (recId == RecId{-1, -1})
       return E_NOTFOUND;

    /* Copy the record with record id (recId) to the record buffer (record)
       For this Instantiate a RecBuffer class object using recId and
       call the appropriate method to fetch the record
    */

   	RecBuffer blockBuffer (recId.block);
   	blockBuffer.getRecord(record, recId.slot);

    return SUCCESS;
}

int BlockAccess::deleteRelation(char relName[ATTR_SIZE]) {
    // if the relation to delete is either Relation Catalog or Attribute Catalog
	// (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
	// you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)
	if (strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0)
        return E_NOTPERMITTED;

    /* reset the searchIndex of the relation catalog using
       RelCacheTable::resetSearchIndex() */
	RelCacheTable::resetSearchIndex(RELCAT_RELID);

    // assign relNameAttr.sVal = relName
    Attribute relNameAttr; // (stores relName as type union Attribute)
    	strcpy((char*)relNameAttr.sVal,(const char*)relName);

    //  linearSearch on the relation catalog for RelName = relNameAttr
	RecId relCatRecId = BlockAccess::linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, relNameAttr ,EQ);

    // if the relation does not exist (linearSearch returned {-1, -1})
	if (relCatRecId == RecId{-1, -1}) return E_RELNOTEXIST;

    // TODO: store the relation catalog record corresponding to the relation in
    // TODO: relCatEntryRecord using RecBuffer.getRecord

	RecBuffer relCatBlockBuffer (relCatRecId.block);

	// relCatEntryRecord : relation `relName` record in relation-catalog
    Attribute relCatEntryRecord[RELCAT_NO_ATTRS];
	relCatBlockBuffer.getRecord(relCatEntryRecord, relCatRecId.slot);

	// TODO: get the first record block & number of attributes of the relation 
	// TODO: (firstBlock) & (numAttrs) using the relation catalog entry record 
	
	int firstBlock = relCatEntryRecord[RELCAT_FIRST_BLOCK_INDEX].nVal;
	int numAttributes = relCatEntryRecord[RELCAT_NO_ATTRIBUTES_INDEX].nVal;

    // TODO: Delete all the record blocks of the relation
	int currentBlockNum = firstBlock;
    
	// for each record block of the relation:
	// Hint: to know if we reached the end, check if nextBlock = -1
	while (currentBlockNum != -1) {
		RecBuffer currentBlockBuffer (currentBlockNum);
        HeadInfo currentBlockHeader;
		currentBlockBuffer.getHeader(&currentBlockHeader);

		// get the next block from the header (rblock)
		currentBlockNum = currentBlockHeader.rblock;

		// release the block using BlockBuffer.releaseBlock
		currentBlockBuffer.releaseBlock();
	}

    /***
        Deleting attribute catalog entries corresponding the relation and index
        blocks corresponding to the relation with relName on its attributes
    ***/

    // reset the searchIndex of the attribute catalog
	RelCacheTable::resetSearchIndex(ATTRCAT_RELID);

    int numberOfAttributesDeleted = 0;

    while(true) {
        // attrCatRecId : `relname`'s entry in `ATTRCAT`
        RecId attrCatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, RELCAT_ATTR_RELNAME, relNameAttr, EQ);

        // if no more attributes to iterate over (attrCatRecId == {-1, -1})
		if (attrCatRecId == RecId{-1, -1}) break;

        numberOfAttributesDeleted++;

        // create a RecBuffer for attrCatRecId.block
        // get the header of the block
        // get the record corresponding to attrCatRecId.slot
		RecBuffer attrCatBlockBuffer (attrCatRecId.block);

		HeadInfo attrCatHeader;
		attrCatBlockBuffer.getHeader(&attrCatHeader);
Attribute attrCatRecord [ATTRCAT_NO_ATTRS];
		attrCatBlockBuffer.getRecord(attrCatRecord, attrCatRecId.slot);

        // declare variable rootBlock which will be used to store the root
        // block field from the attribute catalog record.
        int rootBlock = attrCatRecord[ATTRCAT_ROOT_BLOCK_INDEX].nVal; // get root block from the record
        // (This will be used later to delete any indexes if it exists)
		
        // Update the Slotmap for the block by setting the slot as SLOT_UNOCCUPIED
        // Hint: use RecBuffer.getSlotMap and RecBuffer.setSlotMap
		unsigned char slotmap [attrCatHeader.numSlots];
		attrCatBlockBuffer.getSlotMap(slotmap);

		slotmap[attrCatRecId.slot] = SLOT_UNOCCUPIED;
		attrCatBlockBuffer.setSlotMap(slotmap);

        /* Decrement the numEntries in the header of the block corresponding to
           the attribute catalog entry and then set back the header
           using RecBuffer.setHeader */
		attrCatHeader.numEntries--;
		attrCatBlockBuffer.setHeader(&attrCatHeader);

        /* If number of entries become 0, releaseBlock is called after fixing
           the linked list.
        */
        if (attrCatHeader.numEntries == 0) {
            /* Standard DOUBLY Linked List Delete for a Block
               Get the header of the left block and set it's rblock to this
               block's rblock
            */

            // create a RecBuffer for lblock and call appropriate methods
            		RecBuffer prevBlock (attrCatHeader.lblock);
			
			HeadInfo leftHeader;
			prevBlock.getHeader(&leftHeader);

			leftHeader.rblock = attrCatHeader.rblock;
			prevBlock.setHeader(&leftHeader);


            if (attrCatHeader.rblock != INVALID_BLOCKNUM) 
			{
                /* Get the header of the right block and set it's lblock to
                   this block's lblock */
                // create a RecBuffer for rblock and call appropriate methods
				RecBuffer nextBlock (attrCatHeader.rblock);
				
				HeadInfo rightHeader;
				nextBlock.getHeader(&rightHeader);

				rightHeader.lblock = attrCatHeader.lblock;
				nextBlock.setHeader(&rightHeader);

            } 
			else 
			{
                // (the block being released is the "Last Block" of the relation.)
                /* update the Relation Catalog entry's LastBlock field for this
                   relation with the block number of the previous block. */

				RelCatEntry relCatEntryBuffer;
				RelCacheTable::getRelCatEntry(ATTRCAT_RELID, &relCatEntryBuffer);

				relCatEntryBuffer.lastBlk = attrCatHeader.lblock;
            }

            // (Since the attribute catalog will never be empty(why?), we do not
            //  need to handle the case of the linked list becoming empty - i.e
                        //  every block of the attribute catalog gets released.)

            // call releaseBlock()
			attrCatBlockBuffer.releaseBlock();
        }

		/*
        // (the following part is only relevant once indexing has been implemented)
        // if index exists for the attribute (rootBlock != -1), call bplus destroy
        if (rootBlock != -1) {
            // delete the bplus tree rooted at rootBlock using BPlusTree::bPlusDestroy()
        }
		*/
    }

    /*** Delete the entry corresponding to the relation from relation catalog ***/
    // Fetch the header of Relcat block
	// // relCatBlockBuffer = RecBuffer (RELCAT_BLOCK);

	HeadInfo relCatHeader;
	relCatBlockBuffer.getHeader(&relCatHeader);

    // TODO: Decrement the numEntries in the header of the block corresponding to the
    // TODO: relation catalog entry and set it back
	relCatHeader.numEntries--;
	relCatBlockBuffer.setHeader(&relCatHeader);

    /* Get the slotmap in relation catalog, update it by marking the slot as
       free(SLOT_UNOCCUPIED) and set it back. */
	unsigned char slotmap [relCatHeader.numSlots];
	relCatBlockBuffer.getSlotMap(slotmap);

	slotmap[relCatRecId.slot] = SLOT_UNOCCUPIED;
	relCatBlockBuffer.setSlotMap(slotmap);

    /*** Updating the Relation Cache Table ***/
    /** Update relation catalog record entry (number of records in relation
        catalog is decreased by 1) **/

	// Get the entry corresponding to relation catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)

	RelCatEntry relCatEntryBuffer;
	RelCacheTable::getRelCatEntry(RELCAT_RELID, &relCatEntryBuffer);

	relCatEntryBuffer.numRecs--;
	RelCacheTable::setRelCatEntry(RELCAT_RELID, &relCatEntryBuffer);

    /** Update attribute catalog entry (number of records in attribute catalog
        is decreased by numberOfAttributesDeleted) **/
    // i.e., #Records = #Records - numberOfAttributesDeleted

    // Get the entry corresponding to attribute catalog from the relation
    // cache and update the number of records and set it back
    // (using RelCacheTable::setRelCatEntry() function)

	RelCacheTable::getRelCatEntry(ATTRCAT_RELID, &relCatEntryBuffer);
	relCatEntryBuffer.numRecs -= numberOfAttributesDeleted;
	RelCacheTable::setRelCatEntry(ATTRCAT_RELID, &relCatEntryBuffer);


    return SUCCESS;
}