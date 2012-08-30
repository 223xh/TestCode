unsigned temp;

temp = *(volatile unsigned *)0xc1104134;
temp &= ~0xF3C0;
temp |= 0x8040;
*(volatile unsigned *)0xc1104134 = temp;

temp = *(volatile unsigned *)0xc110413c;
temp &= ~0xF3C0;
temp |= 0x8040;
*(volatile unsigned *)0xc110413c = temp;

*(volatile unsigned *)0xc1104158 &= ~(1<<8);
*(volatile unsigned *)0xc1104160 &= ~(1<<8);