#ifndef S21_MULTISET_H
#define S21_MULTISET_H

#include <vector>

#include "s21_red_black_tree.h"

namespace s21 {

template <typename Key, typename Compare = std::less<Key>,
          typename Alloc = std::allocator<Key>>
class multiset {
 public:
  using value_type = Key;
  using key_type = Key;
  using reference = value_type&;
  using const_reference = const value_type&;
  using BinaryTree = Rb_tree<Key, Key, std::identity, Compare, Alloc>;
  using node_type = BinaryTree::node_type;
  using iterator = typename BinaryTree::const_iterator;
  using const_iterator = typename BinaryTree::const_iterator;
  using size_type = std::size_t;

 private:
  BinaryTree* tree;

 public:
  /**
   * @brief Конструктор по умолчанию, не создает элементов.
   */
  multiset() { tree = new BinaryTree; }

  /**
   * @brief Конструктор из списка инициализации.
   * @note В случае возникновения исключения новые элементы удаляются и multiset
   * остается пустым.
   */
  multiset(std::initializer_list<value_type> const& items) : multiset{} {
    auto curr = std::begin(items);
    try {
      for (; curr != std::end(items); ++curr) {
        tree->insert(*curr, *curr, false);
      }
    } catch (...) {
      ++curr;  // Устанавливаем в позицию после последнего добавленного.
      for (auto it = std::begin(items); it != curr; ++it) {
        node_type* node = tree->search(*it);
        tree->delete_node(node);
      }
      throw;
    }
  }

  /**
   * @brief Конструктор копирования.
   * @param other Сылка на другое множество.
   */
  multiset(const multiset& other) { tree = new BinaryTree(*other.tree); }

  /**
   * @brief Конструктор перемещения.
   * @param other rvalue на другое множество.
   */
  multiset(multiset&& other) noexcept {
    tree = other.tree;
    other.tree = new BinaryTree;
  }

  /**
   * @brief Деструктор удаляет только сами элементы. Важно отметить, что если
   * элементы являются указателями, то память, на которую они указывают, никак
   * не затрагивается. Управление памятью, на которую указывают указатели,
   * является ответственностью пользователя.
   */
  ~multiset() { delete tree; }

  /**
   * @brief Оператор присваивания для множества.
   */
  multiset& operator=(const multiset& other) {
    if (this != &other) {
      delete tree;
      tree = new BinaryTree(*other.tree);
    }
    return *this;
  }

  /**
   * @brief Оператор присваивающего перемещения.
   */
  multiset& operator=(multiset&& other) noexcept {
    if (this != &other) {
      delete tree;
      tree = other.tree;
      other.tree = new BinaryTree;
    }
    return *this;
  }

  /**
   * @brief Возвращает константный итератор, указывающий на первый элемент
   * множества. Итерация выполняется в порядке возрастания ключей.
   */
  inline iterator begin() const { return tree->cbegin(); }

  /**
   * @brief Возвращает константный итератор, указывающий на позицию после
   * последнего элемента множества. Итерация выполняется в порядке возрастания
   * ключей.
   */
  inline iterator end() const { return tree->cend(); }

  /**
   * @brief Возвращает true, если множество пустое.
   */
  inline bool empty() const noexcept { return tree->empty(); }

  /**
   * @brief Возвращает размер множества.
   */
  inline size_type size() const noexcept { return tree->size(); }

  /**
   * @brief Возвращает максимально возможный размер множества.
   */
  inline size_type max_size() noexcept { return tree->max_size(); }

  /**
   * @brief Удаляет все элементы из множества. Важно отметить, что эта функция
   * удаляет только сами элементы, и если элементы являются указателями, то
   * память, на которую они указывают, никак не затрагивается. Управление
   * памятью, на которую указывают указатели, является ответственностью
   * пользователя.
   */
  void clear() { tree->clear(); }

  /**
   * @brief Добавляет элемент в множество.
   * @param value Элемент для вставки.
   * @return Итератор, указывающий на добавленный элемент.
   */
  iterator insert(const value_type& value) {
    auto res = tree->insert(value, value, false);
    return iterator(res.first, tree);
  }

  /**
   * @brief Удаляет элемент переданный в итераторе.
   * @param pos Ожидает итератор, который принадлежит тому же самому
   * контейнеру, из которого происходит удаление.
   */
  void erase(iterator pos) {
    if (pos.is_same_iterator(tree))
      tree->delete_node(const_cast<node_type*>(pos.get_current()));
  }

