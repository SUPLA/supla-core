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

#include "clientobjcontaineritem.h"
#include <stdio.h>   // NOLINT
#include <stdlib.h>  // NOLINT
#include <string.h>  // NOLINT
#include "clientobjcontainer.h"


supla_client_objcontainer_item::supla_client_objcontainer_item(
    supla_client_objcontainer *Container, int Id, const char *Caption)
    : supla_objcontainer_item(Container, Id) {
  this->Caption = NULL;
  setCaption(Caption);
}

supla_client_objcontainer_item::~supla_client_objcontainer_item(void) {
  setCaption(NULL);
}

supla_client_objcontainer *supla_client_objcontainer_item::getContainer(void) {
  return static_cast<supla_client_objcontainer *>(
      supla_objcontainer_item::getContainer());
}

void supla_client_objcontainer_item::setCaption(const char *Caption) {
  if (this->Caption != NULL) {
    free(this->Caption);
    this->Caption = NULL;
  }

  if (Caption) {
    this->Caption = strdup(Caption);
  }
}

char *supla_client_objcontainer_item::getCaption(void) { return Caption; }

void supla_client_objcontainer_item::proto_get_caption(
    char *Caption, unsigned _supla_int_t *CaptionSize, unsigned int MaxSize) {
  if (getCaption()) {
    snprintf(Caption, MaxSize, "%s", getCaption());
    *CaptionSize = strnlen(Caption, MaxSize - 1) + 1;
  } else {
    *CaptionSize = 1;
    Caption[0] = 0;
  }
}
