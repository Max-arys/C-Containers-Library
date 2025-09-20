#ifndef S21_RED_BLACK_TREE_H
#define S21_RED_BLACK_TREE_H

#include <iostream>
#include <stack>

#include "s21_allocator.h"
#include "s21_helpers.h"

namespace s21 {
/**
 * За основу была взята реализация красно черного дерева из книги
 * Thomas H. Cormen, Charles E. Leiserson, Ronald L. Rivest, Clifford Stein
 * "Introduction to Algorithms Fourth Edition"  2022
 */

enum Node_color { Red = false, Black = true };

template <typename V>
struct Node {
  using value_type = V;

  value_type val;
  Node_color color;
  Node *left;
  Node *right;
  Node *p;

  explicit Node(V val = V(), Node_color color = Red, Node *left = nullptr,
                Node *right = nullptr, Node *p = nullptr)
      : val{val}, color(color), left{left}, right{right}, p{p} {};

};  // struct Node

/**
 * @brief Класс с реализацией красно-чёрного дерева.
 * @tparam K тип ключа.
 * @tparam V тип значения.
 * @tparam KeyOfValue функтор извлечения ключа из поля val структуры Node.
 * @tparam Compare функтор для сравнения ключей.
 * @tparam Alloc аллокатор.
 */
template <typename K, typename V, typename KeyOfValue = std::identity,
          typename Compare = std::less<K>, typename Alloc = std::allocator<V>>
class Rb_tree {
 public:
  class Rb_tree_iterator;
  class Rb_tree_const_iterator;

  using key_type = K;
  using value_type = V;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = size_t;
  using allocator_type = Alloc;
  using node_type = Node<V>;

  using iterator = Rb_tree_iterator;
  using const_iterator = Rb_tree_const_iterator;

 private:
  // Перепривязка аллокатора для типа Node
  using node_allocator =
      typename std::allocator_traits<Alloc>::template rebind_alloc<node_type>;

  // Трейты для работы с аллокатором узлов
  using node_alloc_traits = std::allocator_traits<node_allocator>;

  node_type *root;
  node_type *nil_;
  size_type node_count;
  KeyOfValue kov;
  Compare comp;
  node_allocator alloc;

 public:
  Rb_tree() : node_count{}, comp{} {
    nil_ = create_node(value_type());
    nil_->color = Black;
    nil_->left = nil_->right = nil_->p = nil_;
    root = nil_;
  }

  Rb_tree(const Rb_tree &other)
      : node_count{},
        kov{other.kov},
        comp{other.comp},
        alloc{node_alloc_traits::select_on_container_copy_construction(
            other.alloc)} {
    try {
      // 1. Создаём nil-узел.
      nil_ = create_node(value_type());
      nil_->color = Black;
      nil_->left = nil_->right = nil_->p = nil_;
      root = nil_;

      // 2. Копируем основное дерево.
      if (other.get_root() != other.get_nil()) {
        copy_tree(other.get_root(), other.get_nil());
      }
      node_count = other.node_count;

    } catch (...) {
      if (root != nil_) clear();
      throw;
    }
  }

  Rb_tree(Rb_tree &&other) noexcept
      : root(other.root),
        nil_(other.nil_),
        node_count(other.node_count),
        alloc(std::move(other.alloc)) {
    other.nil_ = create_node(value_type());
    other.root = other.nil_;
    other.node_count = 0;
  }

  ~Rb_tree() {
    clear();
    destroy_node(nil_);
  }

  Rb_tree &operator=(const Rb_tree &other) {
    if (this != &other) {
      Rb_tree temp(other);
      swap(temp);
    }
    return *this;
  }

  Rb_tree &operator=(Rb_tree &&other) noexcept {
    if (this != &other) {
      clear();
      std::swap(nil_, other.nil_);
      root = other.root;
      node_count = other.node_count;
      std::swap(other.alloc, alloc);

      other.root = other.nil_;
      other.node_count = 0;
    }
    return *this;
  }

