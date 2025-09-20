#ifndef S21_SET_H
#define S21_SET_H

#include <vector>

#include "s21_red_black_tree.h"

namespace s21 {
template <typename Key, typename Compare = std::less<Key>,
          typename Alloc = std::allocator<Key>>
class set {
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
  set() { tree = new BinaryTree; }

  /**
   * @brief Конструктор из списка инициализации.
   * @note В случае возникновения исключения новые элементы удаляются и set
   * остается пустым.
   */
  set(std::initializer_list<value_type> const& items) : set{} {
    auto curr = std::begin(items);
    try {
      for (; curr != std::end(items); ++curr) {
        tree->insert(*curr, *curr, true);
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
  set(const set& other) { tree = new BinaryTree(*other.tree); }

  /**
   * @brief Конструктор перемещения.
   * @param other rvalue на другое множество.
   */
  set(set&& other) noexcept {
    tree = other.tree;
    other.tree = new BinaryTree;
  }

  /**
   * @brief Деструктор удаляет только сами элементы. Важно отметить, что если
   * элементы являются указателями, то память, на которую они указывают, никак
   * не затрагивается. Управление памятью, на которую указывают указатели,
   * является ответственностью пользователя.
   */
  ~set() { delete tree; }

  /**
   * @brief Оператор присваивания для множества.
   */
  set& operator=(const set& other) {
    if (this != &other) {
      delete tree;
      tree = new BinaryTree(*other.tree);
    }
    return *this;
  }

  /**
   * @brief Оператор присваивающего перемещения.
   */
  set& operator=(set&& other) noexcept {
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
  void clear() noexcept { tree->clear(); }

  /**
   * @brief Пытается вставить элемент в множество
   * @param value Элемент для вставки
   * @return Пара, где первый элемент — итератор, указывающий на возможно
   * вставленный элемент, а второй — булево значение, которое равно true, если
   * элемент действительно был вставлен
   *
   * Эта функция пытается вставить элемент в множество. Множество использует
   * уникальные ключи, поэтому элемент вставляется только если его ещё нет в
   * множестве.
   */
  std::pair<iterator, bool> insert(const value_type& value) {
    auto res = tree->insert(value, value, true);
    return {iterator(res.first, tree), res.second};
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
  void swap(set& other) noexcept {
    if (this != &other) std::swap(tree, other.tree);
  }

  /**
   * @brief Объединяет два множества.
   * @param other Множество для объединения.
   * @note Переместит уникальные значения из второго множества в первое, не
   * уникальные значения для первого множества перенесены не будут.
   */
  void merge(set& other) {
    if (this != &other) tree->merge(other.tree, true);
  }

  /**
   * @brief Пытается найти элемент в множестве.
   * @param key Ключ.
   * @return Итератор, указывающий на искомый элемент, или end(), если элемент
   * не найден.
   */
  iterator find(const Key& key) { return iterator(tree->search(key), tree); }

  /**
   * @brief Проверяет наличие элемента с заданным ключом
   * @param key Ключ элемента для поиска
   * @return true, если элемент с указанным ключом существует
   */
  bool contains(const Key& key) {
    return tree->search(key) == tree->get_nil() ? false : true;
  }

  /**
   * @brief Вставляет несколько уникльных элементов в контейнер за одну
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
      (res.push_back(insert(std::forward<Args>(args))), ...);
    } catch (...) {
      for (auto& it : res)
        if (it.second == true) erase(it.first);
      throw;
    }
    return res;
  }

};  // class set
}  // namespace s21

#endif  // S21_SET_H