#ifndef __BT_H__
#define __BT_H__

void btInit();
void btRegister();
void btSetPairing();
void btSendData(uint8_t* data,int len);

#endif