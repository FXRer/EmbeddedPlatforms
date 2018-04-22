//-----------------------------------------------------------------------------
// Copyright (c) 2002 Jim Brady
// Do not use commercially without author's permission
// Last revised August 2002
// Net HTTP.C
//
// This module is the Web Server
// It currently serves a html text page and a jpeg image, or handles
// a POST message to turn an LED on or off.
// The HTTP protocol specification is at http://www.w3.org/Protocols/ 
//-----------------------------------------------------------------------------
#pragma SMALL ORDER NOPRINT OPTIMIZE(2)
#include <string.h>
#include <stdlib.h>
#include <ctype.h>		// toupper
#include "C8051f.h"
#include "net.h"
#include "serial.h"  
#include "cksum.h"
#include "analog.h"
#include "ip.h"
#include "tcp.h"
#include "http.h"


// These structures keep track of connection information
extern CONNECTION xdata conxn[];
 
extern ULONG code my_ipaddr;
extern char xdata text[];
extern UINT idata cpu_temperature;
extern UINT idata air_temperature;
extern UINT idata cpu_voltage;
extern char code html_header[];
extern char code web_page[];
extern char code jpeg_header[];
extern UCHAR code photo1_jpeg[];
extern UCHAR idata rcve_buf_allocated;
extern UCHAR idata debug;

 
sbit CONTROL_LED = P0^2;

// For timing tests
sbit DEBUG6 = P0^6;
sbit DEBUG7 = P0^7;


void init_http(void)
{
   CONTROL_LED = 0;
}



//------------------------------------------------------------------------
// This function is the standard string search. The Keil library
// does not provide it.  It looks for one string in another string
// and returns a pointer to it if found, otherwise returns NULL. 
//------------------------------------------------------------------------
char * strstr(char * haystack, char * needle)
{
	char *ptr1, *ptr2;
	
	// Protect against NULL pointer
	if (*needle == 0) return(haystack);
	for( ; *haystack; haystack++ )
	{
		// Look for needle in haystack.  If there is a
      // match then this will continue all the way
      // until ptr1 reaches the NULL at the end of needle 
		for(ptr1 = needle, ptr2 = haystack; *ptr1 && (*ptr1 == *ptr2); ++ptr1, ++ptr2);
							
		// If there is a match then return pointer to needle in haystack
		if(*ptr1 == 0) return(haystack);	
	}
	return NULL;			// no matching string found
}



//------------------------------------------------------------------------
// This sends an TCP segment to the ip layer.  The segment is 
// is normally either a web page or a graphic.
// See "TCP/IP Illustrated, Volume 1" Sect 17.3
//------------------------------------------------------------------------
void http_send(UCHAR xdata * outbuf, UINT len, UCHAR nr)
{
   TCP_HEADER xdata * tcp;
   IP_HEADER xdata * ip;
   ULONG idata sum;
   UINT idata result;
          
   // Fill in TCP segment header
   tcp = (TCP_HEADER xdata *)(outbuf + 34);
   ip = (IP_HEADER xdata *)(outbuf + 14);

   tcp->source_port = HTTP_PORT;
   tcp->dest_port = conxn[nr].port;
   tcp->sequence = conxn[nr].my_sequence;
   tcp->ack_number = conxn[nr].his_sequence;
      
	// Header is always 20 bytes long
   tcp->flags = 0x5000 | FLG_ACK | FLG_PSH;
   tcp->window = 1024;
   tcp->checksum = 0;
   tcp->urgent_ptr = 0;
   
   // Compute checksum including 12 bytes of pseudoheader
	// Must pre-fill 2 items in ip header to do this
	ip->dest_ipaddr = conxn[nr].ipaddr;
	ip->source_ipaddr = my_ipaddr;
		
	// Sum source_ipaddr, dest_ipaddr, and entire TCP message 
	sum = (ULONG)cksum(outbuf + 26, 8 + len);
				
	// Add in the rest of pseudoheader which is
	// protocol id and TCP segment length
	sum += (ULONG)0x0006;
	sum += (ULONG)len;

	// In case there was a carry, add it back around
	result = (UINT)(sum + (sum >> 16));
	tcp->checksum = ~result;
   
   if (debug) serial_send("TCP: Sending msg to IP layer\r");
	ip_send(outbuf, conxn[nr].ipaddr, TCP_TYPE, len);

   // (Re)start TCP retransmit timer
   conxn[nr].timer = TCP_TIMEOUT;
}



