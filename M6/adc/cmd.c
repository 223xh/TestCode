#define creg(a) (*(volatile unsigned*)(0xc1100000 + ((a)<<2)))
static int do_ssadc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    char *endp;
    unsigned long tmp, n;
    int i;
    unsigned char tflag;

    if(!argc)
        goto usage;

    if(argc > 1){
        tmp = simple_strtoul(argv[2], &endp, 10);
	    if (*argv[2] == 0 || *endp != 0){
            tmp = 0;
        }
    }

    if(argc > 2){
        n = simple_strtoul(argv[3], &endp, 10);
	    if (*argv[3] == 0 || *endp != 0){
            n = 10;
        }
    }

    if (strcmp(argv[1], "on") == 0){

        saradc_enable();
        get_adc_sample(6);

        creg(0x200d) &= ~(1<<22);
        creg(0x200c) &= ~(1<<22);

        printf("saradc is power on\n");
    }
    else if (strcmp(argv[1], "swt") == 0){
        saradc_temp_sw(tmp);
        printf("saradc temp mode is switch to %u\n", tmp);
    }
    else if (strcmp(argv[1], "sw02") == 0){
        saradc_ctrl02_sw(tmp);
        printf("saradc ctrl0:2 is switch to %u\n", tmp);
    }
    else if (strcmp(argv[1], "off") == 0){
        saradc_disable();
        printf("saradc is power off\n");
    }
    else if (strcmp(argv[1], "get") == 0){
        printf("saradc[%u] value is %d\n", tmp, get_adc_sample(tmp));
    }
    else if (strcmp(argv[1], "getn") == 0){
        creg(0x200d) &= ~(1<<22);
        __udelay(100000);
        printf("saradc[%u] value(L) is:\n", tmp);
        for(i=0;i<n;i++){
            printf("%d,", get_adc_sample(tmp));
            __udelay(100000);
        }
        printf("\b \b\n");

        creg(0x200d) |= (1<<22);
        __udelay(100000);
        printf("saradc[%u] value(H) is:\n", tmp);
        for(i=0;i<n;i++){
            printf("%d,", get_adc_sample(tmp));
            __udelay(100000);
        }
        printf("\b \b\n");
    }
    else if (strcmp(argv[1], "geta") == 0){
        tflag = 1;
TEMP_START:
        printf("saradc[6] value is:\n");
        for(i=0;i<20;i++){
            printf("%d,", get_adc_sample(6));
            __udelay(100000);
        }
        printf("\b \b\n");

        if(tflag){
            tflag = 0;
            saradc_temp_sw(0);
            printf("***saradc temp mode is switch to 0.***\n");
            goto TEMP_START;
        }
    }
    else
        goto usage;

	return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	ssadc,	4,	1,	do_ssadc,
	"saradc test function",
	"....\n"
);