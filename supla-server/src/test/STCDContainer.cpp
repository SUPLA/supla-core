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

#include "test/STCDContainer.h"
#include "log.h"

// static
char STCDContainer::find_by_ptr(void *ptr1, void *ptr2) {
  return ptr1 == ptr2 ? 1 : 0;
}

STCDContainer::STCDContainer() : cdcontainer() { del_count = 0; }

void STCDContainer::cd_delete(cdbase *cd) {
  delete cd;
  del_count++;
}

STCDContainer::~STCDContainer() {}

int STCDContainer::delCount(void) { return del_count; }

cdbase *STCDContainer::findByPtr(void *ptr) { return find(find_by_ptr, ptr); }
