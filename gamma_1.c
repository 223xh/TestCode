/*
#define GAMMA_N		  16
#ifndef GAMMA_MASK
#define GAMMA_MASK  0x3FF //1023
#endif
*/

/*
int gamma_table[GAMMA_N + 1] = {0,64,128,192,256,320,384,448,512,576,640,704,768,832,896,960,1023};

void program_gamma(int color, int *gamma_t)
{
    while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
    *(volatile unsigned *)0x01101208 = color;
    
    int i, j;
    unsigned n = 256/GAMMA_N;
    float delta,  value;
    
    for (i=0;i<GAMMA_N;i++)
    {
        delta = (float)(gamma_t[i+1]-gamma_t[i])/n;
        value = gamma_t[i];

        if (value>GAMMA_MASK) value = GAMMA_MASK;

        for (j=0;j<n;j++)
        {
            while (!(*(volatile unsigned *)0x01101200 & (0x1 << 4)));
        		*(volatile unsigned *)0x01101204 = (int)(value);
            
            value += delta;
            if (value>GAMMA_MASK) value = GAMMA_MASK;
        }
    }

    while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
    *(volatile unsigned *)0x01101208 = (color | 0x23);
}

program_gamma(MASK_R,gamma_table);
program_gamma(MASK_G,gamma_table);
program_gamma(MASK_B,gamma_table);
*/
#define MASK_R 0xc00
#define MASK_G 0xa00
#define MASK_B 0x900

/************************18gamma****************************/
unsigned gamma18_flag = 0;
void program_18gamma(int color, unsigned short *gamma_t)
{
	int i;
		
    while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
    *(volatile unsigned *)0x01101208 = color;
                
    for (i=0;i<256;i++)
    {
       while (!(*(volatile unsigned *)0x01101200 & (0x1 << 4)));
       *(volatile unsigned *)0x01101204 = gamma_t[i];
        
    }
    
    while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
    *(volatile unsigned *)0x01101208 = (color | 0x23);
}

unsigned short gamma_flag = 0;
int gamma_r[GAMMA_N + 1] = {0,60,120,180,240,300,360,420,480,540,600,660,720,780,840,900,960,1023};
int gamma_g[GAMMA_N + 1] = {0,60,120,180,240,300,360,420,480,540,600,660,720,780,840,900,960,1023};
int gamma_b[GAMMA_N + 1] = {0,60,120,180,240,300,360,420,480,540,600,660,720,780,840,900,960,1023};

void program_gamma(int color, int *gamma_t)
{   int i, j;
    unsigned n = 256/GAMMA_N;
    float delta,  value;
    
    for(i=0; i<GAMMA_N+1; i++)
	{
		if(gamma_t[i] > GAMMA_MASK)
			gamma_t[i] = GAMMA_MASK;			
	}
    
    while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
    *(volatile unsigned *)0x01101208 = color;
    
    for (i=0;i<GAMMA_N;i++)
    {
        delta = (float)(gamma_t[i+1]-gamma_t[i])/n;
        value = gamma_t[i];
        
        for (j=0;j<n;j++)
        {
            while (!(*(volatile unsigned *)0x01101200 & (0x1 << 4)));
        		*(volatile unsigned *)0x01101204 = (int)(value + delta * j);
        }
    }
     while (!(*(volatile unsigned *)0x01101200 & (0x1 << 4)));
        *(volatile unsigned *)0x01101204 = gamma_t[GAMMA_N];

    while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
    *(volatile unsigned *)0x01101208 = (color | 0x23);
}

if(gamma_flag)
{
    gamma_flag = 0;
    program_gamma(MASK_R,gamma_r);
    program_gamma(MASK_G,gamma_g);
    program_gamma(MASK_B,gamma_b);
}
/*
int gamma_table[GAMMA_N + 1] = {0,64,128,192,256,320,384,448,512,576,640,704,768,832,896,960,1023};

void program_gamma(int color, int *gamma_t)
{
    while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
    *(volatile unsigned *)0x01101208 = color;
    
    int i, j;
    unsigned n = 256/GAMMA_N;
    float delta,  value;
    
    for (i=0;i<GAMMA_N;i++)
    {
        delta = (float)(gamma_t[i+1]-gamma_t[i])/n;
        value = gamma_t[i];

        if (value>GAMMA_MASK) value = GAMMA_MASK;

        for (j=0;j<n;j++)
        {
            while (!(*(volatile unsigned *)0x01101200 & (0x1 << 4)));
        		*(volatile unsigned *)0x01101204 = (int)(value);
            
            value += delta;
            if (value>GAMMA_MASK) value = GAMMA_MASK;
        }
    }

    while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
    *(volatile unsigned *)0x01101208 = (color | 0x23);
}

program_gamma(MASK_R,gamma_table);
program_gamma(MASK_G,gamma_table);
program_gamma(MASK_B,gamma_table);
*/
int gamma_temp = 0;
unsigned gamma_flag = 0;
unsigned gamma_model = 0; //0:RGB, 1:R Only, 2:G Only, 3:B Only
void program_gamma(int color, int gamma_value)
{
    while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
    *(volatile unsigned *)0x01101208 = color;
    
    for (int i=0;i<256;i++) {
        while (!(*(volatile unsigned *)0x01101200 & (0x1 << 4)));
        *(volatile unsigned *)0x01101204 = gamma_value;
    }

    while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
    *(volatile unsigned *)0x01101208 = (color | 0x23);
}

