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
#include "stdio.h"
#include "timing.h"

//int genKAT_main();
void testspeed();

int main()
{
	printf("hello\n");
	//getchar();
    //doTiming();

	//testspeed_cycle();
	testspeed();

    //return genKAT_main();
	getchar();
	return 0;
}
