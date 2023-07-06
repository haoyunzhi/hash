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
#include "Sponge.h"
#include "interface.h"
#ifdef Reference
#include "displayIntermediateValues.h"
#endif

int InitSponge(spongeState *state, unsigned int rate, unsigned int capacity)
{
   int i;
	if (rate+capacity != 1920)
        return 1;
    if ((rate <= 0) || (rate >= 1920) || ((rate % 64) != 0))
        return 1;
    Initialize();
    state->rate = rate;
    state->capacity = capacity;
    state->fixedOutputLength = 0;
	//for(i=0;i<200;i++)state->state[i]=0;
    InitializeState(state->state);
    memset(state->dataQueue, 0, MaximumRateInBytes);
	//for(i=0;i<192;i++)state->dataQueue[i]=0;
    state->bitsInQueue = 0;
    state->squeezing = 0;
    state->bitsAvailableForSqueezing = 0;

    return 0;
}

void AbsorbQueue(spongeState *state)
{
    // state->bitsInQueue is assumed to be equal to state->rate
    #ifdef Reference
    displayBytes(1, "Block to be absorbed", state->dataQueue, state->rate/8);
    #endif
#ifdef ProvideFast896
    if (state->rate == 896)
        Absorb896bits(state->state, state->dataQueue);
    else 
#endif
#ifdef ProvideFast1152
    if (state->rate == 1152)
        Absorb1152bits(state->state, state->dataQueue);
    else 
#endif
#ifdef ProvideFast1408
    if (state->rate == 1408)
        Absorb1408bits(state->state, state->dataQueue);
    else
#endif
        Absorb(state->state, state->dataQueue, state->rate/64);
    state->bitsInQueue = 0;
}

int Absorb(spongeState *state, const unsigned char *data, unsigned long long databitlen)
{
    unsigned long long i, j, wholeBlocks;
    unsigned int partialBlock, partialByte;
    const unsigned char *curData;

    if ((state->bitsInQueue % 8) != 0)
        return 1; // Only the last call may contain a partial byte
    if (state->squeezing)
        return 1; // Too late for additional input

    i = 0;
    while(i < databitlen) {
        if ((state->bitsInQueue == 0) && (databitlen >= state->rate) && (i <= (databitlen-state->rate))) {
            wholeBlocks = (databitlen-i)/state->rate;
            curData = data+i/8;
#ifdef ProvideFast896
            if (state->rate == 896) {
                for(j=0; j<wholeBlocks; j++, curData+=896/8) {
                    #ifdef Reference
                    displayBytes(1, "Block to be absorbed", curData, state->rate/8);
                    #endif
                    Absorb896bits(state->state, curData);
                }
            }
            else
#endif
#ifdef ProvideFast1152
            if (state->rate == 1152) {
                for(j=0; j<wholeBlocks; j++, curData+=1152/8) {
                    #ifdef Reference
                    displayBytes(1, "Block to be absorbed", curData, state->rate/8);
                    #endif
                    Absorb1152bits(state->state, curData);
                }
            }
            else
#endif
#ifdef ProvideFast1408
            if (state->rate == 1408) {
                for(j=0; j<wholeBlocks; j++, curData+=1408/8) {
                    #ifdef Reference
                    displayBytes(1, "Block to be absorbed", curData, state->rate/8);
                    #endif
                    Absorb1408bits(state->state, curData);
                }
            }
            else
#endif
            {
                for(j=0; j<wholeBlocks; j++, curData+=state->rate/8) {
                    #ifdef Reference
                    displayBytes(1, "Block to be absorbed", curData, state->rate/8);
                    #endif
                    Absorb(state->state, curData, state->rate/64);
                }
            }
            i += wholeBlocks*state->rate;
        }
        else {
            partialBlock = (unsigned int)(databitlen - i);
            if (partialBlock+state->bitsInQueue > state->rate)
                partialBlock = state->rate-state->bitsInQueue;
            partialByte = partialBlock % 8;
            partialBlock -= partialByte;
            memcpy(state->dataQueue+state->bitsInQueue/8, data+i/8, partialBlock/8);
            state->bitsInQueue += partialBlock;
            i += partialBlock;
            if (state->bitsInQueue == state->rate)
                AbsorbQueue(state);
            if (partialByte > 0) {
                unsigned char mask = (1 << partialByte)-1;
                state->dataQueue[state->bitsInQueue/8] = data[i/8] & mask;
                state->bitsInQueue += partialByte;
                i += partialByte;
            }
        }
    }
    return 0;
}

void PadAndSwitchToSqueezingPhase(spongeState *state)
{
    // Note: the bits are numbered from 0=LSB to 7=MSB
    if (state->bitsInQueue + 1 == state->rate) {
        state->dataQueue[state->bitsInQueue/8 ] |= 1 << (state->bitsInQueue % 8);
        AbsorbQueue(state);
        memset(state->dataQueue, 0, state->rate/8);
    }
    else {
        memset(state->dataQueue + (state->bitsInQueue+7)/8, 0, state->rate/8 - (state->bitsInQueue+7)/8);
        state->dataQueue[state->bitsInQueue/8 ] |= 1 << (state->bitsInQueue % 8);
    }
    state->dataQueue[(state->rate-1)/8] |= 1 << ((state->rate-1) % 8);
    AbsorbQueue(state);

    #ifdef Reference
    displayText(1, "--- Switching to squeezing phase ---");
    #endif
#ifdef ProvideFast1024
    if (state->rate == 1024) {
        Extract1024bits(state->state, state->dataQueue);
        state->bitsAvailableForSqueezing = 1024;
    }
    else
#endif
    {
        Extract(state->state, state->dataQueue, state->rate/64);
        state->bitsAvailableForSqueezing = state->rate;
    }
    #ifdef Reference
    displayBytes(1, "Block available for squeezing", state->dataQueue, state->bitsAvailableForSqueezing/8);
    #endif
    state->squeezing = 1;
}

int Squeeze(spongeState *state, unsigned char *output, unsigned long long outputLength)
{
    unsigned long long i;
    unsigned int partialBlock;

    if (!state->squeezing)
        PadAndSwitchToSqueezingPhase(state);
    if ((outputLength % 8) != 0)
        return 1; // Only multiple of 8 bits are allowed, truncation can be done at user level

    i = 0;
    while(i < outputLength) {
        if (state->bitsAvailableForSqueezing == 0) {
            Permutation(state->state);
#ifdef ProvideFast1024
            if (state->rate == 1024) {
                Extract1024bits(state->state, state->dataQueue);
                state->bitsAvailableForSqueezing = 1024;
            }
            else
#endif
            {
                Extract(state->state, state->dataQueue, state->rate/64);
                state->bitsAvailableForSqueezing = state->rate;
            }
            #ifdef Reference
            displayBytes(1, "Block available for squeezing", state->dataQueue, state->bitsAvailableForSqueezing/8);
            #endif
        }
        partialBlock = state->bitsAvailableForSqueezing;
        if ((unsigned long long)partialBlock > outputLength - i)
            partialBlock = (unsigned int)(outputLength - i);
        memcpy(output+i/8, state->dataQueue+(state->rate-state->bitsAvailableForSqueezing)/8, partialBlock/8);
        state->bitsAvailableForSqueezing -= partialBlock;
        i += partialBlock;
    }
    return 0;
}