if(gamma_flag)
	{
		gamma_flag = 0;
		
switch(gamma_model)
{
	case 0: 
		program_gamma(MASK_R,gamma_temp);
		program_gamma(MASK_G,gamma_temp);
		program_gamma(MASK_B,gamma_temp);
		break;
	case 1:
		program_gamma(MASK_R,gamma_temp);
		program_gamma(MASK_G,0);
		program_gamma(MASK_B,0);
		break;
	case 2:
		program_gamma(MASK_R,0);
		program_gamma(MASK_G,gamma_temp);
		program_gamma(MASK_B,0);
		break;
	case 3:
		program_gamma(MASK_R,0);
		program_gamma(MASK_G,0);
		program_gamma(MASK_B,gamma_temp);
		break;
	default:
		program_gamma(MASK_R,gamma_temp);
		program_gamma(MASK_G,gamma_temp);
		program_gamma(MASK_B,gamma_temp);
}

	}
	
if(gamma18_flag)
	{
		gamma18_flag = 0;
		
switch(gamma_model)
{
	case 0: 
		program_gamma(MASK_R,GammaTableR);
		program_gamma(MASK_G,GammaTableG);
		program_gamma(MASK_B,GammaTableB);
		break;
	case 1:
		program_gamma(MASK_R,GammaTableR);
		program_gamma(MASK_G,0);
		program_gamma(MASK_B,0);
		break;
	case 2:
		program_gamma(MASK_R,0);
		program_gamma(MASK_G,GammaTableG);
		program_gamma(MASK_B,0);
		break;
	case 3:
		program_gamma(MASK_R,0);
		program_gamma(MASK_G,0);
		program_gamma(MASK_B,GammaTableB);
		break;
	default:
		program_gamma(MASK_R,GammaTableR);
		program_gamma(MASK_G,GammaTableG);
		program_gamma(MASK_B,GammaTableB);
}

	}
	
if(gamma18_flag)
	{
		gamma18_flag = 0;

		program_18gamma(MASK_R,GammaTableR);
		program_18gamma(MASK_G,GammaTableG);
		program_18gamma(MASK_B,GammaTableB);		
	}

#define GAMMA_BAND  16
#define GAMMA_MASK  0x3FF

void generate_gamma_table_dlCD(void)
{
    int i, j;
    int gamma_adjust[GAMMA_BAND+1] = {0,64,128,192,256,320,384,448,512,576,640,704,768,832,896,960,1023};

    unsigned n = 256/GAMMA_BAND;
    float delta,  value;
    
    for (i=0;i<GAMMA_BAND;i++){
        delta = (float)(gamma_adjust[i+1]-gamma_adjust[i])/n;
        value = gamma_adjust[i];

        if (value>GAMMA_MASK) value = GAMMA_MASK;

        for (j=0;j<n;j++){
            GammaTableR[i*n+j] = (int)(value);
            GammaTableG[i*n+j] = (int)(value);
            GammaTableB[i*n+j] = (int)(value);
            value += delta;
            if (value>GAMMA_MASK) value = GAMMA_MASK;
        }
    } 
}

