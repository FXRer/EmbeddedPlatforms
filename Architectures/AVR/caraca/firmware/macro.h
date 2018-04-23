#ifndef _MACRO_H
#define _MACRO_H

#define addi(reg,val)   subi reg, 256-(val)

/* skip next instruction if not equal to zero */
#define skipne\ 
        brne    jump_loc@ + 2\
jump_loc@:\ 


/* skip next instruction if carry set */
#define skipcs\ 
        brcs    jump_loc@ + 2\
jump_loc@:\ 


/* skip next instruction if carry clear */
#define skipcc\ 
        brcc    jump_loc@ + 2\
jump_loc@:\ 


/* skip next instruction if equal to zero */
#define skipeq\ 
        breq    jump_loc@ + 2\
jump_loc@:\ 


/* skip next instruction if lower */
#define skiplo\ 
        brlo    jump_loc@ + 2\
jump_loc@:\ 


/* skip next instruction if half carry set */
#define skiphs\ 
        brhs    jump_loc@ + 2\
jump_loc@:\ 


/* skip next instruction if half carry clear */
#define skiphc\ 
        brhc    jump_loc@ + 2\
jump_loc@:\ 


/* skip next instruction if T set */
#define skipts\ 
        brts    jump_loc@ + 2\
jump_loc@:\ 


/* skip next instruction if T clear */
#define skiptc\ 
        brtc    jump_loc@ + 2\
jump_loc@:\ 


/* skip next instruction if minus */
#define skipmi\ 
        brmi    jump_loc@ + 2\
jump_loc@:\ 


/* skip next instruction always */
#define skipnext\ 
        rjmp    jump_loc@ + 2\
jump_loc@:\ 

/* wait two cycles but waste only one instruction */
#define doublenop\ 
        rjmp    jump_loc@\
jump_loc@:\ 


/* compare Port bit with T, and branch to loc if are not equal */
#define cpeqPortBit(port,bit,loc)\
        sbic  port,bit\
        rjmp  test_bit1@\
test_bit0@:\
        brts  loc\
        rjmp  end_loc@\
test_bit1@:\
        brtc  loc\
end_loc@:

/* compare Port bit with T, and branch to loc if are equal */
#define cpnePortBit(port,bit,loc)\ 
        sbic  port,bit\ 
        rjmp  test_bit1@\ 
test_bit0@:\
        brtc  loc\ 
        rjmp  end_loc@\ 
test_bit1@:\
        brts  loc\ 
end_loc@:

/* compare Register bit with T, and branch to loc if are not equal */
#define cpeqRegBit(reg,bit,loc)\ 
        sbrc  reg,bit\ 
        rjmp  test_bit1@\ 
test_bit0@:\
        brts  loc\ 
        rjmp  end_loc@\ 
test_bit1@:\
        brtc  loc\ 
end_loc@:

/* compare Register bit with T, and branch to @2 if are equal */
#define cpneRegBit(reg,bit,loc)\ 
        sbrc  reg,bit\ 
        rjmp  test_bit1@\ 
test_bit0@:\
        brtc  loc\ 
        rjmp  end_loc@\ 
test_bit1@:\
        brts  loc\ 
end_loc@:


/***************************************************************************/

#define ldix(val)/* Load the X-pointer with a 16 bit value */\ 
    ldi     xl,low(val)\ 
    ldi     xh,high(val)\ 


#define ldiy(val)/* Load the Y-pointer with a 16 bit value */\ 
    ldi     yl,low(val)\ 
    ldi     yh,high(val)\ 


#define ldiz(val)/* Load the Z-pointer with a 16 bit value */\ 
    ldi     zl,low(val)\ 
    ldi     zh,high(val)\ 



/***************************************************************************/

#define clrx/* Clear the X-pointer */\ 
    clr     xh\ 
    clr     xl\ 


#define clry/* Clear the Y-pointer */\ 
    clr     yh\ 
    clr     yl\ 


#define clrz/* Clear the Z-pointer */\ 
    clr     zh\ 
    clr     zl\ 



/***************************************************************************/

#define incx/* Increment the X-pointer */\ 
    adiw    XL,1\ 


#define incy/* Increment the X-pointer */\ 
    adiw    YL,1\ 


#define incz/* Increment the X-pointer */\ 
    adiw    ZL,1\ 



/****************************************************************************/

#define addix(val)\ 
    adiw    xl,val\ 

    
#define addiy(val)\ 
    adiw    yl,val\ 

    
#define addiz(val)\ 
    adiw    zl,val\ 



/****************************************************************************/

#define subix(val)\ 
    sbiw    xl,val\ 

    
#define subiy(val)\ 
    sbiw    yl,val\ 

    
#define subiz(val)\ 
    sbiw    zl,val\ 


#endif
