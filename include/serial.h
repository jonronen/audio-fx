#ifndef __serial_h__
#define __serial_h__


void serial_puts(const char *s);
void serial_init(void);
int  serial_getc(void);
void serial_putc(const char c);
void serial_puthex(unsigned int c);
int  serial_tstc (void);
void serial_clear_error(void);
void serial_flush(void);


#endif /* __serial_h__ */