generate_gamma_table_dlCD(void)
{
    int i, j;
    int gamma_r_adjust[GAMMA_BAND+1] = {0,64,128,192,256,320,384,448,512,576,640,704,768,832,896,960,1023};
    int gamma_g_adjust[GAMMA_BAND+1] = {0,64,128,192,256,320,384,448,512,576,640,704,768,832,896,960,1023};
    int gamma_b_adjust[GAMMA_BAND+1] = {0,64,128,192,256,320,384,448,512,576,640,704,768,832,896,960,1023};

    unsigned n = 256/GAMMA_BAND;
    float delta_r, delta_g, delta_b, value_r, value_g, value_b;
    
    for (i=0;i<GAMMA_BAND;i++){
        delta_r = (float)(gamma_r_adjust[i+1]-gamma_r_adjust[i])/n;
        delta_g = (float)(gamma_g_adjust[i+1]-gamma_g_adjust[i])/n;
        delta_b = (float)(gamma_b_adjust[i+1]-gamma_b_adjust[i])/n;
        value_r = gamma_r_adjust[i];
        value_g = gamma_g_adjust[i];
        value_b = gamma_b_adjust[i];
        if (value_r>GAMMA_MASK) value_r = GAMMA_MASK;
        if (value_g>GAMMA_MASK) value_g = GAMMA_MASK;
        if (value_b>GAMMA_MASK) value_b = GAMMA_MASK;
        for (j=0;j<n;j++){
            GammaTableR[i*n+j] = (int)(value_r);
            GammaTableG[i*n+j] = (int)(value_g);
            GammaTableB[i*n+j] = (int)(value_b);
            value_r += delta_r;
            value_g += delta_g;
            value_b += delta_b;
            if (value_r>GAMMA_MASK) value_r = GAMMA_MASK;
            if (value_g>GAMMA_MASK) value_g = GAMMA_MASK;
            if (value_b>GAMMA_MASK) value_b = GAMMA_MASK;
        }
    }        
}

/************************18gamma****************************/
#define GAMMA_BAND  17
#define GAMMA_MASK  0x3FF

void generate_gamma_table_dlCD(void)
{
    int i, j;
    int gamma_adjust[GAMMA_BAND+1] = {0,60,120,180,240,300,360,420,480,540,600,660,720,780,840,900,960,1023};

    unsigned n = 255/GAMMA_BAND;
    float delta,  value;
    
    for(i=0; i<GAMMA_BAND+1; i++)
		{
			if(gamma_adjust[i] > GAMMA_MASK)
				gamma_adjust[i] = GAMMA_MASK;			
		}
    
    for (i=0;i<GAMMA_BAND;i++){
        delta = (float)(gamma_adjust[i+1]-gamma_adjust[i])/n;
        value = gamma_adjust[i];
        
        for (j=0;j<n;j++){
            GammaTableR[i*n+j] = (int)(value + delta * j);
            GammaTableG[i*n+j] = (int)(value + delta * j);
            GammaTableB[i*n+j] = (int)(value + delta * j);            
        }
    } 
    
    GammaTableR[255] = gamma_adjust[GAMMA_BAND];
    GammaTableG[255] = gamma_adjust[GAMMA_BAND];
    GammaTableB[255] = gamma_adjust[GAMMA_BAND];
}

generate_gamma_table_dlCD(void)
{
    int i, j;
    int gamma_r_adjust[GAMMA_BAND+1] = {0,60,120,180,240,300,360,420,480,540,600,660,720,780,840,900,960,1023};
    int gamma_g_adjust[GAMMA_BAND+1] = {0,60,120,180,240,300,360,420,480,540,600,660,720,780,840,900,960,1023};
    int gamma_b_adjust[GAMMA_BAND+1] = {0,60,120,180,240,300,360,420,480,540,600,660,720,780,840,900,960,1023};

    unsigned n = 255/GAMMA_BAND;
    float delta_r, delta_g, delta_b, value_r, value_g, value_b;
    
    for(i=0; i<GAMMA_BAND+1; i++)
		{
			if(gamma_r_adjust[i] > GAMMA_MASK)
				gamma_r_adjust[i] = GAMMA_MASK;
			if(gamma_g_adjust[i] > GAMMA_MASK)
				gamma_g_adjust[i] = GAMMA_MASK;
			if(gamma_b_adjust[i] > GAMMA_MASK)
				gamma_b_adjust[i] = GAMMA_MASK;
		}
    
    for (i=0;i<GAMMA_BAND;i++){
        delta_r = (float)(gamma_r_adjust[i+1]-gamma_r_adjust[i])/n;
        delta_g = (float)(gamma_g_adjust[i+1]-gamma_g_adjust[i])/n;
        delta_b = (float)(gamma_b_adjust[i+1]-gamma_b_adjust[i])/n;
        value_r = gamma_r_adjust[i];
        value_g = gamma_g_adjust[i];
        value_b = gamma_b_adjust[i];
        
        for (j=0;j<n;j++){
            GammaTableR[i*n+j] = (int)(value_r + delta_r * j);
            GammaTableG[i*n+j] = (int)(value_g + delta_g * j);
            GammaTableB[i*n+j] = (int)(value_b + delta_b * j);
        }
    } 
    
    GammaTableR[255] = gamma_r_adjust[GAMMA_BAND];
    GammaTableG[255] = gamma_g_adjust[GAMMA_BAND];
    GammaTableB[255] = gamma_b_adjust[GAMMA_BAND];
}