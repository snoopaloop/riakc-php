/**
 * Copyright 2012 Joseph Lambert <joseph.g.lambert@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PHP_RIAKC_H
#define PHP_RIAKC_H

#define PHP_RIAKC_EXTNAME "riakc-php"
#define PHP_RIAKC_VERSION "0.1.0"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <riak_client/cxx/riak_client.hpp>

typedef struct _riakc_client {
	zend_object std;
	
	riak::client_ptr c;
	
	string *host;
	string *port;
	
} riakc_client;

typedef struct _riakc_object {
	zend_object std;
	
	riak::client_ptr client;
	
	char *bucket;
  int bucket_len;
	char *key;
  int key_len;
  
  std::string *content_type;
	
	riak::object_ptr robj;
	
} riakc_object;

#define riakc_new_object(riakc_obj)                    \
  zend_object_value retval;                             \
  riakc_obj *intern;                                    \
  zval *tmp;                                            \
                                                        \
  intern = (riakc_obj*)emalloc(sizeof(riakc_obj));               \
  memset(intern, 0, sizeof(riakc_obj));                          \
                                                                 \
  zend_object_std_init(&intern->std, class_type TSRMLS_CC);      \
  zend_hash_copy(intern->std.properties,                         \
     &class_type->default_properties,                            \
     (copy_ctor_func_t) zval_add_ref,                            \
     (void *) &tmp,                                              \
     sizeof(zval *));                                      \
                                                                 \
  retval.handle = zend_objects_store_put(intern,                 \
     (zend_objects_store_dtor_t) zend_objects_destroy_object,    \
     php_##riakc_obj##_free, NULL TSRMLS_CC);                    \
  retval.handlers = &riakc_default_handlers;                     \
                                                                 \
  return retval;


ZEND_BEGIN_MODULE_GLOBALS(riakc)
char *default_host;
char *default_port;
long w;
long dw;
long r;
ZEND_END_MODULE_GLOBALS(riakc)

#ifdef ZTS
#include <TSRM.h>
# define RiakcGlobal(v) TSRMG(riakc_globals_id, zend_riakc_globals *, v)
#else
# define RiakcGlobal(v) (riakc_globals.v)
#endif

static zend_object_value create_new_riakc_client(zend_class_entry *class_type TSRMLS_DC);
static zend_object_value create_new_riakc_object(zend_class_entry *class_type TSRMLS_DC);
void php_riakc_client_free(void *object TSRMLS_DC);
void php_riakc_object_free(void *object TSRMLS_DC);

PHP_MINIT_FUNCTION(riakc);
PHP_RINIT_FUNCTION(riakc);
PHP_MINFO_FUNCTION(riakc);

PHP_METHOD(RiakClient, __construct);
PHP_METHOD(RiakClient, get);
PHP_METHOD(RiakClient, del);

PHP_METHOD(RiakObject, __construct);
PHP_METHOD(RiakObject, getValue);
PHP_METHOD(RiakObject, setValue);
PHP_METHOD(RiakObject, bucket);
PHP_METHOD(RiakObject, key);
PHP_METHOD(RiakObject, store);
PHP_METHOD(RiakObject, del);
PHP_METHOD(RiakObject, contentType);


#endif