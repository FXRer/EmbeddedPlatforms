#include "speck.h"
#include <stdio.h>

//speck for AVR
// (c) 2016 Dmitry.GR

static SPECK_DATA_TYPE rol(SPECK_DATA_TYPE v, uint8_t by)
{
	return (v << by) | (v >> (8 * sizeof(SPECK_DATA_TYPE) - by));
}

static SPECK_DATA_TYPE ror(SPECK_DATA_TYPE v, uint8_t by)
{
	return (v >> by) | (v << (8 * sizeof(SPECK_DATA_TYPE) - by));
}

static SPECK_DATA_TYPE rorA(SPECK_DATA_TYPE v)
{
	return ror(v, SPECK_ALPHA);
}

static SPECK_DATA_TYPE rorB(SPECK_DATA_TYPE v)
{
	return ror(v, SPECK_BETA);
}

static SPECK_DATA_TYPE rolA(SPECK_DATA_TYPE v)
{
	return rol(v, SPECK_ALPHA);
}

static SPECK_DATA_TYPE rolB(SPECK_DATA_TYPE v)
{
	return rol(v, SPECK_BETA);
}


#ifdef AVR
	#include <avr/pgmspace.h>
	#define READ_KEY(v)	((sizeof(SPECK_DATA_TYPE) == sizeof(uint16_t)) ? pgm_read_word(&v) : pgm_read_dword(&v))
#else
	#define READ_KEY(v)	(v)
#endif

void speckKeySched(SPECK_DATA_TYPE* dst, const SPECK_DATA_TYPE *k)
{
	uint8_t i;
	SPECK_DATA_TYPE L[SPECK_KEY_WORDS];
	
	for (i = 0; i < SPECK_KEY_WORDS - 1; i++)
		L[SPECK_KEY_WORDS - 2 - i] = k[i];

	dst[0] = k[SPECK_KEY_WORDS - 1];
	
	for (i = 0; i < SPECK_NUM_ROUNDS - 1; i++) {
		uint32_t j = (i + SPECK_KEY_WORDS - 1) % SPECK_KEY_WORDS;
		
		L[j] = (dst[i] + rorA(L[i % SPECK_KEY_WORDS])) ^ i;
		dst[i + 1] = rolB(dst[i]) ^ L[j];
	}
}

void speckEncrCbc(SPECK_DATA_TYPE *data, SPECK_DATA_TYPE *iv, const SPECK_DATA_TYPE *key /*prescheduled*/)
{
	uint8_t i;
	
	for (i = 0; i < SPECK_DATA_WORDS; i++)
		data[i] ^= iv[i];
	
	speckEncr(data, key);
	
	for (i = 0; i < SPECK_DATA_WORDS; i++)
		iv[i] = data[i];
}

void speckEncr(SPECK_DATA_TYPE *data, const SPECK_DATA_TYPE *k)
{
	SPECK_DATA_TYPE x = data[0], y = data[1];
	uint8_t i;
	
	for (i = 0; i < SPECK_NUM_ROUNDS; i++) {
		
		x = (rorA(x) + y) ^ READ_KEY(k[i]);
		y = rolB(y) ^ x;
	}
	
	data[0] = x;
	data[1] = y;
}

void speckDecr(SPECK_DATA_TYPE *data, const SPECK_DATA_TYPE *k)
{
	SPECK_DATA_TYPE x = data[0], y = data[1];
	int8_t i;
	
	for (i = SPECK_NUM_ROUNDS - 1; i >= 0; i--) {
		
		y = rorB(x ^ y);
		x = rolA((x ^ READ_KEY(k[i])) - y);
	}
	
	data[0] = x;
	data[1] = y;
}


