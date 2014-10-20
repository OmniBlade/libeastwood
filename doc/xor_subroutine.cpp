__int16 __usercall sub_5D6B0B<ax>(unsigned int a1<edx>, int a2<ecx>, unsigned int a3<ebx>, int a4<edi>, int a5<esi>, int a6)
{
  int v6; // eax@1
  int v7; // edi@5
  char v8; // al@9
  int v9; // eax@14
  int v10; // edi@16
  int v11; // eax@21

  while ( 1 )
  {
    while ( 1 )
    {
      while ( 1 )
      {
        v6 = *a5++;
        if ( !v6 )
        {
          LOBYTE(a2) = *a5++;
          goto LABEL_9;
        }
        if ( v6 < 0 )
          break;
        a2 = v6;
        do
        {
LABEL_4:
          *a4 = *a5;
          ++a1;
          ++a5;
          ++a4;
          if ( a1 == a3 )
          {
            v7 = a4 - a1;
            a1 = 0;
            a4 = a6 + v7;
          }
          --a2;
        }
        while ( a2 );
      }
      v9 = v6 - 128;
      if ( !v9 )
      {
        LOWORD(v9) = *a5;
        a5 += 2;
        if ( v9 <= 0 )
          break;
      }
      v10 = a4 - a1;
      a1 += v9;
      while ( a1 >= a3 )
      {
        a1 -= a3;
        v10 += a6;
      }
      a4 = a1 + v10;
    }
    if ( !v9 )
      return v9;
    v11 = v9 - 32768;
    if ( !(v11 & 0x4000) )
    {
      a2 = v11;
      goto LABEL_4;
    }
    a2 = v11 - 16384;
LABEL_9:
    v8 = *a5++;
    do
    {
      *a4 = v8;
      ++a1;
      ++a4;
      if ( a1 == a3 )
      {
        a1 = 0;
        a4 = a6 + a4 - a3;
      }
      --a2;
    }
    while ( a2 );
  }
}