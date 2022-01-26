#include <stdio.h>
#include <iostream>
#include <unistd.h>

using namespace std;

typedef unsigned char Byte;
//Have a global array of one MB

//Have a pagesize
#define PAGESIZE 4096
//TODO idk the size this is supposed to be
int HEAPSIZE = 1048576;

typedef struct chunkhead
{
    int size;
    // occupied means info = 1, info = 0 is free
    unsigned int info = 0;
    //FIXME changed pointers to chunkheads (used to be unsigned char)
    unsigned char *next, *prev = NULL;
} chunkhead;

void *startofheap = NULL;

chunkhead *get_last_chunk()
{
    if (!startofheap) //I have a global void *startofheap = NULL;
        return NULL;
    chunkhead *ch = (chunkhead *)startofheap;
    for (; ch->next; ch = (chunkhead *)ch->next)
        ;
    return ch;
}

void analyze()
{
    printf("\n--------------------------------------------------------------\n");
    if (!startofheap)
    {
        printf("no heap\n");
        return;
    }
    chunkhead *ch = (chunkhead *)startofheap;
    for (int no = 0; ch; ch = (chunkhead *)ch->next, no++)
    {
        printf("%d | current addr: %p |", no, ch);
        printf("size: %d | ", ch->size);
        printf("info: %d | ", ch->info);
        printf("next: %p | ", ch->next);
        printf("prev: %p", ch->prev);
        printf(" \n");
    }
    printf("program break on address: %p\n", sbrk(0));
}

unsigned char *mymalloc(int size);
void myfree(unsigned char *address);

int main()
{

Byte*a[100];
analyze();//50% points
for(int i=0;i<100;i++)
a[i]= mymalloc(1000);
for(int i=0;i<90;i++)
myfree(a[i]);
analyze(); //50% of points if this is correct
myfree(a[95]);
 a[95] = mymalloc(1000);
analyze();//25% points, this new chunk should fill the smaller free one
//(best fit)
for(int i=90;i<100;i++)
myfree(a[i]);
analyze();// 25% should be an empty heap now with the start address
//from the program start

    return 0;
}

// fix return pointer value
unsigned char *mymalloc(int size)
{

    chunkhead *p = (chunkhead *)startofheap;

    chunkhead *best = NULL;

    for (; p != NULL; p = (chunkhead *)p->next)
    {
        if (p && p->info == 0 && (p->size > (size + sizeof(chunkhead))))
        {
            if (!best || best->size > p->size)
            {
                best = p;
            }
        }
    }

    // splitting best empty chunk
    if (best)
    {
        //page is now occupied
        best->info = 1;

        // allocating the proper amount of space for "size"
        int alloc_size = 0;
        while (size + sizeof(chunkhead) > (alloc_size))
        {
            alloc_size += PAGESIZE;
        }

        // amount of memory remaining = (rest mem) - (size we're allocating) - (new chunkhead size)
        int remaining_mem = best->size - alloc_size;

        // set allocated size for p
        best->size = alloc_size;
        // incrementing program break to include size of p chunk
        // printf("\n\nbefore allocating for %d bytes: %p\n", p->size, sbrk(0));
        // sbrk(p->size );
        // printf("after allocating: %p\n\n", sbrk(0));
        // building next chunk, getting space for head
        // location of next chunk head
        if (remaining_mem > 0)
        {
            printf("THERE WAS SPACE FOR ANOTHER NODE\n");
            Byte *loc = (Byte *)best + alloc_size;
            chunkhead *nextHead = (chunkhead *)loc;
            // chunkhead *nextHead = (chunkhead *)((unsigned char *)p + sizeof(chunkhead) + p->size);
            nextHead->next = best->next;
            nextHead->prev = (unsigned char *)p;
            nextHead->size = remaining_mem;
            best->next = (unsigned char *)nextHead;
        }

        return (unsigned char *)best + sizeof(chunkhead);
    }
    else
    {

        if(HEAPSIZE < size){
            return NULL;
        }

        HEAPSIZE -= size;

        int alloc = 0;
        while ((size + sizeof(chunkhead) > alloc))
        {
            alloc += PAGESIZE;
        }

        chunkhead *newNode = (chunkhead *)sbrk(alloc);

        chunkhead *last = get_last_chunk();

        newNode->prev = (Byte *)last;
        newNode->size = alloc;
        newNode->info = 1;
        if (last)
        {
            last->next = (Byte *)newNode;
        }

        else
        {
            startofheap = newNode;
        }
        return (Byte *)newNode + sizeof(chunkhead);
    }

    // printf("got to the end of for loop without answer for size = %d!\n", size);

    return NULL;
}

//TODO find optimized spot
void myfree(unsigned char *address)
{

    chunkhead *iter = (chunkhead *)startofheap;

    for (; iter != NULL; iter = (chunkhead *)iter->next)
    {

        chunkhead *lastChunk = (chunkhead *)iter->prev;
        chunkhead *nextChunk = (chunkhead *)iter->next;

        //TODO combine neighboring free blocks
        if ((unsigned char *)iter == (unsigned char *)address - sizeof(chunkhead))
        {
            // free
            iter->info = 0;
            // printf("found the address %p\n" , (void *)  address);
        }

        if (iter->info == 0 && nextChunk && nextChunk->info == 0)
        {
            // curr is first, next is second
            chunkhead *kill = nextChunk;
            chunkhead *keep = iter;
            keep->size += kill->size;
            keep->next = kill->next;
            chunkhead *newNext = (chunkhead *)kill->next;
            newNext->prev = (unsigned char *)keep;
        }

        if (iter->info == 0 && lastChunk && lastChunk->info == 0)
        {
            // last is first, curr is second
            chunkhead *kill = iter;
            chunkhead *keep = lastChunk;
            keep->size += kill->size;
            keep->next = kill->next;
            chunkhead *newNext = (chunkhead *)kill->next;
            if (newNext)
            {
                newNext->prev = (unsigned char *)keep;
            }
            iter = keep;
        }

        if (iter->info == 0 && iter == get_last_chunk())
        {
            // sbrk(-(iter->size + sizeof(chunkhead)));
            chunkhead *newLast = (chunkhead *)iter->prev;
            printf("hello\n");
            if (newLast)
            {
                printf("not the last value");
                newLast->next = NULL;
            }
            else
            {
                // set back program break
                printf("trying to remove last value\n");
                startofheap = NULL;
            }
            sbrk(-PAGESIZE);
            return;
        }
    }
}