 public:
  iterator begin() { return iterator(minimum(root), this); }
  const_iterator begin() const { return const_iterator(minimum(root), this); }
  const_iterator cbegin() const { return const_iterator(minimum(root), this); }

  iterator end() noexcept { return iterator(nil_, this); }
  const_iterator end() const { return const_iterator(nil_, this); }
  const_iterator cend() const { return const_iterator(nil_, this); }

  inline node_type *get_root() noexcept { return root; }
  inline const node_type *get_root() const noexcept { return root; }

  inline const node_type *get_nil() const noexcept { return nil_; }

  inline bool empty() const noexcept { return root == nil_ ? true : false; }

  inline size_type size() const noexcept { return node_count; }

  size_type max_size() noexcept { return node_alloc_traits::max_size(alloc); }

  /**
   * @brief Очистка дерева.
   */
  void clear() noexcept {
    destroy_subtree(root, nil_);
    root = nil_;
    node_count = 0;
  }

  /**
   * @brief Добавление нового элемента в дерево по ключу.
   * @param key Ссылка на ключ.
   * @param value Ссылка на значение.
   * @param unique_keys Флаг определяющий будут ли ключи уникльными.
   * @return Объект pair содержащий указатель на созданную ноду и
   * булево значение была ли создана нода.
   */
  std::pair<node_type *, bool> insert(const K &key, const V &value,
                                      bool unique_keys = true) {
    bool created{true};
    node_type *node = find_or_create(key, value, created, unique_keys);

    if (created == true) {
      if (node->p != nil_ && node->p->color == Red && node->p->p != nil_) {
        insert_fixup(node);
      }
      ++node_count;
    }
    return {node, created};
  }

  /**
   * @brief Удаляет ноду.
   * @param z Удаляемая нода.
   */
  void delete_node(node_type *z) noexcept {
    if (z == nil_) return;

    node_type *y;
    node_type *x;
    Node_color y_original_color = z->color;

    if (z->left == nil_) {
      x = z->right;
      transplant(z, z->right);
    } else if (z->right == nil_) {
      x = z->left;
      transplant(z, z->left);
    } else {
      y = minimum(z->right);
      y_original_color = y->color;
      x = y->right;

      if (y != z->right) {
        transplant(y, y->right);
        y->right = z->right;
        y->right->p = y;
      } else {
        x->p = y;
      }

      transplant(z, y);
      y->left = z->left;
      y->left->p = y;
      y->color = z->color;
    }

    destroy_node(z);
    --node_count;

    if (y_original_color == Black) {
      delete_fixup(x);
    }
  }

  /**
   * @brief Находит минимальное значение для поддерева sub_tree.
   * @param sub_tree Нода являющаяся корнем поддерева.
   * @return Нода с минимальным значением.
   */
  node_type *minimum(node_type *sub_tree) {
    while (sub_tree->left != nil_) {
      sub_tree = sub_tree->left;
    }
    return sub_tree;
  }

  const node_type *minimum(const node_type *sub_tree) const {
    while (sub_tree->left != nil_) {
      sub_tree = sub_tree->left;
    }
    return sub_tree;
  }

  /**
   * @brief Находит максимальное значение для поддерева sub_tree.
   * @param sub_tree Нода являющаяся корнем поддерева.
   * @return Нода с максимальным значением.
   */
  node_type *maximum(node_type *sub_tree) {
    while (sub_tree->right != nil_) {
      sub_tree = sub_tree->right;
    }
    return sub_tree;
  }

  const node_type *maximum(const node_type *sub_tree) const {
    while (sub_tree->right != nil_) {
      sub_tree = sub_tree->right;
    }
    return sub_tree;
  }

