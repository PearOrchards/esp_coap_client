#ifndef CLIENTCLASS_H
#define CLIENTCLASS_H

#include "coap3/coap.h"

class CoAPClient {
    unsigned char scratch[100];
    coap_context_t *ctx;
    static int resolve_address(const coap_str_const_t *host, uint16_t port, coap_address_t *dst, int scheme_hint_bits);
    void create_req(const char *in_uri, const coap_response_handler_t &handler, coap_pdu_code_t method);

public:
    CoAPClient();
    ~CoAPClient();
    bool init(coap_log_t log_level = COAP_LOG_INFO);
    void stop() const;

    void get(const char *in_uri, const coap_response_handler_t &handler);
    void post(const char *in_uri, const coap_response_handler_t &handler);
};

#endif //CLIENTCLASS_H
