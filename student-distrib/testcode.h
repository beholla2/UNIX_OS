/*	*********************************************************
	# FILE NAME: testcode.h
	# PURPOSE: Header file for testcode.c
	# AUTHOR: Queeblo OS
	# CREATED: 11/14/2014
	# MODIFIED: 11/14/2014
	********************************************************* */
#ifndef _TESTCODE_H
#define _TESTCODE_H

#include "types.h"
#include "lib.h"
#include "filesys_mod.h"
#include "syscalls.h"
#include "rtc.h"

int testCP1_and_CP2();
int testCP1();
int testCP2();
int testCP3_execute();
int testCP3();
int testCP4();
int testCP5();
int run_tests(int8_t* test_name);


#endif /* _TESTCODE_H */
