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

#include "php_ngx_log.h"
#include "../../ngx_http_php_module.h"

static zend_class_entry *php_ngx_log_class_entry;

PHP_FUNCTION(ngx_log_error)
{
    long                level;
    zend_string         *log_str;
    ngx_http_request_t  *r;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "lS", &level, &log_str) == FAILURE){
        RETURN_NULL();
    }

    r = ngx_php_request;

    ngx_log_error((ngx_uint_t)level, r->connection->log, 0, "%*s", ZSTR_LEN(log_str), ZSTR_VAL(log_str));
}

PHP_METHOD(ngx_log, error)
{
    long                level;
    zend_string         *log_str;
    ngx_http_request_t  *r;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "lS", &level, &log_str) == FAILURE){
        RETURN_NULL();
    }

    r = ngx_php_request;

    ngx_log_error((ngx_uint_t)level, r->connection->log, 0, "%*s", ZSTR_LEN(log_str), ZSTR_VAL(log_str));
}

static const zend_function_entry php_ngx_log_class_functions[] = {
    PHP_ME(ngx_log, error, ngx_log_error_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

void 
php_impl_ngx_log_init(int module_number )
{
    zend_class_entry ngx_log_class_entry;
    INIT_CLASS_ENTRY(ngx_log_class_entry, "ngx_log", php_ngx_log_class_functions);
    php_ngx_log_class_entry = zend_register_internal_class(&ngx_log_class_entry );

    /*zend_declare_class_constant_long(php_ngx_log_class_entry, "STDERR", sizeof("STDERR")-1, NGX_LOG_STDERR );
    zend_declare_class_constant_long(php_ngx_log_class_entry, "EMERG", sizeof("EMERG")-1, NGX_LOG_EMERG );
    zend_declare_class_constant_long(php_ngx_log_class_entry, "ALERT", sizeof("ALERT")-1, NGX_LOG_ALERT );
    zend_declare_class_constant_long(php_ngx_log_class_entry, "CRIT", sizeof("CRIT")-1, NGX_LOG_CRIT );
    zend_declare_class_constant_long(php_ngx_log_class_entry, "ERR", sizeof("ERR")-1, NGX_LOG_ERR );
    zend_declare_class_constant_long(php_ngx_log_class_entry, "WARN", sizeof("WARN")-1, NGX_LOG_WARN );
    zend_declare_class_constant_long(php_ngx_log_class_entry, "NOTICE", sizeof("NOTICE")-1, NGX_LOG_NOTICE );
    zend_declare_class_constant_long(php_ngx_log_class_entry, "INFO", sizeof("INFO")-1, NGX_LOG_INFO );
    zend_declare_class_constant_long(php_ngx_log_class_entry, "DEBUG", sizeof("DEBUG")-1, NGX_LOG_DEBUG );
*/
}