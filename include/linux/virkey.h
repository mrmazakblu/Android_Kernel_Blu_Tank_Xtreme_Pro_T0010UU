#ifndef __VIRKEY_H__
#define __VIRKEY_H__
void virkey_register(int keycode);
void virkey_arr_register(int *keycode,int size);
void virkey_report(int keycode,int value,bool sync);
#endif
