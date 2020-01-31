

#define MAXBOARD 8
#define MAXBUF 64

typedef struct
{
//start of struct cam_param
//values for communication with correct board
  int    boardnr;
  HANDLE hdriver;
  HANDLE headevent;

  int    bufalloc[MAXBUF];
  void   *mapadr[MAXBUF];              //mapped addresses
  int    mapsize[MAXBUF];              //mapped size
  int    mapoffset[MAXBUF];            //mapped offset
  int    mapcount[MAXBUF];             //mapped count
  HANDLE bufevent[MAXBUF];
  BOOLEAN event_internal[MAXBUF];

  HINSTANCE pfcamlib;
}PCC_DEVICE_ENTRY;


typedef struct
{
 int bufnr;
 unsigned int BufferStatus;
 unsigned int counter; //not used 
 HANDLE hBufferEvent;
}PCC_Buflist;
