/************************************************
 *       Shippensburg University Shell          *
 *                   sush.c                     *
 ************************************************
 * Linux shell command interpreter.             *
 * For now just reads a .sushrc file and prints *
 *  to stdout                                   *
 ************************************************
 * Author: Justin Weigle                        *
 * Edited: 11 Mar 2020                          *
 ************************************************/

#include <stdio.h>
#include "sush.h"
#include "tokenizer.h"
#include "rcreader.h"

int main (int argc, char **argv)
{
    read_sushrc();

    return 0;
}
