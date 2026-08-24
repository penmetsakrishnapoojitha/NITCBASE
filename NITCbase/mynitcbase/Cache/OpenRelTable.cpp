#include "OpenRelTable.h"
#include <bits/stdc++.h>
#include <cstring>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];


OpenRelTable::OpenRelTable() {

    /************ Initialize Open Relation Table ************/

    for (int i = 0; i < MAX_OPEN; ++i) {

        tableMetaInfo[i].free = true;
        tableMetaInfo[i].relName[0] = '\0';

        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
    }


    /************ Setting up Relation Cache entries ************/

    RecBuffer relCatBlock(RELCAT_BLOCK);

    Attribute relCatRecord[RELCAT_NO_ATTRS];


    /**** Relation Catalog - Slot 0 ****/

    relCatBlock.getRecord(
        relCatRecord,
        RELCAT_SLOTNUM_FOR_RELCAT
    );

    RelCacheEntry relCacheEntry;

    RelCacheTable::recordToRelCatEntry(
        relCatRecord,
        &relCacheEntry.relCatEntry
    );

    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

    RelCacheTable::relCache[RELCAT_RELID] =
        (RelCacheEntry *)malloc(sizeof(RelCacheEntry));

    *(RelCacheTable::relCache[RELCAT_RELID]) =
        relCacheEntry;


    /**** Attribute Catalog - Slot 1 ****/

    relCatBlock.getRecord(
        relCatRecord,
        RELCAT_SLOTNUM_FOR_ATTRCAT
    );

    RelCacheTable::recordToRelCatEntry(
        relCatRecord,
        &relCacheEntry.relCatEntry
    );

    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

    RelCacheTable::relCache[ATTRCAT_RELID] =
        (RelCacheEntry *)malloc(sizeof(RelCacheEntry));

    *(RelCacheTable::relCache[ATTRCAT_RELID]) =
        relCacheEntry;


    /**** Students Relation - Slot 2 ****/

    relCatBlock.getRecord(
        relCatRecord,
        2
    );

    RelCacheTable::recordToRelCatEntry(
        relCatRecord,
        &relCacheEntry.relCatEntry
    );

    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = 2;

    RelCacheTable::relCache[2] =
        (RelCacheEntry *)malloc(sizeof(RelCacheEntry));

    *(RelCacheTable::relCache[2]) =
        relCacheEntry;


    /************ Setting up Attribute Cache entries ************/

    RecBuffer attrCatBlock(ATTRCAT_BLOCK);

    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

    AttrCacheEntry *head = nullptr;
    AttrCacheEntry *last = nullptr;


    /**** 1. Relation Catalog Attributes - Slots 0 to 5 ****/

    for (int i = 0; i < 6; i++) {

        attrCatBlock.getRecord(
            attrCatRecord,
            i
        );

        AttrCacheEntry *attrCacheEntry =
            (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));

        AttrCacheTable::recordToAttrCatEntry(
            attrCatRecord,
            &attrCacheEntry->attrCatEntry
        );

        attrCacheEntry->recId.block = ATTRCAT_BLOCK;
        attrCacheEntry->recId.slot = i;

        if (i == 0) {
            head = attrCacheEntry;
            last = attrCacheEntry;
        }
        else {
            last->next = attrCacheEntry;
            last = attrCacheEntry;
        }
    }

    last->next = nullptr;

    AttrCacheTable::attrCache[RELCAT_RELID] = head;


    /**** 2. Attribute Catalog Attributes - Slots 6 to 11 ****/

    head = nullptr;
    last = nullptr;

    for (int i = 6; i < 12; i++) {

        attrCatBlock.getRecord(
            attrCatRecord,
            i
        );

        AttrCacheEntry *attrCacheEntry =
            (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));

        AttrCacheTable::recordToAttrCatEntry(
            attrCatRecord,
            &attrCacheEntry->attrCatEntry
        );

        attrCacheEntry->recId.block = ATTRCAT_BLOCK;
        attrCacheEntry->recId.slot = i;

        if (i == 6) {
            head = attrCacheEntry;
            last = attrCacheEntry;
        }
        else {
            last->next = attrCacheEntry;
            last = attrCacheEntry;
        }
    }

    last->next = nullptr;

    AttrCacheTable::attrCache[ATTRCAT_RELID] = head;


    /**** 3. Students Relation Attributes - Slots 12 to 15 ****/

    head = nullptr;
    last = nullptr;

    for (int i = 12; i < 16; i++) {

        attrCatBlock.getRecord(
            attrCatRecord,
            i
        );

        AttrCacheEntry *attrCacheEntry =
            (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));

        AttrCacheTable::recordToAttrCatEntry(
            attrCatRecord,
            &attrCacheEntry->attrCatEntry
        );

        attrCacheEntry->recId.block = ATTRCAT_BLOCK;
        attrCacheEntry->recId.slot = i;

        if (i == 12) {
            head = attrCacheEntry;
            last = attrCacheEntry;
        }
        else {
            last->next = attrCacheEntry;
            last = attrCacheEntry;
        }
    }

    last->next = nullptr;

    AttrCacheTable::attrCache[2] = head;


    /************ Set metadata for catalog relations ************/

    tableMetaInfo[RELCAT_RELID].free = false;
    tableMetaInfo[ATTRCAT_RELID].free = false;

    strcpy(
        tableMetaInfo[RELCAT_RELID].relName,
        RELCAT_RELNAME
    );

    strcpy(
        tableMetaInfo[ATTRCAT_RELID].relName,
        ATTRCAT_RELNAME
    );
}


