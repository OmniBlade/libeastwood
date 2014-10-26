#include "eastwood/codec/lcw.h"
#include "eastwood/Log.h"
#include "eastwood/OStream.h"
#include "eastwood/IStream.h"

#include <algorithm>
#include <vector>

namespace eastwood { namespace codec {
    
int decodeLCW(const uint8_t* source, uint8_t* dest)
{
    uint8_t *start = dest;
    //uint8_t *end = dest + destsize;

    while (true) {
        uint8_t flag;
        uint16_t size;
        uint16_t offset;

        flag = *source++;

        /* Short move, relative */
        if ((flag & 0x80) == 0) {
            size = (flag >> 4) + 3;
            //if (size > end - dest) size = end - dest;

            offset = ((flag & 0xF) << 8) + (*source++);

            /* This decoder assumes memcpy. As some platforms implement memcpy as memmove, this is much safer */
            for (; size > 0; size--) { *dest = *(dest - offset); dest++; }
            continue;
        }

        /* Exit */
        if (flag == 0x80) {
            break;
        }

        /* Long set */
        if (flag == 0xFE) {
            size = *source++;
            size += (*source++) << 8;
            //if (size > end - dest) size = end - dest;

            memset(dest, (*source++), size);
            dest += size;
            continue;
        }

        /* Long move, absolute */
        if (flag == 0xFF) {
            uint8_t *s;

            size = *source++;
            size += (*source++) << 8;
            //if (size > end - dest) size = end - dest;

            offset = *source++;
            offset += (*source++) << 8;

            //this is absolute from start of file in CnC
            //s = end - destsize + offset;
            s = start + offset;
            /* This decoder assumes memcpy. As some platforms implement memcpy as memmove, this is much safer */
            for (; size > 0; size--) *dest++ = *s++;
            continue;
        }

        /* Short move, absolute */
        if ((flag & 0x40) != 0) {
            uint8_t *s;

            size = (flag & 0x3F) + 3;
            //if (size > end - dest) size = end - dest;

            offset = *source++;
            offset += (*source++) << 8;

            //this is absolute from start of file in CnC
            //s = end - destsize + offset;
            s = start + offset;
            /* This decoder assumes memcpy. As some platforms implement memcpy as memmove, this is much safer */
            for (; size > 0; size--) *dest++ = *s++;
            continue;
        }

        /* Short copy */
        {
            size = flag & 0x3F;
            //if (size > end - dest) size = end - dest;

            /* This decoder assumes memcpy. As some platforms implement memcpy as memmove, this is much safer */
            for (; size > 0; size--) *dest++ = *source++;
            continue;
        }
    }
    
    return dest - start;
}

int decodeLCW(std::istream& stream, uint8_t* dest)
{
    IStream& _stream= reinterpret_cast<IStream&>(stream);
    uint8_t *start = dest;
    //uint8_t *end = dest + destsize;

    while (true) {
        uint8_t flag;
        uint16_t size;
        uint16_t offset;

        flag = _stream.get();

        /* Short move, relative */
        /* 0cccpppp p (1) */
        if ((flag & 0x80) == 0) {
            size = (flag >> 4) + 3;
            //if (size > end - dest) size = end - dest;

            offset = ((flag & 0xF) << 8) + (_stream.get());

            /* This decoder assumes memcpy. As some platforms implement memcpy as memmove, this is much safer */
            for (; size > 0; size--) { *dest = *(dest - offset); dest++; }
            continue;
        }

        /* Exit */
        if (flag == 0x80) {
            break;
        }

        /* Long set */
        /* 11111110 c c v(4) */
        if (flag == 0xFE) {
            size = _stream.get();
            size += (_stream.get()) << 8;
            //if (size > end - dest) size = end - dest;

            memset(dest, (_stream.get()), size);
            dest += size;
            continue;
        }

        /* Long move, absolute */
        /* 11111111 c c p p (5) */
        if (flag == 0xFF) {
            uint8_t *s;

            size = _stream.get();
            size += (_stream.get()) << 8;
            //if (size > end - dest) size = end - dest;

            offset = _stream.get();
            offset += (_stream.get()) << 8;
            
            //this is absolute from start of file in CnC
            //s = end - destsize + offset;
            s = start + offset;
            /* This decoder assumes memcpy. As some platforms implement memcpy as memmove, this is much safer */
            for (; size > 0; size--) *dest++ = *s++;
            continue;
        }

        /* Short move, absolute */
        if ((flag & 0x40) != 0) {
            uint8_t *s;

            size = (flag & 0x3F) + 3;
            //if (size > end - dest) size = end - dest;

            offset = _stream.get();
            offset += (_stream.get()) << 8;

            //this is absolute from start of file in CnC
            //s = end - destsize + offset;
            s = start + offset;
            /* This decoder assumes memcpy. As some platforms implement memcpy as memmove, this is much safer */
            for (; size > 0; size--) *dest++ = *s++;
            continue;
        }

        /* Short copy */
        {
            size = flag & 0x3F;
            //if (size > end - dest) size = end - dest;

            /* This decoder assumes memcpy. As some platforms implement memcpy as memmove, this is much safer */
            for (; size > 0; size--) *dest++ = _stream.get();
            continue;
        }
    }
    
    return dest - start;
}

void applyXorDelta(const uint8_t* source , uint8_t* dest)
{
    while (true) {
        uint16_t flag;

        flag = *source++;

        if (flag == 0) {
            flag = *source++;
            for (; flag > 0; flag--) {
                *dest++ ^= *source;
            }
            source++;

            continue;
        }

        if ((flag & 0x80) == 0) {
            for (; flag > 0; flag--) {
                *dest++ ^= *source++;
            }
            continue;
        }

        if (flag != 0x80) {
            dest += flag & 0x7F;
            continue;
        }

        flag = *source++;
        flag += (*source++) << 8;

        if (flag == 0) {
            break;
        }
        if ((flag & 0x8000) == 0) {
            dest += flag;
            continue;
        }

        if ((flag & 0x4000) == 0) {
            flag &= 0x3FFF;
            for (; flag > 0; flag--) {
                *dest++ ^= *source++;
            }
            continue;
        }

        flag &= 0x3FFF;
        for (; flag > 0; flag--) {
                *dest++ ^= *source;
        }
        source++;
        continue;
    }
}

#if 0
//hexrays dump of encoding function from TS
int LCW_Comp(char* srcp, char* destp, int datasize)
{
  char* readp; // esi@1
  char* writep; // edi@1
  char* v5; // edi@2
  char runlencalc_startp; // al@3
  char* v7; // ebx@4
  char* v8; // edi@4
  int v9; // ecx@4
  char* v10; // edi@7
  int16_t v11; // cx@7
  bool v12; // zf@10
  int v13; // ecx@10
  char* v14; // edx@15
  char* v15; // ebx@15
  char* v16; // edi@15
  char v17; // zf@15
  int v18; // ecx@15
  int v19; // eax@20
  char* v20; // edi@22
  int16_t v21; // ax@24
  int16_t v22; // ax@25
  char v23; // t1@25
  char v24; // al@31
  char* deststartp; // [sp+14h] [bp-24h]@1
  char* v27; // [sp+18h] [bp-20h]@1
  char* srcmarkp; // [sp+20h] [bp-18h]@1
  char* destmark2p; // [sp+24h] [bp-14h]@1
  char* endp; // [sp+28h] [bp-10h]@1
  signed int v31; // [sp+2Ch] [bp-Ch]@2
  signed int v32; // [sp+30h] [bp-8h]@1
  char* writemakerp; // [sp+34h] [bp-4h]@2

  v27 = 0;
  endp = datasize + srcp;
  v32 = 1;
  deststartp = destp;
  srcmarkp = srcp;
  destmark2p = destp;
  writep = destp;
  //*writep++ = -127;
  readp = srcp;
  //*writep++ = *readp++;
  while ( readp < endp ) {
    writemakerp = writep;
    v5 = srcmarkp;
    v31 = 1;
LABEL_3:
    while ( 1 )
    {
      runlencalc_startp = *readp;
      if ( *readp != *(readp + 64) )
        break;
      v7 = v5;
      v8 = readp;
      v9 = endp - readp;
      do
      {
        if ( !v9 )
          break;
        v12 = (*v8++) == runlencalc_startp;
        --v9;
      }
      while ( v12 );
      v10 = v8 - 1;
      v11 = v10 - readp;
      //if ( (v10 - readp) < 0x41 )
      if ( v11 < 0x41 )
      {
        v5 = v7;
        break;
      }
      v32 = 0;
      readp = v10;
      *writemakerp = -2;
      *(writemakerp + 1) = v11;
      *(writemakerp + 3) = runlencalc_startp;
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
        v12 = *v5++ == runlencalc_startp;
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
  *writep++ = -128;
  return writep - deststartp;
}
#endif

int encodeLCW(const uint8_t* src, std::ostream& dest, int len)
{
    OStream& _stream= reinterpret_cast<OStream&>(dest);
    std::vector<uint8_t> buf(len);
    int compressed = encodeLCW(src, &buf.at(0), len);
    _stream.write(reinterpret_cast<char*>(&buf.at(0)), compressed);
    return compressed;
}

//anon namespace for encodelcw helpers
namespace {

//helper to write bytes in the correct order as lcw is little endian
inline void writeLE16(int16_t val, uint8_t*& writep)
{
    val = htole16(val);
    *writep++ = val & 0xff;
    *writep++ = val >> 8;
}

//find a run of pixels nearer the start that matches the current run
//returns run length and sets passed pos pointer to start of run
int get_same(const uint8_t* src, const uint8_t* readp, const uint8_t* srcendp, const uint8_t*& pos)
{
    //pointer for evaluating previous uint8_ts
    const uint8_t* checkpos = src;
    //pointer to mark where we started our reads from
    const uint8_t* readstartp = readp;
    const uint8_t* checkendp = readstartp;
    
    //length of best evaluated run
    int candidatelen = 0;
    
    const uint8_t* initpos = checkpos;
    
    while(readp < srcendp && checkpos < checkendp){
        //look for a match
        int runlen = 0;
        while(readp < srcendp && checkpos < checkendp) {
            if(*readp++ == *checkpos++) {
                runlen++;
            } else {
                break;
            }
        }
        
        //reset our current pointer for another pass
        readp = readstartp;
        
        //if we have a good length, set candidate details
        if(runlen > candidatelen) {
            candidatelen = runlen;
            pos = initpos;
        }
        
        //increment the position we look at in already processed part
        checkpos = ++initpos;
    }
    
    return candidatelen;
}

//get number of pixels that are the same from this pos
static int get_run_length(const uint8_t* r, const uint8_t* s_end)
{
	int count = 1;
	int v = *r++;
	while (r < s_end && *r++ == v)
		count++;
	return count;
}

inline void write80_c0(uint8_t*& w, int count, int p)
{
    //command 2
    *w++ = (count - 3) << 4 | p >> 8;
    *w++ = p & 0xff;
}

inline void write80_c1(uint8_t*& w, int count, const uint8_t* r)
{
    //command 1
    do
    {
        int c_write = count < 0x40 ? count : 0x3f;
        *w++ = 0x80 | c_write;
        memcpy(w, r, c_write);
        r += c_write;
        w += c_write;
        count -= c_write;
    }
    while (count);
}

inline void write80_c2(uint8_t*& w, int count, int p)
{
    //command 3
    *w++ = 0xc0 | (count - 3);
    writeLE16(p, w);
}

inline void write80_c3(uint8_t*& w, int count, int v)
{
    //command 4
    *w++ = 0xfe;
    writeLE16(count, w);
    *w++ = v;
}

inline void write80_c4(uint8_t*& w, int count, int p)
{
    //command 5
    *w++ = 0xff;
    writeLE16(count, w);
    writeLE16(p, w);
}

inline void flush_c1(uint8_t*& w, const uint8_t* r, const uint8_t*& copy_from)
{
    //writes command 1 when we have a long run of unique pixels
    if (copy_from)
    {
        write80_c1(w, r - copy_from, copy_from);
        copy_from = NULL;
    }
}

inline void write40_c0(uint8_t*& w, int count, int v)
{
    *w++ = 0;
    *w++ = count;
    *w++ = v;
}

inline void write40_c1(uint8_t*& w, int count, const uint8_t* r)
{
    *w++ = count;
    memcpy(w, r, count);
    w += count;
}

inline void write40_c2(uint8_t*& w, int count)
{
    *w++ = 0x80;
    writeLE16(count, w);
}

inline void write40_c3(uint8_t*& w, int count, const uint8_t* r)
{
    *w++ = 0x80;
    writeLE16(0x8000 | count, w);
    memcpy(w, r, count);
    w += count;
}

inline void write40_c4(uint8_t*& w, int count, int v)
{
    *w++ = 0x80;
    writeLE16(0xc000 | count, w);
    *w++ = v;
}

inline void write40_c5(uint8_t*& w, int count)
{
    *w++ = 0x80 | count;
}

void write40_copy(uint8_t*& w, int count, const uint8_t* r)
{
    while (count)
    {
        if (count < 0x80)
        {
            write40_c1(w, count, r);
            count = 0;
        }
        else
        {
            int c_write = count < 0x4000 ? count : 0x3fff;
            write40_c3(w, c_write, r);
            r += c_write;
            count  -= c_write;
        }
    }
}

void write40_fill(uint8_t*& w, int count, int v)
{
    while (count)
    {
        if (count < 0x100)
        {
            write40_c0(w, count, v);
            count = 0;
        }
        else
        {
            int c_write = count < 0x4000 ? count : 0x3fff;
            write40_c4(w, c_write, v);
            count  -= c_write;
        }
    }
}

void write40_skip(uint8_t*& w, int count)
{
    while (count)
    {
        if (count < 0x80)
        {
            write40_c5(w, count);
            count = 0;
        }
        else
        {
            int c_write = count < 0x8000 ? count : 0x7fff;
            write40_c2(w, c_write);
            count  -= c_write;
        }
    }
}

static void flush_copy(uint8_t*& w, const uint8_t* r, const uint8_t*& copy_from)
{
    if (copy_from)
    {
        write40_copy(w, r - copy_from, copy_from);
        copy_from = NULL;
    }
}

}//anon namespace

int encodeLCW(const uint8_t* src, uint8_t* dest, int datasize)
{
    // full compression
    const uint8_t* s_end = src + datasize;
    const uint8_t* readp = src;
    uint8_t* writep = dest;
    const uint8_t* copy_from = NULL;
    //*writep++ = -127;
    //*writep++ = *readp++;    
    while (readp < s_end)
    {
        const uint8_t* pos;
        int blocksize = get_same(src, readp, s_end, pos);
        int runlen = get_run_length(readp, s_end);
        if (runlen < blocksize && blocksize > 2)
        {
            flush_c1(writep, readp, copy_from);
            if (blocksize - 3 < 8 && readp - pos < 0x1000)
                write80_c0(writep, blocksize, readp - pos);
            else if (blocksize - 3 < 0x3e)
                write80_c2(writep, blocksize, pos - src);
            else 
                write80_c4(writep, blocksize, pos - src);				
            readp += blocksize;
        }
        else
        {
            if (runlen < 3)
            {
                if (!copy_from)
                    copy_from = readp;
            }
            else
            {
                flush_c1(writep, readp, copy_from);
                write80_c3(writep, runlen, *readp);
            }
            readp += runlen;
        }
    }
    flush_c1(writep, readp, copy_from);
    write80_c1(writep, 0, NULL);
    return writep - dest;
}

int createXorDelta(const uint8_t* reference, 
                   const uint8_t* result, uint8_t* dest, int datasize)
{
	// full compression
	uint8_t* s = new uint8_t[datasize];
	{
		uint8_t* a = s;
		int size = datasize;
		while (size--)
			*a++ = *reference++ ^ *result++;
	}
	const uint8_t* s_end = s + datasize;
	const uint8_t* r = s;
	uint8_t* w = dest;
	const uint8_t* copy_from = NULL;
	while (r < s_end)
	{
		int v = *r;
		int t = get_run_length(r, s_end);
		if (!v)
		{
			flush_copy(w, r, copy_from);			
			write40_skip(w, t);
		}
		else if (t > 2)
		{
			flush_copy(w, r, copy_from);			
			write40_fill(w, t, v);
		}
		else
		{
			if (!copy_from)
				copy_from = r;
		}
		r += t;
	}
	flush_copy(w, r, copy_from);
	write40_c2(w, 0);
	delete[] s;
	return w - dest;
}
    
} } //eastwood codec