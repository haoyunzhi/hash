/*
The  sponge function, designed by Guido Bertoni, Joan Daemen,
Michaël Peeters and Gilles Van Assche. For more information, feedback or
questions, please refer to our website: http://keccak.noekeon.org/
*/


#include <stdio.h>
#include "time.h"

/************** Timing routine (for performance measurements) ***********/
/* By Doug Whiting */
/* unfortunately, this is generally assembly code and not very portable */
#if defined(_M_IX86) || defined(__i386) || defined(_i386) || defined(__i386__) || defined(i386) || \
    defined(_X86_)   || defined(__x86_64__) || defined(_M_X64) || defined(__x86_64)
#define _Is_X86_    1
#endif

#if  defined(_Is_X86_) && (!defined(__STRICT_ANSI__)) && (defined(__GNUC__) || !defined(__STDC__)) && \
    (defined(__BORLANDC__) || defined(_MSC_VER) || defined(__MINGW_H) || defined(__GNUC__))
#define HI_RES_CLK_OK         1          /* it's ok to use RDTSC opcode */

#if defined(_MSC_VER) // && defined(_M_X64)
#include <intrin.h>
#pragma intrinsic(__rdtsc)         /* use MSVC rdtsc call where defined */
#endif

#endif

typedef unsigned int uint_32t;

uint_32t HiResTime(void)           /* return the current value of time stamp counter */
    {
#if defined(HI_RES_CLK_OK)
    uint_32t x[2];
#if   defined(__BORLANDC__)
#define COMPILER_ID "BCC"
    __emit__(0x0F,0x31);           /* RDTSC instruction */
    _asm { mov x[0],eax };
#elif defined(_MSC_VER)
#define COMPILER_ID "MSC"
#if defined(_MSC_VER) // && defined(_M_X64)
    x[0] = (uint_32t) __rdtsc();
#else
    _asm { _emit 0fh }; _asm { _emit 031h };
    _asm { mov x[0],eax };
#endif
#elif defined(__MINGW_H) || defined(__GNUC__)
#define COMPILER_ID "GCC"
    asm volatile("rdtsc" : "=a"(x[0]), "=d"(x[1]));
#else
#error  "HI_RES_CLK_OK -- but no assembler code for this platform (?)"
#endif
    return x[0];
#else
    /* avoid annoying MSVC 9.0 compiler warning #4720 in ANSI mode! */
#if (!defined(_MSC_VER)) || (!defined(__STDC__)) || (_MSC_VER < 1300)
    FatalError("No support for RDTSC on this CPU platform\n");
#endif
    return 0;
#endif /* defined(HI_RES_CLK_OK) */
    }

#define TIMER_SAMPLE_CNT (100)

uint_32t calibrate()
{
    uint_32t dtMin = 0xFFFFFFFF;        /* big number to start */
    uint_32t t0,t1,i;

    for (i=0;i < TIMER_SAMPLE_CNT;i++)  /* calibrate the overhead for measuring time */
        {
        t0 = HiResTime();
        t1 = HiResTime();
        if (dtMin > t1-t0)              /* keep only the minimum time */
            dtMin = t1-t0;
        }
    return dtMin;
}

#include "interface.h"
#include "hashstate.h"

uint_32t measureAbsorb1024bits(uint_32t dtMin)
{
    uint_32t tMin = 0xFFFFFFFF;         /* big number to start */
    uint_32t t0,t1,i;
    ALIGN unsigned char state[PermutationSizeInBytes];
    ALIGN unsigned char input[128];

    for (i=0;i < TIMER_SAMPLE_CNT;i++)  /* calibrate the overhead for measuring time */
        {
        t0 = HiResTime();
#ifdef ProvideFast1024
        Absorb1024bits(state, input);
#else
        Absorb(state, input, 1024/64);
#endif
        t1 = HiResTime();
        if (tMin > t1-t0 - dtMin)       /* keep only the minimum time */
            tMin = t1-t0 - dtMin;
        }

    /* now tMin = # clocks required for running RoutineToBeTimed() */

    return tMin;
}

