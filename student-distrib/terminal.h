#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"


int term_open(void);
int term_close(void);
int term_read(uint8_t* fname, void* buf, int nbytes);
int term_write(uint8_t* fname, void* buf, int nbytes);
void update_cursor(int x_pos, int y_pos);
void term_clear(void);


void term_putc(uint8_t c);
void term_scroll_down();

/* change_display_term(uint8_t next_terminal)
 * INPUTS:			next_terminal: the number of the terminal to switch to (0 thru NUM_TERMINALS - 1)
 * RETURN VALUE:	0 for success, -1 for failure
 * PURPOSE: 		Change the display to a different terminal
 */ 
int change_display_term(uint8_t next_terminal);

extern int active_terminal;
extern int display_terminal;

//virtual screen positions
extern int virtual_x[3];
extern int virtual_y[3];


#endif /* _TERMINAL_H */
