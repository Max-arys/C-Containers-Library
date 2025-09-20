#include "testing.h"

class S21MultisetTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std_mset = {1, 2, 3, 3};
    mset1 = {1, 2, 3, 3};
    mset2 = {4, 5, 5};
  }

  s21::multiset<int> mset_empty;
  s21::multiset<int> mset1;
  s21::multiset<int> mset2;
  std::multiset<int> std_mset;
};

// Конструктор по умолчанию.
TEST_F(S21MultisetTest, DefaultConstructor) {
  EXPECT_TRUE(mset_empty.empty());
  EXPECT_EQ(mset_empty.size(), 0);
}

// Конструктор списка инициализации.
TEST_F(S21MultisetTest, InitializerListConstructor) {
  s21::multiset<int> mset = {1, 2, 2, 3};
  EXPECT_EQ(mset.size(), 4);
  EXPECT_TRUE(mset.contains(1));
  EXPECT_TRUE(mset.contains(2));
  EXPECT_EQ(mset.count(2), 2);
}

// Конструктор копирования.
TEST_F(S21MultisetTest, CopyConstructor) {
  s21::multiset<int> mset_copy(mset1);
  EXPECT_EQ(mset_copy.size(), 4);
  EXPECT_TRUE(mset_copy.contains(1));
  EXPECT_TRUE(mset_copy.contains(3));
  EXPECT_EQ(mset_copy.count(3), 2);
}

// Конструктор перемещения.
TEST_F(S21MultisetTest, MoveConstructor) {
  s21::multiset<int> mset_move(std::move(mset1));
  EXPECT_EQ(mset_move.size(), 4);
  EXPECT_TRUE(mset_move.contains(2));
  EXPECT_EQ(mset1.size(), 0);  // Проверка, что исходный контейнер пуст
}

// Оператора присваивания.
TEST_F(S21MultisetTest, MoveAssignment) {
  s21::multiset<int> mset_move;
  mset_move = std::move(mset2);
  EXPECT_EQ(mset_move.size(), 3);
  EXPECT_TRUE(mset_move.contains(5));
  EXPECT_EQ(mset2.size(), 0);
}

// Итераторы
TEST_F(S21MultisetTest, BeginEnd) {
  auto it = mset1.begin();
  EXPECT_EQ(*it, 1);
  ++it;
  EXPECT_EQ(*it, 2);

  int count = 0;
  for (auto itt = mset1.begin(); itt != mset1.end(); ++itt) {
    ++count;
  }
  EXPECT_EQ(count, 4);
}

// Вставка
TEST_F(S21MultisetTest, Insert) {
  auto res = mset_empty.insert(10);
  EXPECT_EQ(*res, 10);
  EXPECT_EQ(mset_empty.size(), 1);

  res = mset_empty.insert(10);  // Дубликат
  EXPECT_EQ(*res, 10);
  EXPECT_EQ(mset_empty.size(), 2);
}

// Удаление по итератору.
TEST_F(S21MultisetTest, EraseByIterator) {
  auto it = mset1.find(2);
  mset1.erase(it);
  EXPECT_FALSE(mset1.contains(2));
  EXPECT_EQ(mset1.size(), 3);
}

// Удаление по ключу.
TEST_F(S21MultisetTest, EraseByKey) {
  mset1.erase(3);
  EXPECT_EQ(mset1.count(3), 1);  // Было 2, осталось 1
  EXPECT_EQ(mset1.size(), 3);
}

// Очистка множества.
TEST_F(S21MultisetTest, Clear) {
  mset1.clear();
  EXPECT_TRUE(mset1.empty());
  EXPECT_EQ(mset1.size(), 0);
}

TEST_F(S21MultisetTest, Swap) {
  s21::multiset<int> mset_swap;
  mset_swap.swap(mset1);
  EXPECT_TRUE(mset1.empty());
  EXPECT_EQ(mset_swap.size(), 4);
  EXPECT_TRUE(mset_swap.contains(3));
}

TEST_F(S21MultisetTest, Merge) {
  mset1.merge(mset2);
  EXPECT_EQ(mset1.size(), 7);
  EXPECT_EQ(mset2.size(), 0);
  EXPECT_EQ(mset1.count(5), 2);  // 5 было в обоих multiset'ах
}

TEST_F(S21MultisetTest, Find) {
  auto it = mset1.find(3);
  EXPECT_NE(it, mset1.end());
  EXPECT_EQ(*it, 3);

  it = mset1.find(99);
  EXPECT_EQ(it, mset1.end());
}

TEST_F(S21MultisetTest, Contains) {
  EXPECT_TRUE(mset1.contains(1));
  EXPECT_FALSE(mset1.contains(99));
}

TEST_F(S21MultisetTest, Count) {
  EXPECT_EQ(mset1.count(3), 2);
  EXPECT_EQ(mset1.count(99), 0);
}

TEST_F(S21MultisetTest, LowerBound) {
  auto it = mset1.lower_bound(2);
  EXPECT_EQ(*it, 2);

  it = mset1.lower_bound(4);
  EXPECT_EQ(it, mset1.end());
}

TEST_F(S21MultisetTest, UpperBound) {
  auto it = mset1.upper_bound(2);
  EXPECT_EQ(*it, 3);

  it = mset1.upper_bound(3);
  EXPECT_EQ(it, mset1.end());
}

TEST_F(S21MultisetTest, EqualRange) {
  auto range = mset1.equal_range(3);
  auto it = range.first;
  int count = 0;
  while (it != range.second) {
    ++it;
    ++count;
  }
  EXPECT_EQ(count, 2);
}

TEST_F(S21MultisetTest, MaxSize) { EXPECT_GT(mset_empty.max_size(), 0); }

TEST_F(S21MultisetTest, InsertMany_Basic) {
  // Вставка нескольких уникальных элементов
  auto my_results = mset1.insert_many(6, 3, 8);
  auto std_results = {std_mset.insert(6), std_mset.insert(3),
                      std_mset.insert(8)};

  // Проверка количества возвращенных результатов
  EXPECT_EQ(my_results.size(), 3);

  // Проверка каждого элемента
  for (size_t i = 0; i < 3; ++i) {
    EXPECT_EQ(*my_results[i].first, *std_results.begin()[i]);
  }

  EXPECT_EQ(std_mset.size(), mset1.size());
}

TEST_F(S21MultisetTest, InsertMany_WithDuplicates) {
  // Пытаемся вставить дубликат и новые элементы
  auto my_results = mset1.insert_many(5, 2, 7);

  std_mset.insert(5);
  std_mset.insert(2);
  std_mset.insert(7);

  EXPECT_EQ(std_mset.size(), mset1.size());
}

TEST_F(S21MultisetTest, InsertMany_Empty) {
  // Вставка нуля элементов
  auto my_results = mset1.insert_many();

  EXPECT_TRUE(my_results.empty());
  EXPECT_EQ(std_mset.size(), mset1.size());
}

TEST_F(S21MultisetTest, InsertMany_MixedTypes) {
  // Проверяем работу с неявным приведением типов
  auto my_results = mset1.insert_many(5, 3.14, 'a');
  auto std_results = {std_mset.insert(5),
                      std_mset.insert(static_cast<int>(3.14)),
                      std_mset.insert(static_cast<int>('a'))};

  EXPECT_EQ(my_results.size(), 3);

  for (size_t i = 0; i < 3; ++i) {
    EXPECT_EQ(*my_results[i].first, *std_results.begin()[i]);
  }

  EXPECT_EQ(std_mset.size(), mset1.size());
}
