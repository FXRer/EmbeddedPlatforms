/***
* EEprom routines: taken from Atmel Appnotes and adapted to CARACA
***/
#include "config.h"
#include "vars.h"
#include "kernel.e"

        seg flash.code

/**
 * TODO: AT90S4433 trigger an interrupt when bEEWE cleared, so we can
 *       Wait() a signal instead of call Yield() in a loop
 **/

;***************************************************************************
;*
;* EEWriteByte( Addr, Data )
;*              Arg1  Arg2
;*
;* This subroutine waits until the EEPROM is ready to be programmed, then
;* programs the EEPROM with register variable "Arg2" at address "Arg1"
;*
;* Low Registers used: None
;* High Registers used: None
;*
;* NOTE: If an interrupt routine should call EEWrite we need to disable
;*       interrupts (cli) before to change EEAR!!
;*
;****************************************************************************
        seg removable flash.code.EEWriteByte
WriteWaitLoop1:
            rcall   Yield
public EEWriteByte:
            sbic    EECR,bEEWE      ;if EEWE not clear
            rjmp    WriteWaitLoop1  ; wait more
            out     EEAR,Arg1       ;output address 
            out     EEDR,Arg2       ;output data
            cli                     ;disable global interrupts
            sbi     EECR,bEEMWE     ;set master write enable, remove if AT90S1200 is used
            sbi     EECR,bEEWE      ;set EEPROM Write strobe
                                    ;This instruction takes 4 clock cycles since
                                    ;it halts the CPU for two clock cycles
            sei                     ;enable global interrupts
            ret


;***************************************************************************
;*
;* EEWriteWord( Addr,   Data )
;*              Arg1  Arg3:Arg4
;*
;* This subroutine waits until the EEPROM is ready to be programmed, then
;* programs the EEPROM with register variable "Arg3:Arg4" at address "Arg1"
;*
;* Low Registers used:None
;* High Registers used: Arg1
;* NOTE: If an interrupt routine should call EEWrite we need to disable
;*       interrupts (cli) before to change EEAR!!
;*
;****************************************************************************
        seg removable flash.code.EEWriteWord
WriteWaitLoop2:
            rcall   Yield
public EEWriteWord:
            sbic    EECR,bEEWE      ;if EEWE not clear
            rjmp    WriteWaitLoop2  ; wait more

            out     EEAR,Arg1       ;output address 
            out     EEDR,Arg4       ;output data LSB data byte
            cli                     ;disable global interrupts
            sbi     EECR,bEEMWE     ;set master write enable, remove if AT90S1200 is used
            sbi     EECR,bEEWE      ;set EEPROM Write strobe
                                    ;This instruction takes 4 clock cycles since
                                    ;it halts the CPU for two clock cycles
            sei                     ;enable global interrupts

            inc     Arg1
WriteWaitLoop3:
            rcall   Yield
            sbic    EECR,bEEWE      ;if EEWE not clear
            rjmp    WriteWaitLoop3  ; wait more

            out     EEAR,Arg1       ;output address 
            out     EEDR,Arg3       ;output MSB data byte
            cli                     ;disable global interrupts
            sbi     EECR,bEEMWE     ;set master write enable, remove if AT90S1200 is used
            sbi     EECR,bEEWE      ;set EEPROM Write strobe
                                    ;This instruction takes 4 clock cycles since
                                    ;it halts the CPU for two clock cycles
            sei                     ;enable global interrupts
            ret


;***************************************************************************
;*
;* Data = EEReadByte( Addr )
;* Res1               Arg1
;*
;* This subroutine waits until the EEPROM is ready to be programmed, then
;* reads the register variable "Res1" from address "Arg1"
;*
;* Number of words : 6 + return
;* Number of cycles : 9 + return (if EEPROM is ready)
;*
;***************************************************************************
        seg removable flash.code.EEReadByte
ReadWaitLoop1:
            rcall   Yield
public EEReadByte:
            sbic    EECR,bEEWE      ;if EEWE not clear
            rjmp    ReadWaitLoop1   ; wait more
            out     EEAR,Arg1       ;output address low byte
            sbi     EECR,bEERE      ;set EEPROM Read strobe
                                    ;This instruction takes 4 clock cycles since
                                    ;it halts the CPU for two clock cycles
            in      Res1,EEDR       ;get data
            ret
            ret

;***************************************************************************
;*
;*   Data    = EEReadWord( Addr )
;* Res1:Res2               Arg1
;*
;* This subroutine waits until the EEPROM is ready to be programmed, then
;* reads the register variable "Res1:Res2" from address "Arg1"
;*
;* NOTE: Return the WORD in Little endian format (LSB at low address)
;*
;***************************************************************************
        seg removable flash.code.EEReadWord
ReadWaitLoop2:
            rcall   Yield
public EEReadWord:
            sbic    EECR,bEEWE      ;if EEWE not clear
            rjmp    ReadWaitLoop2   ; wait more
            out     EEAR,Arg1       ;output address low byte
            sbi     EECR,bEERE      ;set EEPROM Read strobe
                                    ;This instruction takes 4 clock cycles since
                                    ;it halts the CPU for two clock cycles
            in      Res2,EEDR       ;get LSB data byte

            inc     Arg1
            out     EEAR,Arg1       ;output address low byte
            sbi     EECR,bEERE      ;set EEPROM Read strobe
                                    ;This instruction takes 4 clock cycles since
                                    ;it halts the CPU for two clock cycles
            in      Res1,EEDR       ;get MSB data byte

            ret
