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

#include "distributedobjects/dobjects.h"

#include "assert.h"
#include "lck.h"

supla_dobjects::supla_dobjects(
    supla_dobject_repository_factory *repository_factory) {
  this->repository_factory = repository_factory;
  assert(repository_factory != NULL);
  lck = lck_init();
}

supla_dobjects::~supla_dobjects() {
  lck_free(lck);

  delete repository_factory;
  repository_factory = NULL;
}

void supla_dobjects::object_add(supla_dobject *object) {
  objects.push_back(object);
}

void supla_dobjects::access_repository(
    std::function<void(supla_dobject_repository *repository)> on_access) {
  supla_dobject_repository *repo = repository_factory->get_repository();
  if (repo) {
    on_access(repo);
    delete repo;
  }
}

void supla_dobjects::load(void) {
  lock();
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    delete *it;
  }
  objects.clear();

  access_repository([this](supla_dobject_repository *repo) -> void {
    supla_dobject *object = NULL;
    while (NULL != (object = repo->get_next())) {
      object_add(object);
    }
  });

  unlock();
}

void supla_dobjects::load(int id) {
  lock();
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    if ((*it)->get_id() == id) {
      delete *it;
      break;
    }
  }

  access_repository([id, this](supla_dobject_repository *repo) -> void {
    supla_dobject *object = repo->get_object(id);
    if (object) {
      object_add(object);
    }
  });

  unlock();
}

void supla_dobjects::lock(void) { lck_lock(lck); }

void supla_dobjects::unlock(void) { lck_unlock(lck); }

void supla_dobjects::access_object(
    int id, std::function<void(supla_dobject *object)> on_access) {
  if (!id || !on_access) {
    return;
  }

  lock();
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    if ((*it)->get_id() == id) {
      on_access(*it);
      break;
    }
  }
  unlock();
}
