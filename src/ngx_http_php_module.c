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

#include "php/impl/php_ngx.h"
#include "php/impl/php_ngx_core.h"
#include "php/impl/php_ngx_log.h"
#include "php/impl/php_ngx_request.h"
#include "php/impl/php_ngx_socket.h"
#include "php/impl/php_ngx_var.h"
#include "php/impl/php_ngx_sockets.h"
#include "php/impl/php_ngx_header.h"

#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_conf_file.h>
#include <nginx.h>

#include "ngx_http_php_module.h"
#include "ngx_http_php_directive.h"
#include "ngx_http_php_handler.h"

// http init
static ngx_int_t ngx_http_php_init(ngx_conf_t *cf);
static ngx_int_t ngx_http_php_handler_init(ngx_http_core_main_conf_t *cmcf, ngx_http_php_main_conf_t *pmcf);

static void *ngx_http_php_create_main_conf(ngx_conf_t *cf);
static char *ngx_http_php_init_main_conf(ngx_conf_t *cf, void *conf);

static void *ngx_http_php_create_srv_conf(ngx_conf_t *cf);

static void *ngx_http_php_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_php_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child); 

// function init
static ngx_int_t ngx_http_php_init_worker(ngx_cycle_t *cycle);
static void ngx_http_php_exit_worker(ngx_cycle_t *cycle);

