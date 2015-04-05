/* *********************************************************
# FILE NAME: keyboard.c
* PURPOSE: keyboard handler and kb buffer functions
# AUTHOR: Queeblo OS
* MODIFIED: 11/02/2014
********************************************************* */

#include "keyboard.h"
#include "terminal.h"
#include "i8259.h"
#include "lib.h"

/* flags set to 1 if the key is pressed */
static int alt_on;
static int caps_on;
static int ctrl_on;
static int shift_on;
/* indicates that the keyboard buffer is full or enter was pressed*/
int ready_to_read[NUM_TERMINALS];
/* last index of valid data */
int kb_buf_index[NUM_TERMINALS];
char kb_buf[NUM_TERMINALS][MAX_KB_BUF];

/* void keyboard_init(void)
 * INPUT: none
 * OUTPUT: none
 * DESCRIPTION: enables kb interrupts, initializes flags, clears kb buffers
 */
void keyboard_init(void) {
	enable_irq(1); 
	caps_on = 0;
	ctrl_on = 0;
	shift_on = 0;
	/* clear kb values for each terminal */
	clear_keyboard_buf();
}

/* void keyboard_disable(void)
 * INPUT: none
 * OUTPUT: none
 * DESCRIPTION: disables keyboard interrupts, clears kb buffer
 */
void keyboard_disable(void) {
	disable_irq(1);
	clear_keyboard_buf();
}

/* clear_keyboard_buf(void)
 * INPUT: none
 * OUTPUT: none
 * DESCRIPTION: buffer index to start and clears ready to read flag, aka buffer empty
 */
void clear_keyboard_buf(void) {
	int i;
	for (i=0; i<NUM_TERMINALS; i++) {
		ready_to_read[i] = 0;
		kb_buf_index[i] = 0;
	}
}

/* void keyboard_handler(void)
 * INPUT: none
 * OUTPUT: none
 * DESCRIPTION: handler for keyboard interrupts
 * fills kb buffer, echoes pressed keys to the terminal
 */
void keyboard_handler(void) {
	do{
		int pressed;
		pressed = inb(KB_DATA);
		if (kb_buf_index[display_terminal] == MAX_KB_BUF)
		{
			kb_buf_index[display_terminal] = 0;
		}

		switch (pressed)
		{
			case CAPS:
				if (caps_on == 1) {
					caps_on = 0;
				}
				else {
					caps_on = 1;
				}
				break;
			case L_SHIFT_MAKE:
			case R_SHIFT_MAKE:
				shift_on = 1;
				break;
			case L_SHIFT_BREAK:
			case R_SHIFT_BREAK:
				shift_on = 0;
				break;
			case L_CTRL_MAKE:
				ctrl_on = 1;
				break;
			case L_CTRL_BREAK:
				ctrl_on = 0;
				break;
			case L_ALT_MAKE:
				alt_on = 1;
				break;
			case L_ALT_BREAK:
				ctrl_on = 0;
				break;
			case BACKSPACE: /* replaces prev char with space, decrements buffer index */
				if (screen_x > 0) {
					screen_x--;
					putc(' ');
					screen_x--;
					if (kb_buf_index[display_terminal] > 0) {
						kb_buf_index[display_terminal]--;
					}
					update_cursor(screen_x, screen_y);
				}
				break;
			case ENTER: /* prints newline, sets ready to read flag */
				kb_buf[display_terminal][kb_buf_index[display_terminal]] = '\n';
				kb_buf_index[display_terminal]++;
				putc('\n');
				update_cursor(screen_x, screen_y);
				ready_to_read[display_terminal] = 1;
				break;
			default: /* handles printing of characters, adds to kb buffer */
				if (pressed < SUPPORTED_KEYS) {
					uint8_t value;
					
					/** terminal switching **/
					if (alt_on && pressed >= F1)
					{
						cli();	/* AW added because counter is giving coprocessor error */

						/* AW save cursor position */
						virtual_x[display_terminal] = screen_x;
						virtual_y[display_terminal] = screen_y;

						if (pressed == F1) {
							change_display_term(0);
							//save video memory to current display_terminal
							//set new display terminal, and copy its contents to video memory
							
						}
						else if (pressed == F2) {
							change_display_term(1);
						}
						else if (pressed == F3) {
							change_display_term(2);
						}
						// restore the cursor to the proper location when switching terminals
						screen_x = virtual_x[display_terminal];
						screen_y = virtual_y[display_terminal]; 
						update_cursor(screen_x, screen_y);
						sti();	/* AW added because counter is giving coprocessor error */
					}

					if (ctrl_on && pressed == L) {
						term_clear();
						return;
					}

					if (kb_buf_index[display_terminal] == MAX_KB_BUF - 1) {
						return;
					}
					if (caps_on && shift_on) {
						value = caps_shift_key_press[pressed];
					}
					else if (caps_on) {
						value = caps_key_press[pressed];
					}
					else if (shift_on) {
						value = shift_key_press[pressed];
					}
					else {
						value = key_press[pressed];
					}
					if (value != 0) {
						kb_buf[display_terminal][kb_buf_index[display_terminal]] = value;
						kb_buf_index[display_terminal]++;
						putc(value);
						update_cursor(screen_x, screen_y);
					}	
				}
		}
	} while(inb(KB_STATUS) & INPUT_BUFFER_MASK);
}