uint_32t measureAbsorb1088bits(uint_32t dtMin)
{
    uint_32t tMin = 0xFFFFFFFF;         /* big number to start */
    uint_32t t0,t1,i;
    ALIGN unsigned char state[PermutationSizeInBytes];
    ALIGN unsigned char input[136];

    for (i=0;i < TIMER_SAMPLE_CNT;i++)  /* calibrate the overhead for measuring time */
        {
        t0 = HiResTime();
#ifdef ProvideFast1088
        Absorb1088bits(state, input);
#else
        Absorb(state, input, 1088/64);
#endif
        t1 = HiResTime();
        if (tMin > t1-t0 - dtMin)       /* keep only the minimum time */
            tMin = t1-t0 - dtMin;
        }

    /* now tMin = # clocks required for running RoutineToBeTimed() */
    
    return tMin;
}

uint_32t measureAbsorb1344bits(uint_32t dtMin)
{
    uint_32t tMin = 0xFFFFFFFF;         /* big number to start */
    uint_32t t0,t1,i;
    ALIGN unsigned char state[PermutationSizeInBytes];
    ALIGN unsigned char input[200];

    for (i=0;i < TIMER_SAMPLE_CNT;i++)  /* calibrate the overhead for measuring time */
        {
        t0 = HiResTime();
#ifdef ProvideFast1344
        Absorb1344bits(state, input);
#else
        Absorb(state, input, 1344/64);
#endif
        t1 = HiResTime();
        if (tMin > t1-t0 - dtMin)       /* keep only the minimum time */
            tMin = t1-t0 - dtMin;
        }

    /* now tMin = # clocks required for running RoutineToBeTimed() */
    
    return tMin;
}

uint_32t measureAbsorb(uint_32t dtMin, unsigned int laneCount)
{
    uint_32t tMin = 0xFFFFFFFF;         /* big number to start */
    uint_32t t0,t1,i;
    ALIGN unsigned char state[PermutationSizeInBytes];
    ALIGN unsigned char input[64];

    for (i=0;i < TIMER_SAMPLE_CNT;i++)  /* calibrate the overhead for measuring time */
        {
        t0 = HiResTime();
        Absorb(state, input, laneCount);
        t1 = HiResTime();
        if (tMin > t1-t0 - dtMin)       /* keep only the minimum time */
            tMin = t1-t0 - dtMin;
        }

    /* now tMin = # clocks required for running RoutineToBeTimed() */
    
    return tMin;
}

uint_32t measurePermutation(uint_32t dtMin)
{
    uint_32t tMin = 0xFFFFFFFF;         /* big number to start */
    uint_32t t0,t1,i;
    ALIGN unsigned char state[PermutationSizeInBytes];

    for (i=0;i < TIMER_SAMPLE_CNT;i++)  /* calibrate the overhead for measuring time */
        {
        t0 = HiResTime();
        Permutation(state);
        t1 = HiResTime();
        if (tMin > t1-t0 - dtMin)       /* keep only the minimum time */
            tMin = t1-t0 - dtMin;
        }

    /* now tMin = # clocks required for running RoutineToBeTimed() */
    
    return tMin;
}

uint_32t measureHash1block(uint_32t dtMin)
{
    uint_32t tMin = 0xFFFFFFFF;         /* big number to start */
    uint_32t i;
	clock_t t0, t1;
    hashState state;
    ALIGN unsigned char data[30*128];

	for(i=0;i<1000*128;i++)
		data[i]='a';
	t0 =clock();
	Init(&state, 0);
	InitSponge(&state, 576,   1024);
    for (i=0;i <10000000;i++)  /* calibrate the overhead for measuring time */
        {
        
        
       Update(&state, data, 29*1024+29);
        Final(&state, 0);
        
     
        }
	t1 = clock();

	printf("\nhhhhhhhhh:%d\n",(t1-t0));
    /* now tMin = # clocks required for running RoutineToBeTimed() */
    
    return tMin;
}

