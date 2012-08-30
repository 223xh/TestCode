#define creg(a) (*(volatile unsigned*)(0xc1100000 + ((a)<<2)))
#define reg(a) (*(volatile unsigned*)(a))

static void hardi2c_init(void)
{
    hard_i2c_init(400);
    reg(0xc8100014) |= 3<<5;
    udelay(10000);
}

