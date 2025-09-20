#include <cstdio>  // Для rand().

#include "testing.h"

// Проверка инициализации пустого дерева.
TEST(RbTreeTest, EmptyTree) {
  s21::Rb_tree<int, int> tree;
  EXPECT_TRUE(tree.empty());
  EXPECT_EQ(tree.size(), 0);
  EXPECT_EQ(tree.begin(), tree.end());
}

// Проверка вставки
TEST(RbTreeTest, SingleInsert) {
  s21::Rb_tree<int, int> tree;
  auto res = tree.insert(42, 42);
  EXPECT_TRUE(res.second);
  EXPECT_EQ(tree.size(), 1);
  EXPECT_EQ(res.first->val, 42);
  EXPECT_NE(tree.begin(), tree.end());

  // Значения не обновляются.
  res = tree.insert(42, 21);
  EXPECT_FALSE(res.second);
  EXPECT_EQ(tree.size(), 1);
  EXPECT_EQ(res.first->val, 42);
  EXPECT_NE(tree.begin(), tree.end());
}

// Провeрка итератора
TEST(RbTreeTest, Iteration) {
  s21::Rb_tree<int, int> tree;
  std::set<int> values = {5, 3, 7, 2, 4, 6, 8};

  for (int v : values) {
    tree.insert(v, v);
  }

  std::set<int> tree_values;
  for (auto it = tree.begin(); it != tree.end(); ++it) {
    tree_values.insert(*it);
  }

  EXPECT_EQ(values, tree_values);
}

// Провeрка константного итератора.
TEST(RbTreeTest, ConstIteration) {
  s21::Rb_tree<int, int> tree;
  std::set<int> values = {5, 3, 7, 2, 4, 6, 8};

  for (int v : values) {
    tree.insert(v, v);
  }

  decltype(tree)::const_iterator c_start{tree.minimum(tree.get_root()), &tree};
  decltype(tree)::const_iterator c_end{tree.get_nil(), &tree};

  std::set<int> tree_values;
  for (; c_start != c_end; ++c_start) {
    tree_values.insert(*c_start);
  }

  EXPECT_EQ(values, tree_values);

  // Тест декремента.
  std::set<int> tree_values2;

  --c_start;
  for (; c_start != c_end; --c_start) {
    tree_values2.insert(*c_start);
  }

  EXPECT_EQ(values, tree_values2);
}

// Провeрка константного итератора c begin, end.
TEST(RbTreeTest, ConstIterationBeginEnd) {
  s21::Rb_tree<int, int> tree;

  tree.insert(5, 5);

  const decltype(tree)* ctree = &tree;
  auto c_begin = ctree->begin();
  auto c_end = ctree->end();

  EXPECT_TRUE(typeid(c_begin) == typeid(decltype(tree)::const_iterator));
  EXPECT_TRUE(typeid(c_end) == typeid(decltype(tree)::const_iterator));
}

// Вспомогательная функция для проверки отсутствия двух красных узлов подряд
void CheckNoDoubleRed(s21::Node<int>* node, const s21::Node<int>* nil) {
  if (node == nil) return;

  if (node->color == s21::Red) {
    EXPECT_EQ(node->left->color, s21::Black);
    EXPECT_EQ(node->right->color, s21::Black);
  }

  CheckNoDoubleRed(node->left, nil);
  CheckNoDoubleRed(node->right, nil);
}

// Вспомогательная функция для проверки чёрной высоты
int CheckBlackHeight(s21::Node<int>* node, const s21::Node<int>* nil) {
  if (node == nil) return 1;

  int left_height = CheckBlackHeight(node->left, nil);
  int right_height = CheckBlackHeight(node->right, nil);

  EXPECT_EQ(left_height, right_height);

  return (node->color == s21::Black) ? left_height + 1 : left_height;
}

#include <cstdio>
// Проверка свойств красно черного дерева.
TEST(RbTreeTest, RedBlackProperties) {
  s21::Rb_tree<int, int> tree;
  int n;

  // Вставка элементов
  for (int i = 1; i <= 100; ++i) {
    n = rand();
    tree.insert(n, n);
  }

  // Проверка свойств

  // 1. Корень должен быть чёрным
  EXPECT_EQ(tree.get_root()->color, s21::Black);

  // 2. Нет двух красных узлов подряд
  CheckNoDoubleRed(tree.get_root(), tree.get_nil());

  // 3. Чёрная высота одинакова для всех путей
  CheckBlackHeight(tree.get_root(), tree.get_nil());
}

