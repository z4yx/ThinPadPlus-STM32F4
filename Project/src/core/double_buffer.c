#include "common.h"
#include "double_buffer.h"
#include <stdlib.h>

#define DATA_ARRAY (b->selector ? b->data + b->capacity : b->data)

struct DoubleBuffer* DoubleBuffer_New(int size)
{
    struct DoubleBuffer* b;
    b = (struct DoubleBuffer*)malloc(sizeof(struct DoubleBuffer) + size*2);
    if(b){
        b->capacity = size;
        b->size = b->selector = 0;
    }
    return b;
}

void DoubleBuffer_Free(struct DoubleBuffer* b)
{
    free(b);
}

void DoubleBuffer_Clear(struct DoubleBuffer* b)
{
    b->size = 0;
}

bool DoubleBuffer_Push(struct DoubleBuffer* b, uint8_t element)
{
    __disable_irq();
    if(b->size == b->capacity){ //buffer is full
        __enable_irq();
        return false;
    }
    DATA_ARRAY[b->size++] = element;
    __enable_irq();
    return true;
}

int DoubleBuffer_SwapBuffer(struct DoubleBuffer* b, uint8_t **data_ptr)
{
    int size;
    __disable_irq();
    size = b->size;
    b->size = 0;
    *data_ptr = DATA_ARRAY;
    b->selector ^= 1;
    __enable_irq();
    return size;
}

int DoubleBuffer_Size(struct DoubleBuffer* b)
{
    return b->size;
}

