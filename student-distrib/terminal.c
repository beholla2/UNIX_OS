/* *********************************************************
# FILE NAME: terminal.c
* PURPOSE: initalizes terminal
# AUTHOR: Queeblo OS
* MODIFIED: 11/02/2014
********************************************************* */

#include "terminal.h"
#include "keyboard.h"
#include "lib.h"
#include "page.h"
#include "sched.h"

#define VIDEO 					0X000B8000	/* AW address of video memory (found in lib.c) */
#define NUM_COLS 80
#define NUM_ROWS 25
#define ATTRIB 0x7

int virtual_x[3];
int virtual_y[3];
int active_terminal;
int display_terminal;

int term_mem[NUM_TERMINALS] = {0x0B9000, 0x0BA000, 0x0BB000};

/* int term_open(void) 
 * INPUT: none
 * OUTPUT: returns 0 on success
 * DESCRIPTION: initializes the keyboard, clears the terminal
 */
int term_open(void) 
{
	int i;
	keyboard_init();
	term_clear();
	//clears virtual screen positions
	for (i=0; i<NUM_TERMINALS; i++) {
		virtual_x[i] = 0;
		virtual_y[i] = 0;
	}

	return 0;
}

/* int term_close(void) 
 * INPUT: none
 * OUTPUT: returns 0 on success
 * DESCRIPTION: disables keyboard
 */
int term_close(void) 
{
	keyboard_disable();
	return 0;
}

/* int term_read(void* buf, int nbytes)
 * INPUT: buf- buffer to hold read data, nbytes- bytes to read from buf
 * OUTPUT: returns 0 on success, returns -1 if null buffer passed in
 * DESCRIPTION: waits until the kb buf is ready to read, then reads nbytes from it
 */
int term_read(uint8_t* fname, void* buf, int nbytes)
{
	//need 3 kb buffers, read from active buf
	int i;
	int num_bytes_read = 0;
	if (buf == 0) {
		return -1;
	}
	/* wait until ready to read from buf */
	while (!ready_to_read[display_terminal]) {}
	/* read only kb valid data if requested bytes is larger*/
	if (nbytes > kb_buf_index[display_terminal]) {
		memcpy(buf, kb_buf[display_terminal], kb_buf_index[display_terminal]);
		num_bytes_read = kb_buf_index[display_terminal];
		for (i=kb_buf_index[display_terminal]; i<nbytes; i++) {
			*((uint8_t*)buf + i) = 0;
		}
	}
	/* otherwise, read the specified amount of bytes*/
	else {
		memcpy(buf, kb_buf[display_terminal], nbytes);
	}
	/* clears kb buffer */
	clear_keyboard_buf();

	return num_bytes_read;
}

/* int term_write(void* buf, int nbytes)
 * INPUT: buf- buffer with data to be written to terminal, nbytes- bytes to read from buf
 * OUTPUT: returns 0 on success, returns -1 if buf null or nbytes too large
 * DESCRIPTION: writes nbytes from the given buffer to the terminal
 */
int term_write(uint8_t* fname, void* buf, int nbytes)
{
	if (buf == 0 /*|| nbytes > MAX_KB_BUF*/) {
		return -1;
	}
	int i;
	//write to video memory
	if (pq_get_active_term() == display_terminal) {
		for (i=0; i<nbytes; i++) {
			putc( *((uint8_t*)buf + i) );
		}
		update_cursor(screen_x, screen_y);
	}
	//write to terminal memory
	else {
		for (i=0; i<nbytes; i++) {
			term_putc( *((uint8_t*)buf + i) );
		}
	}
	
	return 0;
}

/* void update_cursor(int x_pos, int y_pos)
 * INPUT: x_pos: x position of kernel, y_pos: y position of kernel
 * OUTPUT: none
 * DESCRIPTION: sets the blinking cursor to the given x,y coordinates
 */
void update_cursor(int x_pos, int y_pos)
{
	uint16_t position = y_pos*NUM_COLS + x_pos;
	outb(0x0F, 0x3D4);
	outb((uint8_t)(position&0xFF), 0x3D5);
	outb(0x0E, 0x3D4);
	outb((uint8_t)((position>>8)&0xFF), 0x3D5);
}

/* void term_clear(void)
 * INPUT: none
 * OUTPUT: none
 * DESCRIPTION: clears the terminal, sets the cursor to the top
 */
void term_clear(void)
{
	clear();
	screen_x = 0;
	screen_y = 0;
	update_cursor(0, 0);
}

/* void term_putc(uint8_t c)
 * INPUTS: 	c - character to put to screen
 * OUTPUT: none
 * DESCRIPTION: writes to terminal video memory instead of display memory
 */
void term_putc(uint8_t c)
{
	int active_term = pq_get_active_term();
    if(c == '\n' || c == '\r') {
        virtual_y[active_term]++;
        virtual_x[active_term]=0;
        if (virtual_y[active_term] >= NUM_ROWS) {	/* AW changed == to >= */
			term_scroll_down();
			virtual_y[active_term] = NUM_ROWS - 1;
		}
    } else {
        *(uint8_t *)(term_mem[active_term] + ((NUM_COLS*virtual_y[active_term] + virtual_x[active_term]) << 1)) = c;
        *(uint8_t *)(term_mem[active_term] + ((NUM_COLS*virtual_y[active_term] + virtual_x[active_term]) << 1) + 1) = ATTRIB;
        virtual_x[active_term]++;
        if (virtual_x[active_term] >= NUM_COLS) {	/* AW changed == to >= */
    		virtual_y[active_term]++;
    		if (virtual_y[active_term] >= NUM_ROWS) {
    			term_scroll_down();
    			virtual_y[active_term] = NUM_ROWS - 1;
    		}
    		virtual_x[active_term]=0;
    	}
    }
}


/* void term_scroll_down()
 * INPUTS: none
 * OUTPUTS: none
 * DESCRIPTION: scrolls down the terminal. 
 */
void term_scroll_down() 
{
	int active_term = pq_get_active_term();
	uint32_t x, y;
	for (y=0; y<NUM_ROWS-1; y++) {
		for (x=0; x<NUM_COLS; x++) {
			*(uint8_t *)(term_mem[active_term] + ((NUM_COLS*y + x) << 1)) = *(uint8_t *)(term_mem[active_term] + ((NUM_COLS*(y+1) + x) << 1));
		}
	}
	for (x=0; x<NUM_COLS; x++) {
		*(uint8_t *)(term_mem[active_term] + ((NUM_COLS*(NUM_ROWS-1) + x) << 1)) = ' ';
	}
	
}


/* change_display_term(uint8_t next_terminal)
 * INPUTS:			next_terminal: the number of the terminal to switch to (0 thru NUM_TERMINALS - 1)
 * RETURN VALUE:	0 for success, -1 for failure
 * PURPOSE: 		Change the display to a different terminal
 */
int change_display_term(uint8_t next_terminal)
{
	/* AW get address of current terminal's backing page */
	uint8_t* curr_backing_page = get_backing_page(display_terminal);

	/* AW save video memory to current terminal's backing page */
	copy_4kb_page((uint8_t*)VIDEO, curr_backing_page);

	/* AW copy next terminal's backing page to video memory */
	uint8_t* next_backing_page = get_backing_page(next_terminal);
	copy_4kb_page(next_backing_page, (uint8_t*)VIDEO);

	/* AW update display_terminal global variable */
	display_terminal = next_terminal;

	return 0;
}
