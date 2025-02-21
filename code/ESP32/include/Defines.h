
#pragma once
#define TAG "HydroFloat"

#define WATCHDOG_TIMER 600000 // time in ms to trigger the watchdog
#define EEPROM_SIZE 512
#define AP_BLINK_RATE 600
#define NC_BLINK_RATE 100
#define DEFAULT_AP_PASSWORD "12345678"

#define ADC_Resolution 4095.0
#define SAMPLESIZE 20

#define ASYNC_WEBSERVER_PORT 80
#define WSOCKET_LOG_PORT 7668
#define WSOCKET_HOME_PORT 7669
#define DNS_PORT 53

#define stopLevel_default 20
#define startLeadLevel_default 40
#define startLagLevel_default 60
#define overflowLevel_default 80