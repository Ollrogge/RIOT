#include "net/sock/dns.h"

int main(void) {
    char hostname[] = {"hello.com"};
    char out[256] = {0};
    while (1) {
        sock_dns_query(hostname, out, AF_UNSPEC);
    }
}
