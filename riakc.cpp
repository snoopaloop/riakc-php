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

extern "C" {
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include <zend_exceptions.h>
#include <php_globals.h>
}

#include <string>
#include <sstream>
#include <stdio.h>
#include <iostream>

using namespace std;

#include "php_riakc.h"

zend_class_entry *riakc_client_ce;
zend_class_entry *riakc_object_ce;

zend_object_handlers riakc_default_handlers;

ZEND_DECLARE_MODULE_GLOBALS(riakc)

#if ZEND_MODULE_API_NO >= 20060613
// 5.2+ globals
static PHP_GINIT_FUNCTION(riakc);
#else
// 5.1- globals
static void riakc_init_globals(zend_riakc_globals* g TSRMLS_DC);
#endif /* ZEND_MODULE_API_NO >= 20060613 */

static function_entry riakc_client_methods[] = {
	PHP_ME(RiakClient, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(RiakClient, get, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(RiakClient, del, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

ZEND_BEGIN_ARG_INFO_EX(arginfo_RiakObject__construct, 0, ZEND_RETURN_VALUE, 3)
	ZEND_ARG_OBJ_INFO(0, client, RiakClient, false)
	ZEND_ARG_INFO(0, bucket)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

static function_entry riakc_object_methods[] = {
	PHP_ME(RiakObject, __construct, arginfo_RiakObject__construct, ZEND_ACC_PUBLIC)
	PHP_ME(RiakObject, getValue, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(RiakObject, setValue, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(RiakObject, bucket, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(RiakObject, key, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(RiakObject, store, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(RiakObject, del, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(RiakObject, contentType, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

static zend_object_value create_new_riakc_client(zend_class_entry *class_type TSRMLS_DC)
{
	riakc_new_object(riakc_client);
}

static zend_object_value create_new_riakc_object(zend_class_entry *class_type TSRMLS_DC)
{
	riakc_new_object(riakc_object);
}

void php_riakc_client_free(void *object TSRMLS_DC)
{
	riakc_client *obj = (riakc_client*)object;

	efree(obj);
}

void php_riakc_object_free(void *object TSRMLS_DC)
{
	riakc_object *obj = (riakc_object*)object;
	
	efree(obj);
}

zend_module_entry riakc_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	PHP_RIAKC_EXTNAME,
	NULL,
	PHP_MINIT(riakc),
	NULL,
	PHP_RINIT(riakc),
	NULL,
	PHP_MINFO(riakc),
	PHP_RIAKC_VERSION,
#if ZEND_MODULE_API_NO >= 20060613
	PHP_MODULE_GLOBALS(riakc),
  PHP_GINIT(riakc),
  NULL,
  NULL,
  STANDARD_MODULE_PROPERTIES_EX,
#else
  STANDARD_MODULE_PROPERTIES
#endif
};

#ifdef COMPILE_DL_RIAKC
extern "C" {
ZEND_GET_MODULE(riakc)
}
#endif

/* {{{ PHP_INI */
PHP_INI_BEGIN()
STD_PHP_INI_ENTRY("riakc.default_host", "localhost", PHP_INI_ALL, OnUpdateString, default_host, zend_riakc_globals, riakc_globals)
STD_PHP_INI_ENTRY("riakc.default_port", "27017", PHP_INI_ALL, OnUpdateString, default_port, zend_riakc_globals, riakc_globals)
STD_PHP_INI_ENTRY("riakc.w", "1", PHP_INI_ALL, OnUpdateLong, w, zend_riakc_globals, riakc_globals)
STD_PHP_INI_ENTRY("riakc.dw", "1", PHP_INI_ALL, OnUpdateLong, dw, zend_riakc_globals, riakc_globals)
STD_PHP_INI_ENTRY("riakc.r", "1", PHP_INI_ALL, OnUpdateLong, r, zend_riakc_globals, riakc_globals)
PHP_INI_END()
/* }}} */

PHP_METHOD(RiakClient, __construct)
{
	char *host;
	int host_len;
	long port;
	
	// FIXME: change to char* and do proper casting
	std::string hostStr;
	std::string portStr;
	stringstream strstream;
	riakc_client *client;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &host, &host_len, &port))
	{
		return;
	}
	
	client = (riakc_client*)zend_object_store_get_object(getThis() TSRMLS_CC);
	
	hostStr = string(reinterpret_cast<const char*>(host));
	
	if(ZEND_NUM_ARGS() > 1)
	{
		strstream << port;
		strstream >> portStr;
	}
	else 
	{
		portStr = "8087";
	}
	
	client->host = &hostStr;
	client->port = &portStr;
	
	client->c = riak::new_client(hostStr, portStr);
}

// FIXME: deal with sibling resolution
// FIXME: how do we deal with the client pointers and such?
PHP_METHOD(RiakClient, get)
{
	zval temp;
	char *bucket;
	char *key;
	
	int bucket_len, key_len;
  long r_val;
  
	riakc_client *client;
  riakc_object *intern;
	
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|l", &bucket, &bucket_len, &key, &key_len, &r_val))
	{
		return;
	}
	
	client = (riakc_client*)zend_object_store_get_object(getThis() TSRMLS_CC);
	
	riak::result_ptr result = client->c->fetch(bucket, key, 1);
	
	if(result->not_found())
	{
		RETURN_FALSE;
	}
	
	object_init_ex(return_value, riakc_object_ce);
  intern = (riakc_object *)zend_objects_get_address(return_value TSRMLS_CC);
	intern->robj = result->choose_sibling(0);
	
  intern->bucket = estrdup(bucket);
  intern->key = estrdup(key);
  intern->bucket_len = bucket_len;
  intern->key_len = key_len;
  
  intern->client = client->c;
}

PHP_METHOD(RiakClient, del)
{
	char *bucket;
	char *key;
	
  long dw_val;
  bool result;
	
	int bucket_len, key_len;
	riakc_client *client;
	
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|l", &bucket, &bucket_len, &key, &key_len, &dw_val))
	{
		return;
	}
	
	if(3 > ZEND_NUM_ARGS())
	{
    dw_val = RiakcGlobal(dw);
	}
	
	client = (riakc_client*)zend_object_store_get_object(getThis() TSRMLS_CC);
	std::string bkt = string(reinterpret_cast<const char*>(bucket));
  std::string k = string(reinterpret_cast<const char*>(key));
  
  result = client->c->del(bkt, k, (int)dw_val);
  
  if(true == result) 
  {
    RETURN_TRUE;
  }
  
  RETURN_FALSE;
	
}


// TODO: create a riakobject init function to handle the population of data
// TODO: move the key/bucket to a property
PHP_METHOD(RiakObject, __construct)
{
	pval **args[4];
	int argc = ZEND_NUM_ARGS();
	
	riakc_object *object;
	char *bucket = NULL;
	char *key;
	int bucket_len;
	int key_len;
		
	if(argc < 2 || FAILURE == zend_get_parameters_array_ex(argc, args) || IS_OBJECT != Z_TYPE_PP(args[0])) 
	{
		return;
	}
	
	object = (riakc_object *)zend_object_store_get_object(getThis() TSRMLS_CC);

	if(IS_OBJECT == Z_TYPE_PP(args[0]) && IS_STRING == Z_TYPE_PP(args[1]) && IS_STRING == Z_TYPE_PP(args[2]))
	{
    
		convert_to_string_ex(args[1]);
		convert_to_string_ex(args[2]);
		
		object->bucket = estrdup(Z_STRVAL_PP(args[1]));
		object->bucket_len = (*args[1])->value.str.len;
		object->key = estrdup(Z_STRVAL_PP(args[2]));
		object->key_len = (*args[2])->value.str.len;
		
    std::string content = "";
    std::string content_type = "";
		
		if(3 < ZEND_NUM_ARGS()) 
		{
      convert_to_string_ex(args[3]);
      content = estrdup(Z_STRVAL_PP(args[3]));
		}
		
		if(4 < ZEND_NUM_ARGS())
		{
		  // we have a content type
		  convert_to_string_ex(args[4]);
      content_type = estrdup(Z_STRVAL_PP(args[4]));
		}
		
		riakc_client *client = (riakc_client*)zend_object_store_get_object((*args[0]) TSRMLS_CC);
    object->client = client->c;
		
    std::string bkt = string(reinterpret_cast<const char*>(object->bucket));
    std::string k = string(reinterpret_cast<const char*>(object->key));
    
    riak::object_ptr o = riak::make_object(bkt, k, content);
    object->robj = o;
    
    riak::riak_metadata md = o->update_metadata();
    
    // set default content type
    md.content_type(content_type);
    
    object->robj->update_metadata(md);
	}
}

PHP_METHOD(RiakObject, getValue)
{
	riakc_object *object = (riakc_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
	
	RETURN_STRING(object->robj->content().value().c_str(), 1);
}

PHP_METHOD(RiakObject, setValue)
{
  char *value;
  int value_len;
  
  riakc_object *object;
  
  if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &value, &value_len))
  {
    return;
  }
  
  object = (riakc_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
  std::string val = string(reinterpret_cast<const char*>(value));
  
  object->robj->update_value(val);
  
}

PHP_METHOD(RiakObject, bucket)
{
  pval **args[1];
	int argc = ZEND_NUM_ARGS();
	
  riakc_object *object = (riakc_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
  
  if(1 > ZEND_NUM_ARGS())
  {
    RETURN_STRING(object->bucket, 1);
  }
  
  if(1 < ZEND_NUM_ARGS() || FAILURE == zend_get_parameters_array_ex(argc, args) || IS_STRING != Z_TYPE_PP(args[0])) 
  {
    return;
  }
  
  convert_to_string_ex(args[0]);
  object->bucket = estrdup(Z_STRVAL_PP(args[0]));
	object->bucket_len = (*args[0])->value.str.len;
	
  RETURN_TRUE;
  
}

PHP_METHOD(RiakObject, key)
{
  pval **args[1];
	int argc = ZEND_NUM_ARGS();
	
  riakc_object *object = (riakc_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
  
  if(1 > ZEND_NUM_ARGS())
  {
    RETURN_STRING(object->key, 1);
  }
  
  if(1 < ZEND_NUM_ARGS() || FAILURE == zend_get_parameters_array_ex(argc, args) || IS_STRING != Z_TYPE_PP(args[0])) 
  {
    return;
  }
  
  convert_to_string_ex(args[0]);
  object->key = estrdup(Z_STRVAL_PP(args[0]));
	object->key_len = (*args[0])->value.str.len;
	
  RETURN_TRUE;
}

PHP_METHOD(RiakObject, contentType)
{
  pval **args[1];
	int argc = ZEND_NUM_ARGS();
  char *contentType;
	
  riakc_object *object = (riakc_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
  
  if(0 == ZEND_NUM_ARGS())
  {
    if("" != object->robj->update_metadata().content_type())
    {
      RETURN_STRING(object->robj->update_metadata().content_type().c_str(), 1);
    }
  }
  
  if(1 < ZEND_NUM_ARGS() || FAILURE == zend_get_parameters_array_ex(argc, args) || IS_STRING != Z_TYPE_PP(args[0])) 
  {
    return;
  }

  convert_to_string_ex(args[0]);
  contentType = estrdup(Z_STRVAL_PP(args[0]));

  std::string ct = string(reinterpret_cast<const char*>(contentType));
  object->content_type = &ct;
  
  //cout << "content type: " << object->robj->update_metadata().content_type().c_str() << endl;
  riak::riak_metadata md = object->robj->metadata();
  md.content_type(ct);
  
  object->robj->update_metadata(md);
	
  RETURN_TRUE;
}

// TODO: hydrate with new values, or return new object or whatever
// Also, store parameters
PHP_METHOD(RiakObject, store)
{
  long w_val;
  long dw_val;
  
  if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ll", &w_val, &dw_val))
  {
    return;
  }
  
  riakc_object *object = (riakc_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
  
  if(2 > ZEND_NUM_ARGS())
  {
    dw_val = RiakcGlobal(dw);
  }
  if(1 > ZEND_NUM_ARGS())
  {
    w_val = RiakcGlobal(w);
  }
  
  riak::store_params sp;
  sp.w((int)w_val).dw((int)dw_val).return_body(true);
  
  riak::object_ptr o = object->robj;
  
  riak::result_ptr result = object->client->store(o, sp);
  object->robj = result->choose_sibling(0);
  
  RETURN_TRUE;
}

// TODO: set dw default
PHP_METHOD(RiakObject, del)
{
  long dw_val;
  bool result;
  
  riakc_object *object;
  
  if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &dw_val))
  {
    return;
  }
  
  if(1 > ZEND_NUM_ARGS())
  {
    dw_val = RiakcGlobal(dw);
  }
  
  object = (riakc_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
  
	std::string bkt = string(reinterpret_cast<const char*>(object->bucket));
  std::string k = string(reinterpret_cast<const char*>(object->key));
  
  result = object->client->del(bkt, k, (int)dw_val);
  
  if(true == result)
  {
    RETURN_TRUE;
  }
  
  RETURN_FALSE;
  
}


#if ZEND_MODULE_API_NO >= 20060613
/* {{{ PHP_GINIT_FUNCTION
 */
static PHP_GINIT_FUNCTION(riakc)
#else
/* {{{ riakc_init_globals
 */
static void riakc_init_globals(zend_riakc_globals *riakc_globals TSRMLS_DC)
#endif /* ZEND_MODULE_API_NO >= 20060613 */
{
  riakc_globals->default_host = "127.0.0.1";
  riakc_globals->default_port = "8087";
  riakc_globals->w = 1;
  riakc_globals->dw = 1;
  riakc_globals->r = 1;
}

PHP_MINIT_FUNCTION(riakc)
{
  zend_class_entry ce;
  	
  #if ZEND_MODULE_API_NO < 20060613
    ZEND_INIT_MODULE_GLOBALS(riakc, riakc_init_globals, NULL);
  #endif
  
  REGISTER_INI_ENTRIES();
	
	INIT_CLASS_ENTRY(ce, "RiakClient", riakc_client_methods);
	ce.create_object = create_new_riakc_client;
	riakc_client_ce = zend_register_internal_class(&ce TSRMLS_CC);
	
	// client properties
	zend_declare_property_string(riakc_client_ce, "host", strlen("host"), "127.0.0.1", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(riakc_client_ce, "port", strlen("port"), "8087", ZEND_ACC_PUBLIC TSRMLS_CC);
	
	INIT_CLASS_ENTRY(ce, "RiakObject", riakc_object_methods);
	ce.create_object = create_new_riakc_object;
	riakc_object_ce = zend_register_internal_class(&ce TSRMLS_CC);
	
	// object properties
	zend_declare_property_string(riakc_object_ce, "content_type", strlen("content_type"), "text/plain", ZEND_ACC_PUBLIC TSRMLS_CC);
	
	memcpy(&riakc_default_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	riakc_default_handlers.clone_obj = NULL;
	
	return SUCCESS;
}

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(riakc) {
  php_info_print_table_start();

  php_info_print_table_header(2, "Riakc Support", "enabled");
  php_info_print_table_row(2, "Version", PHP_RIAKC_VERSION);

  php_info_print_table_end();

  DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(riakc) {
  return SUCCESS;
}
/* }}} */
