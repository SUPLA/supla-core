/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "cfg.h"
#include <assert.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "ini.h"
#include "log.h"
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
} TSuplaCfgParam;

typedef struct {
  TSuplaCfgParam **param;
  unsigned char count;
  _func_cfg_callback cb;
} TSuplaCfg;

TSuplaCfg *scfg = NULL;
char *pidfile_path = NULL;
unsigned run_as_daemon = 0;

#ifdef __TEST
char *cfg_sql_dir = NULL;
#endif /*__TEST*/

#ifdef __DEBUG
char debug_mode = 1;
#else
char debug_mode = 0;
#endif

int scfg_ini_handler(void *user, const char *section, const char *name,
                     const char *value) {
  int a;

  if (value == 0) return 1;

  for (a = 0; a < scfg->count; a++)
    if (MATCH(scfg->param[a]->section_name, scfg->param[a]->name)) {
      switch (scfg->param[a]->vtype) {
        case SCFG_VTYPE_STRING:

          if (scfg->param[a]->cval) {
            free(scfg->param[a]->cval);
            scfg->param[a]->cval = NULL;
          }

          scfg->param[a]->cval = strdup(value);
          break;
        case SCFG_VTYPE_INT:
          scfg->param[a]->ival = atoi(value);
          break;
        case SCFG_VTYPE_DOUBLE:
#ifdef __ANDROID__
          scfg->param[a]->dval = strtod(value, NULL);
#else
          scfg->param[a]->dval = atof(value);
#endif
          break;
        case SCFG_VTYPE_BOOLEAN:
          if (strcmp(value, "1") == 0 || strcmp(value, "Y") == 0 ||
              strcmp(value, "y") == 0)
            scfg->param[a]->bval = 1;
          else
            scfg->param[a]->bval = 0;
          break;
      }

      scfg->param[a]->isset = 1;
    }

  if (scfg->cb != 0) scfg->cb(section, name, value);

  return 1;
}

void scfg_set_callback(_func_cfg_callback cb) {
  if (scfg == NULL) {
    scfg = malloc(sizeof(TSuplaCfg));

    if (scfg == NULL) return;

    memset(scfg, 0, sizeof(TSuplaCfg));
  }

  scfg->cb = cb;
}

void scfg_add_param(char *section_name, const char *param_name,
                    unsigned char vtype, char *cval, unsigned char bval,
                    double dval, int ival) {
  TSuplaCfgParam *param;
  TSuplaCfgParam **scfg_param = NULL;

  assert(section_name != NULL && strnlen(section_name, 128) > 0);
  assert(param_name != NULL && strnlen(param_name, 128) > 0);

  if (scfg == NULL) {
    scfg = malloc(sizeof(TSuplaCfg));

    if (scfg == NULL) return;

    memset(scfg, 0, sizeof(TSuplaCfg));
  }

  param = malloc(sizeof(TSuplaCfgParam));

  if (param == NULL) return;

  scfg_param = realloc(scfg->param, sizeof(void *) * (scfg->count + 1));

  if (scfg_param == NULL) {
    free(param);

  } else {
    memset(param, 0, sizeof(TSuplaCfgParam));

    scfg->count++;
    scfg->param = scfg_param;

    param->section_name = section_name;
    param->name = strdup(param_name);
    param->vtype = vtype;
    param->cval = cval == NULL ? NULL : strdup(cval);
    param->bval = bval;
    param->dval = dval;
    param->ival = ival;
    scfg->param[scfg->count - 1] = param;
  }
}

void scfg_add_str_param(char *section_name, const char *param_name,
                        char *default_value) {
  scfg_add_param(section_name, param_name, SCFG_VTYPE_STRING, default_value, 0,
                 0, 0);
}

void scfg_add_double_param(char *section_name, const char *param_name,
                           double default_value) {
  scfg_add_param(section_name, param_name, SCFG_VTYPE_DOUBLE, NULL, 0,
                 default_value, 0);
}

void scfg_add_int_param(char *section_name, const char *param_name,
                        int default_value) {
  scfg_add_param(section_name, param_name, SCFG_VTYPE_INT, NULL, 0, 0,
                 default_value);
}

void scfg_add_bool_param(char *section_name, const char *param_name,
                         unsigned char default_value) {
  scfg_add_param(section_name, param_name, SCFG_VTYPE_BOOLEAN, NULL,
                 default_value, 0, 0);
}

