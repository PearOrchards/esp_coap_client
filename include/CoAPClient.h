#ifndef CLIENTCLASS_H
#define CLIENTCLASS_H

#include "coap3/coap.h"

/**
 * The class responsible for set up and handling CoAP client requests.
 * @author Pawel Kedzia
 */
class CoAPClient {
    unsigned char scratch[100];
    coap_context_t *ctx;
    /**
    * Converts the given URI to a CoAP address.
    * @note Not intended to be used directly.
    * @author Pawel Kedzia
    */
    static int resolve_address(const coap_str_const_t *host, uint16_t port, coap_address_t *dst, int scheme_hint_bits);

    /**
    * Default function which handles all request creation
    * @note Not intended to be used directly, use one of the extending methods instead
    * @author Pawel Kedzia
    */
    void create_req(const char *in_uri, const coap_response_handler_t &handler, coap_pdu_code_t method);

public:
    CoAPClient();
    ~CoAPClient();

    /**
    * Sets up all common parameters to be used between all requests
    * @author Pawel Kedzia
    */

    bool init(coap_log_t log_level = COAP_LOG_INFO);
    /**
    * Stops the CoAP client, freeing up memory
    * @note Usually doesn't need to be called manually, destructor will do this for you
    * @author Pawel Kedzia
    */
    void stop() const;

    /**
    * Creates a GET request
    * @param in_uri The URI to access (coap://example.com/path)
    * @param handler The function to be called after receiving data. Anonymous functions are supported if set up correctly.
    * @author Pawel Kedzia
    */
    void get(const char *in_uri, const coap_response_handler_t &handler);

    /**
    * Creates a POST request
    * @param in_uri The URI to access (coap://example.com/path)
    * @param handler The function to be called after receiving data. Anonymous functions are supported if set up correctly.
    * @author Pawel Kedzia
    */
    void post(const char *in_uri, const coap_response_handler_t &handler);
};

#endif //CLIENTCLASS_H
