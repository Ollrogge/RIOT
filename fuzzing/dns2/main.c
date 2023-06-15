#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <err.h>
#include "net/sock/dns.h"
#include "net/dns.h"

#include "uri_parser.h"
#include "fuzzing.h"

__AFL_FUZZ_INIT()

#pragma clang optimize off

int main(void)
{
    char addr_out[256] = {0};
    uint32_t ttl;
    __AFL_INIT();
    uint8_t *buf = __AFL_FUZZ_TESTCASE_BUF;
    while (__AFL_LOOP(10000)) {
        int len = __AFL_FUZZ_TESTCASE_LEN > 128 ? 128 : __AFL_FUZZ_TESTCASE_LEN;
        dns_msg_parse_reply(buf, len, AF_UNSPEC, addr_out, &ttl);
    }

    exit(EXIT_SUCCESS);
    return 0;
}


/*

#include "net/sock/dns.h"

int main(void) {
    char hostname[] = {"hello.com"};
    char out[256] = {0};
    while (1) {
        sock_dns_query(hostname, out, AF_UNSPEC);
    }
}

*/
