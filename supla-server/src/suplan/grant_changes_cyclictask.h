// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SUPLA_SUPLAN_GRANT_CHANGES_CYCLICTASK_H_
#define SUPLA_SUPLAN_GRANT_CHANGES_CYCLICTASK_H_
#include <vector>

#include "cyclictasks/abstract_cyclictask.h"

// Runs in the existing cyclic agent, not a Device tick or a new worker.
class supla_suplan_grant_changes_cyclictask : public supla_abstract_cyclictask {
 protected:
  unsigned int task_interval_sec() override { return 5; }
  void run(const std::vector<supla_user *> *users,
           supla_abstract_db_access_provider *) override;
 public:
  bool db_access_needed() override { return false; }
};
#endif  // SUPLA_SUPLAN_GRANT_CHANGES_CYCLICTASK_H_
