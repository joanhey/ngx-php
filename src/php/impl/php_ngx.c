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

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_ngx.h"
#include "php_ngx_core.h"
#include "php_ngx_log.h"
#include "php_ngx_request.h"
#include "php_ngx_sockets.h"
#include "php_ngx_var.h"
#include "php_ngx_header.h"
#include "php_ngx_cookie.h"
#include "zend_frameless_function.h"

#include "../../ngx_http_php_module.h"

/* If you declare any globals in php_php_ngx.h uncomment this: */
ZEND_DECLARE_MODULE_GLOBALS(php_ngx)


/* True global resources - no need for thread safety here */
//static int le_php_ngx;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("php_ngx.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_php_ngx_globals, php_ngx_globals)
    STD_PHP_INI_ENTRY("php_ngx.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_php_ngx_globals, php_ngx_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_php_ngx_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_php_ngx_compiled)
{
    char *arg = NULL;
    size_t arg_len;
    zend_string *strg;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
        return;
    }

    strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "php_ngx", arg);

    RETURN_STR(strg);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_php_ngx_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_php_ngx_init_globals(zend_php_ngx_globals *php_ngx_globals)
{
    php_ngx_globals->global_value = 0;
    php_ngx_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(php_ngx)
{
    /* If you have INI entries, uncomment these lines
    REGISTER_INI_ENTRIES();
    */

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(php_ngx)
{
    /* uncomment this line if you have INI entries
    UNREGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(php_ngx)
{
#if defined(COMPILE_DL_PHP_NGX) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif

    PHP_NGX_G(global_r) = NULL;

    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(php_ngx)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(php_ngx)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "php_ngx support", "enabled");
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}
/* }}} */

/* {{{ php_ngx_functions[]
 *
 * Every user visible function must have an entry in php_ngx_functions[].
 */
const zend_function_entry php_ngx_functions[] = {
    PHP_FE(confirm_php_ngx_compiled,    NULL)       /* For testing, remove later. */
    PHP_FE_END  /* Must be the last line in php_ngx_functions[] */
};
/* }}} */

/* {{{ php_ngx_module_entry
 */
zend_module_entry php_ngx_module_entry = {
    STANDARD_MODULE_HEADER,
    "php_ngx",
    php_ngx_functions,
    PHP_MINIT(php_ngx),
    PHP_MSHUTDOWN(php_ngx),
    PHP_RINIT(php_ngx),     /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(php_ngx), /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(php_ngx),
    PHP_PHP_NGX_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PHP_NGX
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(php_ngx)
#endif

#if defined(PHP_WIN32) && defined(ZTS)
ZEND_TSRMLS_CACHE_DEFINE()
#endif

const char HARDCODED_INI[] =
    "html_errors=0\n"
    "register_argc_argv=1\n"
    "implicit_flush=1\n"
    "output_buffering=0\n"
    "max_execution_time=0\n"
    "max_input_time=-1\n\0";

static int php_ngx_startup(sapi_module_struct *sapi_module)
{
#if (PHP_MAJOR_VERSION >= 8 && PHP_MINOR_VERSION > 1) 
    return php_module_startup(sapi_module, NULL);
#else
    return php_module_startup(sapi_module, NULL, 0);
#endif
}

static int php_ngx_deactivate()
{
    return SUCCESS;
}

static size_t php_ngx_ub_write(const char *str, size_t str_length )
{
    return str_length;
}

static void php_ngx_flush(void *server_context)
{
}

static int php_ngx_header_handler(sapi_header_struct *sapi_header, sapi_header_op_enum op, sapi_headers_struct *sapi_headers )
{
    return 0;
}

static size_t php_ngx_read_post(char *buffer, size_t count_bytes )
{
    return 0;
}

static char* php_ngx_read_cookies()
{
    return NULL;
}

static void php_ngx_register_variables(zval *track_vars_array )
{
    php_import_environment_variables(track_vars_array );

    /*if (SG(request_info).request_method) {
        php_register_variable("REQUEST_METHOD", (char *)SG(request_info).request_method, track_vars_array );
    }
    if (SG(request_info).request_uri){
        php_register_variable("DOCUMENT_URI", (char *)SG(request_info).request_uri, track_vars_array );
    }
    if (SG(request_info).query_string){
        php_register_variable("QUERY_STRING", (char *)SG(request_info).query_string, track_vars_array );
    }*/
}

/*static void php_ngx_log_message(char *message)
{
}*/

sapi_module_struct php_ngx_module = {
#if (PHP_MAJOR_VERSION >= 8 && PHP_MINOR_VERSION > 2)
    "ngx-php",
#else
    "cli-server",                                          /* name */
#endif
    "Ngx-php PHP embedded in Nginx",                    /* pretty name */

    php_ngx_startup,                                    /* startup */
    php_module_shutdown_wrapper,                        /* shutdown */

    NULL,                                               /* activate */
    php_ngx_deactivate,                                 /* deactivate */

    php_ngx_ub_write,                                   /* unbuffered write */
    php_ngx_flush,                                      /* flush */
    NULL,                                               /* get uid */
    NULL,                                               /* getenv */

    php_error,                                          /* error handler */

    php_ngx_header_handler,                             /* header handler */
    NULL,                                               /* send headers handler */
    NULL,                                               /* send header handler */

    php_ngx_read_post,                                  /* read POST data */
    php_ngx_read_cookies,                               /* read Cookies */

    php_ngx_register_variables,                         /* register server variables */
    NULL,                                               /* Log message */
    NULL,                                               /* Get request time */
    NULL,                                               /* Child terminate */

#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 1
    NULL,                                               /* php_ini_path_override */
    NULL,                                               /* block_interruptions */
    NULL,                                               /* unblock_interruptions */
    NULL,                                               /* default_post_reader */
    NULL,                                               /* treat_data */
    NULL,                                               /* executable_location */

    0,                                                  /* php_ini_ignore */
    0,                                                  /* php_ini_ignore_cwd */

    NULL,                                               /* get_fd */

    NULL,                                               /* force_http_10 */

    NULL,                                               /* get_target_uid */
    NULL,                                               /* get_target_gid */

    NULL,                                               /* input_filter */

    NULL,                                               /* ini_defaults */
    0,                                                  /* phpinfo_as_text */

    NULL,                                               /* ini_entries */
    NULL,                                               /* additional_functions */
    NULL,                                               /* input_filter_init */
#endif

    STANDARD_SAPI_MODULE_PROPERTIES
};
/* }}} */

/* {{{ arginfo ext/standard/dl.c */
ZEND_BEGIN_ARG_INFO(arginfo_dl, 0)
    ZEND_ARG_INFO(0, extension_filename)
ZEND_END_ARG_INFO()
/* }}} */

static const zend_function_entry additional_functions[] = {
    ZEND_FE(dl,                             arginfo_dl)

    PHP_FE(ngx_exit,                        ngx_exit_arginfo)
    PHP_FE(ngx_status,                      ngx_status_arginfo)
    PHP_FE(ngx_query_args,                  ngx_query_args_arginfo)
    PHP_FE(ngx_post_args,                   ngx_post_args_arginfo)
    PHP_FE(ngx_sleep,                       ngx_sleep_arginfo)
    PHP_FE(ngx_msleep,                      ngx_msleep_arginfo)

    PHP_FE(ngx_log_error,                   ngx_log_error_arginfo)

    PHP_FE(ngx_request_method,              ngx_request_method_arginfo)
    PHP_FE(ngx_request_document_root,       ngx_request_document_root_arginfo)
    PHP_FE(ngx_request_document_uri,        ngx_request_document_uri_arginfo)
    PHP_FE(ngx_request_script_name,         ngx_request_script_name_arginfo)
    PHP_FE(ngx_request_script_filename,     ngx_request_script_filename_arginfo)
    PHP_FE(ngx_request_query_string,        ngx_request_query_string_arginfo)
    PHP_FE(ngx_request_uri,                 ngx_request_request_uri_arginfo)
    PHP_FE(ngx_request_server_protocol,     ngx_request_server_protocol_arginfo)
    PHP_FE(ngx_request_remote_addr,         ngx_request_remote_addr_arginfo)
    PHP_FE(ngx_request_server_addr,         ngx_request_server_addr_arginfo)
    PHP_FE(ngx_request_remote_port,         ngx_request_remote_port_arginfo)
    PHP_FE(ngx_request_server_port,         ngx_request_server_port_arginfo)
    PHP_FE(ngx_request_server_name,         ngx_request_server_name_arginfo)
    PHP_FE(ngx_request_headers,             ngx_request_headers_arginfo)
    PHP_FE(ngx_request_body,                ngx_request_body_arginfo)

    PHP_FE(ngx_socket_create,               arginfo_ngx_socket_create)
    PHP_FE(ngx_socket_iskeepalive,          arginfo_ngx_socket_iskeepalive)
    PHP_FE(ngx_socket_connect,              arginfo_ngx_socket_connect)
    PHP_FE(ngx_socket_close,                arginfo_ngx_socket_close)
    PHP_FE(ngx_socket_send,                 arginfo_ngx_socket_send)
    PHP_FE(ngx_socket_recv,                 arginfo_ngx_socket_recv)
    PHP_FE(ngx_socket_recvpage,             arginfo_ngx_socket_recvpage)
    PHP_FE(ngx_socket_recvwait,             arginfo_ngx_socket_recvwait)
    PHP_FE(ngx_socket_recvsync,             arginfo_ngx_socket_recvsync)
    PHP_FE(ngx_socket_clear,                arginfo_ngx_socket_clear)
    PHP_FE(ngx_socket_destroy,              arginfo_ngx_socket_destroy)

    PHP_FE(ngx_var_get,                     ngx_var_get_arginfo)
    PHP_FE(ngx_var_set,                     ngx_var_set_arginfo)

    PHP_FE(ngx_header_set,                  ngx_header_set_arginfo)
    PHP_FE(ngx_header_get,                  ngx_header_get_arginfo)
    PHP_FE(ngx_header_get_all,              ngx_header_get_all_arginfo)

    PHP_FE(ngx_redirect,                    ngx_redirect_arginfo)

    PHP_FE(ngx_cookie_get_all,              ngx_cookie_get_all_arginfo)
    PHP_FE(ngx_cookie_get,                  ngx_cookie_get_arginfo)
    PHP_FE(ngx_cookie_set,                  ngx_cookie_set_arginfo)

    {NULL, NULL, NULL, 0, 0}
};

int php_ngx_module_init()
{
    zend_llist global_vars;
/*#ifdef ZTS
    void ***tsrm_ls = NULL;
#endif*/

#ifdef HAVE_SIGNAL_H
#if defined(SIGPIPE) && defined(SIG_IGN)
    signal(SIGPIPE, SIG_IGN); /* ignore SIGPIPE in standalone mode so
                                 that sockets created via fsockopen()
                                 don't kill PHP if the remote site
                                 closes it.  in apache|apxs mode apache
                                 does that for us!  thies@thieso.net
                                 20000419 */
#endif
#endif

#ifdef ZTS
  tsrm_startup(1, 1, 0, NULL);
  (void)ts_resource(0);
  ZEND_TSRMLS_CACHE_UPDATE();
#endif

//#ifdef ZEND_SIGNALS
//    zend_signal_startup();
//#endif

  sapi_startup(&php_ngx_module);

#ifdef PHP_WIN32
  _fmode = _O_BINARY;                       /*sets default for file streams to binary */
  setmode(_fileno(stdin), O_BINARY);        /* make the stdio mode be binary */
  setmode(_fileno(stdout), O_BINARY);       /* make the stdio mode be binary */
  setmode(_fileno(stderr), O_BINARY);       /* make the stdio mode be binary */
#endif

#if (PHP_MAJOR_VERSION >= 8 && PHP_MINOR_VERSION > 2)
  php_ngx_module.ini_entries = HARDCODED_INI;
#else
  php_ngx_module.ini_entries = malloc(sizeof(HARDCODED_INI));
  memcpy(php_ngx_module.ini_entries, HARDCODED_INI, sizeof(HARDCODED_INI));
#endif

  php_ngx_module.additional_functions = additional_functions;

  php_ngx_module.executable_location = NULL;

  if (php_ngx_module.startup(&php_ngx_module) == FAILURE){
    return FAILURE;
  }

  zend_llist_init(&global_vars, sizeof(char *), NULL, 0); 

  return SUCCESS;
}

int php_ngx_request_init()
{
    if (php_request_startup()==FAILURE) {
        return FAILURE;
    }

    SG(headers_sent) = 0;
    SG(request_info).no_headers = 1;
    php_register_variable("PHP_SELF", "-", NULL );

    return SUCCESS;
}

void php_ngx_request_shutdown()
{
    SG(headers_sent) = 1;
    php_request_shutdown((void *)0);
}

void php_ngx_module_shutdown()
{
    php_module_shutdown();
    sapi_shutdown();
#ifdef ZTS
    tsrm_shutdown();
#endif

#if (PHP_MAJOR_VERSION <= 8 && PHP_MINOR_VERSION <= 2)
    if (php_ngx_module.ini_entries){
        free(php_ngx_module.ini_entries);
        php_ngx_module.ini_entries = NULL;
    }
#endif

}