  /**
   * @brief Находит ноду с указаным ключём.
   * @param key Ссылка на ключ.
   * @return Указатель на найденую ноду, если такого ключа нет будет возвращена
   * концевая нода nil_.
   */
  node_type *search(const K &key) {
    node_type *res = root;
    K node_key = kov(res->val);
    while (res != nil_ && key != node_key) {
      if (comp(key, node_key)) {
        res = res->left;
      } else {
        res = res->right;
      }
      node_key = kov(res->val);
    }
    return res;
  }

  /**
   *  @brief Находит начало подпоследовательности, соответствующей заданному
   * ключу.
   *  @param key - ключ для поиска элементов.
   *  @return Нода элемента, значение которой равно или больший, чем
   * ключ или end().
   */
  node_type *lower_bound(const K &key) noexcept {
    node_type *res = root;
    node_type *prev = root;
    K node_key = kov(res->val);
    while (res != nil_ && key != node_key) {
      if (comp(key, node_key)) {
        prev = res;
        res = res->left;
      } else {
        prev = res;
        res = res->right;
      }
      node_key = kov(res->val);
    }
    if (res == nil_ && comp(key, kov(prev->val))) res = prev;
    return res;
  }

  /**
   * @brief Добавляет ноды из other в текущее дерево, елси добавление не удалось
   * нода остается в other.
   * @param other Указатель на дерево для слияния.
   * @param unique_keys Опредеяет будут ли ключи уникалными.
   */
  void merge(Rb_tree *other, bool unique_keys) noexcept {
    node_type *old_root = other->root;
    old_root->p = other->nil_;
    other->nil_->p = other->nil_;
    other->root = other->nil_;
    other->node_count = 0;

    if (old_root == other->nil_) return;

    post_order_process(old_root, other->nil_,
                       [this, &other, unique_keys](node_type *node) {
                         if (this->insert_node(node, unique_keys) == true) {
                           ++(this->node_count);

                         } else {
                           other->insert_node(node, unique_keys);
                           ++(other->node_count);
                         }
                       });
  }

  void swap(Rb_tree &other) noexcept {
    if (this != &other) {
      std::swap(root, other.root);
      std::swap(nil_, other.nil_);
      std::swap(node_count, other.node_count);
      std::swap(comp, other.comp);
      std::swap(alloc, other.alloc);
    }
  }

 private:
  /**
   * @brief Добавление новую ноду в дерево.
   * @param node Добавляемая нода.
   * @param unique_keys Флаг определяющий будут ли ключи уникльными.
   * @return Если нода добавлена в дерево true, иначе false
   */
  bool insert_node(node_type *node, bool unique_keys = false) {
    bool created{true};

    node_type *father = nil_;
    node_type *current = root;

    // Поиск родителя.
    while (current != nil_) {
      father = current;

      // Если только уникльные ключи и ключ уже существует.
      if (unique_keys == true && !comp(kov(node->val), kov(current->val)) &&
          !comp(kov(current->val), kov(node->val))) {
        created = false;
        break;
      }

      if (comp(kov(node->val), kov(current->val))) {
        current = current->left;
      } else {
        current = current->right;
      }
    }

    if (created == true) {
      link_new_node(father, node);

      if (node->p != nil_ && node->p->color == Red && node->p->p != nil_) {
        insert_fixup(node);
      }
    }
    return created;
  }

  /**
   * @brief Выполняет post_order обход с применением переданной функции.
   * @param subtree_root Указатель на корень дерева.
   * @param nil Указатель на концевой узел дерева.
   * @param action Функция.
   */
  template <typename Action>
  void post_order_process(node_type *subtree_root, node_type *nil,
                          Action action) noexcept {
    if (subtree_root == nil) return;

    node_type *current = subtree_root;
    while (current != nil) {
      if (current->left != nil) {
        // Идём влево
        current = current->left;
      } else if (current->right != nil) {
        // Идём вправо
        current = current->right;
      } else {
        // Лист - обрабатываем
        node_type *parent = current->p;

        // Отсоединяем от родителя (если он есть)
        if (parent != nil) {
          if (current == parent->left) {
            parent->left = nil;
          } else {
            parent->right = nil;
          }
        }

        node_type *to_process = current;
        current = parent;  // Возвращаемся на уровень выше

        to_process->color = Red;
        action(to_process);  // Применяем действие к узлу
      }
    }
  }

