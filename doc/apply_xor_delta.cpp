int __cdecl Apply_XOR_Delta(int a1, int a2)
{
  int v2; // edi@1
  int v3; // esi@1
  int v4; // ecx@1
  int v5; // eax@2
  char v6; // al@8
  int result; // eax@11
  int v8; // eax@15

  v2 = a1;
  v3 = a2;
  v4 = 0;
  while ( 1 )
  {
    while ( 1 )
    {
      while ( 1 )
      {
        v5 = *v3++;
        if ( !v5 )
        {
          LOBYTE(v4) = *v3++;
          goto LABEL_8;
        }
        if ( v5 < 0 )
          break;
        v4 = v5;
        do
        {
LABEL_5:
          *v2++ ^= *v3++;
          --v4;
        }
        while ( v4 );
      }
      result = v5 - 128;
      if ( !result )
      {
        LOWORD(result) = *v3;
        v3 += 2;
        if ( result <= 0 )
          break;
      }
      v2 += result;
    }
    if ( !result )
      return result;
    v8 = result - 32768;
    if ( !(v8 & 0x4000) )
    {
      v4 = v8;
      goto LABEL_5;
    }
    v4 = v8 - 16384;
LABEL_8:
    v6 = *v3++;
    do
    {
      *v2++ ^= v6;
      --v4;
    }
    while ( v4 );
  }
}