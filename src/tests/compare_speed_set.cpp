/**
 * @file Сравнение производительности s21::set и std::set.
 * Использование pool_allocator в s21::set дает выигрыш в скорости по сравнению
 * с std::set ~20%, а разница в скорости s21::set с pool_allocator и с
 * стандартным allocator достигает 40%.
 *
 * Использование оптимизации -O1 ускоряет работу примерно в 3 раза для обоих
 * множеств.
 */

#include <chrono>
#include <random>

#include "testing.h"  // ваш класс set

using namespace std::chrono;
// using Set_type = s21::set<int>;
using Set_type = s21::set<int, std::less<int>, s21::pool_allocator<int>>;

// using Std_set_type = std::set<int>;
using Std_set_type = std::set<int, std::less<int>, s21::pool_allocator<int>>;

class PerformanceTest : public ::testing::Test {
 protected:
  static constexpr size_t kNumElements = 500'000;

  std::vector<int> GenerateRandomValues(size_t n) {
    std::vector<int> values(n);
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dist(1, 1'000'000);
    for (size_t i = 0; i < n; ++i) {
      values[i] = dist(gen);
    }
    return values;
  }
};

TEST_F(PerformanceTest, InsertPerformance) {
  auto values = GenerateRandomValues(kNumElements);

  Set_type s21_set;
  auto start = high_resolution_clock::now();
  for (const auto& val : values) s21_set.insert(val);
  auto end = high_resolution_clock::now();
  auto s21_duration = duration_cast<milliseconds>(end - start).count();

  Std_set_type std_set;
  start = high_resolution_clock::now();
  for (const auto& val : values) std_set.insert(val);
  end = high_resolution_clock::now();
  auto std_duration = duration_cast<milliseconds>(end - start).count();

  std::cout << "Insert: s21_set = " << s21_duration
            << " ms, std::set = " << std_duration << " ms\n";
}

TEST_F(PerformanceTest, ErasePerformance) {
  auto values = GenerateRandomValues(kNumElements);

  Set_type s21_set;
  for (const auto& val : values) s21_set.insert(val);
  std::set<int> std_set(values.begin(), values.end());

  auto start = high_resolution_clock::now();
  for (const auto& val : values) s21_set.erase(val);
  auto end = high_resolution_clock::now();
  auto s21_duration = duration_cast<milliseconds>(end - start).count();

  start = high_resolution_clock::now();
  for (const auto& val : values) std_set.erase(val);
  end = high_resolution_clock::now();
  auto std_duration = duration_cast<milliseconds>(end - start).count();

  std::cout << "Erase: s21_set = " << s21_duration
            << " ms, std::set = " << std_duration << " ms\n";
}

TEST_F(PerformanceTest, CopyConstructorPerformance) {
  auto values = GenerateRandomValues(kNumElements);

  Set_type s21_set;
  for (const auto& val : values) s21_set.insert(val);

  std::set<int> std_set(values.begin(), values.end());

  auto start = high_resolution_clock::now();
  Set_type s21_copy(s21_set);
  auto end = high_resolution_clock::now();
  auto s21_duration = duration_cast<milliseconds>(end - start).count();

  start = high_resolution_clock::now();
  std::set<int> std_copy(std_set);
  end = high_resolution_clock::now();
  auto std_duration = duration_cast<milliseconds>(end - start).count();

  std::cout << "Copy constructor: s21_set = " << s21_duration
            << " ms, std::set = " << std_duration << " ms\n";
}

TEST_F(PerformanceTest, MergePerformance) {
  auto values1 = GenerateRandomValues(kNumElements);
  auto values2 = GenerateRandomValues(kNumElements);

  Set_type s21_set1;
  Set_type s21_set2;
  for (const auto& val : values1) s21_set1.insert(val);
  for (const auto& val : values2) s21_set2.insert(val);

  std::set<int> std_set1(values1.begin(), values1.end());
  std::set<int> std_set2(values2.begin(), values2.end());

  auto start = high_resolution_clock::now();
  s21_set1.merge(s21_set2);
  auto end = high_resolution_clock::now();
  auto s21_duration = duration_cast<milliseconds>(end - start).count();

  start = high_resolution_clock::now();
  std_set1.merge(std_set2);
  end = high_resolution_clock::now();
  auto std_duration = duration_cast<milliseconds>(end - start).count();

  std::cout << "Merge: s21_set = " << s21_duration
            << " ms, std::set = " << std_duration << " ms\n";
}
