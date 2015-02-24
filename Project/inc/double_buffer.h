#ifndef DOUBLE_BUFFER_H__
#define DOUBLE_BUFFER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct DoubleBuffer
{
    int size, capacity;
    uint8_t selector;
    uint8_t data[];
};

struct DoubleBuffer* DoubleBuffer_New(int size);
void DoubleBuffer_Clear(struct DoubleBuffer* b);
void DoubleBuffer_Free(struct DoubleBuffer* b);
bool DoubleBuffer_Push(struct DoubleBuffer* b, uint8_t element);
int DoubleBuffer_SwapBuffer(struct DoubleBuffer* b, uint8_t **data_ptr);
int DoubleBuffer_Size(struct DoubleBuffer* b);

#ifdef __cplusplus
};
#endif

#endif