/************************************************************
 * getFreeOpenRelTableEntry
 ************************************************************/

int OpenRelTable::getFreeOpenRelTableEntry() {

    for (int i = 0; i < MAX_OPEN; i++) {

        if (tableMetaInfo[i].free == true) {
            return i;
        }
    }

    return E_CACHEFULL;
}


/************************************************************
 * getRelId
 ************************************************************/

int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

    for (int i = 0; i < MAX_OPEN; i++) {

        if (!tableMetaInfo[i].free &&
            strcmp(tableMetaInfo[i].relName, relName) == 0) {

            return i;
        }
    }

    return E_RELNOTOPEN;
}


/************************************************************
 * openRel
 ************************************************************/

int OpenRelTable::openRel(char relName[ATTR_SIZE]) {

    /************ Check whether relation is already open ************/

    int ret = OpenRelTable::getRelId(relName);

    if (ret >= 0) {
        return ret;
    }


    /************ Get free OpenRelTable entry ************/

    int relId =
        OpenRelTable::getFreeOpenRelTableEntry();

    if (relId == E_CACHEFULL) {
        return E_CACHEFULL;
    }


    /************ Search Relation Catalog ************/

    Attribute attrVal;

    strcpy(
        attrVal.sVal,
        relName
    );

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    RecId relcatRecId =
        BlockAccess::linearSearch(
            RELCAT_RELID,
            RELCAT_ATTR_RELNAME,
            attrVal,
            EQ
        );


    /************ Relation does not exist ************/

    if (relcatRecId.block == -1 &&
        relcatRecId.slot == -1) {

        return E_RELNOTEXIST;
    }


    /************ Read Relation Catalog record ************/

    RecBuffer relationBuffer(
        relcatRecId.block
    );

    Attribute relationRecord[RELCAT_NO_ATTRS];

    relationBuffer.getRecord(
        relationRecord,
        relcatRecId.slot
    );


    /************ Create Relation Cache Entry ************/

    RelCacheEntry *relCacheBuffer =
        (RelCacheEntry *)malloc(
            sizeof(RelCacheEntry)
        );


    RelCacheTable::recordToRelCatEntry(
        relationRecord,
        &(relCacheBuffer->relCatEntry)
    );


    relCacheBuffer->recId.block =
        relcatRecId.block;

    relCacheBuffer->recId.slot =
        relcatRecId.slot;


    /************ Store in Relation Cache ************/

    RelCacheTable::relCache[relId] =
        relCacheBuffer;


    /************ Number of attributes ************/

    int numberOfAttributes =
        RelCacheTable::relCache[relId]
            ->relCatEntry.numAttrs;


    /************ Create Attribute Cache linked list ************/

    AttrCacheEntry *head = nullptr;
    AttrCacheEntry *last = nullptr;

    for (int i = 0;
         i < numberOfAttributes;
         i++) {

        AttrCacheEntry *newEntry =
            (AttrCacheEntry *)malloc(
                sizeof(AttrCacheEntry)
            );

        newEntry->next = nullptr;

        if (head == nullptr) {

            head = newEntry;
            last = newEntry;
        }
        else {

            last->next = newEntry;
            last = newEntry;
        }
    }


    /************ Fill Attribute Cache entries ************/

    AttrCacheEntry *attrCacheEntry = head;

    RelCacheTable::resetSearchIndex(
        ATTRCAT_RELID
    );


    for (int attr = 0;
         attr < numberOfAttributes;
         attr++) {

        RecId attrcatRecId =
            BlockAccess::linearSearch(
                ATTRCAT_RELID,
                RELCAT_ATTR_RELNAME,
                attrVal,
                EQ
            );


        /************ Attribute not found ************/

        if (attrcatRecId.block == -1 &&
            attrcatRecId.slot == -1) {

            /**** Free already allocated attribute entries ****/

            AttrCacheEntry *curr = head;

            while (curr != nullptr) {

                AttrCacheEntry *next =
                    curr->next;

                free(curr);

                curr = next;
            }

            /**** Free relation cache entry ****/

            free(
                RelCacheTable::relCache[relId]
            );

            RelCacheTable::relCache[relId] =
                nullptr;

            return E_RELNOTEXIST;
        }


        /************ Read Attribute Catalog record ************/

        RecBuffer attrCatBlock(
            attrcatRecId.block
        );

        Attribute attrCatRecord[
            ATTRCAT_NO_ATTRS
        ];


        attrCatBlock.getRecord(
            attrCatRecord,
            attrcatRecId.slot
        );


        /************ Convert record to cache entry ************/

        AttrCacheTable::recordToAttrCatEntry(
            attrCatRecord,
            &(attrCacheEntry->attrCatEntry)
        );


        attrCacheEntry->recId.block =
            attrcatRecId.block;

        attrCacheEntry->recId.slot =
            attrcatRecId.slot;


        attrCacheEntry =
            attrCacheEntry->next;
    }


    /************ Store Attribute Cache list ************/

    AttrCacheTable::attrCache[relId] =
        head;


    /************ Update Open Relation Table metadata ************/

    tableMetaInfo[relId].free = false;

    strcpy(
        tableMetaInfo[relId].relName,
        relName
    );


    return relId;
}