// Проверка удаления.
TEST(RbTreeTest, DeleteLeaf) {
  s21::Rb_tree<int, int> tree;
  tree.insert(10, 10);
  tree.insert(5, 5);
  tree.insert(15, 15);

  auto node = tree.search(5);
  tree.delete_node(node);

  EXPECT_EQ(tree.size(), 2);
  EXPECT_EQ(tree.search(5), tree.get_nil());
}

// Проверка удаления.
TEST(RbTreeTest, DeleteRoot) {
  s21::Rb_tree<int, int> tree;
  tree.insert(10, 10);
  tree.insert(5, 5);
  tree.insert(15, 15);

  auto node = tree.search(10);
  tree.delete_node(node);

  EXPECT_EQ(tree.size(), 2);
  EXPECT_EQ(tree.search(10), tree.get_nil());
  EXPECT_NE(tree.get_root(), tree.get_nil());
}

// Проверка удаления всех элементов.
TEST(RbTreeTest, DeleteAllElements) {
  s21::Rb_tree<int, int> tree;
  for (int i = 1; i <= 20; ++i) {
    tree.insert(i, i);
  }

  for (int i = 1; i <= 20; ++i) {
    tree.delete_node(tree.get_root());
  }

  EXPECT_TRUE(tree.empty());

  for (int i = 20; i > 0; --i) {
    tree.insert(i, i);
  }

  for (int i = 1; i <= 20; ++i) {
    tree.delete_node(tree.get_root());
  }

  EXPECT_TRUE(tree.empty());
}

// Проверка балансировки для случая когда балансируемая нода имеет черного брата
// с красным левым потомком и черным правым.
TEST(RbTreeTest, DeletedLeft) {
  s21::Rb_tree<int, int> tree;
  for (int i = 1; i <= 11; ++i) {
    tree.insert(i, i);
  }

  tree.delete_node(tree.search(8));
  tree.delete_node(tree.search(9));
  tree.delete_node(tree.search(10));
  tree.delete_node(tree.search(4));

  // Проверка свойств

  // 1. Корень должен быть чёрным
  EXPECT_EQ(tree.get_root()->color, s21::Black);

  // 2. Нет двух красных узлов подряд
  CheckNoDoubleRed(tree.get_root(), tree.get_nil());

  // 3. Чёрная высота одинакова для всех путей
  CheckBlackHeight(tree.get_root(), tree.get_nil());
}

// Проверка балансировки для случая когда балансируемая нода имеет черного брата
// с красным правым потомком и черным левым.
TEST(RbTreeTest, DeletedRight) {
  s21::Rb_tree<int, int> tree;
  for (int i = 11; i >= 1; --i) {
    tree.insert(i, i);
  }

  tree.delete_node(tree.search(4));
  tree.delete_node(tree.search(2));
  tree.delete_node(tree.search(3));
  tree.insert(5, 5);
  tree.delete_node(tree.search(1));
  tree.delete_node(tree.search(7));

  // Проверка свойств

  // 1. Корень должен быть чёрным
  EXPECT_EQ(tree.get_root()->color, s21::Black);

  // 2. Нет двух красных узлов подряд
  CheckNoDoubleRed(tree.get_root(), tree.get_nil());

  // 3. Чёрная высота одинакова для всех путей
  CheckBlackHeight(tree.get_root(), tree.get_nil());
}

// Проверка очистки дерева.
TEST(RbTreeTest, Clear) {
  s21::Rb_tree<int, int> tree;
  for (int i = 1; i <= 20; ++i) {
    tree.insert(i, i);
  }

  tree.clear();

  EXPECT_TRUE(tree.empty());
}

// Проверка поиска.
TEST(RbTreeTest, SearchExisting) {
  s21::Rb_tree<int, int> tree;
  tree.insert(10, 10);
  tree.insert(5, 5);
  tree.insert(15, 15);

  auto node = tree.search(5);
  EXPECT_NE(node, tree.get_nil());
  EXPECT_EQ(node->val, 5);
}

