// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------
#include "udp.h"
#include "tcpGpio.h"
#include "tcpDebug.h"

// ----------------------------------------------------------------------------
#define SSID "wifi_name"
#define PASSWORD "wifi_pass"

// ----------------------------------------------------------------------------
static os_timer_t loopTimer;

/*---------------------------------------------------------------------------*/
void loopTimer_cb(void *arg)
{
  // If hardware serial RX buffer have something, forward to TCP port
  while(Serial.available())
  {
    int32_t ch = Serial.read();
    tcpDebug_sendChar(ch);
  }

  // If TCP RX buffer have something, forward to hardware serial
  while(tcpDebug_rxHaveData())
  {
    int32_t ch = tcpDebug_getChar();
    Serial.write(ch);
  }

  // Very basic GPIO handler
  if(tcpGpio_readState())
  {
    digitalWrite(2, HIGH);
  }
  else
  {
    digitalWrite(2, LOW);
  }

  // Re-arm timer
  os_timer_arm(&loopTimer, 1, 0);
}

// ----------------------------------------------------------------------------
void wifiClient_connectToKnownHost(char ssid[32], char password[64])
{
  struct station_config stationConf;
  wifi_station_disconnect();
  wifi_set_opmode(STATION_MODE);
  os_memcpy(&stationConf.ssid, ssid, 32);
  os_memcpy(&stationConf.password, password, 64);
  wifi_station_set_config_current(&stationConf);
  wifi_station_connect();
}

// ----------------------------------------------------------------------------
void setup()
{
  // Initialise hardware uart
  Serial.begin(115200);
  Serial.setRxBufferSize(8192);
  Serial.setDebugOutput(false);

  // Set GPIO2 as output for remote reset
  pinMode(2, OUTPUT);

  // Connect to a 'compile time known' wireless network
  wifiClient_connectToKnownHost(SSID, PASSWORD);

  // Initialise UDP broadcast library to find out IP address of the device
  // after boot up.
  udp_init();

  // Initilaise the TCP gpio library to control single output pin to reset
  // remote device.
  tcpGpio_init();

  // Initlaise the TCP debug library to transfer serial port messages to the
  // host computer and vice versa.
  tcpDebug_init();

  // Fire the loop timer
  os_timer_disarm(&loopTimer);
  os_timer_setfn(&loopTimer, (os_timer_func_t *)loopTimer_cb, NULL);
  os_timer_arm(&loopTimer, 1, 0);
}

// ----------------------------------------------------------------------------
void loop()
{
  delay(1000);
}