void scfg_print_help(const char *app_name) {
  printf("The application supports the following options:\n");
  printf("    -c <config file>\n");
  printf("    -p <pid file>\n");
#ifdef __TEST
  printf("    --sqldir    Specifies the path of the sql script directory\n");
#endif /**/
  printf("    -d                  run in daemon mode\n");
#ifdef __DEBUG
  printf("    --debug-off         run in normal mode\n");
#else
  printf("    -D                  run in debug mode\n");
#endif /*__DEBUG*/
  printf("    -h                  prints this help\n");
  printf("\n");
}

unsigned char scfg_load(int argc, char *argv[], char default_file[]) {
  int a;
  char *cfg_path = NULL;

  for (a = 0; a < argc; a++) {
    if (strcmp("-c", argv[a]) == 0 && a < argc - 1) {
      cfg_path = argv[a + 1];
      a++;
    } else if (strcmp("-p", argv[a]) == 0 && a < argc - 1 &&
               strnlen(argv[a + 1], 1030) < 1024) {
      pidfile_path = strdup(argv[a + 1]);
      a++;
    } else if (strcmp("-d", argv[a]) == 0) {
      run_as_daemon = 1;
#ifdef __TEST
    } else if (strcmp("--sqldir", argv[a]) == 0 && a < argc - 1) {
      cfg_sql_dir = strdup(argv[a + 1]);
#endif /*__TEST*/
#ifdef __DEBUG
    } else if (strcmp("--debug-off", argv[a]) == 0) {
      debug_mode = 0;
#else
    } else if (strcmp("-D", argv[a]) == 0) {
      debug_mode = 1;
#endif /*__DEBUG*/
    } else if (strcmp("-h", argv[a]) == 0) {
      scfg_print_help(argv[0]);
      return 0;
    }
  }

  if (cfg_path == NULL) {
    cfg_path = default_file;
  }

  if (st_file_exists(cfg_path) == 0) {
    supla_log(LOG_ERR, "Configuration file not found: %s",
              cfg_path == NULL ? "" : cfg_path);
    return 0;
  }

  if (ini_parse(cfg_path, scfg_ini_handler, NULL) < 0) {
    supla_log(LOG_ERR, "Config read error");
    return 0;
  }

  return 1;
}

void scfg_names_free() {
  int a;
  for (a = 0; a < scfg->count; a++)
    if (scfg->param[a]->name != NULL) {
      free(scfg->param[a]->name);
      scfg->param[a]->name = NULL;
    }
}

void scfg_free(void) {
  int a;

  if (scfg == NULL) return;

  for (a = 0; a < scfg->count; a++) {
    if (scfg->param[a]->name != NULL) free(scfg->param[a]->name);

    if (scfg->param[a]->cval != NULL) {
      free(scfg->param[a]->cval);
    }

    free(scfg->param[a]);
  }

  free(scfg->param);
  free(scfg);
  scfg = NULL;

  if (pidfile_path) {
    free(pidfile_path);
    pidfile_path = NULL;
  }

#ifdef __TEST
  if (cfg_sql_dir) {
    free(cfg_sql_dir);
    cfg_sql_dir = NULL;
  }
#endif /*__TEST*/
}

char *scfg_string(unsigned char param_id) {
  assert(scfg != NULL && scfg->count > param_id);
  return scfg->param[param_id]->cval;
}

int scfg_int(unsigned char param_id) {
  assert(scfg != NULL && scfg->count > param_id);
  return scfg->param[param_id]->ival;
}

double scfg_double(unsigned char param_id) {
  assert(scfg != NULL && scfg->count > param_id);
  return scfg->param[param_id]->dval;
}

unsigned char scfg_bool(unsigned char param_id) {
  assert(scfg != NULL && scfg->count > param_id);
  return scfg->param[param_id]->bval;
}

int scfg_getuid(unsigned char param_id) {
  char *name = scfg_string(param_id);

  if (name && strnlen(name, 256) > 0) {
    struct passwd *pwd =
        getpwnam(name); /* don't free, see man getpwnam() for details */
    if (pwd) return pwd->pw_uid;
  }
  return getuid();
}

int scfg_getgid(unsigned char param_id) {
  char *name = scfg_string(param_id);

  if (name && strnlen(name, 256) > 0) {
    struct group *gr =
        getgrnam(name); /* don't free, see man getgrnam() for details */
    if (gr) return gr->gr_gid;
  }

  return getgid();
}

unsigned char scfg_getid(unsigned char param_id, char *id,
                         unsigned char id_size, unsigned char gen) {
  return 0;
}
