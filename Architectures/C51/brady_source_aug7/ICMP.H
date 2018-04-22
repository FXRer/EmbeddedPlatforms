//-----------------------------------------------------------------------------
// ICMP.H
//
//-----------------------------------------------------------------------------

void ping_send(UCHAR xdata *, ULONG, UINT);
void dest_unreach_send(UCHAR xdata *, ULONG);
void icmp_rcve(UCHAR xdata *, UINT);

