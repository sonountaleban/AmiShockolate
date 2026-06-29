#ifdef __AROS__
#include <stdlib.h>
#include <stdio.h>
#include "lg_types.h"

/*struct Entry *pFirstEntry = NULL;

struct Entry *searchEntry(const char *pMemoryBlock)
{
    struct Entry *pEntry = pFirstEntry;
    while (pEntry)
    {
        if (pEntry->pMemoryBlock == pMemoryBlock)
        {
            return pEntry;
        }

        pEntry = pEntry->pNextEntry;
    }

    return NULL;
}

char *addEntry(const char *pFileName, int lineNumber, int sizeMemoryBlock)
{
    struct Entry *pEntry;

    if (sizeMemoryBlock <= 0)
    {
        return NULL;
    }

    pEntry = malloc(sizeof(struct Entry));
    if (!pEntry)
    {
        return NULL;
    }

    sprintf(pEntry->fileNameLineNumber, "%s:%d", pFileName, lineNumber);
    pEntry->pNextEntry = NULL;
    pEntry->sizeMemoryBlock = sizeMemoryBlock;
    pEntry->pMemoryBlock = malloc(sizeof(char) * sizeMemoryBlock);
    if (!pEntry->pMemoryBlock)
    {
        free(pEntry);

        return NULL;
    }

    if (pFirstEntry)
    {
        struct Entry *pCursor = pFirstEntry;
        while (pCursor->pNextEntry)
        {
            pCursor = pCursor->pNextEntry;
        }

        pCursor->pNextEntry = pEntry;
        pEntry->pPreviousEntry = pCursor;
    }
    else
    {
        pEntry->pPreviousEntry = NULL;
        pFirstEntry = pEntry;
    }

    return pEntry->pMemoryBlock;
}

void removeEntry(const char *pMemoryBlock)
{
    if (!pMemoryBlock)
    {
        return;
    }

    struct Entry *pEntry = searchEntry(pMemoryBlock);
    if (pEntry)
    {
        free(pEntry->pMemoryBlock);

        if (pEntry == pFirstEntry)
        {
            pFirstEntry = pEntry->pNextEntry;
        }
        else
        {
            pEntry->pPreviousEntry->pNextEntry = pEntry->pNextEntry;
            if (pEntry->pNextEntry)
            {
                pEntry->pNextEntry->pPreviousEntry = pEntry->pPreviousEntry;
            }
        }

        free(pEntry);
    }
}

void printEntries()
{
    struct Entry *pEntry = pFirstEntry;
    while (pEntry)
    {
        printf("%s, %d bytes\n", pEntry->fileNameLineNumber, pEntry->sizeMemoryBlock);

        pEntry = pEntry->pNextEntry;
    }
}

void saveEntries(const char *pFileName)
{
    if (!pFileName || strlen(pFileName) <= 0)
    {
        return;
    }

    FILE *pFile = fopen(pFileName, "w");
    if (pFile)
    {
        struct Entry *pEntry = pFirstEntry;
        while (pEntry)
        {
            fprintf(pFile, "%s ### %d ###", pEntry->fileNameLineNumber, pEntry->sizeMemoryBlock);
            for (int i = 0; i < pEntry->sizeMemoryBlock; i ++)
            {
                fprintf(pFile, " %02x", pEntry->pMemoryBlock[i]);
            }
            fprintf(pFile, "\n");

            pEntry = pEntry->pNextEntry;
        }

        fclose(pFile);
    }
}*/
#endif