  /**
   * @brief Создает копию дерева.
   * @return Указатель на корень нового дерева.
   */
  void copy_tree(const node_type *other_root, const node_type *other_nil) {
    if (other_root == other_nil) {
      root = nil_;
    }
    // Стек для эмуляции рекурсии: храним пары <оригинальный узел, родитель
    // копии>
    std::stack<std::pair<const node_type *, node_type *>> stack;
    node_type *new_root = create_node(other_root->val);
    new_root->color = other_root->color;
    new_root->p = nil_;  // Родитель корня — nil_
    root = new_root;

    stack.push({other_root, new_root});

    while (!stack.empty()) {
      auto [orig_node, copy_node] = stack.top();
      stack.pop();
      // Копируем правое поддерево (кладём в стек первым, чтобы обработать левое
      // раньше)
      if (orig_node->right != other_nil) {
        copy_node->right = create_node(orig_node->right->val);
        copy_node->right->color = orig_node->right->color;
        copy_node->right->p = copy_node;
        copy_node->right->right = copy_node->right->left = nil_;
        stack.push({orig_node->right, copy_node->right});
      } else {
        copy_node->right = nil_;
      }
      // Копируем левое поддерево
      if (orig_node->left != other_nil) {
        copy_node->left = create_node(orig_node->left->val);
        copy_node->left->color = orig_node->left->color;
        copy_node->left->p = copy_node;
        copy_node->left->right = copy_node->left->left = nil_;
        stack.push({orig_node->left, copy_node->left});
      } else {
        copy_node->left = nil_;
      }
    }
  }

  /**
   * @brief Удаляет все ноды переданного дерева.
   * @param subtree_root Корень удаляемого дерева.
   * @param nil Концевой узел удаляемого дерева.
   */
  void destroy_subtree(node_type *subtree_root, node_type *nil) noexcept {
    post_order_process(subtree_root, nil, [this](node_type *node) {
      if (node != nil_) destroy_node(node);
    });
  }

  /**
   * @brief Корректно уничтожает узел.
   * @param node Указатель на узел для удаления.
   */
  void destroy_node(node_type *node) noexcept {
    node_alloc_traits::destroy(alloc, node);
    node_alloc_traits::deallocate(alloc, node, 1);
  }

  /**
   * @brief Связывает новый узел с родителем.
   * @param father Указатель на родительскую ноду.
   * @param new_node Указатель на ноду потомка.
   */
  void link_new_node(node_type *father, node_type *new_node) {
    new_node->left = new_node->right = nil_;

    if (father == nil_) {
      root = new_node;
      root->p = nil_;
      root->color = Black;
    } else {
      new_node->p = father;
      // Определяем new_node левый или правый потомок.
      if (comp(kov(new_node->val), kov(father->val))) {
        father->left = new_node;
      } else {
        father->right = new_node;
      }
    }
  }

  /**
   * @brief Создает новую ноду инициализированную переданными занчениями.
   * @param value Ссылка на значение.
   * @return Созданную ноду.
   * @throw std::bad_alloc или исключение брошенное конструктором пердаваемого
   * типа.
   */
  node_type *create_node(const V &value) {
    node_type *new_node = node_alloc_traits::allocate(alloc, 1);
    try {
      node_alloc_traits::construct(alloc, new_node, value);
      return new_node;
    } catch (...) {
      node_alloc_traits::deallocate(alloc, new_node, 1);
      throw;
    }
  }

