#include "https_client.h"
#include "pico/cyw43_arch.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"

typedef struct TLS_CLIENT_T_
{
    struct altcp_pcb *pcb;
    bool complete;
    int error;
    const char *http_request;
    char *data;
    size_t len;
} TLS_CLIENT_T;

static struct altcp_tls_config *tls_config = NULL;

static err_t tls_client_close(void *arg)
{
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    err_t err = ERR_OK;

    state->complete = true;
    if (state->pcb != NULL)
    {
        altcp_arg(state->pcb, NULL);
        altcp_poll(state->pcb, NULL, 0);
        altcp_recv(state->pcb, NULL);
        altcp_err(state->pcb, NULL);
        err = altcp_close(state->pcb);
        if (err != ERR_OK)
        {
            altcp_abort(state->pcb);
            err = ERR_ABRT;
        }
        state->pcb = NULL;
    }
    return err;
}

static err_t tls_client_connected(void *arg, struct altcp_pcb *pcb, err_t err)
{
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    if (err != ERR_OK)
        return tls_client_close(state);
    err = altcp_write(state->pcb, state->http_request, strlen(state->http_request), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK)
        return tls_client_close(state);
    return ERR_OK;
}

static err_t tls_client_poll(void *arg, struct altcp_pcb *pcb)
{
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    state->error = PICO_ERROR_TIMEOUT;
    return tls_client_close(arg);
}

static void tls_client_err(void *arg, err_t err)
{
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    tls_client_close(state);
    state->error = PICO_ERROR_GENERIC;
}

static err_t tls_client_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err)
{
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    if (!p)
        return tls_client_close(state);

    if (p->tot_len > 0)
    {
        if (!state->data)
        {
            char *content_start = strnstr(p->payload, "\r\n\r\n", p->tot_len);
            if (content_start)
            {
                const int offset = content_start + 4 - (char *)p->payload;
                state->len = p->tot_len - offset;
                state->data = malloc(state->len + 1);
                pbuf_copy_partial(p, state->data, p->tot_len, offset);
                state->data[state->len] = 0;
            }
        }
        else
        {
            const int new_len = state->len + p->tot_len;
            state->data = realloc(state->data, new_len + 1);
            pbuf_copy_partial(p, state->data + state->len, p->tot_len, 0);
            state->data[new_len] = 0;
            state->len = new_len;
        }
        altcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);
    return ERR_OK;
}

static void tls_client_connect_to_server_ip(const ip_addr_t *ipaddr, TLS_CLIENT_T *state)
{
    err_t err = altcp_connect(state->pcb, ipaddr, 443, tls_client_connected);
    if (err != ERR_OK)
        tls_client_close(state);
}

static void tls_client_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg)
{
    if (ipaddr)
    {
        tls_client_connect_to_server_ip(ipaddr, (TLS_CLIENT_T *)arg);
    }
    else
    {
        tls_client_close(arg);
    }
}

static bool tls_client_open(const char *hostname, void *arg)
{
    TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
    state->pcb = altcp_tls_new(tls_config, IPADDR_TYPE_ANY);
    if (!state->pcb)
        return false;

    altcp_arg(state->pcb, state);
    altcp_poll(state->pcb, tls_client_poll, 5);
    altcp_recv(state->pcb, tls_client_recv);
    altcp_err(state->pcb, tls_client_err);

    mbedtls_ssl_set_hostname(altcp_tls_context(state->pcb), hostname);

    ip_addr_t server_ip;
    cyw43_arch_lwip_begin();
    const err_t err = dns_gethostbyname(hostname, &server_ip, tls_client_dns_found, state);
    if (err == ERR_OK)
    {
        tls_client_connect_to_server_ip(&server_ip, state);
    }
    else if (err != ERR_INPROGRESS)
    {
        tls_client_close(state);
    }
    cyw43_arch_lwip_end();
    return err == ERR_OK || err == ERR_INPROGRESS;
}

char *https_request(const char *server, const char *request, size_t *len)
{
    tls_config = altcp_tls_create_config_client(NULL, 0);
    assert(tls_config);

    TLS_CLIENT_T *state = calloc(1, sizeof(TLS_CLIENT_T));
    if (!state)
        return false;

    state->http_request = request;
    state->data = 0;
    if (!tls_client_open(server, state))
        return false;

    while (!state->complete)
    {
#if PICO_CYW43_ARCH_POLL
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
#else
        // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
        // is done via interrupt in the background. This sleep is just an example of some (blocking)
        // work you might be doing.
        sleep_ms(1000);
#endif
    }
    const int err = state->error;
    char *data = state->data;
    if (len)
        *len = state->len;
    free(state);
    altcp_tls_free_config(tls_config);
    if (err)
    {
        if (data)
            free(data);
        return 0;
    }
    return data;
}

char *https_get_request(const char *server, const char *path, size_t *len)
{
    char buf[512];
    sniprintf(buf, sizeof(buf), "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", path, server);
    return https_request(server, buf, len);
}