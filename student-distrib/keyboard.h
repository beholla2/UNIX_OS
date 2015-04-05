/* *********************************************************
# FILE NAME: keyboard.h
* PURPOSE: header for keyboard.c
# AUTHOR: Queeblo OS
* MODIFIED: 11/02/2014
********************************************************* */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

/* used to access keyboard ports*/
#define KB_DATA 0x60
#define KB_STATUS 0x64
#define INPUT_BUFFER_MASK 0x2
/* defined in CP2 spec */
#define MAX_KB_BUF 128

#define NUM_TERMINALS 3

/* scan codes for modifiers */
#define CAPS 0x3A
#define L_SHIFT_MAKE 0x2A
#define R_SHIFT_MAKE 0x36
#define L_SHIFT_BREAK 0xAA
#define R_SHIFT_BREAK 0xB6
#define L_CTRL_MAKE 0x1D
#define L_CTRL_BREAK 0x9D
#define L_ALT_MAKE 0x38
#define L_ALT_BREAK 0xB8
#define L 0x26
#define ENTER 0x1C
#define BACKSPACE 0x0E

/* F keys */
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D

#define NUM_COLS 80

/* size of key press array, F10 is 0x44 */
#define SUPPORTED_KEYS 0x45

/* index is scancode, value is pressed key */
static const char key_press[] = {
	0, 0 /*ESC*/, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 
	'-', '=', 0 /*BSP*/, 0 /*TAB*/, 'q', 'w', 'e', 'r', 't', 'y', 
	'u', 'i', 'o', 'p', '[', ']', '\n'/*ENTER*/, 0/*L-CTRL*/, 'a',
	's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0 /*L-SHIFT*/, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', 
	',', '.', '/', 0/*R-SHIFT*/, '*', 0/*L-ALT*/, ' ', 0 /*CAPS*/,
	0 /*F1*/, 0 /*F2*/, 0 /*F3*/, 0 /*F4*/, 0 /*F5*/, 0 /*F6*/,
	0 /*F7*/, 0 /*F8*/, 0 /*F9*/, 0 /*F10*/
};
static const char caps_key_press[] = {
	0, 0 /*ESC*/, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 
	'-', '=', 0 /*BSP*/, 0 /*TAB*/, 'Q', 'W', 'E', 'R', 'T', 'Y', 
	'U', 'I', 'O', 'P', '[', ']', '\n'/*ENTER*/, 0/*L-CTRL*/, 'A',
	'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',
	0 /*L-SHIFT*/, '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', 
	',', '.', '/', 0/*R-SHIFT*/, '*', 0/*L-ALT*/, ' ', 0 /*CAPS*/,
	0 /*F1*/, 0 /*F2*/, 0 /*F3*/, 0 /*F4*/, 0 /*F5*/, 0 /*F6*/,
	0 /*F7*/, 0 /*F8*/, 0 /*F9*/, 0 /*F10*/
};
static const char shift_key_press[] = {
	0, 0 /*ESC*/, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', 
	'_', '+', 0 /*BSP*/, 0 /*TAB*/, 'Q', 'W', 'E', 'R', 'T', 'Y', 
	'U', 'I', 'O', 'P', '{', '}', '\n'/*ENTER*/, 0/*L-CTRL*/, 'A',
	'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
	0 /*L-SHIFT*/, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', 
	'<', '>', '?', 0/*R-SHIFT*/, '*', 0/*L-ALT*/, ' ', 0 /*CAPS*/,
	0 /*F1*/, 0 /*F2*/, 0 /*F3*/, 0 /*F4*/, 0 /*F5*/, 0 /*F6*/,
	0 /*F7*/, 0 /*F8*/, 0 /*F9*/, 0 /*F10*/
};
static const char caps_shift_key_press[] = {
	0, 0 /*ESC*/, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', 
	'_', '+', 0 /*BSP*/, 0 /*TAB*/, 'q', 'w', 'e', 'r', 't', 'y', 
	'u', 'i', 'o', 'p', '{', '}', '\n'/*ENTER*/, 0/*L-CTRL*/, 'a',
	's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~',
	0 /*L-SHIFT*/, '|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', 
	'<', '>', '?', 0/*R-SHIFT*/, '*', 0/*L-ALT*/, ' ', 0 /*CAPS*/,
	0 /*F1*/, 0 /*F2*/, 0 /*F3*/, 0 /*F4*/, 0 /*F5*/, 0 /*F6*/,
	0 /*F7*/, 0 /*F8*/, 0 /*F9*/, 0 /*F10*/
};

extern int ready_to_read[NUM_TERMINALS];
/* last index of valid data */
extern int kb_buf_index[NUM_TERMINALS];
extern char kb_buf[NUM_TERMINALS][MAX_KB_BUF];

void keyboard_init(void);
void keyboard_disable(void);
void clear_keyboard_buf(void);
void keyboard_handler(void);

#endif /* _KEYBOARD_H */
