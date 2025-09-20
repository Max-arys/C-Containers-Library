#ifndef S21_MAP_H
#define S21_MAP_H

#include <vector>

#include "s21_red_black_tree.h"

namespace s21 {
template <typename K, typename T, typename Compare = std::less<K>,
          typename Alloc = std::allocator<std::pair<const K, T>>>
class map {
 public:
  using key_type = K;
  using mapped_type = T;
  using value_type = std::pair<const K, T>;
  using reference = value_type&;
  using const_reference = const value_type&;
  using BinaryTree = Rb_tree<K, value_type, s21::Select1st, Compare, Alloc>;
  using iterator = typename BinaryTree::iterator;
  using const_iterator = typename BinaryTree::const_iterator;
  using size_type = std::size_t;

 private:
  using node_type = BinaryTree::node_type;
  BinaryTree* tree;

 public:
  /**
   * @brief Конструктор по умолчанию, не создает элементов.
   */
  map() { tree = new BinaryTree; }

  /**
   * @brief Конструктор из списка инициализации.
   * @note В случае возникновения исключения новые элементы удаляются и set
   * остается пустым.
   */
  map(std::initializer_list<value_type> const& items) : map{} {
    auto curr = std::begin(items);
    try {
      for (; curr != std::end(items); ++curr) {
        tree->insert((*curr).first, *curr, true);
      }
    } catch (...) {
      ++curr;  // Устанавливаем в позицию после последнего добавленного.
      for (auto it = std::begin(items); it != curr; ++it) {
        node_type* node = tree->search((*it).first);
        tree->delete_node(node);
      }
      throw;
    }
  }

  /**
   * @brief Конструктор копирования.
   * @param other Сылка на другой map.
   */
  map(const map& other) { tree = new BinaryTree(*other.tree); }

  /**
   * @brief Конструктор перемещения.
   * @param other rvalue на другой map.
   */
  map(map&& other) noexcept {
    tree = other.tree;
    other.tree = new BinaryTree;
  }

  /**
   * @brief Деструктор удаляет только сами элементы. Важно отметить, что если
   * элементы являются указателями, то память, на которую они указывают, никак
   * не затрагивается. Управление памятью, на которую указывают указатели,
   * является ответственностью пользователя.
   */
  ~map() { delete tree; }

  /**
   * @brief Оператор присваивания для map.
   */
  map& operator=(const map& other) {
    if (this != &other) {
      delete tree;
      tree = new BinaryTree(*other.tree);
    }
    return *this;
  }

  /**
   * @brief Оператор присваивающего перемещения.
   */
  map& operator=(map&& other) noexcept {
    if (this != &other) {
      delete tree;
      tree = other.tree;
      other.tree = new BinaryTree;
    }
    return *this;
  }

  /**
   * @brief Доступ к данным map через оператор индексации ( [] ).
   * @param key Ключ, по которому нужно получить данные.
   * @return Ссылка на данные.
   *
   * Обеспечивает удобный поиск с помощью оператора индексации ( @c [] ).
   * Возвращает данные, связанные с ключом, указанным в квадратных скобках.
   * Если ключ не существует, создается пара с этим ключом, используя значения
   * по умолчанию, которая затем возвращается.
   */
  T& operator[](const K& key) {
    auto node = tree->search(key);
    if (node == tree->get_nil()) {
      auto p = insert(value_type(key, mapped_type()));
      // Из пары получаем итератор, из итератора получаем ноду, из ноды(пара),
      // получаем значение.
      return (*(p.first)).second;
    } else {
      return node->val.second;
    }
  }

  /**
   * @brief Поучает ссылку на значение по ключу.
   * @param key Константная ссылка на ключ.
   * @return Значение по ссылке.
   * @throw std::out_of_range("map::at") если такого ключа нет в map.
   */
  mapped_type& at(const key_type& key) {
    auto node = tree->search(key);
    if (node == tree->get_nil()) {
      throw std::out_of_range("map::at");
    }
    return (node->val).second;
  }

  /**
   * Возвращает изменяемый (read/write) итератор, указывающий на первую пару в
   * map. Итерация выполняется в порядке возрастания ключей.
   */
  inline iterator begin() { return tree->begin(); }

  /**
   * Возвращает константный (только для чтения) итератор, указывающий на первую
   * пару в map. Итерация выполняется в порядке возрастания ключей.
   */
  inline const_iterator begin() const { return tree->cbegin(); }

