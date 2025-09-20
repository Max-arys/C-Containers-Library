#include "testing.h"

// Тесты для Identity
TEST(FunctorsTest, IdentityReturnsSameValue) {
  s21::Identity identity;
  int value = 42;
  const int& result = identity(value);
  EXPECT_EQ(result, value);
  EXPECT_EQ(&result, &value);
}

// Тесты для Select1st
TEST(FunctorsTest, Select1stReturnsFirstElement) {
  s21::Select1st select1st;
  std::pair<int, double> p{10, 3.14};
  const int& result = select1st(p);
  EXPECT_EQ(result, 10);
  EXPECT_EQ(&result, &p.first);
}

// Тесты для less
TEST(FunctorsTest, LessComparesCorrectly) {
  s21::less<int> comparator;
  EXPECT_TRUE(comparator(1, 2));
  EXPECT_FALSE(comparator(2, 1));
  EXPECT_FALSE(comparator(1, 1));
}

// Дополнительные тесты для less с разными типами
TEST(FunctorsTest, LessWorksWithDifferentTypes) {
  s21::less<double> double_comparator;
  EXPECT_TRUE(double_comparator(1.5, 2.5));
  EXPECT_FALSE(double_comparator(2.5, 1.5));
}

// Тест на constexpr для less (если нужна проверка времени компиляции)
TEST(FunctorsTest, LessIsConstexpr) {
  constexpr s21::less<int> comparator;
  static_assert(comparator(1, 2), "1 should be less than 2");
  static_assert(!comparator(2, 1), "2 should not be less than 1");
}