  /**
   * @brief Создает новую ноду.
   * @param key Ссылка на ключ.
   * @param value Ссылка на значение.
   * @param created Ссылка на значение определяющее была ли создана нода.
   * @param unique_keys Флаг определяющий будут ли ключи уникльными.
   * @return Новая нода.
   * @note Если ключи должны быть уникальными при нахождении дубликата будет
   * возвращен дубликат и нода не будет создана.
   */
  node_type *find_or_create(const K &key, const V &value, bool &created,
                            bool unique_keys) {
    node_type *father = nil_;
    node_type *current = root;
    created = true;

    // Поиск родителя.
    while (current != nil_) {
      father = current;

      // Если только уникльные ключи и ключ уже существует.
      if (unique_keys == true && !comp(key, kov(current->val)) &&
          !comp(kov(current->val), key)) {
        created = false;
        break;
      }

      if (comp(key, kov(current->val))) {
        current = current->left;
      } else {
        current = current->right;
      }
    }

    if (created == true) {
      node_type *new_node = create_node(value);
      link_new_node(father, new_node);
      current = new_node;
    }

    return current;
  }

  /**
   * @brief Производит перебалансировку дерева в случае необхоимости.
   * @param node Указатель на ноду для которой выполняется ребаланс дерева.
   */
  void insert_fixup(node_type *node) noexcept {
    while (node->p->color == Red) {
      if (node->p == node->p->p->left) {
        node = rebalance_left(node);
      } else {
        node = rebalance_right(node);
      }
    }
    root->color = Black;
  }

  /**
   * @brief Производит перебалансировку для потомка слева.
   * @param node Указатель на ноду для которой выполняется ребаланс дерева.
   * @return Следующая нода для которой нужно выполнить балансировку.
   */
  node_type *rebalance_left(node_type *node) noexcept {
    // temp - дядя ноды.
    node_type *temp = node->p->p->right;
    // Если дядя красный перекрашиваем родителя и дядю в черный, а дедушку в
    // красный.
    if (temp->color == Red) {
      temp->color = node->p->color = Black;
      node->p->p->color = Red;
      node = node->p->p;
    } else {
      // Если Left-right imbalance (LR) приводим к Left-left imbalance (LL).
      if (node == node->p->right) {
        node = node->p;
        left_rotate(node);
      }
      node->p->color = Black;
      node->p->p->color = Red;
      right_rotate(node->p->p);
    }

    return node;
  }

  /**
   * @brief Производит перебалансировку для потомка справа.
   * @param node Указатель на ноду для которой выполняется ребаланс дерева.
   * @return Следующая нода для которой нужно выполнить балансировку.
   */
  node_type *rebalance_right(node_type *node) {
    // temp - дядя ноды
    node_type *temp = node->p->p->left;
    // Если дядя красный перекрашиваем родителя и дядю в черный, а дедушку в
    // красный.
    if (temp->color == Red) {
      temp->color = node->p->color = Black;
      node->p->p->color = Red;
      node = node->p->p;
    } else {
      // Если Right-left imbalance (RL) приводим к Right-right imbalance (RR)
      if (node == node->p->left) {
        node = node->p;
        right_rotate(node);
      }
      node->p->color = Black;
      node->p->p->color = Red;
      left_rotate(node->p->p);
    }

    return node;
  }

  /**
   * @brief Выполняет поворот вправо.
   * @param parent_node Указатель на ноду (родитель).
   */

  void right_rotate(node_type *const parent_node) noexcept {
    node_type *const child = parent_node->left;

    // Перемещаем правое поддерево child в левое поддерево parent_node.
    parent_node->left = child->right;

    // Обновляем родителя перемещённого поддерева.
    if (child->right != nil_) {
      child->right->p = parent_node;
    }

    // Дед становится родителем внука.
    child->p = parent_node->p;
    if (parent_node->p == nil_) {
      root = child;
    } else if (parent_node == parent_node->p->right) {
      parent_node->p->right = child;
    } else {
      parent_node->p->left = child;
    }

    // Делаем parent_node правым потомком child.
    child->right = parent_node;
    parent_node->p = child;
  }

