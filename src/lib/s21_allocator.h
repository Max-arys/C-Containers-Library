#ifndef S21_ALLOCATOR_H
#define S21_ALLOCATOR_H

#include <cassert>
#include <cstddef>
#include <memory>
#include <vector>

namespace s21 {

/**
 * @brief Аллокатор для контейнеров set, multiset, map. Актуален для случаев
 * обработки большого колличества элементов c базовым типом.
 * @tparam T Тип элементов, для которых выделяется память.
 * @note Позволяет получить прибавку к производительности около 30%, но
 * увеличивает расход памяти.
 * Память выделяется фрагментами. Стоит учитывать, что освобождение памяти
 * происходит
 */
template <typename T>
class pool_allocator {
 public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal = std::false_type;  // Аллокаторы не взаимозаменяемы

  explicit pool_allocator(size_type chunk_size = 1024)
      : chunk_size_(chunk_size) {}

  template <typename U>
  pool_allocator(const pool_allocator<U>& other) noexcept
      : free_list_(nullptr), chunks_(), chunk_size_(other.chunk_size()) {
    if (chunk_size_ > 0) {
      allocate_new_chunk();
    }
  }

  pool_allocator(const pool_allocator& other) noexcept
      : free_list_(nullptr), chunks_(), chunk_size_(other.chunk_size_) {
    if (other.chunk_size_ > 0) {
      allocate_new_chunk();
    }
  }

  ~pool_allocator() {
    for (void* chunk : chunks_) {
      ::operator delete(chunk);
    }
  }

  pool_allocator& operator=(const pool_allocator& other) noexcept {
    if (this != &other) {
      // Освобождаем текущие блоки
      for (void* chunk : chunks_) {
        ::operator delete(chunk);
      }
      chunks_.clear();
      free_list_ = nullptr;

      // Копируем chunk_size_ и выделяем новый пул
      chunk_size_ = other.chunk_size_;
      if (chunk_size_ > 0) {
        allocate_new_chunk();
      }
    }
    return *this;
  }

  /**
   * @brief Выделяет память для n элементов типа T.
   * @param n Количество элементов, для которых нужно выделить память.
   * @return Указатель на выделенную память.
   * @throw std::bad_alloc Если запрашиваемый размер слишком велик.
   */
  [[nodiscard]] T* allocate(size_type n) {
    if (n == 0) return nullptr;
    if (n > max_size()) {
      throw std::bad_alloc();
    }

    // Если запрашивают один элемент — берем из пула
    if (n == 1) {
      if (free_list_ == nullptr) {
        allocate_new_chunk();
      }
      auto* ptr = free_list_;
      free_list_ = free_list_->next;
      // Пирведение типов указателей на уровне битов.
      return reinterpret_cast<T*>(ptr);
    }
    // Иначе — стандартное выделение (редкий случай для set)
    else {
      if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
        return static_cast<T*>(
            ::operator new(n * sizeof(T), std::align_val_t{alignof(T)}));
      } else {
        return static_cast<T*>(::operator new(n * sizeof(T)));
      }
    }
  }

  /**
   * @brief Для одиночных объектов возвращает память аллокатору, для нескольких
   * возвращает память системе.
   * @param p Указатель на память, которую нужно освободить.
   * @param n Количество элементов, для которых нужно освободить память.
   */
  void deallocate(T* p, size_type n) noexcept {
    if (n == 1) {
      auto* node = reinterpret_cast<FreeNode*>(p);
      node->next = free_list_;
      free_list_ = node;
    } else {
      if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
        ::operator delete(p, n * sizeof(T), std::align_val_t{alignof(T)});
      } else {
        ::operator delete(p, n * sizeof(T));
      }
    }
  }

  /**
   * @brief Возвращает максимально возможное колличество элементов.
   */
  size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(T);
  }

  /**
   * @brief Возвращает размер одного блока.
   */
  size_type chunk_size() const noexcept { return chunk_size_; }

  /**
   * @brief Создает объект типа U в выделенной памяти.
   * @tparam U Тип создаваемого объекта.
   * @tparam Args Шаблон с переменным числом аргументов.
   * @param p Указатель на выделенную память для объекта.
   * @param args Аргументы для передачи в конструктор U.
   * @note Использует размещающий new (placement new) для создания объекта
   *       в заранее выделенной памяти.
   */
  template <typename U, typename... Args>
  void construct(U* p, Args&&... args) {
    ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
  }

  /**
   * @brief Уничтожает объект типа U без освобождения памяти.
   * @tparam U Тип уничтожаемого объекта.
   * @param p Указатель на объект, который нужно уничтожить.
   * @note Явно вызывает деструктор объекта, но не освобождает память.
   *       Память должна быть освобождена отдельно при необходимости.
   */
  template <typename U>
  void destroy(U* p) noexcept {
    p->~U();
  }

  /**
   * @brief Структура для перепривязки аллокатора к другому типу.
   * @tparam U Тип, для которого нужно создать аллокатор.
   */
  template <typename U>
  struct rebind {
    using other = pool_allocator<U>;
  };

  /**
   * @brief Структура для хранения свободной памяти.
   */
 private:
  struct FreeNode {
    FreeNode* next;
  };

  /**
   * @brief Выделение нового блока памяти.
   */
  void allocate_new_chunk() {
    const size_t node_size = std::max(sizeof(T), sizeof(FreeNode));
    auto* chunk =
        static_cast<FreeNode*>(::operator new(chunk_size_ * node_size));
    chunks_.push_back(chunk);

    // Формируем список свободных нод
    for (size_type i = 0; i < chunk_size_ - 1; ++i) {
      auto* current = reinterpret_cast<FreeNode*>(
          reinterpret_cast<char*>(chunk) + i * sizeof(T));
      auto* next = reinterpret_cast<FreeNode*>(reinterpret_cast<char*>(chunk) +
                                               (i + 1) * sizeof(T));
      current->next = next;
    }

    // Последний элемент указывает на nullptr
    auto* last = reinterpret_cast<FreeNode*>(reinterpret_cast<char*>(chunk) +
                                             (chunk_size_ - 1) * sizeof(T));
    last->next = nullptr;

    // Обновляем free_list_
    free_list_ = chunk;
  }

  FreeNode* free_list_ = nullptr;  // Список свободных блоков
  std::vector<void*> chunks_;      // Выделенные блоки памяти
  size_type chunk_size_;           // Размер одного блока
};  // class pool_allocator

}  // namespace s21

#endif  // S21_ALLOCATOR_H