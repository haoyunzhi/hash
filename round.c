/*
The  sponge function, designed by Guido Bertoni, Joan Daemen,
Michaël Peeters and Gilles Van Assche. For more information, feedback or
questions, please refer to our website: http://keccak.noekeon.org/

Implementation by the designers,
hereby denoted as "the implementer".

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/

#include <string.h>
#include "brg_endian.h"
#include "settings.h"
#include "interface.h"

typedef unsigned char UINT8;
typedef unsigned long long int UINT64;

#if defined(__GNUC__)
#define ALIGN __attribute__ ((aligned(32)))
#elif defined(_MSC_VER)
#define ALIGN __declspec(align(32))
#else
#define ALIGN
#endif

#define ROL64(a, offset) ((((UINT64)a) << offset) ^ (((UINT64)a) >> (64-offset)))

    #include "Hash-1920-64.macros"
#endif

#include "unrolling.macros"

void PermutationOnWords(UINT64 *state)
{
    declareABC
#if (Unrolling != 10)
    unsigned int i;
#endif

    copyFromState(A, state)
    rounds

}

void PermutationOnWordsAfterXoring(UINT64 *state, const UINT64 *input, unsigned int laneCount)
{
    declareABC
#if (Unrolling != 10)
    unsigned int i;
#endif
	unsigned int j;

    for(j=0; j<laneCount; j++)
        state[j] ^= input[j];	
    copyFromState(A, state)
    rounds

}

#ifdef ProvideFast896
void PermutationOnWordsAfterXoring896bits(UINT64 *state, const UINT64 *input)
{
    declareABC
#if (Unrolling != 10)
    unsigned int i;
#endif

    copyFromStateAndXor896bits(A, state, input)
    rounds

}
#endif

#ifdef ProvideFast1152
void PermutationOnWordsAfterXoring1152bits(UINT64 *state, const UINT64 *input)
{
    declareABC
#if (Unrolling != 10)
    unsigned int i;
#endif

    copyFromStateAndXor1152bits(A, state, input)
    rounds

}
#endif

#ifdef ProvideFast1024
void PermutationOnWordsAfterXoring1024bits(UINT64 *state, const UINT64 *input)
{
    declareABC
#if (Unrolling != 10)
    unsigned int i;
#endif

    copyFromStateAndXor1024bits(A, state, input)
    rounds

}
#endif

#ifdef ProvideFast1408
void PermutationOnWordsAfterXoring1408bits(UINT64 *state, const UINT64 *input)
{
    declareABC
#if (Unrolling != 10)
    unsigned int i;
#endif

    copyFromStateAndXor1408bits(A, state, input)
    rounds

}
#endif


void Initialize()
{
}

void InitializeState(unsigned char *state)
{
    memset(state, 0, 200);
#ifdef UseBebigokimisa
    ((UINT64*)state)[ 1] = ~(UINT64)0;
    ((UINT64*)state)[ 2] = ~(UINT64)0;
    ((UINT64*)state)[ 8] = ~(UINT64)0;
    ((UINT64*)state)[12] = ~(UINT64)0;
    ((UINT64*)state)[17] = ~(UINT64)0;
    ((UINT64*)state)[20] = ~(UINT64)0;
#endif
}

void Permutation(unsigned char *state)
{
    // We assume the state is always stored as words
    PermutationOnWords((UINT64*)state);
}

void fromBytesToWord(UINT64 *word, const UINT8 *bytes)
{
    unsigned int i;

    *word = 0;
    for(i=0; i<(64/8); i++)
        *word |= (UINT64)(bytes[i]) << (8*i);
}

#ifdef ProvideFast896
void Absorb896bits(unsigned char *state, const unsigned char *data)
{
#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    PermutationOnWordsAfterXoring576bits((UINT64*)state, (const UINT64*)data);
#else
    UINT64 dataAsWords[14];
    unsigned int i;

    for(i=0; i<14; i++)
        fromBytesToWord(dataAsWords+i, data+(i*8));
    PermutationOnWordsAfterXoring896bits((UINT64*)state, dataAsWords);
#endif
}
#endif

#ifdef ProvideFast1152
void Absorb1152bits(unsigned char *state, const unsigned char *data)
{
#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    PermutationOnWordsAfterXoring1152bits((UINT64*)state, (const UINT64*)data);
#else
    UINT64 dataAsWords[18];
    unsigned int i;

    for(i=0; i<18; i++)
        fromBytesToWord(dataAsWords+i, data+(i*8));
    PermutationOnWordsAfterXoring1152bits((UINT64*)state, dataAsWords);
#endif
}
#endif

#ifdef ProvideFast1024
void Absorb1024bits(unsigned char *state, const unsigned char *data)
{
#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    PermutationOnWordsAfterXoring1024bits((UINT64*)state, (const UINT64*)data);
#else
    UINT64 dataAsWords[16];
    unsigned int i;

    for(i=0; i<16; i++)
        fromBytesToWord(dataAsWords+i, data+(i*8));
    PermutationOnWordsAfterXoring1024bits((UINT64*)state, dataAsWords);
#endif
}
#endif

#ifdef ProvideFast1408
void Absorb1408bits(unsigned char *state, const unsigned char *data)
{
#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    PermutationOnWordsAfterXoring1088bits((UINT64*)state, (const UINT64*)data);
#else
    UINT64 dataAsWords[22];
    unsigned int i;

    for(i=0; i<22; i++)
        fromBytesToWord(dataAsWords+i, data+(i*8));
    PermutationOnWordsAfterXoring1408bits((UINT64*)state, dataAsWords);
#endif
}
#endif


void Absorb(unsigned char *state, const unsigned char *data, unsigned int laneCount)
{
#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    PermutationOnWordsAfterXoring((UINT64*)state, (const UINT64*)data, laneCount);
#else
    UINT64 dataAsWords[25];
    unsigned int i;

    for(i=0; i<laneCount; i++)
        fromBytesToWord(dataAsWords+i, data+(i*8));
    PermutationOnWordsAfterXoring((UINT64*)state, dataAsWords, laneCount);
#endif
}

void fromWordToBytes(UINT8 *bytes, const UINT64 word)
{
    unsigned int i;

    for(i=0; i<(64/8); i++)
        bytes[i] = (word >> (8*i)) & 0xFF;
}

#ifdef ProvideFast1024
void Extract1024bits(const unsigned char *state, unsigned char *data)
{
#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    memcpy(data, state, 128);
#else
    unsigned int i;

    for(i=0; i<16; i++)
        fromWordToBytes(data+(i*8), ((const UINT64*)state)[i]);
#endif
#ifdef UseBebigokimisa
    ((UINT64*)data)[ 1] = ~((UINT64*)data)[ 1];
    ((UINT64*)data)[ 2] = ~((UINT64*)data)[ 2];
    ((UINT64*)data)[ 8] = ~((UINT64*)data)[ 8];
    ((UINT64*)data)[12] = ~((UINT64*)data)[12];
#endif
}
#endif

void Extract(const unsigned char *state, unsigned char *data, unsigned int laneCount)
{
#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
    memcpy(data, state, laneCount*8);
#else
    unsigned int i;

    for(i=0; i<laneCount; i++)
        fromWordToBytes(data+(i*8), ((const UINT64*)state)[i]);
#endif
#ifdef UseBebigokimisa
    if (laneCount > 1) {
        ((UINT64*)data)[ 1] = ~((UINT64*)data)[ 1];
        if (laneCount > 2) {
            ((UINT64*)data)[ 2] = ~((UINT64*)data)[ 2];
            if (laneCount > 8) {
                ((UINT64*)data)[ 8] = ~((UINT64*)data)[ 8];
                if (laneCount > 12) {
                    ((UINT64*)data)[12] = ~((UINT64*)data)[12];
                    if (laneCount > 17) {
                        ((UINT64*)data)[17] = ~((UINT64*)data)[17];
                        if (laneCount > 20) {
                            ((UINT64*)data)[20] = ~((UINT64*)data)[20];
                        }
                    }
                }
            }
        }
    }
#endif
}
