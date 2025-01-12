/*
==============================================================================
Copyright (c) 2016-2020, rryqszq4 <rryqszq@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
==============================================================================
*/

#include "php_ngx_core.h"
#include "../../ngx_http_php_module.h"
#include "../../ngx_http_php_sleep.h"

static zend_class_entry *php_ngx_class_entry;

PHP_FUNCTION(ngx_exit)
{
    long status = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "l", &status) == FAILURE){
        RETURN_NULL();
    }

    EG(exit_status) = status;

    zend_bailout();
}

PHP_FUNCTION(ngx_status)
{
    ngx_http_request_t  *r;
    long status = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "l", &status) == FAILURE){
        RETURN_NULL();
    }

    r = ngx_php_request;

    r->headers_out.status = status;

    EG(exit_status) = status;
}

PHP_FUNCTION(ngx_query_args)
{
    ngx_http_request_t  *r;
    u_char              *buf, *p;
    char                *query_key=NULL;
    u_char              *last;
    //int                 idx;
    unsigned            parsing_value = 0;

    r = ngx_php_request;

    array_init(return_value);

    if (r->args.len == 0) {
        RETURN_ARR(Z_ARRVAL_P(return_value));
    }

    //idx = 0;

    buf = ngx_palloc(r->pool, r->args.len);
    ngx_memcpy(buf, r->args.data, r->args.len);
    last = buf + r->args.len;

    p = buf;
    parsing_value = 0;

    while (buf != last) {
        if (*buf == '=') {
            query_key = ngx_pnalloc(r->pool, buf-p+1);
            ngx_memzero(query_key, buf-p+1);
            ngx_memcpy(query_key, p, buf-p);
            //php_printf("%d, %.*s\n", buf-p,buf-p, p);
            buf++;
            p = buf;
            parsing_value = 1;
        }else if (*buf == '&') {
            //php_printf("%.*s", buf-p, p);
            add_assoc_stringl(return_value, (char *)query_key, (char *)p, buf-p);
            ngx_pfree(r->pool, query_key);
            //add_index_stringl(return_value, idx, (char *)p, buf-p);
            //idx++;

            buf++;
            p = buf;
            parsing_value = 0;
        }
        
        if (buf != last && *buf != '&'){
            buf++;
        }
    }

    if (parsing_value){
        //php_printf("%.*s", buf-p, p);
        add_assoc_stringl(return_value, (char *)query_key, (char *)p, buf-p);
        ngx_pfree(r->pool, query_key);
        //add_index_stringl(return_value, idx, (char *)p, buf-p);
        //idx++;
    }

    ngx_pfree(r->pool, buf);
}

PHP_FUNCTION(ngx_post_args)
{
    ngx_http_request_t  *r;
    u_char              *buf, *p;
    char                *post_key=NULL;
    size_t              len;
    ngx_chain_t         *cl;
    u_char              *last;
    //int                 idx;
    unsigned            parsing_value = 0;

    r = ngx_php_request;

    array_init(return_value);

    if (r->discard_body || r->request_body == NULL || 
        r->request_body->temp_file || r->request_body->bufs == NULL) {
        RETURN_ARR(Z_ARRVAL_P(return_value));
    }

    len = 0;
    for (cl = r->request_body->bufs; cl; cl = cl->next) {
        len += cl->buf->last - cl->buf->pos;
    }

    if (len == 0) {
        RETURN_ARR(Z_ARRVAL_P(return_value));
    }

    //idx = 0;

    buf = ngx_palloc(r->pool, len);
    p = buf;
    for (cl = r->request_body->bufs; cl; cl = cl->next) {
        p = ngx_copy(p, cl->buf->pos, cl->buf->last - cl->buf->pos);
    }

    ngx_php_debug("post body: %.*s", (int)len, buf);

    last = buf + len;

    p = buf;
    parsing_value = 0;

    while (buf != last) {
        if (*buf == '=') {
            post_key = ngx_pnalloc(r->pool, buf-p+1);
            //ngx_sprintf(post_key, "%*s", buf-p, p);
            ngx_memzero(post_key, buf-p+1);
            ngx_memcpy(post_key, p, buf-p);
            //php_printf("%d, %.*s\n", buf-p,buf-p, p);
            buf++;
            p = buf;
            parsing_value = 1;
        }else if (*buf == '&') {
            //php_printf("%.*s", buf-p, p);
            add_assoc_stringl(return_value, (char *)post_key, (char *)p, buf-p);
            ngx_pfree(r->pool, post_key);
            //add_index_stringl(return_value, idx, (char *)p, buf-p);
            //idx++;

            buf++;
            p = buf;
            parsing_value = 0;
        }
        
        if (buf != last && *buf != '&'){
            buf++;
        }
    }

    if (parsing_value){
        //php_printf("%.*s", buf-p, p);
        add_assoc_stringl(return_value, (char *)post_key, (char *)p, buf-p);
        ngx_pfree(r->pool, post_key);
        //add_index_stringl(return_value, idx, (char *)p, buf-p);
        //idx++;

    }

    ngx_pfree(r->pool, buf);
}

