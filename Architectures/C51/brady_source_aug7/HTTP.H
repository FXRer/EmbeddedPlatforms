//-----------------------------------------------------------------------------
// HTTP.H
//
//-----------------------------------------------------------------------------


#define NONE						0
#define GET_PAGE          		1
#define GET_JPEG             	2
#define POST_PAGE					3

void init_http(void);
UINT http_server(UCHAR xdata *, UINT, UCHAR, UCHAR);