static ngx_command_t ngx_http_php_commands[] = {

    {ngx_string("php_ini_path"),
     NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
     ngx_http_php_ini_path,
     NGX_HTTP_MAIN_CONF_OFFSET,
     0,
     NULL
    },
/*
    {ngx_string("init_by_php"),
     NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
     ngx_http_php_init_inline_phase,
     NGX_HTTP_MAIN_CONF_OFFSET,
     0,
     ngx_http_php_init_inline_handler
    },

    {ngx_string("init_by_php_file"),
     NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
     ngx_http_php_init_file_phase,
     NGX_HTTP_MAIN_CONF_OFFSET,
     0,
     NULL
    },
*/
    {ngx_string("init_worker_by_php"),
     NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
     ngx_http_php_init_worker_inline_phase,
     NGX_HTTP_MAIN_CONF_OFFSET,
     0,
     NULL
    },

    {ngx_string("init_worker_by_php_block"),
     NGX_HTTP_MAIN_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
     ngx_http_php_init_worker_block_phase,
     NGX_HTTP_MAIN_CONF_OFFSET,
     0,
     NULL
    },

    {ngx_string("php_worker_init"),
     NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
     ngx_http_php_init_worker_inline_phase,
     NGX_HTTP_MAIN_CONF_OFFSET,
     0,
     NULL
    },

    {ngx_string("rewrite_by_php_file"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_rewrite_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_rewrite_file_handler
    },

    {ngx_string("rewrite_by_php"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_rewrite_inline_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_rewrite_inline_handler
    },

    {ngx_string("php_rewrite"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_rewrite_inline_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_rewrite_inline_handler
    },

    {ngx_string("access_by_php_file"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_access_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_access_file_handler
    },

    {ngx_string("access_by_php"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_access_inline_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_access_inline_handler
    },

    {ngx_string("php_access"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_access_inline_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_access_inline_handler
    },

    {ngx_string("content_by_php_file"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_content_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_content_file_handler
    },

    {ngx_string("content_by_php"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_content_inline_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_content_inline_handler
    },

    {ngx_string("rewrite_by_php_block"),
     NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
     ngx_http_php_rewrite_block_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     NULL
    },

    {ngx_string("access_by_php_block"),
     NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
     ngx_http_php_access_block_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     NULL
    },

    {ngx_string("content_by_php_block"),
     NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
     ngx_http_php_content_block_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     NULL
    },

    {ngx_string("log_by_php_block"),
     NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
     ngx_http_php_log_block_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     NULL
    },

    {ngx_string("header_filter_by_php_block"),
     NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
     ngx_http_php_header_filter_block_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     NULL
    },

    {ngx_string("body_filter_by_php_block"),
     NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
     ngx_http_php_body_filter_block_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     NULL
    },

    {ngx_string("php_content"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_content_inline_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_content_inline_handler
    },

    {ngx_string("opcode_by_php"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_opcode_inline_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_opcode_inline_handler
    },

    {ngx_string("stack_by_php"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_stack_inline_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_stack_inline_handler
    },

    {ngx_string("log_by_php"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_log_inline_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_log_inline_handler
    },

    {ngx_string("php_log"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_log_inline_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_log_inline_handler
    },

    {ngx_string("header_filter_by_php"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_header_filter_inline_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_header_filter_inline_handler
    },

    {ngx_string("php_header_filter"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_header_filter_inline_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_header_filter_inline_handler
    },

    {ngx_string("body_filter_by_php"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_body_filter_inline_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_body_filter_inline_handler
    },

    {ngx_string("php_body_filter"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_TAKE1,
     ngx_http_php_body_filter_inline_phase,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_body_filter_inline_handler
    },

    {ngx_string("php_keepalive"),
     NGX_HTTP_SRV_CONF|NGX_CONF_TAKE1,
     ngx_http_php_conf_keepalive,
     NGX_HTTP_SRV_CONF_OFFSET,
     0,
     NULL
    },

    {ngx_string("php_socket_keepalive"),
     NGX_HTTP_SRV_CONF|NGX_CONF_TAKE1,
     ngx_http_php_conf_socket_keepalive,
     NGX_HTTP_SRV_CONF_OFFSET,
     0,
     NULL
    },

    {ngx_string("php_socket_buffer_size"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF
          |NGX_HTTP_LIF_CONF|NGX_CONF_TAKE1,
     ngx_conf_set_size_slot,
     NGX_HTTP_LOC_CONF_OFFSET,
     offsetof(ngx_http_php_loc_conf_t, buffer_size),
     NULL
    },

    {ngx_string("php_set"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_2MORE,
     ngx_http_php_set_inline2,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     NULL
    },
/*
#if defined(NDK) && NDK

    {ngx_string("set_by_php"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_2MORE,
     ngx_http_php_set_inline,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_set_inline_handler
    },

    {ngx_string("set_run_by_php"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_2MORE,
     ngx_http_php_set_run_inline,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_set_run_inline_handler
    },

    {ngx_string("set_by_php_file"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_2MORE,
     ngx_http_php_set_file,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_set_file_handler
    },

    {ngx_string("set_run_by_php_file"),
     NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
        |NGX_CONF_2MORE,
     ngx_http_php_set_run_file,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     ngx_http_php_set_run_file_handler
    },

#endif
*/
    ngx_null_command
};

static ngx_http_module_t ngx_http_php_module_ctx = {
    NULL,                          /* preconfiguration */
    ngx_http_php_init,             /* postconfiguration */

    ngx_http_php_create_main_conf, /* create main configuration */
    ngx_http_php_init_main_conf,   /* init main configuration */

    ngx_http_php_create_srv_conf,  /* create server configuration */
    NULL,                          /* merge server configuration */

    ngx_http_php_create_loc_conf,  /* create location configuration */
    ngx_http_php_merge_loc_conf    /* merge location configuration */

};


ngx_module_t ngx_http_php_module = {
    NGX_MODULE_V1,
    &ngx_http_php_module_ctx,       /* module context */
    ngx_http_php_commands,          /* module directives */
    NGX_HTTP_MODULE,                /* module type */
    NULL,                           /* init master */
    NULL,                           /* init module */
    ngx_http_php_init_worker,       /* init process */
    NULL,                           /* init thread */
    NULL,                           /* exit thread */
    ngx_http_php_exit_worker,       /* exit process */
    NULL,                           /* exit master */
    NGX_MODULE_V1_PADDING
};

ngx_http_request_t *ngx_php_request;

#if PHP_MAJOR_VERSION >= 8 
#if PHP_MINOR_VERSION > 0
void (*old_zend_error_cb)(int, zend_string *, const uint32_t, zend_string *);
#else
void (*old_zend_error_cb)(int, const char *, const uint32_t, zend_string *);
#endif
#else
void (*old_zend_error_cb)(int, const char *, const uint, const char *, va_list);
#endif

static ngx_int_t 
ngx_http_php_init(ngx_conf_t *cf)
{
    ngx_http_core_main_conf_t *cmcf;
    ngx_http_php_main_conf_t *pmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
    pmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_php_module);

    ngx_php_request = NULL;

    if (ngx_http_php_handler_init(cmcf, pmcf) != NGX_OK){
        return NGX_ERROR;
    }

    if (pmcf->enabled_header_filter) {
        ngx_http_php_header_filter_init();
    }

    if (pmcf->enabled_body_filter) {
        ngx_http_php_body_filter_init();
    }

    return NGX_OK;
}

static ngx_int_t 
ngx_http_php_handler_init(ngx_http_core_main_conf_t *cmcf, ngx_http_php_main_conf_t *pmcf)
{
    ngx_int_t i;
    ngx_http_handler_pt *h;
    ngx_http_phases phase;
    ngx_http_phases phases[] = {
        NGX_HTTP_POST_READ_PHASE,
        NGX_HTTP_REWRITE_PHASE,
        NGX_HTTP_ACCESS_PHASE,
        NGX_HTTP_CONTENT_PHASE,
        NGX_HTTP_LOG_PHASE
    };
    ngx_int_t phases_c;

    phases_c = sizeof(phases) / sizeof(ngx_http_phases);
    for (i = 0; i < phases_c; i++){
        phase = phases[i];
        switch (phase){
            case NGX_HTTP_POST_READ_PHASE:
                h = ngx_array_push(&cmcf->phases[phase].handlers);
                if (h == NULL){
                    return NGX_ERROR;
                }
                *h = ngx_http_php_post_read_handler;
                break;
            case NGX_HTTP_REWRITE_PHASE:
                if (pmcf->enabled_rewrite_handler){
                    h = ngx_array_push(&cmcf->phases[phase].handlers);
                    if (h == NULL){
                        return NGX_ERROR;
                    }
                    *h = ngx_http_php_rewrite_handler;
                }
                break;
            case NGX_HTTP_ACCESS_PHASE:
                if (pmcf->enabled_access_handler){
                    h = ngx_array_push(&cmcf->phases[phase].handlers);
                    if (h == NULL){
                        return NGX_ERROR;
                    }
                    *h = ngx_http_php_access_handler;
                }
                break;
            case NGX_HTTP_CONTENT_PHASE:
                if (pmcf->enabled_content_handler){
                    h = ngx_array_push(&cmcf->phases[phase].handlers);
                    if (h == NULL){
                        return NGX_ERROR;
                    }
                    *h = ngx_http_php_content_handler;
                }
                if (pmcf->enabled_opcode_handler) {
                    h = ngx_array_push(&cmcf->phases[phase].handlers);
                    if (h == NULL) {
                        return NGX_ERROR;
                    }
                    *h = ngx_http_php_opcode_handler;
                }
                if (pmcf->enabled_stack_handler) {
                    h = ngx_array_push(&cmcf->phases[phase].handlers);
                    if (h == NULL) {
                        return NGX_ERROR;
                    }
                    *h = ngx_http_php_stack_handler;
                }
                break;
            case NGX_HTTP_LOG_PHASE:
                if (pmcf->enabled_log_handler) {
                    h = ngx_array_push(&cmcf->phases[phase].handlers);
                    if (h == NULL) {
                        return NGX_ERROR;
                    }
                    *h = ngx_http_php_log_handler;
                }
                break;
            default:
                break;
        }
    }

    return NGX_OK;
}

static void *
ngx_http_php_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_php_main_conf_t *pmcf;

    pmcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_php_main_conf_t));
    if (pmcf == NULL){
        return NULL;
    }

    pmcf->state = ngx_pcalloc(cf->pool, sizeof(ngx_http_php_state_t));
    if (pmcf->state == NULL){
        return NULL;
    }

    pmcf->state->php_init = 0;
    pmcf->state->php_shutdown = 0;

    pmcf->ini_path.len = 0;
    pmcf->init_code = NGX_CONF_UNSET_PTR;
    pmcf->init_inline_code = NGX_CONF_UNSET_PTR;
    pmcf->init_worker_inline_code = NGX_CONF_UNSET_PTR;

    return pmcf;
}

static char *
ngx_http_php_init_main_conf(ngx_conf_t *cf, void *conf)
{
    return NGX_CONF_OK;
}

static void *
ngx_http_php_create_srv_conf(ngx_conf_t *cf)
{
    ngx_http_php_srv_conf_t *pscf;

    pscf = ngx_pcalloc(cf->pool, sizeof(ngx_http_php_srv_conf_t));
    if (pscf == NULL) {
        return NULL;
    }

    pscf->keepalive_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_php_keepalive_conf_t));
    if (pscf->keepalive_conf == NULL) {
        return NULL;
    }

    pscf->keepalive_conf->max_cached = 0;

    return pscf;
}

static void *
ngx_http_php_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_php_loc_conf_t *plcf;

    plcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_php_loc_conf_t));
    if (plcf == NULL){
        return NGX_CONF_ERROR;
    }

    plcf->document_root.len = 0;

    plcf->rewrite_code = NGX_CONF_UNSET_PTR;
    plcf->rewrite_inline_code = NGX_CONF_UNSET_PTR;

    plcf->access_code = NGX_CONF_UNSET_PTR;
    plcf->access_inline_code = NGX_CONF_UNSET_PTR;

    plcf->content_code = NGX_CONF_UNSET_PTR;
    plcf->content_inline_code = NGX_CONF_UNSET_PTR;

    plcf->opcode_inline_code = NGX_CONF_UNSET_PTR;

    plcf->stack_inline_code = NGX_CONF_UNSET_PTR;

    plcf->log_code = NGX_CONF_UNSET_PTR;
    plcf->log_inline_code = NGX_CONF_UNSET_PTR;

    plcf->header_filter_code = NGX_CONF_UNSET_PTR;
    plcf->header_filter_inline_code = NGX_CONF_UNSET_PTR;
    plcf->body_filter_code = NGX_CONF_UNSET_PTR;
    plcf->body_filter_inline_code = NGX_CONF_UNSET_PTR;

    plcf->enabled_rewrite_inline_compile = 0;
    plcf->enabled_access_inline_compile = 0;
    plcf->enabled_content_inline_compile = 0;
    plcf->enabled_log_inline_compile = 0;
    plcf->enabled_header_filter_inline_compile = 0;
    plcf->enabled_body_filter_inline_compile = 0;

    plcf->send_lowat = NGX_CONF_UNSET_SIZE;
    plcf->buffer_size = NGX_CONF_UNSET_SIZE;

    return plcf;
}

static char *
ngx_http_php_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_core_loc_conf_t  *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    ngx_http_php_loc_conf_t *prev = parent;
    ngx_http_php_loc_conf_t *conf = child;

    conf->document_root.len = clcf->root.len;
    conf->document_root.data = clcf->root.data;

    if ( conf->rewrite_code == NGX_CONF_UNSET_PTR ) {
        conf->rewrite_code = prev->rewrite_code;
    }

    if ( conf->rewrite_inline_code == NGX_CONF_UNSET_PTR ) {
        conf->rewrite_inline_code = prev->rewrite_inline_code;
        conf->rewrite_handler = prev->rewrite_handler;
    }

    if ( conf->access_code == NGX_CONF_UNSET_PTR ) {
        conf->access_code = prev->access_code;
    }
    
    if ( conf->access_inline_code == NGX_CONF_UNSET_PTR ) {
        conf->access_inline_code = prev->access_inline_code;
        conf->access_handler = prev->access_handler;
    }

    if ( conf->content_code == NGX_CONF_UNSET_PTR ){
        conf->content_code = prev->content_code;
    }

    if ( conf->content_inline_code == NGX_CONF_UNSET_PTR ){
        conf->content_inline_code = prev->content_inline_code;
        conf->content_handler = prev->content_handler;
    }

    prev->opcode_inline_code = conf->opcode_inline_code;

    prev->stack_inline_code = conf->stack_inline_code;

    if ( conf->log_code == NGX_CONF_UNSET_PTR ) {
        conf->log_code = prev->log_code;
    }

    if ( conf->log_inline_code == NGX_CONF_UNSET_PTR ) {
        conf->log_inline_code = prev->log_inline_code;
        conf->log_handler = prev->log_handler;
    }

    if ( conf->header_filter_code == NGX_CONF_UNSET_PTR ) {
        conf->header_filter_code = prev->header_filter_code;
    }

    if ( conf->header_filter_inline_code == NGX_CONF_UNSET_PTR ) {
        conf->header_filter_inline_code = prev->header_filter_inline_code;
        conf->header_filter_handler = prev->header_filter_handler;
    }

    if ( conf->body_filter_code == NGX_CONF_UNSET_PTR ) {
        conf->body_filter_code = prev->body_filter_code;
    }

    if ( conf->body_filter_inline_code == NGX_CONF_UNSET_PTR ) {
        conf->body_filter_inline_code = prev->body_filter_inline_code;
        conf->body_filter_handler = prev->body_filter_handler;
    }

    prev->enabled_rewrite_inline_compile = conf->enabled_rewrite_inline_compile;
    prev->enabled_access_inline_compile = conf->enabled_access_inline_compile;
    prev->enabled_content_inline_compile = conf->enabled_content_inline_compile;
    prev->enabled_log_inline_compile = conf->enabled_log_inline_compile;
    prev->enabled_header_filter_inline_compile = conf->enabled_header_filter_inline_compile;
    prev->enabled_body_filter_inline_compile = conf->enabled_body_filter_inline_compile;

    ngx_conf_merge_size_value(conf->buffer_size,
                              prev->buffer_size,
                              (size_t) ngx_pagesize);

    return NGX_CONF_OK;
}

static ngx_int_t 
ngx_http_php_init_worker(ngx_cycle_t *cycle)
{
    ngx_http_php_main_conf_t *pmcf;

    pmcf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_php_module);

    php_ngx_module.ub_write = ngx_http_php_code_ub_write;
    //php_ngx_module.flush = ngx_http_php_code_flush;
    //php_ngx_module.log_message = ngx_http_php_code_log_message;
    //php_ngx_module.register_server_variables = ngx_http_php_code_register_server_variables;
    //php_ngx_module.read_post = ngx_http_php_code_read_post;
    //php_ngx_module.read_cookies = ngx_http_php_code_read_cookies;
    //php_ngx_module.header_handler = ngx_http_php_code_header_handler;

    if (pmcf->ini_path.len != 0){
        php_ngx_module.php_ini_path_override = (char *)pmcf->ini_path.data;
    }
    
    php_ngx_module_init();

#if PHP_MAJOR_VERSION == 7 && PHP_MINOR_VERSION < 2
    zend_startup_module(&php_ngx_module_entry);
#else
    EG(current_module) = &php_ngx_module_entry;
    EG(current_module)->type = MODULE_PERSISTENT;
#endif

    php_ngx_request_init();

    if (pmcf->enabled_init_worker_handler) {
        zend_first_try {
            pmcf = ngx_http_cycle_get_module_main_conf(cycle, ngx_http_php_module);
            zend_eval_stringl_ex(
                pmcf->init_worker_inline_code->code.string,
                ngx_strlen(pmcf->init_worker_inline_code->code.string), 
                NULL, 
                "ngx_php eval code", 
                1
            );
        } zend_end_try();
    }

    //old_zend_error_cb = zend_error_cb;
    //zend_error_cb = ngx_php_error_cb;
    
    //ori_compile_file = zend_compile_file;
    //zend_compile_file = ngx_compile_file;

    //ori_compile_string = zend_compile_string;
    //zend_compile_string = ngx_compile_string;

    //ori_execute_ex = zend_execute_ex;
    //zend_execute_ex = ngx_execute_ex;

    //zend_execute_internal = ngx_execute_internal;
    
    php_impl_ngx_core_init(0 );
    php_impl_ngx_log_init(0 );
    php_impl_ngx_request_init(0 );
    php_impl_ngx_socket_init(0 );
    php_impl_ngx_var_init(0 );
    php_impl_ngx_sockets_init(0 );
    php_impl_ngx_header_init(0 );

    return NGX_OK;
}

static void 
ngx_http_php_exit_worker(ngx_cycle_t *cycle)
{
    php_ngx_request_shutdown();
    php_ngx_module_shutdown();
}

