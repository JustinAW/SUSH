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
#include "includes/sush.h"
#include "includes/tokenizer.h"
#include "includes/rcreader.h"

int main (int argc, char **argv)
{
    read_sushrc();

    return 0;
}