  /**
   * @brief Выполняет поворот влево.
   * @param parent_node Указатель на ноду.
   */
  void left_rotate(node_type *const parent_node) noexcept {
    node_type *const child = parent_node->right;

    // Перемещаем левое поддерево child в правое поддерево parent_node.
    parent_node->right = child->left;

    // Обновляем родителя перемещённого поддерева.
    if (child->left != nil_) {
      child->left->p = parent_node;
    }

    // Дед становится родителем внука.
    child->p = parent_node->p;
    if (parent_node->p == nil_) {
      root = child;
      // Для левого потомка.
    } else if (parent_node == parent_node->p->left) {
      parent_node->p->left = child;
      // Для правого.
    } else {
      parent_node->p->right = child;
    }

    // Делаем parent_node левым потомком child.
    child->left = parent_node;
    parent_node->p = child;
  }

  /**
   * @brief Меняет ноду u на ноду v.
   * @param u Удаляемая нода.
   * @param v Нода которая станет на место удаляемой.
   * @note Цвет замененной ноды такой же как ноды v.
   */
  void transplant(node_type *u, node_type *v) noexcept {
    if (u->p == nil_) {
      root = v;
    } else if (u == u->p->left) {
      u->p->left = v;
    } else {
      u->p->right = v;
    }

    v->p = u->p;
  }

  /**
   * @brief Производит пeребалансировку дерева.
   * @param x Нода с которой начинается балансировка.
   */
  void delete_fixup(node_type *x) noexcept {
    while (x != root && x->color == Black) {
      if (x == x->p->left) {
        x = delete_fixup_left(x);
      } else {
        x = delete_fixup_right(x);
      }
    }
    x->color = Black;
  }

  /**
   * @brief Производит пребалансировку дерева левого потомка.
   * @param x Нода с которой начинается балансировка.
   */
  node_type *delete_fixup_left(node_type *x) noexcept {
    node_type *w = x->p->right;
    if (w->color == Red) {
      w->color = Black;
      x->p->color = Red;
      left_rotate(x->p);
      w = x->p->right;
    }

    if (w->left->color == Black && w->right->color == Black) {
      w->color = Red;
      x = x->p;
    } else {
      if (w->right->color == Black) {
        w->left->color = Black;
        w->color = Red;
        right_rotate(w);
        w = x->p->right;
      }

      w->color = x->p->color;
      w->right->color = Black;
      x->p->color = Black;
      left_rotate(x->p);
      x = root;
    }
    return x;
  }

  /**
   * @brief Производит пребалансировку дерева праваого потомка.
   * @param x Нода с которой начинается балансировка.
   */
  node_type *delete_fixup_right(node_type *x) noexcept {
    node_type *w = x->p->left;
    if (w->color == Red) {
      w->color = Black;
      x->p->color = Red;
      right_rotate(x->p);
      w = x->p->left;
    }

    if (w->right->color == Black && w->left->color == Black) {
      w->color = Red;
      x = x->p;
    } else {
      if (w->left->color == Black) {
        w->right->color = Black;
        w->color = Red;
        left_rotate(w);
        w = x->p->left;
      }
      w->color = x->p->color;
      w->left->color = Black;
      x->p->color = Black;
      right_rotate(x->p);
      x = root;
    }
    return x;
  }

 public:
  /**
   * @brief Обычный итератор.
   * @note Если итератор выходит за пределы поледовательности то его текущее
   * значение будет nil. Если текущее значение nil то при инкременте итератор
   * возвратится на начало коллекции, при декременте в конец.
   */
  class Rb_tree_iterator {
    node_type *current;
    Rb_tree *tree;

   public:
    using value_type = V;
    using reference = value_type &;

    explicit Rb_tree_iterator(node_type *node, Rb_tree *t)
        : current(node), tree(t) {}

    node_type *get_current() { return current; }

    /**
     * @brief Проверяет принадлежит ли итератор переданному дереву.
     * @param other Дерево.
     */
    bool is_same_iterator(const Rb_tree *other) const { return tree == other; }

    reference operator*() { return current->val; }