PHP_FUNCTION(ngx_sleep)
{
    ngx_http_request_t  *r;
    ngx_http_php_ctx_t  *ctx;
    long                time;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "l", &time) == FAILURE) {
        RETURN_NULL();
    }

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {

    }

    ctx->delay_time = time * 1000;

    ngx_http_php_sleep(r);

}

PHP_FUNCTION(ngx_msleep)
{
    ngx_http_request_t  *r;
    ngx_http_php_ctx_t  *ctx;
    long                time;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "l", &time) == FAILURE) {
        RETURN_NULL();
    }

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {

    }

    ctx->delay_time = time;

    ngx_http_php_sleep(r);
}

PHP_FUNCTION(ngx_redirect)
{
    ngx_http_request_t  *r;
    ngx_http_php_ctx_t  *ctx;
    zend_string         *uri;
    zend_long           status = NGX_HTTP_MOVED_TEMPORARILY;

    ngx_table_elt_t     *h;
    u_char              *ngx_uri;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "S|l", &uri, &status) == FAILURE) {
        RETURN_FALSE;
    }

    if (status != NGX_HTTP_MOVED_TEMPORARILY
        && status != NGX_HTTP_MOVED_PERMANENTLY
        && status != NGX_HTTP_SEE_OTHER
        && status != NGX_HTTP_TEMPORARY_REDIRECT
        && status != NGX_HTTP_PERMANENT_REDIRECT) 
    {
        RETURN_FALSE;
    }

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {
        RETURN_FALSE;
    }

    ngx_uri = ngx_palloc(r->pool, ZSTR_LEN(uri));
    if (ngx_uri == NULL) {
        RETURN_FALSE;
    }

    ngx_memcpy(ngx_uri, ZSTR_VAL(uri), ZSTR_LEN(uri));

    h = ngx_list_push(&r->headers_out.headers);
    if (h == NULL) {
        RETURN_FALSE;
    }

    h->hash = 1;

    h->value.len = ZSTR_LEN(uri);
    h->value.data = ngx_uri;
    ngx_str_set(&h->key, "Location");

    r->headers_out.status = status;

    EG(exit_status) = status;

    if (ZSTR_LEN(uri) && ngx_uri[0] != '/') {
        r->headers_out.location = h;
    }

    RETURN_TRUE;

}

PHP_METHOD(ngx, _exit)
{
    long status = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "l", &status) == FAILURE){
        RETURN_NULL();
    }

    EG(exit_status) = status;

    zend_bailout();
}

PHP_METHOD(ngx, query_args)
{
    ngx_http_request_t  *r;
    u_char              *buf, *p;
    char                *query_key=NULL;
    u_char              *last;
    //int                 idx;
    unsigned            parsing_value = 0;

    r = ngx_php_request;

    array_init(return_value);

    if (r->args.len == 0) {
        RETURN_ARR(Z_ARRVAL_P(return_value));
    }

    //idx = 0;

    buf = ngx_palloc(r->pool, r->args.len);
    ngx_memcpy(buf, r->args.data, r->args.len);
    last = buf + r->args.len;

    p = buf;
    parsing_value = 0;

    while (buf != last) {
        if (*buf == '=') {
            query_key = ngx_pnalloc(r->pool, buf-p+1);
            ngx_memzero(query_key, buf-p+1);
            ngx_memcpy(query_key, p, buf-p);
            //php_printf("%d, %.*s\n", buf-p,buf-p, p);
            buf++;
            p = buf;
            parsing_value = 1;
        }else if (*buf == '&') {
            //php_printf("%.*s", buf-p, p);
            add_assoc_stringl(return_value, (char *)query_key, (char *)p, buf-p);
            ngx_pfree(r->pool, query_key);
            //add_index_stringl(return_value, idx, (char *)p, buf-p);
            //idx++;

            buf++;
            p = buf;
            parsing_value = 0;
        }
        
        if (buf != last && *buf != '&'){
            buf++;
        }
    }

    if (parsing_value){
        //php_printf("%.*s", buf-p, p);
        add_assoc_stringl(return_value, (char *)query_key, (char *)p, buf-p);
        ngx_pfree(r->pool, query_key);
        //add_index_stringl(return_value, idx, (char *)p, buf-p);
        //idx++;
    }

    ngx_pfree(r->pool, buf);
}

