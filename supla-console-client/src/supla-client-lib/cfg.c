/*
 ============================================================================
 Name        : cfg.c
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include "cfg.h"
#include "log.h"
#include "ini.h"
#include "tools.h"


#define MATCH(s, n) strcasecmp(section, s) == 0 && strcasecmp(name, n) == 0

typedef struct {

	unsigned char isset;
	char *section_name;
	char *name;
	int vtype;

	char *cval;
	unsigned char bval;
	double dval;
	int ival;

}TSuplaCfgParam;

typedef struct {

	TSuplaCfgParam **param;
	unsigned char count;
	_func_cfg_callback cb;

}TSuplaCfg;

TSuplaCfg *scfg = NULL;
char *pidfile_path = NULL;
unsigned run_as_daemon = 0;

#ifdef __DEBUG
char debug_mode = 1;
#else
char debug_mode = 0;
#endif

int scfg_ini_handler(void* user, const char* section, const char* name,
                   const char* value)
{
	  int a;

	  if ( value == 0 )
		  return 1;

	  for(a=0;a<scfg->count;a++)
		  if ( MATCH(scfg->param[a]->section_name, scfg->param[a]->name) ) {

			 switch(scfg->param[a]->vtype) {
   			     case SCFG_VTYPE_STRING:

   			    	 if ( scfg->param[a]->cval ) {
   			    		free(scfg->param[a]->cval);
   			    		scfg->param[a]->cval = NULL;
   			    	 }

   			    	 scfg->param[a]->cval = strdup(value);
   			    	 break;
			     case SCFG_VTYPE_INT:
			    	 scfg->param[a]->ival = atoi(value);
   			    	 break;
			     case SCFG_VTYPE_DOUBLE:
			    	 scfg->param[a]->dval = atof(value);
   			    	 break;
			     case SCFG_VTYPE_BOOLEAN:
			    	 if ( strcmp(value, "1") == 0
			    			 || strcmp(value, "Y") == 0
			    			 || strcmp(value, "y") == 0 )
			    		 scfg->param[a]->bval = 1;
			    	 else
			    		 scfg->param[a]->bval = 0;
   			    	 break;
			 }

             scfg->param[a]->isset = 1;
		  }

	  if ( scfg->cb != 0 )
		  scfg->cb(section, name, value);

      return 1;
};

void scfg_set_callback(_func_cfg_callback cb) {

	if ( scfg == NULL ) {
		scfg = malloc(sizeof(TSuplaCfg));
		memset(scfg, 0, sizeof(TSuplaCfg));
	}

	scfg->cb = cb;
}

void scfg_add_param(char *section_name, const char *param_name, unsigned char vtype, char *cval, unsigned char bval, double dval, int ival) {

	TSuplaCfgParam *param;

	assert( section_name != NULL && strlen(section_name) > 0);
	assert( param_name != NULL && strlen(param_name) > 0);

	if ( scfg == NULL ) {
		scfg = malloc(sizeof(TSuplaCfg));
		memset(scfg, 0, sizeof(TSuplaCfg));
	}

	scfg->count++;

	scfg->param = realloc(scfg->param, sizeof(void*)*scfg->count);
	param = malloc(sizeof(TSuplaCfgParam));
	memset(param, 0, sizeof(TSuplaCfgParam));
	param->section_name = section_name;
	param->name = strdup(param_name);
	param->vtype = vtype;
	param->cval = cval == NULL ? NULL : strdup(cval);
	param->bval = bval;
	param->dval = dval;
	param->ival = ival;
	scfg->param[scfg->count-1] = param;

}

void scfg_add_str_param(char *section_name, const char *param_name, char *default_value) {
	scfg_add_param(section_name, param_name, SCFG_VTYPE_STRING, default_value, 0, 0, 0);
}

void scfg_add_double_param(char *section_name, const char *param_name, double default_value) {
	scfg_add_param(section_name, param_name, SCFG_VTYPE_DOUBLE, NULL, 0, default_value, 0);
}

void scfg_add_int_param(char *section_name, const char *param_name, int default_value) {
	scfg_add_param(section_name, param_name, SCFG_VTYPE_INT, NULL, 0, 0, default_value);
}

void scfg_add_bool_param(char *section_name, const char *param_name, unsigned char default_value) {
	scfg_add_param(section_name, param_name, SCFG_VTYPE_BOOLEAN, NULL, default_value, 0, 0);
}

unsigned char scfg_load(int argc, char* argv[], char default_file[]) {

	   int a;
	   char *cfg_path = NULL;

	   for(a=0;a<argc;a++) {
	           if ( strcmp("-c", argv[a]) == 0 && a<argc-1 ) {
	                   cfg_path = argv[a+1];
	                   a++;
	           } else if ( strcmp("-p", argv[a]) == 0 && a<argc-1 && strlen(argv[a+1]) < 1024 ) {
	        	       pidfile_path = strdup(argv[a+1]);
	                           a++;
	           } else if ( strcmp("-d", argv[a]) == 0 ) {
	                   run_as_daemon = 1;
	           } else if ( strcmp("-D", argv[a]) == 0 ) {
	        	       debug_mode = 1;
	           }
	   }

	   if ( cfg_path == NULL ) {
	        cfg_path = default_file;
	   }

	   if ( st_file_exists(cfg_path) == 0 ) {
	          supla_log(LOG_ERR, "Configuration file not found: %s", cfg_path == NULL ? "" : cfg_path);
	          return 0;
	   }

	   if ( ini_parse(cfg_path, scfg_ini_handler, NULL) < 0 ) {
		    supla_log(LOG_ERR, "Config read error");
	        return 0;
	   }


	   return 1;
}

void scfg_names_free() {

	int a;
	for(a=0;a<scfg->count;a++)
	   if ( scfg->param[a]->name != NULL ) {
		   free(scfg->param[a]->name);
		   scfg->param[a]->name = NULL;
	   }
}

void scfg_free(void) {

	int a;

	if ( scfg == NULL ) return;

	for(a=0;a<scfg->count;a++) {

		if ( scfg->param[a]->name != NULL )
			free(scfg->param[a]->name);

		if ( scfg->param[a]->cval != NULL ) {
			free(scfg->param[a]->cval);
		}

		free(scfg->param[a]);

	}

	free(scfg->param);
	free(scfg);
	scfg = NULL;

	if ( pidfile_path ) {
		free(pidfile_path);
		pidfile_path = NULL;
	}
}

char* scfg_string(unsigned char param_id) {
	assert(scfg!=NULL && scfg->count > param_id);
	return scfg->param[param_id]->cval;
}

int scfg_int(unsigned char param_id) {
	assert(scfg!=NULL && scfg->count > param_id);
	return scfg->param[param_id]->ival;
}

double scfg_double(unsigned char param_id) {
	assert(scfg!=NULL && scfg->count > param_id);
	return scfg->param[param_id]->dval;
}

unsigned char scfg_bool(unsigned char param_id) {
	assert(scfg!=NULL && scfg->count > param_id);
	return scfg->param[param_id]->bval;
}

int scfg_getuid(unsigned char param_id) {

	char *name = scfg_string(param_id);

	if( name
		&& strlen(name) > 0 ) {

        struct passwd *pwd = getpwnam(name); /* don't free, see man getpwnam() for details */
        if( pwd )
        	return pwd->pw_uid;
    }
  return getuid();

}

int scfg_getgid(unsigned char param_id) {

	char *name = scfg_string(param_id);

	if( name
		&& strlen(name) > 0 ) {

        struct group *gr = getgrnam(name); /* don't free, see man getgrnam() for details */
        if( gr )
        	return gr->gr_gid;
    }

	return getgid();
}

unsigned char scfg_getid(unsigned char param_id, char *id, unsigned char id_size, unsigned char gen) {

	return 0;
}

