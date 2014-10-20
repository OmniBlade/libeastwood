int __fastcall LCW_Comp(int a1, int a2, int a3)
{
  int v3; // esi@1
  int v4; // edi@1
  int v5; // edi@2
  char v6; // al@3
  int v7; // ebx@4
  int v8; // edi@4
  int v9; // ecx@4
  int v10; // edi@7
  __int16 v11; // cx@7
  char v12; // zf@10
  int v13; // ecx@10
  int v14; // edx@15
  int v15; // ebx@15
  int v16; // edi@15
  char v17; // zf@15
  int v18; // ecx@15
  int v19; // eax@20
  int v20; // edi@22
  __int16 v21; // ax@24
  __int16 v22; // ax@25
  char v23; // t1@25
  char v24; // al@31
  int v26; // [sp+14h] [bp-24h]@1
  int v27; // [sp+18h] [bp-20h]@1
  int v28; // [sp+20h] [bp-18h]@1
  int v29; // [sp+24h] [bp-14h]@1
  int v30; // [sp+28h] [bp-10h]@1
  signed int v31; // [sp+2Ch] [bp-Ch]@2
  signed int v32; // [sp+30h] [bp-8h]@1
  int v33; // [sp+34h] [bp-4h]@2

  v27 = 0;
  v30 = a3 + a1;
  v32 = 1;
  v26 = a2;
  v28 = a1;
  v29 = a2;
  *a2 = -127;
  v3 = a1 + 1;
  *(a2 + 1) = *a1;
  v4 = a2 + 2;
  do
  {
    v33 = v4;
    v5 = v28;
    v31 = 1;
LABEL_3:
    while ( 1 )
    {
      v6 = *v3;
      if ( *v3 != *(v3 + 64) )
        break;
      v7 = v5;
      v8 = v3;
      v9 = v30 - v3;
      do
      {
        if ( !v9 )
          break;
        v12 = *v8++ == v6;
        --v9;
      }
      while ( v12 );
      v10 = v8 - 1;
      v11 = v10 - v3;
      if ( (v10 - v3) < 0x41 )
      {
        v5 = v7;
        break;
      }
      v32 = 0;
      v3 = v10;
      *v33 = -2;
      *(v33 + 1) = v11;
      *(v33 + 3) = v6;
      v33 += 4;
      v5 = v7;
    }
    while ( 1 )
    {
      v13 = v3 - v5;
      v12 = v3 == v5;
      if ( v3 == v5 )
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
      if ( *(v3 + v31 - 1) == *(v5 + v31 - 2) )
      {
        v14 = v3;
        v15 = v5;
        v16 = v5 - 1;
        v18 = v30 - v3;
        v17 = v30 == v3;
        do
        {
          if ( !v18 )
            break;
          v17 = *v3++ == *v16++;
          --v18;
        }
        while ( v17 );
        if ( v17 )
          ++v16;
        v3 = v14;
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
    v20 = v33;
    if ( v31 <= 2 )
    {
      if ( v32 )
        goto LABEL_30;
      while ( 1 )
      {
        v29 = v20;
        *v20++ = -128;
LABEL_30:
        if ( *v29 != -65 )
        {
          ++*v29;
          v24 = *v3++;
          *v20 = v24;
          v4 = v20 + 1;
          v32 = 1;
          goto LABEL_35;
        }
      }
    }
    if ( v31 > 0xA || (v21 = v3 - v27, (v3 - v27) > 0xFFF) )
    {
      if ( v31 > 0x40 )
      {
        *v33 = -1;
        *(v33 + 1) = v31;
        v20 = v33 + 3;
      }
      else
      {
        *v33 = (v31 - 3) | 0xC0;
        v20 = v33 + 1;
      }
      v22 = v27 - v28;
    }
    else
    {
      v23 = 16 * (v31 - 3) + HIBYTE(v21);
      HIBYTE(v22) = v21;
      LOBYTE(v22) = v23;
    }
    *v20 = v22;
    v4 = v20 + 2;
    v3 += v31;
    v32 = 0;
LABEL_35:
    ;
  }
  while ( v3 < v30 );
  *v4 = -128;
  return v4 + 1 - v26;
}