// Проверка поиска несуществующего элемента.
TEST(RbTreeTest, SearchNonExisting) {
  s21::Rb_tree<int, int> tree;
  for (int i = 1; i <= 10; ++i) {
    tree.insert(i, i);
  }

  auto node = tree.search(42);
  EXPECT_EQ(node, tree.get_nil());
}

// Проверка метода lower_bound.
TEST(RbTreeTest, LowerBoundTests) {
  s21::Rb_tree<int, int> tree;
  for (int i = 1; i <= 10; ++i) {
    tree.insert(i, i);
  }

  // Поиск существующего ключа
  auto node = tree.lower_bound(5);
  EXPECT_EQ(node->val, 5);

  // Поиск несуществующего ключа, между 4 и 6
  tree.delete_node(tree.search(5));
  node = tree.lower_bound(5.5);
  EXPECT_EQ(node->val, 6);

  // Поиск значения меньше любого ключа (к примеру, 0)
  node = tree.lower_bound(0);
  EXPECT_EQ(node->val, 1);

  // Поиск значения больше любого ключа (к примеру, 42)
  node = tree.lower_bound(42);
  EXPECT_EQ(node, tree.get_nil());
}

// Проверка правильного порядка получения элементов итаратором при инкременте.
TEST(RbTreeTest, IteratorIncrement) {
  s21::Rb_tree<int, int> tree;
  std::vector<int> values = {5, 3, 7, 2, 4, 6, 8};
  for (int v : values) {
    tree.insert(v, v);
  }
  std::sort(values.begin(), values.end());

  auto it = tree.begin();
  for (size_t i = 0; i < values.size(); ++i) {
    EXPECT_EQ(*it, values[i]);
    ++it;
  }
  EXPECT_EQ(it, tree.end());
}

// Проверка правильного порядка получения элементов итаратором при декременте.
TEST(RbTreeTest, IteratorDecrement) {
  s21::Rb_tree<int, int> tree;
  std::vector<int> values = {5, 3, 7, 2, 4, 6, 8};
  for (int v : values) {
    tree.insert(v, v);
  }
  std::sort(values.rbegin(), values.rend());

  auto it = tree.end();
  --it;
  for (size_t i = 0; i < values.size(); ++i) {
    EXPECT_EQ(*it, values[i]);
    --it;
  }
}

// Проверка работы инкремента и декремента для iterator, const_iterator.
TEST(RbTreeTest, IncrementDecrement) {
  s21::Rb_tree<int, int> tree;
  tree.insert(10, 10);
  tree.insert(5, 5);
  tree.insert(15, 15);
  const decltype(tree)* c_tree = &tree;
  auto start = tree.begin();
  auto c_start = c_tree->begin();
  start++;
  start--;
  ++start;
  --start;
  c_start++;
  c_start--;
  ++c_start;
  --c_start;
  EXPECT_EQ(5, *start);
  EXPECT_EQ(5, *c_start);
}

// Проверка работы метода Merge с уникальными ключами.
TEST(RbTreeTest, MergeUnique) {
  std::set<int> set1;
  s21::Rb_tree<int, int> tree1;
  for (int i = 1; i <= 15; ++i) {
    tree1.insert(i, i);
    set1.insert(i);
  }

  std::set<int> set2;
  s21::Rb_tree<int, int> tree2;
  for (int i = 7; i <= 20; ++i) {
    tree2.insert(i, i);
    set2.insert(i);
  }

  tree1.merge(&tree2, true);
  set1.merge(set2);

  EXPECT_EQ(tree1.size(), set1.size());
  EXPECT_EQ(tree2.size(), set2.size());

  auto tree1_it = tree1.begin();
  auto set1_it = set1.begin();
  while (tree1_it != tree1.end() && set1_it != set1.end()) {
    EXPECT_EQ(*tree1_it, *set1_it);
    ++tree1_it;
    ++set1_it;
  }

  // Проверка, что все элементы были пройдены (нет расхождений в размерах)
  EXPECT_TRUE(tree1_it == tree1.end() && set1_it == set1.end());

  auto tree2_it = tree2.begin();
  auto set2_it = set2.begin();
  while (tree2_it != tree2.end() && set2_it != set2.end()) {
    EXPECT_EQ(*tree2_it, *set2_it);
    ++tree2_it;
    ++set2_it;
  }

  EXPECT_TRUE(tree2_it == tree2.end() && set2_it == set2.end());
}

