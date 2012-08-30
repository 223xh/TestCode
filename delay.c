void delay_us(unsigned count)
{
    int i ;
    for(; count > 0 ; count--) 
      for(i=360; i > 0; i--) 
        {_ASM("nop");}
}