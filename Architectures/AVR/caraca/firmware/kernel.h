#ifndef _KERNEL_H
#define _KERNEL_H

#define IsrFlushSignal(Mask,Thr,Tmp)\
            ldd     Tmp,Thr+NO_OF_THREAD\
            cbr     Tmp,Mask\
            std     Thr+NO_OF_THREAD,Tmp\


#define FlushSignal(Mask,Thr,Tmp)\
            cli\
            IsrFlushSignal(Mask,Thr,Tmp)\
            sei


#define IsrSignal(Mask,Thr,Tmp) \
            ldd   Tmp,Thr+NO_OF_THREAD  /* load the received signal mask in Tmp */\
            or    Tmp,Mask\
            std   Thr+NO_OF_THREAD,Tmp  /* update received signal mask */\
            ld    Mask,Thr              /* load the wait signal mask in Mask */\
            and   Mask,Tmp              /* test if the dest thread was waiting for this signals */\
            breq  sig_nowakeup@         /* Wake up the destination thread */\
            clr   Mask\
            st    Thr,Mask              /* clear wait mask to make the thread ready to run */\
            swap  KernelStatus\
            inc   KernelStatus          /* increment the number of ready threads */\
            swap  KernelStatus\
sig_nowakeup@:


    extern  _IsrContextSwitch

/* Put this at end of ISR instead of 
 *          out     SREG,SaveStatus
 *          reti
 * instructions to perform a reschedule */
#define IsrSchedule\
            ldi     IntTmp,(1<<7)\
            or      SaveStatus,IntTmp   /* set I flag in SREG to emulate reti */\
            push    r0\
            mov     r0,SaveStatus\
            rjmp    _IsrContextSwitch\

#endif
