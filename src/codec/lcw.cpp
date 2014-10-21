#include "eastwood/codec/lcw.h"
#include "eastwood/Exception.h"

#include <algorithm>

namespace eastwood { namespace codec {

int decodeLCW(const uint8_t* source, uint8_t* dest, int destsize)
{
    uint8_t *start = dest;
    uint8_t *end = dest + destsize;

    while (dest != end) {
        uint8_t flag;
        uint16_t size;
        uint16_t offset;

        flag = *source++;

        /* Short move, relative */
        if ((flag & 0x80) == 0) {
            size = (flag >> 4) + 3;
            if (size > end - dest) size = end - dest;

            offset = ((flag & 0xF) << 8) + (*source++);

            /* This decoder assumes memcpy. As some platforms implement memcpy as memmove, this is much safer */
            for (; size > 0; size--) { *dest = *(dest - offset); dest++; }
            continue;
        }

        /* Exit */
        if (flag == 0x80) break;

        /* Long set */
        if (flag == 0xFE) {
            size = *source++;
            size += (*source++) << 8;
            if (size > end - dest) size = end - dest;

            memset(dest, (*source++), size);
            dest += size;
            continue;
        }

        /* Long move, absolute */
        if (flag == 0xFF) {
            uint8_t *s;

            size = *source++;
            size += (*source++) << 8;
            if (size > end - dest) size = end - dest;

            offset = *source++;
            offset += (*source++) << 8;

            s = end - destsize + offset;
            /* This decoder assumes memcpy. As some platforms implement memcpy as memmove, this is much safer */
            for (; size > 0; size--) *dest++ = *s++;
            continue;
        }

        /* Short move, absolute */
        if ((flag & 0x40) != 0) {
            uint8_t *s;

            size = (flag & 0x3F) + 3;
            if (size > end - dest) size = end - dest;

            offset = *source++;
            offset += (*source++) << 8;

            s = end - destLength + offset;
            /* This decoder assumes memcpy. As some platforms implement memcpy as memmove, this is much safer */
            for (; size > 0; size--) *dest++ = *s++;
            continue;
        }

        /* Short copy */
        {
            size = flag & 0x3F;
            if (size > end - dest) size = end - dest;

            /* This decoder assumes memcpy. As some platforms implement memcpy as memmove, this is much safer */
            for (; size > 0; size--) *dest++ = *source++;
            continue;
        }
    }

    return dest - start;
}

int LCW_Comp(char *srcp, char* destp, int datasize)
{
  char* readp; // esi@1
  char* writep; // edi@1
  int v5; // edi@2
  char v6; // al@3
  int v7; // ebx@4
  int v8; // edi@4
  int v9; // ecx@4
  int v10; // edi@7
  int16_t v11; // cx@7
  char v12; // zf@10
  int v13; // ecx@10
  int v14; // edx@15
  int v15; // ebx@15
  int v16; // edi@15
  char v17; // zf@15
  int v18; // ecx@15
  int v19; // eax@20
  int v20; // edi@22
  int16_t v21; // ax@24
  int16_t v22; // ax@25
  char v23; // t1@25
  char v24; // al@31
  char* destmark1p; // [sp+14h] [bp-24h]@1
  int v27; // [sp+18h] [bp-20h]@1
  char* srcmarkp; // [sp+20h] [bp-18h]@1
  char* destmark2p; // [sp+24h] [bp-14h]@1
  char* endp; // [sp+28h] [bp-10h]@1
  signed int v31; // [sp+2Ch] [bp-Ch]@2
  signed int v32; // [sp+30h] [bp-8h]@1
  int writemakerp; // [sp+34h] [bp-4h]@2

  v27 = 0;
  endp = datasize + srcp;
  v32 = 1;
  destmark1p = destp;
  srcmarkp = srcp;
  destmark2p = destp;
  *destp = -127;
  readp = srcp + 1;
  *(destp + 1) = *srcp;
  writep = destp + 2;
  do
  {
    writemakerp = writep;
    v5 = srcmarkp;
    v31 = 1;
LABEL_3:
    while ( 1 )
    {
      v6 = *readp;
      if ( *readp != *(readp + 64) )
        break;
      v7 = v5;
      v8 = readp;
      v9 = endp - readp;
      do
      {
        if ( !v9 )
          break;
        v12 = *v8++ == v6;
        --v9;
      }
      while ( v12 );
      v10 = v8 - 1;
      v11 = v10 - readp;
      if ( (v10 - readp) < 0x41 )
      {
        v5 = v7;
        break;
      }
      v32 = 0;
      readp = v10;
      *writemakerp = -2;
      *(writemakerp + 1) = v11;
      *(writemakerp + 3) = v6;
      writemakerp += 4;
      v5 = v7;
    }
    while ( 1 )
    {
      v13 = readp - v5;
      v12 = readp == v5;
      if ( readp == v5 )
        break;
      do
      {
        if ( !v13 )
          break;
        v12 = *v5++ == v6;
        --v13;
      }
      while ( !v12 );
      if ( !v12 )
        break;
      if ( *(readp + v31 - 1) == *(v5 + v31 - 2) )
      {
        v14 = readp;
        v15 = v5;
        v16 = v5 - 1;
        v18 = endp - readp;
        v17 = endp == readp;
        do
        {
          if ( !v18 )
            break;
          v17 = *readp++ == *v16++;
          --v18;
        }
        while ( v17 );
        if ( v17 )
          ++v16;
        readp = v14;
        v19 = v16 - v15;
        v5 = v15;
        if ( v19 >= v31 )
        {
          v31 = v19;
          v27 = v15 - 1;
        }
        goto LABEL_3;
      }
    }
    v20 = writemakerp;
    if ( v31 <= 2 )
    {
      if ( v32 )
        goto LABEL_30;
      while ( 1 )
      {
        destmark2p = v20;
        *v20++ = -128;
LABEL_30:
        if ( *destmark2p != -65 )
        {
          ++*destmark2p;
          v24 = *readp++;
          *v20 = v24;
          writep = v20 + 1;
          v32 = 1;
          goto LABEL_35;
        }
      }
    }
    if ( v31 > 0xA || (v21 = readp - v27, (readp - v27) > 0xFFF) )
    {
      if ( v31 > 0x40 )
      {
        *writemakerp = -1;
        *(writemakerp + 1) = v31;
        v20 = writemakerp + 3;
      }
      else
      {
        *writemakerp = (v31 - 3) | 0xC0;
        v20 = writemakerp + 1;
      }
      v22 = v27 - srcmarkp;
    }
    else
    {
      v23 = 16 * (v31 - 3) + (*((char*)&(v21)+1));
      (*((char*)&(v22)+1)) = v21;
      (*((char*)&(v22))) = v23;
    }
    *v20 = v22;
    writep = v20 + 2;
    readp += v31;
    v32 = 0;
LABEL_35:
    ;
  }
  while ( readp < endp );
  *writep = -128;
  return writep + 1 - destmark1p;
}

int encodeLcw(const uint8_t* src, uint8_t* dest, int datasize)
{
    
}
    
} } //eastwood codec