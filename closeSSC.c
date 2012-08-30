unsigned temp;

temp = *(volatile unsigned *)0x1100580;
temp &= ~0xF3C0;
temp |= 0x8040;
*(volatile unsigned *)0x1100580 = temp;

temp = *(volatile unsigned *)0x1100590;
temp &= ~0xF3C0;
temp |= 0x8040;
*(volatile unsigned *)0x1100590 = temp;

*(volatile unsigned *)0x1100584 &= ~(1<<8);
*(volatile unsigned *)0x1100594 &= ~(1<<8);