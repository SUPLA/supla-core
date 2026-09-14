// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "SafeArrayTest.h"
#include "gtest/gtest.h"  // NOLINT
#include "safearray.h"    // NOLINT

char safe_array_test_find_cnd(void *ptr, void *user_param) {
  return ptr == user_param ? 1 : 0;
}

char safe_array_test_del_cnd1(void *ptr) { return 0; }

char safe_array_test_del_cnd2(void *ptr) { return 1; }

namespace {

class SafeArrayTest : public ::testing::Test {
 protected:
};

TEST_F(SafeArrayTest, all) {
  void *arr = safe_array_init();
  ASSERT_FALSE(arr == NULL);

  ASSERT_EQ(0, safe_array_add(arr, (void *)10));
  ASSERT_EQ(1, safe_array_add(arr, (void *)20));
  ASSERT_EQ(2, safe_array_add(arr, (void *)30));
  ASSERT_EQ(3, safe_array_add(arr, (void *)40));

  ASSERT_EQ(4, safe_array_count(arr));

  safe_array_delete(arr, 1);
  ASSERT_EQ(3, safe_array_count(arr));

  safe_array_remove(arr, (void *)20);
  ASSERT_EQ(3, safe_array_count(arr));

  safe_array_remove(arr, (void *)30);
  ASSERT_EQ(2, safe_array_count(arr));

  ASSERT_EQ(1, safe_array_find(arr, (void *)40));

  ASSERT_TRUE((void *)10 == safe_array_get(arr, 0));

  ASSERT_EQ(2, safe_array_add(arr, (void *)50));
  ASSERT_EQ(3, safe_array_add(arr, (void *)60));
  ASSERT_EQ(4, safe_array_add(arr, (void *)70));

  ASSERT_TRUE((void *)10 == safe_array_pop(arr));
  ASSERT_TRUE((void *)40 == safe_array_pop(arr));
  ASSERT_TRUE(safe_array_findcnd(arr, &safe_array_test_find_cnd, (void *)60));
  ASSERT_FALSE(safe_array_findcnd(arr, &safe_array_test_find_cnd, (void *)100));

  safe_array_clean(arr, &safe_array_test_del_cnd1);
  ASSERT_EQ(3, safe_array_count(arr));

  safe_array_clean(arr, &safe_array_test_del_cnd2);
  ASSERT_EQ(0, safe_array_count(arr));

  safe_array_free(arr);
}

TEST_F(SafeArrayTest, move_to_begin) {
  void *arr = safe_array_init();
  ASSERT_FALSE(arr == NULL);

  ASSERT_EQ(0, safe_array_add(arr, (void *)10));
  ASSERT_EQ(1, safe_array_add(arr, (void *)20));
  ASSERT_EQ(2, safe_array_add(arr, (void *)30));
  ASSERT_EQ(3, safe_array_add(arr, (void *)40));

  ASSERT_EQ(10, (long long)safe_array_get(arr, 0));
  ASSERT_EQ(20, (long long)safe_array_get(arr, 1));
  ASSERT_EQ(30, (long long)safe_array_get(arr, 2));
  ASSERT_EQ(40, (long long)safe_array_get(arr, 3));

  safe_array_move_to_begin(arr, 2);

  ASSERT_EQ(30, (long long)safe_array_get(arr, 0));
  ASSERT_EQ(10, (long long)safe_array_get(arr, 1));
  ASSERT_EQ(20, (long long)safe_array_get(arr, 2));
  ASSERT_EQ(40, (long long)safe_array_get(arr, 3));

  safe_array_move_to_begin(arr, 3);

  ASSERT_EQ(40, (long long)safe_array_get(arr, 0));
  ASSERT_EQ(30, (long long)safe_array_get(arr, 1));
  ASSERT_EQ(10, (long long)safe_array_get(arr, 2));
  ASSERT_EQ(20, (long long)safe_array_get(arr, 3));

  safe_array_move_to_begin(arr, 1);

  ASSERT_EQ(30, (long long)safe_array_get(arr, 0));
  ASSERT_EQ(40, (long long)safe_array_get(arr, 1));
  ASSERT_EQ(10, (long long)safe_array_get(arr, 2));
  ASSERT_EQ(20, (long long)safe_array_get(arr, 3));

  safe_array_move_to_begin(arr, 0);

  ASSERT_EQ(30, (long long)safe_array_get(arr, 0));
  ASSERT_EQ(40, (long long)safe_array_get(arr, 1));
  ASSERT_EQ(10, (long long)safe_array_get(arr, 2));
  ASSERT_EQ(20, (long long)safe_array_get(arr, 3));

  safe_array_free(arr);
}
}  // namespace