//------------------------------------------------------------------------
// This searches a web page looking for a specified tag.  If found,
// it replaces the tag with the text in * sub.  Tags are fixed length -
// The first 4 chars of the tag is always "TAG:" and the rest of it
// is always 4 chars for a total of 8 chars. 
//------------------------------------------------------------------------
void replace_tag(UCHAR xdata * start, char * tag, char * sub) 
{ 
   UCHAR idata i, flg;
   UCHAR xdata * ptr;
   
   // Find tag.  If not found - just return
   ptr = strstr(start, tag);
	if (ptr == NULL) return;
   
	flg = TRUE;

   // Replace the 8 char tag with the substitute text
	// Pad on the right with spaces
   for (i=0; i < 8; i++)
	{
   	if (sub[i] == 0) flg = FALSE;
   	if (flg) ptr[i] = sub[i]; else ptr[i] = SPACE;
	}
}



//------------------------------------------------------------------------
//	This serves up either a HTML page, a JPEG image, or controls an 
// LED,  depending what it gets from the browser.  The received header
// must contain the word "GET" or "POST" to be considered a valid request.
// With HTTP 1.1 where the connection is left open, the header I send
// should include content length. With HTTP 1.0 you can just close the
// connection after sending the page and the browser knows its done. 
//
// The HTTP protocol specification is at http://www.w3.org/Protocols/ 
//------------------------------------------------------------------------
UINT http_server(UCHAR xdata * inbuf, UINT header_len, UCHAR nr, UCHAR resend)
{
	UINT idata body_len, hhdr_len, jhdr_len, page_len, jpeg_len;
	UINT idata sent, remaining;
	UCHAR xdata * outbuf;
	UCHAR xdata * ptr;
	UCHAR xdata * tcp_data;
	UCHAR idata request;
   static UCHAR idata post_flg = FALSE;
		    	
	// Make sure this is a valid connection
	if (nr == NO_CONNECTION) return 0;
	
	// Compute start of TCP data
   
   // Save first 20 chars and seq number just in case
   // we need to re-generate page
   // TODO: if post, then save switch state infomation
   if (!resend)
   {
      tcp_data = inbuf + 34 + header_len;
      memcpy(conxn[nr].query, tcp_data, 20);
      conxn[nr].old_sequence = conxn[nr].my_sequence;
   }
   // If this is a resend, set sequence number to what it was
   // the last time we sent this
   else
   {
      tcp_data = inbuf;
      conxn[nr].my_sequence = conxn[nr].old_sequence;   
   }
   
   // Start off with no request
   request = NONE;
      
	// TODO: Calling strstr() on a large buffer takes a lot of time
   // so perhaps we could speed things up by limiting the search
   // range to the portion of the buffer where the item is expected
   // to be found
	
	// If it is a POST, then set a flag to start looking for the post
	// data of interest, which is the string "switch=".  It may arrive
	// in a later segment (Netscape seems to split up the POST message)
   if (strstr(tcp_data, "POST") != NULL) post_flg = TRUE; 
	   
   // See if this is a GET message
   else if (strstr(tcp_data, "GET") != NULL)
   {
      post_flg = FALSE;
      if (strstr(tcp_data, "photo1") != NULL) request = GET_JPEG;
      else if (strstr(tcp_data, "index") != NULL) request = GET_PAGE;
      else if (strstr(tcp_data, "/ ") != NULL) request = GET_PAGE;
   }
   
   // If POST flag is "armed" then look for the "switch=" string
   // and if found, turn the LED on or off according to whether 
   // the browser is sending a 1 or a 0.
   if (post_flg)
   {
      ptr = strstr(tcp_data, "switch=");
      if (ptr != NULL)
      {
         // Move to space after equals sign
         // Set control indicator accordingly
         post_flg = FALSE;
         request = POST_PAGE;
         ptr += 7;
         if (*ptr == '1') CONTROL_LED = ON;   
         else if (*ptr == '0') CONTROL_LED = OFF;
      }
   }

   if ((request == GET_PAGE) || (request == POST_PAGE))
   {
      // Figure out sizes
      hhdr_len = strlen(html_header);
      page_len = strlen(web_page);
      body_len = hhdr_len + page_len;
      
      // Free memory holding received message.  The message from the
      // browser can be 500+ bytes long so this is a significant 
      // chunk out of the available malloc space of 1500 bytes
      if (!resend) {free(inbuf); rcve_buf_allocated = FALSE;}

      // Allocate memory for entire outgoing message including
      // 14 byte Ethernet + 20 byte IP + 20 byte TCP headers
      // Allow 1 byte for NULL char at the end
      outbuf = (UCHAR xdata *)malloc(54 + body_len + 1);
      if (outbuf == NULL)
      {
         if (debug) serial_send("TCP: Oops, out of memory\r");
         return 0;
      }
      
      // Copy page data.  This moves data from flash into RAM.  It is
      // an undesirable process, but must get data into RAM to replace
      // tags 
		memcpy(outbuf + 54, html_header, hhdr_len);
		memcpy(outbuf + 54 + hhdr_len, web_page, page_len);
   	   	   	
   	outbuf[54 + body_len] = 0;		// Append NULL 
   
      // Replace length tag with actual value
      itoa(page_len, text, 10);
		replace_tag(outbuf + 54, "TAG:LEN1", text);
		
		// Replace CPU temperature tag with actual value
		itoa(cpu_temperature, text, 10);
		strcat(text, " F");
		replace_tag(outbuf + 54, "TAG:TMP1", text);

		// Replace air temperature tag with actual value
		itoa(air_temperature, text, 10);
		strcat(text, " F");
		replace_tag(outbuf + 54, "TAG:TMP2", text);

		// Replace CPU voltage tag with actual value X 100
		// Insert decimal point between first and second digits
		itoa(cpu_voltage, text + 1, 10);
		text[0] = text[1];
		text[1] = '.';
		strcat(text, " V");
		replace_tag(outbuf + 54, "TAG:VOL1", text);
      
      // Insert the word CHECKED into the html so the browser will
		// check the correct LED state indicator box 
      if (CONTROL_LED == ON) replace_tag(outbuf + 54, "TAG:CHK1", "CHECKED");
      else replace_tag(outbuf + 54, "TAG:CHK2", "CHECKED");
      
      // Segment length = body_len + 20
      http_send(outbuf, body_len + 20, nr);
      conxn[nr].my_sequence += body_len;
   }
   else if (request == GET_JPEG)
   {
      // Ths browser has requested the jpeg image.  First figure out
      // sizes - cannot use sizeof() for jpeg here because it is in
      // another module.  Just directly specify length of it
      jhdr_len = strlen(jpeg_header);
      jpeg_len = 6194;
      body_len = jhdr_len + jpeg_len;
      
      // Free memory holding received message.  The message from the
      // browser can be 500+ bytes long so this is a significant 
      // chunk out of the available malloc space of 1500 bytes
      if (!resend) {free(inbuf); rcve_buf_allocated = FALSE;}

      // First send the header and enough of the jpeg to make 1000 bytes.
      // The value of 1000 is arbitrary, but must be stay under 1500. 
      if (body_len < 1000) remaining = body_len; else remaining = 1000; 
      outbuf = (UCHAR xdata *)malloc(54 + remaining + 1);
      if (outbuf == NULL)
      {
         if (debug) serial_send("TCP: Oops, out of memory\r");
         return 0;
      }
      
      memcpy(outbuf + 54, jpeg_header, jhdr_len);
		memcpy(outbuf + 54 + jhdr_len, photo1_jpeg, remaining - jhdr_len);
                  
      outbuf[54 + remaining] = 0;		// Append NULL

      // Replace jpeg length tag with actual value
      itoa(jpeg_len, text, 10);
		replace_tag(outbuf + 54, "TAG:LEN2", text);

      http_send(outbuf, remaining + 20, nr);
      sent = remaining - jhdr_len;
      conxn[nr].my_sequence += remaining;
     
      // Send the rest of the jpeg file in 1000 byte chunks.  This sends about
      // 6 segments of 1000 bytes back to back, but we should actually process
      // acks from the other end to make sure we are not sending more than the
      // other end can receive.  Most systems can handle 6K
      while (sent < jpeg_len)
      {
         remaining = jpeg_len - sent;
         if (remaining > 1000) remaining = 1000;
         outbuf = (UCHAR xdata *)malloc(54 + remaining + 1);
         if (outbuf == NULL)
         {
            if (debug) serial_send("TCP: Oops, out of memory\r");
            return 0;
         }
         
         memcpy(outbuf + 54, photo1_jpeg + sent, remaining);
                           
         outbuf[54 + remaining] = 0;		// Append NULL
         http_send(outbuf, remaining + 20, nr);
         sent += remaining;
         conxn[nr].my_sequence += remaining;
      }
   }
   else
   {
      // The incoming HTTP message did not warrant a response
      if (debug) serial_send("HTTP: No response sent\r");
      return 0;
   }
   
   // Return number of bytes sent, not including TCP header
	return(body_len);
}





