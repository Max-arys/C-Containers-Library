#include "testing.h"

TEST(MapTest, DefaultConstructor) {
  s21::map<int, std::string> m;
  EXPECT_TRUE(m.empty());
  EXPECT_EQ(m.size(), 0);
}

TEST(MapTest, InitializerListConstructor) {
  s21::map<int, std::string> m{{1, "one"}, {2, "two"}, {3, "three"}};
  EXPECT_EQ(m.size(), 3);
  EXPECT_EQ(m[1], "one");
  EXPECT_EQ(m[2], "two");
  EXPECT_EQ(m[3], "three");
}

TEST(MapTest, CopyConstructor) {
  s21::map<int, std::string> m{{1, "one"}, {2, "two"}};
  s21::map<int, std::string> copy(m);
  EXPECT_EQ(copy.size(), 2);
  EXPECT_EQ(copy[1], "one");
  EXPECT_EQ(copy[2], "two");
}

TEST(MapTest, MoveConstructor) {
  s21::map<int, std::string> m{{1, "one"}, {2, "two"}};
  s21::map<int, std::string> moved(std::move(m));
  EXPECT_EQ(moved.size(), 2);
  EXPECT_TRUE(m.empty());
}

TEST(MapTest, Insert) {
  s21::map<int, std::string> m;
  auto res = m.insert({1, "one"});
  EXPECT_TRUE(res.second);
  EXPECT_EQ((*res.first).first, 1);
  EXPECT_EQ((*res.first).second, "one");

  auto res2 = m.insert({1, "uno"});
  EXPECT_FALSE(res2.second);

  res = m.insert(2, "two");
  EXPECT_TRUE(res.second);
  EXPECT_EQ((*res.first).first, 2);
  EXPECT_EQ((*res.first).second, "two");
}

TEST(MapTest, InsertOrAssign) {
  s21::map<int, std::string> m;
  m.insert_or_assign(1, "one");
  EXPECT_EQ(m[1], "one");

  m.insert_or_assign(1, "uno");
  EXPECT_EQ(m[1], "uno");
}

TEST(MapTest, At) {
  s21::map<int, std::string> m{{1, "one"}};
  EXPECT_EQ(m.at(1), "one");
  EXPECT_THROW(m.at(2), std::out_of_range);
}

TEST(MapTest, OperatorBracket) {
  s21::map<int, std::string> m;
  m[1] = "one";
  EXPECT_EQ(m[1], "one");
  EXPECT_EQ(m.size(), 1);
}

TEST(MapTest, Erase) {
  s21::map<int, std::string> m{{1, "one"}, {2, "two"}, {3, "three"}};
  m.erase(m.begin());
  EXPECT_EQ(m.size(), 2);
  EXPECT_FALSE(m.contains(1));
}

TEST(MapTest, Swap) {
  s21::map<int, std::string> m1{{1, "one"}};
  s21::map<int, std::string> m2{{2, "two"}};

  m1.swap(m2);
  EXPECT_EQ(m1.size(), 1);
  EXPECT_EQ(m1[2], "two");
  EXPECT_EQ(m2.size(), 1);
  EXPECT_EQ(m2[1], "one");
}

TEST(MapTest, Merge) {
  s21::map<int, std::string> m1{{1, "one"}, {3, "three"}};
  s21::map<int, std::string> m2{{2, "two"}, {3, "tres"}};

  m1.merge(m2);
  EXPECT_EQ(m1.size(), 3);
  EXPECT_EQ(m1[3], "three");  // Должен сохранит оригинальное значение.
  EXPECT_EQ(m2.size(), 1);    // Дубликат должен остаться.
}

TEST(MapTest, Contains) {
  s21::map<int, std::string> m{{1, "one"}};
  EXPECT_TRUE(m.contains(1));
  EXPECT_FALSE(m.contains(2));
}

TEST(MapTest, MaxSize) {
  s21::map<int, std::string> m;
  EXPECT_GT(m.max_size(), 0);
}

TEST(MapTest, Clear) {
  s21::map<int, std::string> m{{1, "one"}, {2, "two"}};
  m.clear();
  EXPECT_TRUE(m.empty());
  EXPECT_EQ(m.size(), 0);
}

TEST(MapTest, Iterators) {
  s21::map<int, std::string> m{{1, "one"}, {2, "two"}, {3, "three"}};

  int count = 0;
  for (auto it = m.begin(); it != m.end(); ++it) {
    ++count;
  }
  EXPECT_EQ(count, 3);

  const auto& cm = m;
  count = 0;
  for (auto it = cm.begin(); it != cm.end(); ++it) {
    ++count;
  }
  EXPECT_EQ(count, 3);
}

// Тест оператора копирования.
TEST(MapTest, AssignmentOperators) {
  s21::map<int, std::string> m1{{1, "one"}, {2, "two"}};
  s21::map<int, std::string> m2;

  m2 = m1;
  EXPECT_EQ(m2.size(), 2);
  EXPECT_EQ(m2[1], "one");

  s21::map<int, std::string> m3;
  m3 = std::move(m1);
  EXPECT_EQ(m3.size(), 2);
  EXPECT_TRUE(m1.empty());
}

TEST(MapTest, InsertMany_Basic) {
  s21::map<int, std::string> my_map;
  using VT = decltype(my_map)::value_type;
  std::map<int, std::string> std_map;
  // Вставка нескольких уникальных элементов
  auto my_results =
      my_map.insert_many(VT(1, "one"), VT(2, "two"), VT(3, "three"));
  auto std_results = {std_map.insert({1, "one"}), std_map.insert({2, "two"}),
                      std_map.insert({3, "three"})};

  // Проверка количества возвращенных результатов
  EXPECT_EQ(my_results.size(), 3);

  // Проверка каждого элемента
  for (size_t i = 0; i < 3; ++i) {
    EXPECT_EQ(*my_results[i].first, *std_results.begin()[i].first);
    EXPECT_EQ(my_results[i].second, std_results.begin()[i].second);
  }

  EXPECT_EQ(std_map.size(), my_map.size());
}

TEST(MapTest, InsertMany_WithDuplicates) {
  s21::map<int, std::string> my_map{{1, "one"}, {2, "two"}, {3, "three"}};
  using VT = decltype(my_map)::value_type;
  std::map<int, std::string> std_map{{1, "one"}, {2, "two"}, {3, "three"}};
  // Пытаемся вставить дубликат и новые элементы
  auto my_results = my_map.insert_many(VT(3, "three"), VT(4, "four"));

  // // Проверяем что дубликат не был добавлен
  EXPECT_EQ(my_results[0].second, false);
  EXPECT_EQ(my_results[1].second, true);
  std_map.insert({3, "three"});
  std_map.insert({4, "four"});

  EXPECT_EQ(std_map.size(), my_map.size());
}

TEST(MapTest, InsertMany_Empty) {
  // Вставка нуля элементов
  s21::map<int, std::string> my_map;
  auto my_results = my_map.insert_many();

  EXPECT_TRUE(my_results.empty());
}
