// RingBuffer.hpp

#pragma once

#include <Arduino.h>

/***********************************************************************************
 Class RingBuffer<M, N>
************************************************************************************

  Circular Buffer FIFO data structure, with M arrays of N int elements
*/

template <int M, int N> class RingBuffer
{
  public:
    // Constructor
    RingBuffer() 
    {
      pStart = buffer;
      pHead = pStart; 
      pTail = pStart; 
      pEnd = &buffer[M*N-1];
      size = 0;
    }
    
    // Number of elements
    int Size() {return size;}

    // Operations
    void Push(int t[]) 
    {
      if (size < M)
      {
        size++;
        for (int i=0; i<N; i++) *pTail++ = *t++;
        if (pTail>pEnd) pTail = pStart; 
      }
    }

    int Pop(int t[]) // Before test if Size()>0
    {
      size--;
      for (int i=0; i<N; i++) *t++ = *pHead++;
      if (pHead>pEnd) pHead = pStart; 
    }

  private:
    int buffer[M*N];
    int *pHead, *pTail, *pStart, *pEnd;
    int size;
};