/************************************************************
 * closeRel
 ************************************************************/

int OpenRelTable::closeRel(int relId) {

    /************ Catalog relations cannot be closed ************/

    if (relId == RELCAT_RELID ||
        relId == ATTRCAT_RELID) {

        return E_NOTPERMITTED;
    }


    /************ Check bounds ************/

    if (relId < 0 ||
        relId >= MAX_OPEN) {

        return E_OUTOFBOUND;
    }


    /************ Check whether relation is open ************/

    if (tableMetaInfo[relId].free) {

        return E_RELNOTOPEN;
    }


    /************ Free Relation Cache ************/

    if (RelCacheTable::relCache[relId] != nullptr) {

        free(
            RelCacheTable::relCache[relId]
        );

        RelCacheTable::relCache[relId] =
            nullptr;
    }


    /************ Free Attribute Cache linked list ************/

    AttrCacheEntry *curr =
        AttrCacheTable::attrCache[relId];

    while (curr != nullptr) {

        AttrCacheEntry *next =
            curr->next;

        free(curr);

        curr = next;
    }


    AttrCacheTable::attrCache[relId] =
        nullptr;


    /************ Update metadata ************/

    tableMetaInfo[relId].free = true;

    tableMetaInfo[relId].relName[0] = '\0';


    return SUCCESS;
}
OpenRelTable::~OpenRelTable() {

    /************ Close all opened user relations ************/

    for (int i = 2;
         i < MAX_OPEN;
         ++i) {

        if (!tableMetaInfo[i].free) {

            OpenRelTable::closeRel(i);
        }
    }

    int i = 0;
    while (i < 2) {
        if (RelCacheTable::relCache[i] != nullptr) {

            free(
                RelCacheTable::relCache[i]
            );

            RelCacheTable::relCache[i] =
                nullptr;
        }

        if (AttrCacheTable::attrCache[i] != nullptr) {

            AttrCacheEntry *curr =
                AttrCacheTable::attrCache[i];

            while (curr != nullptr) {

                AttrCacheEntry *next =
                    curr->next;

                free(curr);

                curr = next;
            }

            AttrCacheTable::attrCache[i] =
                nullptr;
        }

        i++;
    }
}