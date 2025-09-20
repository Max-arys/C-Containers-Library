#include "testing.h"

// В системе недостаточно памяти.
TEST(BadAllocTest, MapMultisetSet) {
  // При установлении лимита валгринд не сможет получить достаточно памяти и
  // выдаст ошибку.
  if (RUNNING_ON_VALGRIND) {
    GTEST_SKIP() << "Skipping test under Valgrind";
  }
#if defined(__unix__) || defined(__APPLE__)

  // Получаем текущие лимиты
  rlimit old_limit{};
  if (getrlimit(RLIMIT_AS, &old_limit) != 0) {
    FAIL() << "Failed to get current memory limits";
  }

  // Пытаемся установить новый soft limit.
  rlimit new_limit = old_limit;
  new_limit.rlim_cur = 1024 * 1024 * 20;  // 20 МБ

  // Если новый soft limit превышает hard limit - уменьшаем его.
  if (new_limit.rlim_cur > new_limit.rlim_max) {
    new_limit.rlim_cur = new_limit.rlim_max;
    std::cerr << "Warning: Soft limit capped at hard limit ("
              << new_limit.rlim_max / (1024 * 1024) << " MB)\n";
  }

  if (setrlimit(RLIMIT_AS, &new_limit) != 0) {
    FAIL() << "Failed to set new memory limit (no permissions?)";
  }

#define S s21
  // Тесе map.
  S::map<int, int> m1;
  size_t count_items = 150000;
  for (size_t i = 0; i < count_items; ++i) m1.insert(i, i);

  auto lam = [&m1]() { S::map<int, int> m2{m1}; };
  ASSERT_THROW(lam(), std::bad_alloc);

  // Занимаем всю доступную память.
  try {
    for (size_t i = count_items; i < count_items * 2; ++i) m1.insert(i, i);
  } catch (...) {
  }

  // Освобождение памяти для следующих тестов.
  m1.erase(m1.find(1));
  m1.erase(m1.find(2));

  // Тест оператора инициализации из списка инициализации.

  // map.
  auto lam_map = []() {
    S::map<int, int> m2{{1, 11}, {2, 22}, {3, 33}, {4, 44}};
  };
  ASSERT_THROW(lam_map(), std::bad_alloc);

  // multiset.
  auto lam_multiset = []() {
    S::multiset<int> s2{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  };
  ASSERT_THROW(lam_multiset(), std::bad_alloc);

  // set.
  auto lam_set = []() { S::set<int> s2{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}; };
  ASSERT_THROW(lam_set(), std::bad_alloc);

  // Освобождение памяти для следующих тестов.
  for (size_t i = 0; i < 15; ++i) {
    m1.erase(m1.find(i));
  }

  // Тест insert_many.

  // map.
  S::map<int, int> m2;
  using VT = decltype(m2)::value_type;
  ASSERT_THROW(
      m2.insert_many(VT(1, 1), VT(2, 2), VT(3, 3), VT(4, 4), VT(5, 5), VT(6, 6),
                     VT(7, 7), VT(8, 8), VT(9, 9), VT(10, 10)),
      std::bad_alloc);
  EXPECT_TRUE(m2.empty());

  // multiset.
  S::multiset<int> ms2;
  ASSERT_THROW(ms2.insert_many(1, 2, 3, 4, 5, 6, 7, 8, 9, 10), std::bad_alloc);
  EXPECT_TRUE(ms2.empty());

  // set.
  S::set<int> s2;
  ASSERT_THROW(s2.insert_many(1, 2, 3, 4, 5, 6, 7, 8, 9, 10), std::bad_alloc);
  EXPECT_TRUE(s2.empty());

  // Восстанавливаем старый лимит.
  setrlimit(RLIMIT_AS, &old_limit);

#elif defined(_WIN32)
  FAIL() << "This test is not supported on Windows";
#endif
}
