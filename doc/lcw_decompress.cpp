int __cdecl LCW_Uncompress(int a1, char *a2, unsigned __int32 a3)
{
  char *v3; // edi@1
  int v4; // ebx@1
  int v5; // eax@3
  int v6; // esi@3
  int v7; // ecx@4
  unsigned int v8; // ecx@4
  int v9; // ecx@9
  int v10; // ecx@13
  unsigned int v11; // ecx@14
  int v12; // eax@14
  int v13; // eax@20
  char v14; // dl@21
  char v15; // dl@22
  unsigned int v16; // ecx@22
  int v17; // edx@22
  int i; // ecx@23
  char *v19; // eax@29
  const void *v20; // esi@29
  char v21; // dl@35
  int v22; // edx@35
  char v23; // dl@36
  unsigned int v24; // ecx@36
  char *v26; // [sp+Ch] [bp-Ch]@1
  int v27; // [sp+10h] [bp-8h]@3

  v3 = a2;
  v26 = &a2[a3];
  v4 = a1;
  while ( v26 != v3 )
  {
    v27 = v26 - v3;
    v5 = *v4;
    v6 = v4 + 1;
    if ( v5 < 0 )
    {
      if ( v5 & 0x40 )
      {
        v10 = (v5 & 0x3F) + 3;
        if ( v5 == -2 )
        {
          v11 = *v6;
          v12 = *(v4 + 3);
          v4 += 4;
          if ( v11 > v27 )
            v11 = v26 - v3;
          if ( v11 & 0xFFE0 )
          {
            BYTE1(v12) = v12;
            v13 = v12 | (v12 << 16);
            if ( v3 & 3 )
            {
              *v3 = v13;
              v14 = v3;
              v3 = ((v3 & 0xFFFFFFFC) + 4);
              v11 -= (((v14 & 3) - 1) ^ 3);
            }
            v15 = v11;
            v16 = v11 >> 2;
            memset32(v3, v13, v16);
            v3 += 4 * v16;
            v17 = v15 & 3;
            if ( v17 )
            {
              for ( i = v17; i; --i )
                *v3++ = v13;
            }
          }
          else
          {
            while ( v11 )
            {
              *v3++ = v12;
              --v11;
            }
          }
        }
        else
        {
          if ( v5 == -1 )
          {
            HIWORD(v5) = 0;
            v10 = *v6;
            v6 = v4 + 3;
          }
          LOWORD(v5) = *v6;
          v19 = &a2[v5];
          v4 = v6 + 2;
          v20 = v19;
          if ( v10 > v27 )
            v10 = v26 - v3;
          if ( v10 & 0xFFE0 )
          {
            if ( v19 <= v3 - 4 )
            {
              if ( v3 & 3 )
              {
                *v3 = *v19;
                v21 = v3;
                v3 = ((v3 & 0xFFFFFFFC) + 4);
                v22 = (((v21 & 3) - 1) ^ 3);
                v10 -= v22;
                v20 = &v19[v22];
              }
              v23 = v10;
              v24 = v10 >> 2;
              memcpy(v3, v20, 4 * v24);
              v20 = v20 + 4 * v24;
              v3 += 4 * v24;
              v10 = v23 & 3;
            }
            memcpy(v3, v20, v10);
            v3 += v10;
          }
          else
          {
            memcpy(v3, v19, v10);
            v3 += v10;
          }
        }
      }
      else
      {
        if ( v5 == -128 )
          return v3 - a2;
        v9 = v5 & 0x3F;
        if ( v9 > v27 )
          v9 = v26 - v3;
        memcpy(v3, v6, v9);
        v3 += v9;
        v4 = v6 + v9;
      }
    }
    else
    {
      v7 = *v4;
      BYTE1(v5) = v5 & 0xF;
      LOBYTE(v7) = v7 >> 4;
      v8 = v7 + 3;
      if ( v8 > v27 )
        v8 = v26 - v3;
      LOBYTE(v5) = *v6;
      v4 += 2;
      memcpy(v3, &v3[-v5], v8);
      v3 += v8;
    }
  }
  return v3 - a2;
}