  /**
   * Возвращает изменяемый (read/write) итератор, указывающий на позицию после
   * последней пары в map. Итерация выполняется в порядке возрастания ключей.
   */
  inline iterator end() { return tree->end(); }

  /**
   * Возвращает константный (только для чтения) итератор, указывающий на позицию
   * после последней пары в map. Итерация выполняется в порядке возрастания
   * ключей.
   */
  inline const_iterator end() const { return tree->cend(); }

  /**
   * Возвращает true, если карта пуста (в этом случае begin() будет равен
   * end()).
   */
  inline bool empty() const noexcept { return tree->empty(); }

  /**
   * Возвращает количество элементов (размер) в map.
   */
  inline size_type size() const noexcept { return tree->size(); }

  /**
   * @brief Возвращает максимально возможный размер map.
   */
  inline size_type max_size() noexcept { return tree->max_size(); }

  /**
   * @brief Удаляет все элементы из map. Важно отметить, что эта функция
   * удаляет только сами элементы, и если элементы являются указателями, то
   * память, на которую они указывают, никак не затрагивается. Управление
   * памятью, на которую указывают указатели, является ответственностью
   * пользователя.
   */
  inline void clear() { tree->clear(); }

  /**
   * @brief Добавляет новое значение в коллекцию только если такого ключа еще
   * нет.
   * @param value Пара ключ-значение.
   * @return Пара итератор на элемент сообтветствующий ключу, булево значение
   * указывающее на то был ли зоздан элемент.
   */
  std::pair<iterator, bool> insert(const value_type& value) {
    auto res = tree->insert(value.first, value, true);
    return {iterator(res.first, tree), res.second};
  }

  /**
   * @brief Добавляет новое значение в коллекцию только если такого ключа еще
   * нет.
   * @param key Ключ.
   * @param obj Значение.
   * @return Пара итератор на элемент сообтветствующий ключу, булево значение
   * указывающее на то был ли зоздан элемент.
   */
  std::pair<iterator, bool> insert(const K& key, const T& obj) {
    auto res = tree->insert(key, {key, obj}, true);
    return {iterator(res.first, tree), res.second};
  }

  /**
   * @brief Добавляет новое значение в коллекцию, если ключ существует заменит
   * его.
   * @param key Ключ.
   * @param obj Значение.
   * @note Пара итератор на элемент сообтветствующий ключу, булево значение
   * указывающее на то был ли зоздан элемент.
   */
  std::pair<iterator, bool> insert_or_assign(const K& key, const T& obj) {
    auto res = tree->insert(key, {key, obj}, true);
    // Из пары получаем ноду, из ноды значение(пара), из значения второе
    // значение.
    if (res.second == false) res.first->val.second = obj;
    return {iterator(res.first, tree), res.second};
  }

  /**
   * @brief Удаляет элемент переданный в итераторе.
   * @param pos Ожидает итератор, который принадлежит тому же самому
   * контейнеру, из которого происходит удаление.
   */
  void erase(iterator pos) {
    if (pos.is_same_iterator(tree)) tree->delete_node(pos.get_current());
  }

  /**
   * @brief Обменивает данные с другом map.
   * @param other map того же типа элементов.
   */
  void swap(map& other) noexcept {
    if (this != &other) std::swap(tree, other.tree);
  }

  /**
   * @brief Объединяет два map.
   * @param other map для объединения.
   * @note Переместит уникальные значения из второго map в первое, не
   * уникальные значения для первого map перенесены не будут.
   */
  void merge(map& other) {
    if (this != &other) tree->merge(other.tree, true);
  }

  /**
   * @brief Проверяет наличие элемента с заданным ключом
   * @param key Ключ элемента для поиска
   * @return true, если элемент с указанным ключом существует
   */
  bool contains(const K& key) {
    return tree->search(key) == tree->get_nil() ? false : true;
  }

  /**
   * @brief Пытается найти элемент в map.
   * @param key Ключ.
   * @return Итератор, указывающий на искомый элемент, или end(), если элемент
   * не найден.
   */
  iterator find(const K& key) { return iterator(tree->search(key), tree); }

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
      for (auto it : res)
        if (it.second == true) erase(it.first);
      throw;
    }
    return res;
  }

};  // class map
}  // namespace s21

#endif  // S21_MAP_H