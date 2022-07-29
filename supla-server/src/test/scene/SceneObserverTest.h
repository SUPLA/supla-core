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

#ifndef SCENEOBSERVERTEST_H_
#define SCENEOBSERVERTEST_H_

#include "SceneTest.h"
#include "asynctask/abstract_asynctask_observer.h"
#include "gtest/gtest.h"

namespace testing {

class SceneObserverTest : public SceneTest,
                          public supla_abstract_asynctask_observer {
 private:
 protected:
  int start_count;
  int finish_count;
  struct timeval finished_at;

 public:
  virtual ~SceneObserverTest();
  virtual void SetUp();
  virtual void TearDown();
  virtual void on_asynctask_started(supla_abstract_asynctask *asynctask);
  virtual void on_asynctask_finished(supla_abstract_asynctask *asynctask);
};

} /* namespace testing */

#endif /* SCENEOBSERVERTEST_H_ */
