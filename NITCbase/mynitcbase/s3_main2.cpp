#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
//#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
#include <cstring>
int main(int argc, char *argv[]) {

    Disk disk_run;
    StaticBuffer buffer;
    OpenRelTable cache;

    /*
     * Print relation catalog information
     * for RELCAT and ATTRCAT.
     */

    for (int i = 0; i <= 1; i++) {

        RelCatEntry relCatEntry;

        // Get relation catalog entry
        RelCacheTable::getRelCatEntry(i, &relCatEntry);

        printf("Relation: %s\n", relCatEntry.relName);

        // Print all attributes of this relation
        for (int j = 0; j < relCatEntry.numAttrs; j++) {

            AttrCatEntry attrCatEntry;

            // Get attribute catalog entry
            AttrCacheTable::getAttrCatEntry(
                i,
                j,
                &attrCatEntry
            );

            printf("  %s: %d\n",
                   attrCatEntry.attrName,
                   attrCatEntry.attrType);
        }
    }

    return 0;
}