uint_32t measureHash2blocks(uint_32t dtMin)
{
    uint_32t tMin = 0xFFFFFFFF;         /* big number to start */
    uint_32t t0,t1,i;
    hashState state;
    ALIGN unsigned char data[256];

    for (i=0;i < TIMER_SAMPLE_CNT;i++)  /* calibrate the overhead for measuring time */
        {
        t0 = HiResTime();
        Init(&state, 0);
        Update(&state, data, 1024+29);
        Final(&state, 0);
        t1 = HiResTime();
        if (tMin > t1-t0 - dtMin)       /* keep only the minimum time */
            tMin = t1-t0 - dtMin;
        }

    /* now tMin = # clocks required for running RoutineToBeTimed() */
    
    return tMin;
}

uint_32t measureHash3blocks(uint_32t dtMin)
{
    uint_32t tMin = 0xFFFFFFFF;         /* big number to start */
    uint_32t t0,t1,i;
    hashState state;
    ALIGN unsigned char data[384];

    for (i=0;i < TIMER_SAMPLE_CNT;i++)  /* calibrate the overhead for measuring time */
        {
        t0 = HiResTime();
        Init(&state, 0);
        Update(&state, data, 2048+29);
        Final(&state, 0);
        t1 = HiResTime();
        if (tMin > t1-t0 - dtMin)       /* keep only the minimum time */
            tMin = t1-t0 - dtMin;
        }

    /* now tMin = # clocks required for running RoutineToBeTimed() */
    
    return tMin;
}

uint_32t measureHash10blocks(uint_32t dtMin)
{
    uint_32t tMin = 0xFFFFFFFF;         /* big number to start */
    uint_32t t0,t1,i;
    hashState state;
    ALIGN unsigned char data[10*128];

    for (i=0;i < TIMER_SAMPLE_CNT;i++)  /* calibrate the overhead for measuring time */
        {
        t0 = HiResTime();
        Init(&state, 0);
        Update(&state, data, 9*1024+29);
        Final(&state, 0);
        t1 = HiResTime();
        if (tMin > t1-t0 - dtMin)       /* keep only the minimum time */
            tMin = t1-t0 - dtMin;
        }

    /* now tMin = # clocks required for running RoutineToBeTimed() */
    
    return tMin;
}

uint_32t measureHash30blocks(uint_32t dtMin)
{
    uint_32t tMin = 0xFFFFFFFF;         /* big number to start */
    uint_32t t0,t1,i;
    hashState state;
    ALIGN unsigned char data[30*128];

    for (i=0;i < TIMER_SAMPLE_CNT;i++)  /* calibrate the overhead for measuring time */
        {
        t0 = HiResTime();
        Init(&state, 0);
        Update(&state, data, 29*1024+29);
        Final(&state, 0);
        t1 = HiResTime();
        if (tMin > t1-t0 - dtMin)       /* keep only the minimum time */
            tMin = t1-t0 - dtMin;
        }

    /* now tMin = # clocks required for running RoutineToBeTimed() */
    
    return tMin;
}

uint_32t measureHash100blocks(uint_32t dtMin)
{
    uint_32t tMin = 0xFFFFFFFF;         /* big number to start */
    uint_32t t0,t1,i;
    hashState state;
    ALIGN unsigned char data[100*128];

    for (i=0;i < TIMER_SAMPLE_CNT;i++)  /* calibrate the overhead for measuring time */
        {
        t0 = HiResTime();
        Init(&state, 0);
        Update(&state, data, 99*1024+29);
        Final(&state, 0);
        t1 = HiResTime();
        if (tMin > t1-t0 - dtMin)       /* keep only the minimum time */
            tMin = t1-t0 - dtMin;
        }

    /* now tMin = # clocks required for running RoutineToBeTimed() */
    
    return tMin;
}

