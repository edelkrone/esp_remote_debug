/*-----------------------------------------------------------------------------
/
/
/----------------------------------------------------------------------------*/
#include "tcpDebug.h"
#include "RingBuffer.h"

/*---------------------------------------------------------------------------*/
#define TCPDEBUG_BUFSIZE 8192

/*---------------------------------------------------------------------------*/
static uint16_t tcpPacketInd;
static uint8_t tcpDataSentFlag;
static struct espconn tcpServer;
static os_timer_t tcpDebugTimer_p;
static uint8_t tcpDebug_isConnected;
static uint8_t tcpPacket[TCPDEBUG_BUFSIZE];

/*---------------------------------------------------------------------------*/
static RingBuffer_t txRbuff;
static RingBuffer_t rxRbuff;
static uint8_t txRbuff_data[TCPDEBUG_BUFSIZE];
static uint8_t rxRbuff_data[TCPDEBUG_BUFSIZE];

/*---------------------------------------------------------------------------*/
static void ICACHE_FLASH_ATTR
tcpDebugTimer_cb(void *arg)
{
  if(tcpDebug_isConnected)
  {
    if(tcpDataSentFlag)
    {
      while(RingBuffer_GetCount(&txRbuff))
      {
        uint8_t ch = RingBuffer_Remove(&txRbuff);
        tcpPacket[tcpPacketInd] = ch;
        tcpPacketInd += 1;
        if(tcpPacketInd == 2920)
        {
          tcpDataSentFlag = 0;
          while(espconn_send(&tcpServer,tcpPacket,2920) == 0);
          tcpPacketInd = 0;
          break;
        }
      }

      if(tcpPacketInd)
      {
        tcpDataSentFlag = 0;
        while(espconn_send(&tcpServer,tcpPacket,tcpPacketInd) == 0);
        tcpPacketInd = 0;
      }
    }
  }
  os_timer_arm(&tcpDebugTimer_p, 10, 0);
}

/*---------------------------------------------------------------------------*/
static void ICACHE_FLASH_ATTR
shell_tcp_disconcb(void *arg)
{
  struct espconn* pespconn = (struct espconn*) arg;

  tcpDataSentFlag = 1;
  tcpDebug_isConnected = 0;
  os_timer_disarm(&tcpDebugTimer_p);
  espconn_disconnect(pespconn);
}

/*---------------------------------------------------------------------------*/
static void ICACHE_FLASH_ATTR
shell_tcp_recvcb(void* arg, char* data, uint16_t length)
{
  struct espconn* pespconn = (struct espconn*) arg;

  uint16_t i;
  for(i=0;i<length;i++)
  {
    if(!RingBuffer_IsFull(&rxRbuff))
    {
      RingBuffer_Insert(&rxRbuff,data[i]);
    }
  }
}

/*---------------------------------------------------------------------------*/
static void ICACHE_FLASH_ATTR
tcpserver_connectcb(void *arg)
{
  struct espconn* pespconn = (struct espconn*) arg;

  tcpDataSentFlag = 1;
  tcpDebug_isConnected = 1;

  os_timer_disarm(&tcpDebugTimer_p);
  os_timer_setfn(&tcpDebugTimer_p, (os_timer_func_t *)tcpDebugTimer_cb, NULL);
  os_timer_arm(&tcpDebugTimer_p, 10, 0);
}

/*---------------------------------------------------------------------------*/
static void ICACHE_FLASH_ATTR
tcpserver_sentcb(void *arg)
{
  struct espconn* pespconn = (struct espconn*) arg;

  tcpDataSentFlag = 1;
}

/*---------------------------------------------------------------------------*/
uint8_t
tcpDebug_rxHaveData()
{
  return (RingBuffer_GetCount(&rxRbuff) != 0);
}

/*---------------------------------------------------------------------------*/
int32_t
tcpDebug_getChar()
{
  if(tcpDebug_rxHaveData())
  {
    uint8_t ch = RingBuffer_Remove(&rxRbuff);
    return ch;
  }
  else
  {
    return -1;
  }
}

/*---------------------------------------------------------------------------*/
void ICACHE_FLASH_ATTR
tcpDebug_init()
{
  tcpPacketInd = 0;
  tcpDataSentFlag = 1;
  tcpDebug_isConnected = 0;
  RingBuffer_InitBuffer(&txRbuff, txRbuff_data, sizeof(txRbuff_data));
  RingBuffer_InitBuffer(&rxRbuff, rxRbuff_data, sizeof(rxRbuff_data));

  tcpServer.type = ESPCONN_TCP;
  tcpServer.state = ESPCONN_NONE;
  tcpServer.proto.tcp = (esp_tcp *)calloc(sizeof(esp_tcp),1);
  tcpServer.proto.tcp->local_port = TCPDEBUG_PORT;

  espconn_regist_sentcb(&tcpServer, tcpserver_sentcb);
  espconn_regist_recvcb(&tcpServer, shell_tcp_recvcb);
  espconn_regist_disconcb(&tcpServer, shell_tcp_disconcb);
  espconn_regist_connectcb(&tcpServer, tcpserver_connectcb);

  espconn_accept(&tcpServer);
  espconn_regist_time(&tcpServer, 12 * 3600, 0);
}

/*---------------------------------------------------------------------------*/
void
tcpDebug_sendChar(uint8_t ch)
{
  if(!RingBuffer_IsFull(&txRbuff))
  {
    RingBuffer_Insert(&txRbuff,ch);
  }
}
