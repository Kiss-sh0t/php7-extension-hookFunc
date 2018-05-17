/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_hello.h"
#include "inttypes.h"

/* If you declare any globals in php_hello.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(hello)
*/
typedef void (*php_func)(INTERNAL_FUNCTION_PARAMETERS);

/* True global resources - no need for thread safety here */
static int le_hello;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("hello.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_hello_globals, hello_globals)
    STD_PHP_INI_ENTRY("hello.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_hello_globals, hello_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_hello_compiled(string arg)
   Return a string to confirm that the module is compiled in 
PHP_FUNCTION(confirm_hello_compiled)
{
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "hello", arg);

	RETURN_STR(strg);
}*/
//
static struct {
	php_func ini_get;
} origin_funcs;

#define INIT_O_FUNC(m) (origin_funcs.m)


//PHP7 user defined hook func
static int do_fcall_handle(ZEND_OPCODE_HANDLER_ARGS){
	//get file name	
	char *cgi_name = (char*)zend_get_executed_filename(TSRMLS_C);
	php_printf("[!] In file %s, ",cgi_name);
	
	//get arg1
	

	zend_string *funcName = EG(current_execute_data)->call->func->common.function_name;
	//if(strcmp(ZSTR_VAL(funcName), "system")==0){
		php_printf("hooked `%s`, ",ZSTR_VAL(funcName));

		int arg_count = EG(current_execute_data)->call->This.u2.num_args;
		for(int i=0; i<arg_count; i++){
			//test var method
			zval *temp = EG(vm_stack_top)-(i+1);//first
			
			int method = temp->u1.v.type;
			switch (method){//copied from .gdbinit
				case 0:			
					php_printf("arg%d is UNDEF, ", i+1);
					break;
				case 1:
					php_printf("arg%d is NULL, ", i+1);
					break;
				case 2:
					php_printf("arg%d is `bool: false`, ", i+1);
					break;
				case 3:
					php_printf("arg%d is `bool: true`, ", i+1);
					break;
				case 4:
					php_printf("arg%d is `long: %ld`, ", i+1, temp->value.lval);
					break;
				case 5:
					php_printf("arg%d is `double: %f`, ", i+1, temp->value.dval);
					break;
				case 6:
					php_printf("arg%d is `string: %s`, ", i+1, temp->value.str->val);
					break;
				default:
					php_printf("arg%d is not process yet, ", i+1);
			}	
		}
		php_printf("hook end;\n");		
	//}
	
	return ZEND_USER_OPCODE_DISPATCH;
}


//PHP7 PHP_FUNCTION hook
static void php_override_func(const char* name, size_t len, php_func handler, php_func *stash){
        zend_function *func;
        //find target func by name
        if((func= zend_hash_str_find_ptr(CG(function_table), name, strlen(name)))!= NULL){
		if(stash){
			*stash = func->internal_function.handler;
		}
		func->internal_function.handler = handler;	
	}
}

PHP_FUNCTION(fake_ini_get)
{
    	
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}
	//output arg
	zend_error(E_WARNING, arg);
	
	//EG(execute_data);
	INIT_O_FUNC(ini_get)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		
}

//PHP_FUNCTION(hello_world)
//{
//	php_printf("Hello World!");
//	RETURN_TRUE;
//}



/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_hello_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_hello_init_globals(zend_hello_globals *hello_globals)
{
	hello_globals->global_value = 0;
	hello_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(hello)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
		
	//php_override_func("ini_get", sizeof("ini_get"), PHP_FN(fake_ini_get), &INIT_O_FUNC(ini_get));
	zend_set_user_opcode_handler(ZEND_DO_ICALL, do_fcall_handle);
	//zend_set_user_opcode_handler(ZEND_DO_FCALL_BY_NAME, do_fcall_handle);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(hello)
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
PHP_RINIT_FUNCTION(hello)
{
#if defined(COMPILE_DL_HELLO) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(hello)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(hello)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "hello support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ hello_functions[]
 *
 * Every user visible function must have an entry in hello_functions[].
 */
const zend_function_entry hello_functions[] = {
	//PHP_FE(confirm_hello_compiled,	NULL)		/* For testing, remove later. */
	//PHP_FE(hello_world, NULL)
	PHP_FE_END	/* Must be the last line in hello_functions[] */
};
/* }}} */

/* {{{ hello_module_entry
 */
zend_module_entry hello_module_entry = {
	STANDARD_MODULE_HEADER,
	"hello",
	hello_functions,
	PHP_MINIT(hello),
	PHP_MSHUTDOWN(hello),
	PHP_RINIT(hello),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(hello),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(hello),
	PHP_HELLO_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_HELLO
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(hello)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