#if defined(AVR) && (SPECK_NUM_ROUNDS == 27) && (SPECK_ALPHA == 8) && (SPECK_BETA == 3)	//ASM!!!!
	
	void __attribute__((naked)) speckDecrCbc(SPECK_DATA_TYPE *data, SPECK_DATA_TYPE *iv, const SPECK_DATA_TYPE *k)
	{
		//data = r24:r25
		//iv = r22:r23
		//k = r20:r21
		asm volatile(
			"		push	r29				\n"		//We'll need Y
			"		push	r28				\n"		//We'll need Y
			"		push	r17				\n"		//We'll need space
			"		push	r16				\n"		//We'll need space
			"		movw	r26, r24		\n"		//X = data
			"		movw	r28, r22		\n"		//Y = iv
			"		movw	r30, r20		\n"		//Z = key
			"		subi	r30, -104		\n"		//Z += sizeof(key) /* == NUM ROUNDS * sizeof(uint32_t) */ - sizeof(uint32_t)
			"		sbci	r31, -1			\n"
			"		adiw	r28, 8			\n"		//Y = iv + sizeof(iv)
			
			//get x
			"		ld		r18, X+			\n"		//r18 ix x:0..7
			"		ld		r19, X+			\n"		//r19 is x:8..15
			"		ld		r20, X+			\n"		//r20 is x:16..23
			"		ld		r21, X+			\n"		//r21 is x:24..31
			
			//get y
			"		ld		r22, X+			\n"		//r22 ix y:0..7
			"		ld		r23, X+			\n"		//r23 is y:8..15
			"		ld		r24, X+			\n"		//r24 is y:16..23
			"		ld		r25, X+			\n"		//r25 is y:24..31
			
			//ROUND LOOP
			"		ldi		r17, 27			\n"		//r17 is rounds counter
			"1:								\n"
			
			//Y ^= X
			"		eor		r22, r18		\n"
			"		eor		r23, r19		\n"
			"		eor		r24, r20		\n"
			"		eor		r25, r21		\n"
			
			//Y = Y ror 3
			"		clr		r0				\n"
			"		ldi		r16, 3			\n"
			"2:								\n"
			"		lsr		r25				\n"
			"		ror		r24				\n"
			"		ror		r23				\n"
			"		ror		r22				\n"
			"		ror		r0				\n"
			"		dec		r16				\n"
			"		brne	2b				\n"
			"		or		r25, r0			\n"
			
			//X ^= *key--
			"		lpm		r16, Z+			\n"
			"		eor		r18, r16		\n"
			"		lpm		r16, Z+			\n"
			"		eor		r19, r16		\n"
			"		lpm		r16, Z+			\n"
			"		eor		r20, r16		\n"
			"		lpm		r16, Z+			\n"
			"		eor		r21, r16		\n"
			"		sbiw	r30, 8			\n"
			
			//X = X - Y
			"		sub		r18, r22		\n"
			"		sbc		r19, r23		\n"
			"		sbc		r20, r24		\n"
			"		sbc		r21, r25		\n"
			
			//X = X rol 8
			"		mov		r16, r21		\n"
			"		mov		r21, r20		\n"
			"		mov		r20, r19		\n"
			"		mov		r19, r18		\n"
			"		mov		r18, r16		\n"
			
			//round loop maintenance
			"		dec		r17				\n"
			"		brne	1b				\n"
			
			//ROUND LOOP OVER
			
			//do the CBC thing (temp = data[i]; data[i] ^= iv[i]; iv[i] = temp) for Y and then X 
			
			//to allow us to do it in a loop (& thus save code space) we use the stac since we cannot iterate over regs in a loop
			//this can be unrolled & use no stack, but at the cost of ~25 extra instra
			//it woudl be done like this:
			//	"ld              r16, -Y                 \n"     //r16 = iv_byte
			//	"ld              r17, -X                 \n"     //r17 = data_byte
			//	"st              Y, r17                  \n"     //iv_byte = r17
			//	"eor             r@@, r16                \n"     //new_X_byte ^= iv_orig_byte
			//	"st              X, r@@                  \n" 	//data_orig_byte = X_byte
			//	this iwll be duplicarted 8 tymes with r@@ = r25..r18

			
			//push regs in proper order
			"		push	r18				\n"
			"		push	r19				\n"
			"		push	r20				\n"
			"		push	r21				\n"
			"		push	r22				\n"
			"		push	r23				\n"
			"		push	r24				\n"
			"		push	r25				\n"
			
			//loop
			"		ldi		r17, 8			\n"
			"1:								\n"
			"		ld		r20, -Y			\n"	//r20 = *iv
			"		ld		r21, -X			\n"	//r21 = *data
			"		pop		r22				\n"	//r22 = new_data_byte
			"		eor		r22, r20		\n"	//new_data_byte ^= iv_byte
			"		st		Y, r21			\n"	//*iv = orig_data_byte
			"		st		X, r22			\n"	//*data = new_data_byte
			
			"		dec		r17				\n"	//loop maintenance
			"		brne	1b				\n"
			
			
			//exit
			"		pop		r16				\n"
			"		pop		r17				\n"
			"		pop		r28				\n"
			"		pop		r29				\n"
			"		ret						\n"
		);
		
		//shup up gcc
		(void)data;
		(void)iv;
		(void)k;
	}


#else

	void speckDecrCbc(SPECK_DATA_TYPE *data, SPECK_DATA_TYPE *iv, const SPECK_DATA_TYPE *key /*prescheduled*/)
	{
		SPECK_DATA_TYPE newIv[SPECK_DATA_WORDS];
		uint8_t i;
		
		for (i = 0; i < SPECK_DATA_WORDS; i++)
			newIv[i] = data[i];
		
		speckDecr(data, key);
		
		for (i = 0; i < SPECK_DATA_WORDS; i++) {
			data[i] ^= iv[i];
			iv[i] = newIv[i];
		}
	}
#endif
