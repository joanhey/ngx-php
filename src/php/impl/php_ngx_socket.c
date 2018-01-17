/**
 *    Copyright(c) 2016-2018 rryqszq4
 *
 *
 */

#include "php_ngx_socket.h"
#include "../../ngx_http_php_module.h"
#include "../../ngx_http_php_socket.h"

static zend_class_entry *php_ngx_socket_class_entry;

ZEND_BEGIN_ARG_INFO_EX(ngx_socket_create_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_socket_connect_arginfo, 0, 0, 3)
    ZEND_ARG_INFO(0, socket)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_socket_send_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, socket)
    ZEND_ARG_INFO(0, buf)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_socket_recv_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, socket)
    ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ngx_socket_close_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, socket)
ZEND_END_ARG_INFO()


PHP_METHOD(ngx_socket, create)
{

}

PHP_METHOD(ngx_socket, connect)
{
    ngx_http_request_t *r;
    ngx_http_php_ctx_t *ctx;

    zend_string *host_str;
    long port;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Sl", &host_str, &port) == FAILURE){
        RETURN_NULL();
    }

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ctx->host.data = ngx_palloc(r->pool, ZSTR_LEN(host_str) + 1);
    ctx->host.len = ZSTR_LEN(host_str);

    ngx_memcpy(ctx->host.data, (u_char *)ZSTR_VAL(host_str), ZSTR_LEN(host_str) + 1);
    ctx->host.data[ZSTR_LEN(host_str)] = '\0';

    ctx->port = port;

    ngx_http_php_socket_connect(r);

}

PHP_METHOD(ngx_socket, send)
{

}

PHP_METHOD(ngx_socket, recv)
{

}

PHP_METHOD(ngx_socket, close)
{
    ngx_http_request_t *r;
    ngx_http_php_ctx_t *ctx;

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    ngx_http_php_socket_close(r);
}

static const zend_function_entry php_ngx_socket_class_functions[] = {
    PHP_ME(ngx_socket, create, ngx_socket_create_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_socket, connect, ngx_socket_connect_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_socket, send, ngx_socket_send_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_socket, recv, ngx_socket_recv_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_socket, close, ngx_socket_close_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    {NULL, NULL, NULL, 0, 0}
};

void
ext_php_ngx_socket_init(int module_number TSRMLS_DC)
{
    zend_class_entry ngx_socket_class_entry;
    INIT_CLASS_ENTRY(ngx_socket_class_entry, "ngx_socket", php_ngx_socket_class_functions);
    php_ngx_socket_class_entry = zend_register_internal_class(&ngx_socket_class_entry TSRMLS_CC);
}