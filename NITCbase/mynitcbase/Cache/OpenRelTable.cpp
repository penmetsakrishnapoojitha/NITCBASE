#include "OpenRelTable.h"
#include <bits/stdc++.h>
#include <cstring>

OpenRelTable::OpenRelTable() {

  // Initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
  }

  /************ Setting up Relation Cache entries ************/

  RecBuffer relCatBlock(RELCAT_BLOCK);
  Attribute relCatRecord[RELCAT_NO_ATTRS];

  /**** Relation Catalog in Relation Cache (Slot 0) ****/
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);
  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

  /**** Attribute Catalog in Relation Cache (Slot 1) ****/
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;
  RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = relCacheEntry;

  /**** Students Relation in Relation Cache (Slot 2) ****/
  relCatBlock.getRecord(relCatRecord, 2);
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = 2;
  RelCacheTable::relCache[2] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[2]) = relCacheEntry;


  /************ Setting up Attribute Cache entries ************/

  RecBuffer attrCatBlock(ATTRCAT_BLOCK);
  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  struct AttrCacheEntry *head = nullptr, *last = nullptr;

  /**** 1. Relation Catalog Attributes (Slots 0 to 5) ****/
  for (int i = 0; i < 6; i++) {
    attrCatBlock.getRecord(attrCatRecord, i);
    struct AttrCacheEntry* attrCacheEntry = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &attrCacheEntry->attrCatEntry);
    attrCacheEntry->recId.block = ATTRCAT_BLOCK;
    attrCacheEntry->recId.slot = i;
    if (i == 0) {
      head = attrCacheEntry;
      last = attrCacheEntry;
    } else {
      last->next = attrCacheEntry;
      last = attrCacheEntry;
    }
  }
  last->next = nullptr;
  AttrCacheTable::attrCache[RELCAT_RELID] = head;


  /**** 2. Attribute Catalog Attributes (Slots 6 to 11) - WAS MISSING ****/
  for (int i = 6; i < 12; i++) {
    attrCatBlock.getRecord(attrCatRecord, i);
    struct AttrCacheEntry* attrCacheEntry = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &attrCacheEntry->attrCatEntry);
    attrCacheEntry->recId.block = ATTRCAT_BLOCK;
    attrCacheEntry->recId.slot = i;
    if (i == 6) {
      head = attrCacheEntry;
      last = attrCacheEntry;
    } else {
      last->next = attrCacheEntry;
      last = attrCacheEntry;
    }
  }
  last->next = nullptr;
  AttrCacheTable::attrCache[ATTRCAT_RELID] = head;


  /**** 3. Students Relation Attributes (Slots 12 to 15) ****/
  for (int i = 12; i < 16; i++) {
    attrCatBlock.getRecord(attrCatRecord, i);
    struct AttrCacheEntry* attrCacheEntry = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &attrCacheEntry->attrCatEntry);
    attrCacheEntry->recId.block = ATTRCAT_BLOCK;
    attrCacheEntry->recId.slot = i;
    if (i == 12) {
      head = attrCacheEntry;
      last = attrCacheEntry;
    } else {
      last->next = attrCacheEntry;
      last = attrCacheEntry;
    }
  }
  last->next = nullptr;
  AttrCacheTable::attrCache[2] = head;

}

int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {
  if (!strcmp(relName, RELCAT_RELNAME)) return RELCAT_RELID;
  if (!strcmp(relName, ATTRCAT_RELNAME)) return ATTRCAT_RELID;
  if (!strcmp(relName, "Students")) return 2;

  return E_RELNOTOPEN;
}

OpenRelTable::~OpenRelTable() {
  for (int i = 0; i < MAX_OPEN; ++i) {
    if (RelCacheTable::relCache[i] != nullptr) {
      free(RelCacheTable::relCache[i]);
      RelCacheTable::relCache[i] = nullptr;
    }

    if (AttrCacheTable::attrCache[i] != nullptr) {
      AttrCacheEntry *curr = AttrCacheTable::attrCache[i];
      while (curr != nullptr) {
        AttrCacheEntry *next = curr->next;
        free(curr);
        curr = next;
      }
      AttrCacheTable::attrCache[i] = nullptr;
    }
  }
}