uint_32t measureHash1000blocks(uint_32t dtMin)
{
    uint_32t tMin = 0xFFFFFFFF;         /* big number to start */
    uint_32t t0,t1,i;
    hashState state;
    ALIGN unsigned char data[1000*128];

    for (i=0;i < TIMER_SAMPLE_CNT;i++)  /* calibrate the overhead for measuring time */
        {
        t0 = HiResTime();
        Init(&state, 0);
        Update(&state, data, 999*1024+29);
        Final(&state, 0);
        t1 = HiResTime();
        if (tMin > t1-t0 - dtMin)       /* keep only the minimum time */
            tMin = t1-t0 - dtMin;
        }

    /* now tMin = # clocks required for running RoutineToBeTimed() */
    
    return tMin;
}

void doTiming()
{

	
	
	uint_32t calibration;
    uint_32t measurementPermutation;
    uint_32t measurementAbsorb1024bits;
    uint_32t measurementAbsorb1088bits;
    uint_32t measurementAbsorb1344bits;
    uint_32t measurementAbsorb;
    uint_32t measurementHash1block;
    uint_32t measurementHash2blocks;
    uint_32t measurementHash3blocks;
    uint_32t measurementHash10blocks;
    uint_32t measurementHash30blocks;
    uint_32t measurementHash100blocks;
    uint_32t measurementHash1000blocks;
    
    calibration = calibrate();

    measurementPermutation = measurePermutation(calibration);
    printf("Cycles for Permutation(state): %d\n", measurementPermutation);
    printf("Cycles per byte for rate 1024: %f\n", measurementPermutation/128.0);
    printf("\n");

    measurementAbsorb1024bits = measureAbsorb1024bits(calibration);
    printf("Cycles for Absorb1024bits(state, input): %d\n", measurementAbsorb1024bits);
    printf("Cycles per byte for rate 1024: %f\n", measurementAbsorb1024bits/128.0);
    printf("\n");

    measurementAbsorb1088bits = measureAbsorb1088bits(calibration);
    printf("Cycles for Absorb1088bits(state, input): %d\n", measurementAbsorb1088bits);
    printf("Cycles per byte for rate 1088: %f\n", measurementAbsorb1088bits/136.0);
    printf("\n");

    measurementAbsorb1344bits = measureAbsorb1344bits(calibration);
    printf("Cycles for Absorb1344bits(state, input): %d\n", measurementAbsorb1344bits);
    printf("Cycles per byte for rate 1344: %f\n", measurementAbsorb1344bits/168.0);
    printf("\n");

    measurementAbsorb = measureAbsorb(calibration, 18);
    printf("Cycles for Absorb(state, input, 1152bits): %d\n", measurementAbsorb);
    printf("Cycles per byte for rate 1152: %f\n", measurementAbsorb/144.0);
    measurementAbsorb = measureAbsorb(calibration, 13);
    printf("Cycles for Absorb(state, input, 832bits): %d\n", measurementAbsorb);
    printf("Cycles per byte for rate 832: %f\n", measurementAbsorb/104.0);
    measurementAbsorb = measureAbsorb(calibration, 9);
    printf("Cycles for Absorb(state, input, 576bits): %d\n", measurementAbsorb);
    printf("Cycles per byte for rate 576: %f\n", measurementAbsorb/72.0);
    printf("\n");
    printf("Cycles for Init, Update, Finalize (1 block): %d\n", measurementHash1block);
    printf("\n");

    measurementHash2blocks = measureHash2blocks(calibration);
    printf("Cycles for Init, Update, Finalize (2 blocks): %d\n", measurementHash2blocks);
    printf("\n");

    measurementHash3blocks = measureHash3blocks(calibration);
    printf("Cycles for Init, Update, Finalize (3 blocks): %d\n", measurementHash3blocks);
    printf("\n");

    measurementHash1block = measureHash1block(calibration);

    measurementHash10blocks = measureHash10blocks(calibration);
    printf("Cycles for Init, Update, Finalize (10 blocks): %d\n", measurementHash10blocks);
    printf("\n");

    measurementHash30blocks = measureHash30blocks(calibration);
    printf("Cycles for Init, Update, Finalize (30 blocks): %d\n", measurementHash30blocks);
    printf("\n");

    measurementHash100blocks = measureHash100blocks(calibration);
    printf("Cycles for Init, Update, Finalize (100 blocks): %d\n", measurementHash100blocks);
    printf("\n");

    measurementHash1000blocks = measureHash1000blocks(calibration);
    printf("Cycles for Init, Update, Finalize (1000 blocks): %d\n", measurementHash1000blocks);
    printf("\n");

	
}