    Rb_tree_iterator &operator++() {
      increment();
      return *this;
    }

    Rb_tree_iterator operator++(int) {
      Rb_tree_iterator tmp = *this;
      increment();
      return tmp;
    }

    Rb_tree_iterator &operator--() {
      decrement();
      return *this;
    }

    Rb_tree_iterator operator--(int) {
      Rb_tree_iterator tmp = *this;
      decrement();
      return tmp;
    }

    bool operator==(const Rb_tree_iterator &other) const {
      return current == other.current;
    }

    bool operator!=(const Rb_tree_iterator &other) const {
      return !(*this == other);
    }

   private:
    void increment() {
      const node_type *nil = tree->get_nil();
      if (current == nil) {
        current = tree->minimum(tree->get_root());
        return;
      }
      if (current->right != nil) {
        current = current->right;
        while (current->left != nil) current = current->left;
      } else {
        node_type *father = current->p;
        while (father != nil && current == father->right) {
          current = father;
          father = father->p;
        }
        current = father;
      }
    }

    void decrement() {
      const node_type *nil = tree->get_nil();
      if (current == nil) {
        current = tree->maximum(tree->get_root());
        return;
      }
      if (current->left != nil) {
        current = current->left;
        while (current->right != nil) current = current->right;
      } else {
        Node<V> *father = current->p;
        while (father != nil && current == father->left) {
          current = father;
          father = father->p;
        }
        current = father;
      }
    }
  };  // class Rb_tree_iterator

  /**
   * @brief Константный итератор
   * @note Если итератор выходит за пределы поледовательности то его текущее
   * значение будет nil. Если текущее значение nil то при инкременте итератор
   * возвратится на начало коллекции, при декременте в конец.
   */
  class Rb_tree_const_iterator {
    const node_type *current;
    const Rb_tree *tree;

   public:
    using value_type = V;
    using const_reference = const value_type &;

    explicit Rb_tree_const_iterator(const node_type *node, const Rb_tree *t)
        : current(node), tree(t) {}

    const node_type *get_current() const noexcept { return current; }

    /**
     * @brief Проверяет принадлежит ли итератор переданному дереву.
     * @param other Дерево.
     */
    bool is_same_iterator(const Rb_tree *other) const { return tree == other; }

    const_reference operator*() const { return current->val; }

    Rb_tree_const_iterator &operator++() {
      increment();
      return *this;
    }

    Rb_tree_const_iterator operator++(int) {
      Rb_tree_const_iterator tmp = *this;
      increment();
      return tmp;
    }

    Rb_tree_const_iterator &operator--() {
      decrement();
      return *this;
    }

    Rb_tree_const_iterator operator--(int) {
      Rb_tree_const_iterator tmp = *this;
      decrement();
      return tmp;
    }

    bool operator==(const Rb_tree_const_iterator &other) const {
      return current == other.current;
    }

    bool operator!=(const Rb_tree_const_iterator &other) const {
      return !(*this == other);
    }

   private:
    void increment() {
      const node_type *nil = tree->get_nil();
      if (current == nil) {
        current = tree->minimum(tree->get_root());
        return;
      }
      if (current->right != nil) {
        current = current->right;
        while (current->left != nil) current = current->left;
      } else {
        const node_type *father = current->p;
        while (father != nil && current == father->right) {
          current = father;
          father = father->p;
        }
        current = father;
      }
    }

    void decrement() {
      const node_type *nil = tree->get_nil();
      if (current == nil) {
        current = tree->maximum(tree->get_root());
        return;
      }
      if (current->left != nil) {
        current = current->left;
        while (current->right != nil) current = current->right;
      } else {
        const node_type *father = current->p;
        while (father != nil && current == father->left) {
          current = father;
          father = father->p;
        }
        current = father;
      }
    }
  };  // class Rb_tree_const_iterator
};  // class Rb_tree

}  // namespace s21

#endif  // S21_RED_BLACK_TREE_H