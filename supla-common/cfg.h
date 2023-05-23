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

#ifndef suplacfg_H_
#define suplacfg_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SCFG_VTYPE_STRING 0
#define SCFG_VTYPE_INT 1
#define SCFG_VTYPE_DOUBLE 2
#define SCFG_VTYPE_BOOLEAN 3

extern char *pidfile_path;
extern char debug_mode;
extern unsigned run_as_daemon;

#ifdef __TEST
extern char *cfg_sql_dir;
#endif /*__TEST*/

typedef void (*_func_cfg_callback)(const char *section, const char *name,
                                   const char *value);

// section_name must be allocated until call scfg_load
void scfg_print_help(const char *app_name);
unsigned char scfg_load(int argc, char *argv[], char default_file[]);
void scfg_add_str_param(int index, char *section_name, const char *param_name,
                        char *default_value);
void scfg_add_double_param(int index, char *section_name,
                           const char *param_name, double default_value);
void scfg_add_int_param(int index, char *section_name, const char *param_name,
                        int default_value);
void scfg_add_bool_param(int index, char *section_name, const char *param_name,
                         unsigned char default_value);
void scfg_set_callback(_func_cfg_callback cb);
void scfg_names_free();
void scfg_free(void);

char *scfg_string(unsigned char param_id);
int scfg_int(unsigned char param_id);
float scfg_float(unsigned char param_id);
unsigned char scfg_bool(unsigned char param_id);

int scfg_getuid(unsigned char param_id);
int scfg_getgid(unsigned char param_id);
unsigned char scfg_getid(unsigned char param_id, char *id,
                         unsigned char id_size, unsigned char gen);

#ifdef __cplusplus
}
#endif

#endif /* suplacfg_H_ */