  /**
   * @brief Удаляет элемент по ключу.
   * @param key Ключ.
   */
  void erase(key_type key) { tree->delete_node(tree->search(key)); }

  /**
   * @brief Обменивает данные с другим множеством
   * @param other Множество того же типа элементов.
   */
  void swap(multiset& other) noexcept {
    if (this != &other) std::swap(tree, other.tree);
  }

  /**
   * @brief Объединяет два множества.
   * @param other Множество для объединения.
   * @note Переместит уникальные значения из второго множества в первое, не
   * уникальные значения для первого множества перенесены не будут.
   */
  void merge(multiset& other) {
    if (this != &other) tree->merge(other.tree, false);
  }

  /**
   *  @brief Пытается найти элемент в наборе.
   *  @param key Элемент который должен быть найден.
   *  @return Итератор, указывающий на искомый элемент или end(), если он не
   * найден.
   */
  iterator find(const Key& key) { return iterator(tree->search(key), tree); }

  /**
   *  @brief Подсчитывает количество элементов с указанным ключом.
   *  @param key  Ключ к элементам, которые необходимо найти.
   *  @return Количество элементов с указанным ключом.
   */
  size_type count(const Key& key) {
    auto it = find(key);
    size_type count = 0;
    while (*it == key) {
      ++count;
      ++it;
    }
    return count;
  }

  /**
   *  @brief Определяет, существует ли элемент с заданным ключом..
   *  @param key - ключ для поиска элементов.
   *  @return Значение True, если существует какой-либо элемент с указанным
   * ключом.
   */
  bool contains(const Key& key) {
    return tree->search(key) == tree->get_nil() ? false : true;
  }

  /**
   *  @brief Находит начало подпоследовательности, соответствующей заданному
   * ключу.
   *  @param key - ключ для поиска элементов.
   *  @return Итератор, указывающий на первый элемент, равный или больший, чем
   * ключ или end().
   */
  iterator lower_bound(const Key& key) {
    return iterator(tree->lower_bound(key), tree);
  }

  /**
   *  @brief Находит конец подпоследовательности, соответствующей заданному
   * ключу.
   *  @param key - ключ для поиска элементов.
   *  @return Итератор, указывающий на первый элемент который больше, чем key
   * или end().
   */
  iterator upper_bound(const Key& key) {
    iterator it = iterator(tree->lower_bound(key), tree);
    while (it != end() && *it == key) ++it;
    return it;
  }

  /**
   *  @brief Находит подпоследовательность, соответствующую заданному ключу.
   *  @param key - ключ для поиска элементов.
   *  @return Пару итераторов, которые, возможно, указывают на
   * подпоследовательность, соответствующую заданному ключу.
   *
   *  This function is equivalent to
   *  @code
   *    std::make_pair(c.lower_bound(val),
   *                   c.upper_bound(val))
   */
  std::pair<iterator, iterator> equal_range(const Key& key) {
    iterator it_start = iterator(tree->lower_bound(key), tree);
    iterator it_end = it_start;
    while (it_end != end() && *it_end == key) ++it_end;
    return {it_start, it_end};
  }

  /**
   * @brief Вставляет несколько элементов в контейнер за одну
   * операцию.
   *
   * @tparam Args Типы аргументов (должны быть конвертируемы в `value_type`
   * контейнера).
   * @param args Аргументы для вставки (поддерживает perfect forwarding).
   *
   * @return std::vector<std::pair<iterator, bool>> Вектор пар:
   *         - Итератор на вставленный/существующий элемент.
   *         - Флаг `bool`: `true` если элемент был вставлен, `false` если уже
   * существовал.
   *
   * @throws Любые исключения, которые могут сгенерировать `insert()` для
   * отдельного элемента. В этом случае контейнер остаётся в исходном состоянии
   * (строгая гарантия исключений).
   *
   * @note Если хотя бы одна вставка завершается исключением, все предыдущие
   * вставки откатываются.
   */
  template <typename... Args>
  std::vector<std::pair<iterator, bool>> insert_many(Args&&... args) {
    std::vector<std::pair<iterator, bool>> res;
    res.reserve(sizeof...(Args));
    try {
      (res.push_back({insert(std::forward<Args>(args)), true}), ...);
    } catch (...) {
      for (auto& it : res) erase(it.first);
      throw;
    }
    return res;
  }

};  // class multiset

}  // namespace s21

#endif  // S21_MULTISET_H
