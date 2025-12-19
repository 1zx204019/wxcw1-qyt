#ifndef _KEY_H_
#define _KEY_H_

void key_init(void);
void key_scan(void);
unsigned char Key_GetValue(void); 
extern unsigned char key1_pressed;
extern unsigned char key2_pressed;
extern unsigned char key3_pressed;
extern unsigned char key4_pressed;
#endif 
