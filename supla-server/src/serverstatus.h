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

#ifndef SERVERSTATUS_H_
#define SERVERSTATUS_H_

#include <stdio.h>
#include <sys/time.h>

class serverstatus {
 private:
  static serverstatus *_globalInstance;
  void *lck;
  char *last_file;
  int last_line;
  struct timeval last_mainloop_heartbeat;

 public:
  static serverstatus *globalInstance(void);
  static void globalInstanceRelease(void);
  serverstatus(void);
  virtual ~serverstatus(void);
  void mainLoopHeartbeat(void);
  void currentLine(const char *file, int line);
  void getStatus(char *buffer, size_t buffer_size);
};

#endif /*SERVERSTATUS_H_*/
