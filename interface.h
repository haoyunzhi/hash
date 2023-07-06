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

#ifndef _PermutationInterface_h_
#define _PermutationInterface_h_

#include "Hash-1920-int-set.h"

void Initialize( void );
void InitializeState(unsigned char *state);
void Permutation(unsigned char *state);
#ifdef ProvideFast896
void Absorb896bits(unsigned char *state, const unsigned char *data);
#endif
#ifdef ProvideFast1152
void Absorb1152bits(unsigned char *state, const unsigned char *data);
#endif
#ifdef ProvideFast1024
void Absorb1024bits(unsigned char *state, const unsigned char *data);
#endif
#ifdef ProvideFast1408
void Absorb1408bits(unsigned char *state, const unsigned char *data);
#endif
void Absorb(unsigned char *state, const unsigned char *data, unsigned int laneCount);
#ifdef ProvideFast1024
void Extract1024bits(const unsigned char *state, unsigned char *data);
#endif
void Extract(const unsigned char *state, unsigned char *data, unsigned int laneCount);

#endif
