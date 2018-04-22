//-----------------------------------------------------------------------------
// SERIAL.H
//
//-----------------------------------------------------------------------------

#define BUFSIZE 40

void init_serial(void);
void serial_send(char *);
void byte2text(UCHAR);
void wait_msec(UINT);
char * itoa(UINT, char *, UCHAR);

