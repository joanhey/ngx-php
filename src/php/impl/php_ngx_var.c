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

#include "php_ngx_var.h"
#include "../../ngx_http_php_module.h"
#include "../../ngx_http_php_variable.h"

static zend_class_entry *php_ngx_var_class_entry;

PHP_FUNCTION(ngx_var_get)
{
    zend_string                 *key_str;
    ngx_http_variable_value_t   *var;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() , "S", &key_str) == FAILURE){
        RETURN_NULL();
    }

    var = ngx_http_php_var_get(ZSTR_VAL(key_str),ZSTR_LEN(key_str));

    ZVAL_STRINGL(return_value, (char *) var->data, var->len);
}

PHP_FUNCTION(ngx_var_set)
{
    zend_string *key_str;
    zend_string *value_str;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "SS", &key_str, &value_str) == FAILURE) {
        RETURN_NULL();
    }

    ngx_http_php_var_set(ZSTR_VAL(key_str), ZSTR_LEN(key_str), ZSTR_VAL(value_str), ZSTR_LEN(value_str));
}

PHP_METHOD(ngx_var, get)
{
    zend_string                 *key_str;
    ngx_http_variable_value_t   *var;
  	
    if (zend_parse_parameters(ZEND_NUM_ARGS() , "S", &key_str) == FAILURE){
        RETURN_NULL();
    }

    var = ngx_http_php_var_get(ZSTR_VAL(key_str),ZSTR_LEN(key_str));

    ZVAL_STRINGL(return_value, (char *) var->data, var->len);

}

PHP_METHOD(ngx_var, set)
{
    zend_string *key_str;
    zend_string *value_str;

    if (zend_parse_parameters(ZEND_NUM_ARGS() , "SS", &key_str, &value_str) == FAILURE) {
        RETURN_NULL();
    }

    ngx_http_php_var_set(ZSTR_VAL(key_str), ZSTR_LEN(key_str), ZSTR_VAL(value_str), ZSTR_LEN(value_str));
}

static const zend_function_entry php_ngx_var_class_functions[] = {
    PHP_ME(ngx_var, get, ngx_var_get_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(ngx_var, set, ngx_var_set_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_FE_END
};

void 
php_impl_ngx_var_init(int module_number )
{
    zend_class_entry ngx_var_class_entry;
    INIT_CLASS_ENTRY(ngx_var_class_entry, "ngx_var", php_ngx_var_class_functions);
    php_ngx_var_class_entry = zend_register_internal_class(&ngx_var_class_entry );

}