PHP_METHOD(ngx, post_args)
{
    ngx_http_request_t  *r;
    u_char              *buf, *p;
    char                *post_key=NULL;
    size_t              len;
    ngx_chain_t         *cl;
    u_char              *last;
    //int                 idx;
    unsigned            parsing_value = 0;

    r = ngx_php_request;

    array_init(return_value);

    if (r->discard_body || r->request_body == NULL || 
        r->request_body->temp_file || r->request_body->bufs == NULL) {
        RETURN_ARR(Z_ARRVAL_P(return_value));
    }

    len = 0;
    for (cl = r->request_body->bufs; cl; cl = cl->next) {
        len += cl->buf->last - cl->buf->pos;
    }

    if (len == 0) {
        RETURN_ARR(Z_ARRVAL_P(return_value));
    }

    //idx = 0;

    buf = ngx_palloc(r->pool, len);
    p = buf;
    for (cl = r->request_body->bufs; cl; cl = cl->next) {
        p = ngx_copy(p, cl->buf->pos, cl->buf->last - cl->buf->pos);
    }

    ngx_php_debug("post body: %.*s", (int)len, buf);

    last = buf + len;

    p = buf;
    parsing_value = 0;

    while (buf != last) {
        if (*buf == '=') {
            post_key = ngx_pnalloc(r->pool, buf-p+1);
            //ngx_sprintf(post_key, "%*s", buf-p, p);
            ngx_memzero(post_key, buf-p+1);
            ngx_memcpy(post_key, p, buf-p);
            //php_printf("%d, %.*s\n", buf-p,buf-p, p);
            buf++;
            p = buf;
            parsing_value = 1;
        }else if (*buf == '&') {
            //php_printf("%.*s", buf-p, p);
            add_assoc_stringl(return_value, (char *)post_key, (char *)p, buf-p);
            ngx_pfree(r->pool, post_key);
            //add_index_stringl(return_value, idx, (char *)p, buf-p);
            //idx++;

            buf++;
            p = buf;
            parsing_value = 0;
        }
        
        if (buf != last && *buf != '&'){
            buf++;
        }
    }

    if (parsing_value){
        //php_printf("%.*s", buf-p, p);
        add_assoc_stringl(return_value, (char *)post_key, (char *)p, buf-p);
        ngx_pfree(r->pool, post_key);
        //add_index_stringl(return_value, idx, (char *)p, buf-p);
        //idx++;
    }

    ngx_pfree(r->pool, buf);
}

PHP_METHOD(ngx, sleep)
{
    ngx_http_request_t  *r;
    ngx_http_php_ctx_t  *ctx;
    long                time;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "l", &time) == FAILURE) {
        RETURN_NULL();
    }

    r = ngx_php_request;
    ctx = ngx_http_get_module_ctx(r, ngx_http_php_module);

    if (ctx == NULL) {

    }

    ctx->delay_time = time * 1000;

    ngx_http_php_sleep(r);

}

