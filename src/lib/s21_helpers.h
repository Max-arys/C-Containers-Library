#ifndef S21_HELPERS_H
#define S21_HELPERS_H

namespace s21 {

/**
 * @brief Структура-функтор принимает сонстантную ссылку на объект и возвращает
 * его.
 * @note Функтор возвращает контантную ссылку переданного значения.
 */
struct Identity {
  template <typename Val>
  const Val& operator()(const Val& k) const {
    return k;
  }
};

/**
 * @brief Структура-функтор принимает сонстантную ссылку на объект pair и
 * возвращает его первое консеантное значение.
 * @note Функтор возвращает контантную ссылку на первый элемент объекта pair.
 */
struct Select1st {
  template <typename Pair>
  const typename Pair::first_type& operator()(const Pair& p) const {
    return p.first;
  }
};

/**
 * @brief Структура-функтор сравнивает два значения одного типа.
 */
template <typename T>
struct less {
  constexpr bool operator()(const T& lhs, const T& rhs) const {
    return lhs < rhs;
  }
};

}  // namespace s21

#endif  // S21_HELPERS_H