#ifndef RFD900X_H
#define RFD900X_H

#include <Arduino.h>

// platformio doesn't seem to have F(), but has FPSTR and PSTR
#define F(string_literal) (FPSTR(PSTR(string_literal)))

// define where parameters are instead of putting randing numbers everywhere meaning nothing
#define  PARAM_FORMAT_STR   "S0"
#define  PARAM_NETID_STR    "S3"
#define  PARAM_MINFREQ_STR  "S8"
#define  PARAM_MAXFREQ_STR  "S9"
#define  PARAM_ENCRYPT_STR  "S15"
#define  PARAM_RCIN_STR     "S16"
#define  PARAM_RCOUT_STR    "S17"
#define  PARAM_STATLED_STR  "S21"
#define  PARAM_FRAMELEN_STR "S26"
#define  PARAM_AIR_SPEED_STR "S2"

#define TEST_NETID_STR "88"
#define TEST_MINFREQ_STR "902000"
#define TEST_MAXFREQ_STR "908000"

void r900x_initiate_serials(void);
void r900x_setup(bool reflash);
void r900x_attempt_factory_reset(void);
int r900x_getparams(String filename, bool factory_reset_first);
bool r900x_saveparams(String filename);
int r900x_savesingle_param_and_verify(String prefix, String ParamID, String ParamVAL); 
int r900x_savesingle_param_and_verify_more(String prefix, String ParamID, String ParamVAL, bool save_and_reboot); 
int r900x_readsingle_param(String prefix, String ParamID);
uint8_t r900x_rssi_percentage(uint8_t mav_rssi_109);
int r900x_readcachedparam(String filename, String ParameterName);

#endif