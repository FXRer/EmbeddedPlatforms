#ifndef _SPECK_H_
#define _SPECK_H_

#include "winTypes.h"

//speck with 32-bit data and 64-bit key

//speck for AVR
// (c) 2016 Dmitry.GR


//speck config
#define SPECK_DATA_TYPE				uint32_t
#define SPECK_KEY_WORDS				4
#define SPECK_NUM_ROUNDS			27
#define SPECK_ALPHA					8
#define SPECK_BETA					3


////speck config
//#define SPECK_DATA_TYPE				uint16_t
//#define SPECK_KEY_WORDS				4
//#define SPECK_NUM_ROUNDS			22
//#define SPECK_ALPHA					7
//#define SPECK_BETA					2


//speck spec
#define SPECK_DATA_WORDS			2

//key schedule
void speckKeySched(SPECK_DATA_TYPE* dst, const SPECK_DATA_TYPE *k);

//ECB
void speckEncr(SPECK_DATA_TYPE *data, const SPECK_DATA_TYPE *key /*prescheduled*/);
void speckDecr(SPECK_DATA_TYPE *data, const SPECK_DATA_TYPE *key /*prescheduled*/);

//CBC
void speckEncrCbc(SPECK_DATA_TYPE *data, SPECK_DATA_TYPE *iv, const SPECK_DATA_TYPE *key /*prescheduled*/);
void speckDecrCbc(SPECK_DATA_TYPE *data, SPECK_DATA_TYPE *iv, const SPECK_DATA_TYPE *key /*prescheduled*/);

#endif
