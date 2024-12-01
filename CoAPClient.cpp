#include "CoAPClient.h"
#include "coap3/coap.h"

int CoAPClient::resolve_address(const coap_str_const_t *host, const uint16_t port, coap_address_t *dst, const int scheme_hint_bits) {
    // This was lifted pretty much exactly from the libcoap examples.
    int ret = 0;

    coap_addr_info_t *addr_info = coap_resolve_address_info(host, port, port,  port, port,AF_UNSPEC, scheme_hint_bits, COAP_RESOLVE_TYPE_REMOTE);
    if (addr_info) {
        ret = 1;
        *dst = addr_info->addr;
    }

    coap_free_address_info(addr_info);
    return ret;
}

void CoAPClient::create_req(const char *in_uri, const coap_response_handler_t &handler, const coap_pdu_code_t method) {
    // I've heard it's typically bad practice to define as you go along, but I'm not doing all that.

    // Parse URI
    coap_uri_t uri;
    int len = coap_split_uri(reinterpret_cast<const unsigned char *>(in_uri), strlen(in_uri), &uri);
    if (len != 0) coap_log_err("URI parsing failed on %s\n", in_uri);

    // Resolve URI
    coap_address_t dst;
    len = resolve_address(&uri.host, uri.port, &dst, 1 << uri.scheme);
    if (len <= 0) coap_log_err("Failed to resolve addr %*.*s\n", static_cast<int>(uri.host.length), static_cast<int>(uri.host.length), reinterpret_cast<const char *>(uri.host.s));

    coap_session_t *session;
    if (uri.scheme == COAP_URI_SCHEME_COAP) {
        session = coap_new_client_session(ctx, nullptr, &dst, COAP_PROTO_UDP);
    } else {
        coap_log_emerg("Unimplemented scheme used for URI %s\n", in_uri);
        return;
    }

    if (!session) coap_log_emerg("Cannot create client session!\n");

    coap_pdu_t *pdu = coap_pdu_init(COAP_MESSAGE_CON, method, coap_new_message_id(session), coap_session_max_pdu_size(session));
    if (!pdu) {
        coap_log_emerg("Failed to create PDU for %s\n", in_uri);
        return;
    }

    coap_register_response_handler(ctx, handler);

    coap_optlist_t *optlist = nullptr;
    len = coap_uri_into_options(&uri, &dst, &optlist, 1, scratch, sizeof(scratch));
    if (len) {
        coap_log_err("Failed to create options for %s\n", in_uri);
        return;
    }
    if (optlist) {
        if (const int res = coap_add_optlist_pdu(pdu, &optlist); res != 1) {
            coap_log_err("Failed to add options to PDU for %s\n", in_uri);
            return;
        }
    }

    coap_show_pdu(COAP_LOG_INFO, pdu);

    if (coap_send(session, pdu) == COAP_INVALID_MID) {
        coap_log_emerg("Failed to send PDU to %s\n", in_uri);
        return;
    }

    // Timeout functionality.
    int wait_ms = (coap_session_get_default_leisure(session).integer_part + 1) * 1000;
    if (const int res = coap_io_process(ctx, 1000); res >= 0 && wait_ms > 0) {
        if (static_cast<unsigned>(res) >= wait_ms) {
            fprintf(stdout, "Timed Out.\n");
        } else {
            wait_ms -= res;
        }
    }
}

CoAPClient::CoAPClient() : scratch{}, ctx(nullptr) {}
CoAPClient::~CoAPClient() { stop(); }

bool CoAPClient::init(const coap_log_t log_level) {
    coap_startup();
    coap_set_log_level(log_level);

    ctx = coap_new_context(nullptr);
    if (!ctx) {
        coap_log_emerg("Failed to create CoAP context\n");
        stop();
    }

    // Is this necessary?
    coap_context_set_block_mode(ctx,COAP_BLOCK_USE_LIBCOAP | COAP_BLOCK_SINGLE_BODY);
    return true;
}
void CoAPClient::stop() const {
    coap_free_context(ctx);
    coap_cleanup();
}

void CoAPClient::get(const char *in_uri, const coap_response_handler_t &handler) {
    create_req(in_uri, handler, COAP_REQUEST_CODE_GET);
}
void CoAPClient::post(const char *in_uri, const coap_response_handler_t &handler) {
    create_req(in_uri, handler, COAP_REQUEST_CODE_POST);
}