void HashMessage(BitSequence *data, unsigned int DataByteLen, BitSequence *out,int *DigestLen)
{
	hashState state;
//	BitSequence out[65]={0};
	Init(&state, 256);
    Update(&state, data, DataByteLen<<3);
    Final(&state, out);
	*DigestLen=64;
}
void testspeed()
{
	void (*Hash_Speed) (unsigned char *InMessage,unsigned int MessageLen,unsigned char *OutDigest, int *DigestLen);
	
	unsigned char *MSG;
	unsigned char out[200]={0};
    int outlen;
    int i;
	int j;
	int N = 1024;
	int Num=1000000, Mb=1<<20;
	double speed=0;
	clock_t start,finish;
	int N_array[5]={16, 64, 1024, 8192, 1024*1024};
	int Num_a[5]={30000000, 30000000, 1000000, 200, 30};
	Hash_Speed = HashMessage;
	for(j=0; j<5; j++)
	{
		N = N_array[j];
		MSG = (unsigned char *)malloc(sizeof(unsigned char)*N);
	
		for(i=0;i<N;i++)
		{
			MSG[i]='a';
		}
		Num = Num_a[j];
		start = clock();
		for(i=0;i<Num;i++)
			Hash_Speed(MSG,N,out,&outlen);
		finish = clock();
		free(MSG);
		speed = (Num*N*8.0*1000/Mb/(finish-start));
		printf("%dB speed:%f Mbps\n",N,speed);
	}
	printf("\n");
	
}

int testspeed_cycle()
{
	
	unsigned char *MSG;
	unsigned char out[200]={0};
    int outlen;
    int i, in, t1, t0;
	int j;
	int N = 1024;
	int Num=1000000, Mb=1<<20;
	double speed=0;
	int N_array[5]={16, 64, 1024, 8192, 1024*1024};
	int Num_a[5]={30000000, 30000000, 1000000, 200, 30};
	unsigned int tMin = 0xFFFFFFFF;        /* big number to start */
	unsigned int dtMin = calibrate();
	//void (*Hash_Speed) (unsigned char *InMessage,int MessageLen,unsigned char *OutDigest, int *DigestLen);
	//Hash_Speed = HashMessage;
	
	
	for(j=0; j<5; j++)
	{
		N = N_array[j];
		MSG = (unsigned char *)malloc(sizeof(unsigned char)*N);
	
		for(i=0;i<N;i++)
		{
			MSG[i]='a';
		}
		Num = Num_a[j];
		tMin = 0xFFFFFFFF;
		for (in=0;in < TIMER_SAMPLE_CNT;in++)  /* calibrate the overhead for measuring time */
        {
        t0 = HiResTime();
		HashMessage(MSG,N,out,&outlen);
		 t1 = HiResTime();
        if (tMin > t1-t0 - dtMin)       /* keep only the minimum time */
            tMin = t1-t0 - dtMin;
        }
		free(MSG);
		speed = tMin/N;
		printf("Msglen=%d bytes, speed:%.2f cycles/byte\n",N,speed);
	}
	printf("\n");
	return tMin;
}

void main()
{
	//doTiming();
	testspeed();
	testspeed_cycle();
	
	getchar();
}


