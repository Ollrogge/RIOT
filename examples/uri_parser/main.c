
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "uri_parser.h"

int main(void) {
    //stdio_init();
    char buf[0x80];

    while (1) {

        //DEBUG("reading stuff \n");
        size_t cnt = read(STDIN_FILENO,buf, sizeof(buf));

        uri_parser_result_t uri_res;
        uri_parser_process(&uri_res, buf, cnt);
    }
}