static const zend_function_entry php_ngx_class_functions[] = {
    PHP_ME(ngx, _exit, ngx_exit_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx, query_args, ngx_query_args_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx, post_args, ngx_post_args_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx, sleep, ngx_sleep_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

void php_impl_ngx_core_init(int module_number )
{
    zend_class_entry ngx_class_entry;
    INIT_CLASS_ENTRY(ngx_class_entry, "ngx", php_ngx_class_functions);
    php_ngx_class_entry = zend_register_internal_class(&ngx_class_entry );

    REGISTER_STRINGL_CONSTANT("NGINX_VAR", NGINX_VAR, sizeof(NGINX_VAR)-1, CONST_CS);
    REGISTER_STRINGL_CONSTANT("NGINX_VERSION", NGINX_VERSION, sizeof(NGINX_VERSION)-1, CONST_CS);
    
    REGISTER_STRINGL_CONSTANT("NGX_HTTP_PHP_MODULE_VERSION", NGX_HTTP_PHP_MODULE_VERSION, sizeof(NGX_HTTP_PHP_MODULE_VERSION)-1, CONST_CS);
    REGISTER_STRINGL_CONSTANT("NGX_HTTP_PHP_MODULE_NAME", NGX_HTTP_PHP_MODULE_NAME, sizeof(NGX_HTTP_PHP_MODULE_NAME)-1, CONST_CS);

    REGISTER_LONG_CONSTANT("NGX_OK", NGX_OK, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_ERROR", NGX_ERROR, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_AGAIN", NGX_AGAIN, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_BUSY", NGX_BUSY, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_DONE", NGX_DONE, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_DECLINED", NGX_DECLINED, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_ABORT", NGX_ABORT, CONST_CS);

    REGISTER_LONG_CONSTANT("NGX_HTTP_CONTINUE", NGX_HTTP_CONTINUE, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_SWITCHING_PROTOCOLS", NGX_HTTP_SWITCHING_PROTOCOLS, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_PROCESSING", NGX_HTTP_PROCESSING, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_OK", NGX_HTTP_OK, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_CREATED", NGX_HTTP_CREATED, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_ACCEPTED", NGX_HTTP_ACCEPTED, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_NO_CONTENT", NGX_HTTP_NO_CONTENT, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_PARTIAL_CONTENT", NGX_HTTP_PARTIAL_CONTENT, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_SPECIAL_RESPONSE", NGX_HTTP_SPECIAL_RESPONSE, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_MOVED_PERMANENTLY", NGX_HTTP_MOVED_PERMANENTLY, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_MOVED_TEMPORARILY", NGX_HTTP_MOVED_TEMPORARILY, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_SEE_OTHER", NGX_HTTP_SEE_OTHER, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_NOT_MODIFIED", NGX_HTTP_NOT_MODIFIED, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_TEMPORARY_REDIRECT", NGX_HTTP_TEMPORARY_REDIRECT, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_PERMANENT_REDIRECT", NGX_HTTP_PERMANENT_REDIRECT, CONST_CS);

    REGISTER_LONG_CONSTANT("NGX_HTTP_BAD_REQUEST", NGX_HTTP_BAD_REQUEST, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_UNAUTHORIZED", NGX_HTTP_UNAUTHORIZED, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_FORBIDDEN", NGX_HTTP_FORBIDDEN, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_NOT_FOUND", NGX_HTTP_NOT_FOUND, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_NOT_ALLOWED", NGX_HTTP_NOT_ALLOWED, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_REQUEST_TIME_OUT", NGX_HTTP_REQUEST_TIME_OUT, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_CONFLICT", NGX_HTTP_CONFLICT, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_LENGTH_REQUIRED", NGX_HTTP_LENGTH_REQUIRED, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_PRECONDITION_FAILED", NGX_HTTP_PRECONDITION_FAILED, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_REQUEST_ENTITY_TOO_LARGE", NGX_HTTP_REQUEST_ENTITY_TOO_LARGE, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_REQUEST_URI_TOO_LARGE", NGX_HTTP_REQUEST_URI_TOO_LARGE, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_UNSUPPORTED_MEDIA_TYPE", NGX_HTTP_UNSUPPORTED_MEDIA_TYPE, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_RANGE_NOT_SATISFIABLE", NGX_HTTP_RANGE_NOT_SATISFIABLE, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_CLOSE", NGX_HTTP_CLOSE, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_NGINX_CODES", NGX_HTTP_NGINX_CODES, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_REQUEST_HEADER_TOO_LARGE", NGX_HTTP_REQUEST_HEADER_TOO_LARGE, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTPS_CERT_ERROR", NGX_HTTPS_CERT_ERROR, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTPS_NO_CERT", NGX_HTTPS_NO_CERT, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_TO_HTTPS", NGX_HTTP_TO_HTTPS, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_CLIENT_CLOSED_REQUEST", NGX_HTTP_CLIENT_CLOSED_REQUEST, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_INTERNAL_SERVER_ERROR", NGX_HTTP_INTERNAL_SERVER_ERROR, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_NOT_IMPLEMENTED", NGX_HTTP_NOT_IMPLEMENTED, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_BAD_GATEWAY", NGX_HTTP_BAD_GATEWAY, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_SERVICE_UNAVAILABLE", NGX_HTTP_SERVICE_UNAVAILABLE, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_GATEWAY_TIME_OUT", NGX_HTTP_GATEWAY_TIME_OUT, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_HTTP_INSUFFICIENT_STORAGE", NGX_HTTP_INSUFFICIENT_STORAGE, CONST_CS);

    REGISTER_LONG_CONSTANT("NGX_LOG_STDERR", NGX_LOG_STDERR, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_LOG_EMERG", NGX_LOG_EMERG, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_LOG_ALERT", NGX_LOG_ALERT, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_LOG_CRIT", NGX_LOG_CRIT, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_LOG_ERR", NGX_LOG_ERR, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_LOG_WARN", NGX_LOG_WARN, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_LOG_NOTICE", NGX_LOG_NOTICE, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_LOG_INFO", NGX_LOG_INFO, CONST_CS);
    REGISTER_LONG_CONSTANT("NGX_LOG_DEBUG", NGX_LOG_DEBUG, CONST_CS);

}
