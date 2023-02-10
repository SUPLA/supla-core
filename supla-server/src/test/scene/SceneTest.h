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

#ifndef SCENETEST_H_
#define SCENETEST_H_

#include <doubles/device/ValueGetterMock.h>
#include <scene/scene_asynctask.h>
#include "asynctask/AsyncTaskTest.h"
#include "doubles/scene/SceneActionExecutorMock.h"
#include "gtest/gtest.h"

namespace testing {

class SceneTest : public AsyncTaskTest {
 private:
 protected:
  SceneActionExecutorMock *action_executor;
  ValueGetterMock *value_getter;
  supla_scene_operations *operations;

 public:
  virtual void SetUp();
  SceneTest();
  virtual ~SceneTest();
};

} /* namespace testing */

#endif /* SCENETEST_H_ */
