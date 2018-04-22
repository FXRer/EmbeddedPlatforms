//-----------------------------------------------------------------------------
// ARP.H
//
//-----------------------------------------------------------------------------


// Allow up to 10 entries in ARP cache
#define CACHESIZE		10

// ARP cache entry expiration time = 20 minutes
#define CACHETIME		20		// 60 second intervals

// Allow 2 seconds to receive an ARP reply
#define ARP_TIMEOUT	 4		  // 0.5 second intervals

void init_arp(void);
UCHAR xdata * arp_resolve(ULONG);
void arp_rcve(UCHAR xdata *);
void age_arp_cache(void);
void arp_retransmit(void);