// Проверка работы метода Merge с повторяющимися ключами.
TEST(RbTreeTest, MergeNotUnique) {
  std::multiset<int> mset1;
  s21::Rb_tree<int, int> tree1;
  for (int i = 1; i <= 15; ++i) {
    tree1.insert(i, i);
    mset1.insert(i);
  }

  std::multiset<int> mset2;
  s21::Rb_tree<int, int> tree2;
  for (int i = 7; i <= 20; ++i) {
    tree2.insert(i, i);
    mset2.insert(i);
  }

  tree1.merge(&tree2, false);
  mset1.merge(mset2);

  EXPECT_EQ(tree1.size(), mset1.size());
  EXPECT_EQ(tree2.size(), mset2.size());

  auto tree1_it = tree1.begin();
  auto set1_it = mset1.begin();
  while (tree1_it != tree1.end() && set1_it != mset1.end()) {
    EXPECT_EQ(*tree1_it, *set1_it);
    ++tree1_it;
    ++set1_it;
  }

  // Проверка, что все элементы были пройдены (нет расхождений в размерах)
  EXPECT_TRUE(tree1_it == tree1.end() && set1_it == mset1.end());

  auto tree2_it = tree2.begin();
  auto set2_it = mset2.begin();
  while (tree2_it != tree2.end() && set2_it != mset2.end()) {
    EXPECT_EQ(*tree2_it, *set2_it);
    ++tree2_it;
    ++set2_it;
  }

  EXPECT_TRUE(tree2_it == tree2.end() && set2_it == mset2.end());
}

TEST(RbTreeTest, CopyInitial) {
  std::set<int> set1;
  s21::Rb_tree<int, int> tree1;
  for (int i = 1; i <= 15; ++i) {
    tree1.insert(i, i);
    set1.insert(i);
  }

  std::set<int> set2{set1};
  s21::Rb_tree<int, int> tree2{tree1};

  EXPECT_EQ(tree1.size(), set1.size());
  EXPECT_EQ(tree2.size(), set2.size());

  auto tree1_it = tree1.begin();
  auto set1_it = set1.begin();
  while (tree1_it != tree1.end() && set1_it != set1.end()) {
    EXPECT_EQ(*tree1_it, *set1_it);
    ++tree1_it;
    ++set1_it;
  }

  // Проверка, что все элементы были пройдены (нет расхождений в размерах)
  EXPECT_TRUE(tree1_it == tree1.end() && set1_it == set1.end());

  auto tree2_it = tree2.begin();
  auto set2_it = set2.begin();
  while (tree2_it != tree2.end() && set2_it != set2.end()) {
    EXPECT_EQ(*tree2_it, *set2_it);
    ++tree2_it;
    ++set2_it;
  }

  EXPECT_TRUE(tree2_it == tree2.end() && set2_it == set2.end());
}

// 1. Тест оператора присваивания
TEST(RbTreeTest, AssignmentOperator) {
  s21::Rb_tree<int, int> tree1;
  tree1.insert(2, 2);
  tree1.insert(3, 3);
  s21::Rb_tree<int, int> tree2;
  tree2.insert(22, 22);
  tree2.insert(33, 33);
  tree2.insert(44, 44);
  tree2 = tree1;

  // Проверяем, что деревья стали идентичны
  EXPECT_EQ(tree1.size(), tree2.size());
  EXPECT_TRUE(std::equal(tree1.begin(), tree1.end(), tree2.begin()));
  EXPECT_TRUE(std::equal(tree2.begin(), tree2.end(), tree1.begin()));
}

// 1. Тест конструктора и оператора перемещения.
TEST(RbTreeTest, MoveConstructorOperator) {
  s21::Rb_tree<int, int> tree1;
  tree1.insert(2, 2);
  tree1.insert(3, 3);
  s21::Rb_tree<int, int> tree2(std::move(tree1));
  EXPECT_EQ(tree1.size(), 0);
  EXPECT_EQ(tree2.size(), 2);

  s21::Rb_tree<int, int> tree11;
  tree11.insert(2, 2);
  tree11.insert(3, 3);
  s21::Rb_tree<int, int> tree22;
  tree22 = std::move(tree11);
  EXPECT_EQ(tree11.size(), 0);
  EXPECT_EQ(tree22.size(), 2);
}