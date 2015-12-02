/*
 ============================================================================
 Name        : cfg.h
 Author      : Przemyslaw Zygmunt p.zygmunt@acsoftware.pl [AC SOFTWARE]
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
 */

#ifndef suplacfg_H_
#define suplacfg_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SCFG_VTYPE_STRING   0
#define SCFG_VTYPE_INT      1
#define SCFG_VTYPE_DOUBLE   2
#define SCFG_VTYPE_BOOLEAN  3

extern char *pidfile_path;
extern char debug_mode;
extern unsigned run_as_daemon;

typedef void (*_func_cfg_callback)(const char* section, const char* name, const char* value);

// section_name must be allocated until call scfg_load
unsigned char scfg_load(int argc, char* argv[], char default_file[]);
void scfg_add_str_param(char *section_name, const char *param_name, char *default_value);
void scfg_add_double_param(char *section_name, const char *param_name, double default_value);
void scfg_add_int_param(char *section_name, const char *param_name, int default_value);
void scfg_add_bool_param(char *section_name, const char *param_name, unsigned char default_value);
void scfg_set_callback(_func_cfg_callback cb);
void scfg_names_free();
void scfg_free(void);

char* scfg_string(unsigned char param_id);
int scfg_int(unsigned char param_id);
float scfg_float(unsigned char param_id);
unsigned char scfg_bool(unsigned char param_id);

int scfg_getuid(unsigned char param_id);
int scfg_getgid(unsigned char param_id);
unsigned char scfg_getid(unsigned char param_id, char *id, unsigned char id_size, unsigned char gen);

#ifdef __cplusplus
}
#endif

#endif /* suplacfg_H_ */
