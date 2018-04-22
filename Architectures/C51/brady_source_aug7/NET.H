//-----------------------------------------------------------------------------
// NET.H
//
//-----------------------------------------------------------------------------


typedef unsigned char  				UCHAR;
typedef unsigned int 				UINT;
typedef unsigned long				ULONG;


// Constants
#define TRUE      					1
#define ON								1
#define OK                    	1
#define FALSE     					0
#define OFF								0
#define MATCH							0
#define RET		   					0x0D
#define LF								0x0A
#define SPACE		 					0x20

// Port numbers
#define ECHO_PORT						7
#define DAYTIME_PORT					13
#define CHARGEN_PORT					19
#define TIME_PORT						37
#define HTTP_PORT  					80


// Event word bits
#define EVENT_ETH_ARRIVED			0x0001
#define EVENT_AGE_ARP_CACHE		0x0002
#define EVENT_TCP_RETRANSMIT		0x0004
#define EVENT_TCP_INACTIVITY		0x0008
#define EVENT_ARP_RETRANSMIT		0x0010
#define EVENT_READ_ANALOG			0x0020
#define EVENT_RS232_ARRIVED		0x0040


// Type number field in Ethernet frame
#define IP_PACKET             	0x0800
#define ARP_PACKET            	0x0806
#define RARP_PACKET					0x8035

// Protocol identifier field in IP datagram
#define ICMP_TYPE             	1
#define IGMP_TYPE						2
#define TCP_TYPE              	6
#define UDP_TYPE              	17

// Message type field in ARP messages 
#define ARP_REQUEST           	1
#define ARP_RESPONSE          	2
#define RARP_REQUEST		        	3
#define RARP_RESPONSE         	4

// Hardware type field in ARP message
#define DIX_ETHERNET          	1
#define IEEE_ETHERNET         	6



typedef struct
{
   ULONG ipaddr;
   UCHAR hwaddr[6];
   UCHAR timer; 
} ARP_CACHE;


typedef struct
{
	UCHAR xdata * buf;
	ULONG ipaddr;
	UCHAR proto_id;
	UINT  len;
	UCHAR timer;
} WAIT;


typedef struct
{
	UINT  hardware_type; 
   UINT  protocol_type;           
   UCHAR hwaddr_len;
   UCHAR ipaddr_len;               
   UINT  message_type;
   UCHAR source_hwaddr[6];              
   ULONG source_ipaddr;
   UCHAR dest_hwaddr[6];    
   ULONG dest_ipaddr;
} ARP_HEADER;


typedef struct
{
  UCHAR dest_hwaddr[6];
  UCHAR source_hwaddr[6];
  UINT  frame_type;
} ETH_HEADER;


typedef struct
{
   UCHAR ver_len;
   UCHAR type_of_service;
   UINT  total_length;
   UINT  identifier;
   UINT  fragment_info;
   UCHAR time_to_live;
   UCHAR protocol_id;
   UINT  header_cksum;
   ULONG source_ipaddr;
   ULONG dest_ipaddr;
} IP_HEADER;


typedef struct
{
   UCHAR msg_type;
   UCHAR msg_code;
   UINT  checksum;
   UINT  identifier;
   UINT  sequence;
	UCHAR echo_data;
} PING_HEADER;


typedef struct
{
   UCHAR msg_type;
   UCHAR msg_code;
   UINT  checksum;
   ULONG msg_data;
   UCHAR echo_data;
} ICMP_ERR_HEADER;


typedef struct 
{
   UINT  source_port;
   UINT  dest_port;
   UINT  length;
   UINT  checksum;
	UCHAR msg_data;
} UDP_HEADER;


typedef struct
{
   UINT  source_port;
   UINT  dest_port;
   ULONG sequence;
   ULONG ack_number;
   UINT  flags;
   UINT  window;
   UINT  checksum;
   UINT  urgent_ptr;
	UCHAR options;
} TCP_HEADER;


typedef struct
{
  ULONG ipaddr;
  UINT  port;
  ULONG his_sequence;
  ULONG my_sequence;
  ULONG old_sequence;
  ULONG his_ack;
  UCHAR timer;
  UCHAR inactivity;	 
  UCHAR state;
  char  query[20];
} CONNECTION;


 
