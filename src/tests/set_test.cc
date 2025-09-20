#include "testing.h"

// Создание данных для тестов. Для каждого теста создается свой набор.
class SetTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std_set = {1, 2, 3, 4, 5};
    my_set = {1, 2, 3, 4, 5};
  }

  std::set<int> std_set;
  s21::set<int> my_set;
};

// Тест конструктора по умолчанию.
TEST_F(SetTest, DefaultConstructor) {
  std::set<int> std_empty;
  s21::set<int> my_empty;

  EXPECT_EQ(std_empty.size(), my_empty.size());
  EXPECT_EQ(std_empty.empty(), my_empty.empty());
}

// Инициализации из списка инициализации.
TEST_F(SetTest, InitializerListConstructor) {
  std::set<int> std_temp{1, 2, 3};
  s21::set<int> my_temp{1, 2, 3};

  EXPECT_EQ(std_temp.size(), my_temp.size());
  EXPECT_EQ(*std_temp.begin(), *my_temp.begin());
}

// Tест конструктора копирования.
TEST_F(SetTest, CopyConstructor) {
  std::set<int> std_copy(std_set);
  s21::set<int> my_copy(my_set);

  EXPECT_EQ(std_copy.size(), my_copy.size());
  EXPECT_EQ(*std_copy.begin(), *my_copy.begin());

  my_copy = my_copy;
}

// Tест конструктора перемещения.
TEST_F(SetTest, MoveConstructor) {
  std::set<int> std_moved(std::move(std_set));
  s21::set<int> my_moved(std::move(my_set));

  EXPECT_EQ(std_moved.size(), my_moved.size());
  EXPECT_EQ(*std_moved.begin(), *my_moved.begin());
  EXPECT_EQ(std_set.size(), my_set.size());
}

// Тест оператора копирования.
TEST_F(SetTest, AssignmentOperator) {
  std::set<int> std_assign;
  s21::set<int> my_assign;

  std_assign = std_set;
  my_assign = my_set;

  EXPECT_EQ(std_assign.size(), my_assign.size());
  EXPECT_EQ(*std_assign.begin(), *my_assign.begin());
}

TEST_F(SetTest, Empty) {
  std::set<int> std_empty;
  s21::set<int> my_empty;

  EXPECT_TRUE(std_empty.empty());
  EXPECT_TRUE(my_empty.empty());
  EXPECT_FALSE(std_set.empty());
  EXPECT_FALSE(my_set.empty());
}

TEST_F(SetTest, Size) {
  EXPECT_EQ(std_set.size(), my_set.size());
  std::set<int> std_empty;
  s21::set<int> my_empty;
  EXPECT_EQ(std_empty.size(), my_empty.size());
}

TEST_F(SetTest, Clear) {
  std_set.clear();
  my_set.clear();

  EXPECT_EQ(std_set.size(), my_set.size());
  EXPECT_TRUE(std_set.empty());
  EXPECT_TRUE(my_set.empty());
}

TEST_F(SetTest, Insert) {
  auto std_res = std_set.insert(6);
  auto my_res = my_set.insert(6);

  EXPECT_EQ(*std_res.first, *my_res.first);
  EXPECT_EQ(std_res.second, my_res.second);
  EXPECT_EQ(std_set.size(), my_set.size());
}

// Тест удаления элемента.
TEST_F(SetTest, Erase) {
  std_set.erase(std_set.begin());
  my_set.erase(my_set.begin());

  EXPECT_EQ(std_set.size(), my_set.size());
  EXPECT_EQ(*std_set.begin(), *my_set.begin());
}

TEST_F(SetTest, Swap) {
  std::set<int> std_swap = {10, 20, 30};
  s21::set<int> my_swap = {10, 20, 30};

  std_set.swap(std_swap);
  my_set.swap(my_swap);

  EXPECT_EQ(std_set.size(), my_set.size());
  EXPECT_EQ(std_swap.size(), my_swap.size());
  EXPECT_EQ(*std_set.begin(), *my_set.begin());
}

TEST_F(SetTest, Merge) {
  std::set<int> std_merge = {6, 7, 8};
  s21::set<int> my_merge = {6, 7, 8};

  std_set.merge(std_merge);
  my_set.merge(my_merge);

  EXPECT_EQ(std_set.size(), my_set.size());
  EXPECT_EQ(std_merge.size(), my_merge.size());
}

TEST_F(SetTest, MergeNotUniq) {
  std::set<int> std_merge = {1, 2, 3};
  s21::set<int> my_merge = {1, 2, 3};

  std_set.merge(std_merge);
  my_set.merge(my_merge);

  EXPECT_EQ(std_set.size(), my_set.size());
  EXPECT_EQ(std_merge.size(), my_merge.size());
}

TEST_F(SetTest, Find) {
  auto std_it = std_set.find(3);
  auto my_it = my_set.find(3);

  EXPECT_NE(std_it, std_set.end());
  EXPECT_NE(my_it, my_set.end());
  EXPECT_EQ(*std_it, *my_it);
}

TEST_F(SetTest, Contains) {
  EXPECT_TRUE(my_set.contains(3));
  EXPECT_FALSE(my_set.contains(99));
}

TEST_F(SetTest, InsertMany_Basic) {
  // Вставка нескольких уникальных элементов
  auto my_results = my_set.insert_many(6, 3, 8);
  auto std_results = {std_set.insert(6), std_set.insert(3), std_set.insert(8)};

  // Проверка количества возвращенных результатов
  EXPECT_EQ(my_results.size(), 3);

  // Проверка каждого элемента
  for (size_t i = 0; i < 3; ++i) {
    EXPECT_EQ(*my_results[i].first, *std_results.begin()[i].first);
    EXPECT_EQ(my_results[i].second, std_results.begin()[i].second);
  }

  EXPECT_EQ(std_set.size(), my_set.size());
}

TEST_F(SetTest, InsertMany_WithDuplicates) {
  // Пытаемся вставить дубликат и новые элементы
  auto my_results = my_set.insert_many(5, 2, 7);
  EXPECT_EQ(my_results.size(), 3);

  // // Проверяем что дубликат не был добавлен
  EXPECT_EQ(my_results[0].second, false);
  EXPECT_EQ(my_results[1].second, false);
  EXPECT_EQ(my_results[2].second, true);  // 7 новый
  std_set.insert(5);
  std_set.insert(2);
  std_set.insert(7);

  EXPECT_EQ(std_set.size(), my_set.size());
}

TEST_F(SetTest, InsertMany_Empty) {
  // Вставка нуля элементов
  auto my_results = my_set.insert_many();

  EXPECT_TRUE(my_results.empty());
  EXPECT_EQ(std_set.size(), my_set.size());
}

TEST_F(SetTest, InsertMany_MixedTypes) {
  // Проверяем работу с неявным приведением типов
  auto my_results = my_set.insert_many(5, 3.14, 'a');
  auto std_results = {std_set.insert(5), std_set.insert(static_cast<int>(3.14)),
                      std_set.insert(static_cast<int>('a'))};

  EXPECT_EQ(my_results.size(), 3);

  for (size_t i = 0; i < 3; ++i) {
    EXPECT_EQ(*my_results[i].first, *std_results.begin()[i].first);
  }

  EXPECT_EQ(std_set.size(), my_set.size());
}
