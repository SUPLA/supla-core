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

#ifndef SceneRepositoryTest_H_
#define SceneRepositoryTest_H_

#include "integration/IntegrationTest.h"
#include "scene/scene_repository.h"

namespace testing {

class SceneRepositoryTest : public IntegrationTest, public Test {
 protected:
  supla_scene_repository *repo;
  void VerifyScene1(supla_scene *scene);
  void VerifyScene2(supla_scene *scene);
  void VerifyScene3(supla_scene *scene);
  void VerifyScene4(supla_scene *scene);
  void VerifyScene5(supla_scene *scene);
  void VerifyScene6(supla_scene *scene);

 public:
  SceneRepositoryTest();
  virtual ~SceneRepositoryTest();
  virtual void SetUp();
  virtual void TearDown();
};

} /* namespace testing */

#endif /* SceneRepositoryTest_H_ */
