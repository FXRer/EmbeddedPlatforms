//-----------------------------------------------------------------------------
// TCP.H
//
//-----------------------------------------------------------------------------

// TCP states
#define STATE_CLOSED					0
#define STATE_LISTEN					1
#define STATE_SYN_RCVD				2
#define STATE_ESTABLISHED			3
#define STATE_CLOSE_WAIT			4
#define STATE_LAST_ACK				5
#define STATE_FIN_WAIT_1			6
#define STATE_FIN_WAIT_2			7
#define STATE_CLOSING				8
#define STATE_TIME_WAIT				9


// TCP flag bits
#define FLG_FIN						0x0001
#define FLG_SYN						0x0002
#define FLG_RST						0x0004
#define FLG_PSH						0x0008
#define FLG_ACK						0x0010
#define FLG_URG						0x0020


// Miscellaneous
#define NO_CONNECTION  			5 
#define TCP_TIMEOUT				4		// = 2 seconds
#define INACTIVITY_TIME			30		// = 15 seconds

void tcp_send(UINT, UINT, UCHAR);
void tcp_rcve(UCHAR xdata *, UINT);
void tcp_retransmit(void);
void tcp_inactivity(void);
void init_tcp(void);



