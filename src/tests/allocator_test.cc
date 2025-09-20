#include "testing.h"

TEST(PoolAllocatorTest, AllocateDeallocateSingle) {
  s21::pool_allocator<int> alloc;

  // Выделяем память под один int
  int* ptr = alloc.allocate(1);
  ASSERT_NE(ptr, nullptr);

  // Освобождаем
  alloc.deallocate(ptr, 1);
}

TEST(PoolAllocatorTest, AllocateMultiple) {
  s21::pool_allocator<double> alloc;

  // Выделяем память под 10 double
  double* arr = alloc.allocate(10);
  ASSERT_NE(arr, nullptr);

  // Освобождаем
  alloc.deallocate(arr, 10);
}

TEST(PoolAllocatorTest, ZeroAllocation) {
  s21::pool_allocator<char> alloc;

  // Запрос нулевого размера должен вернуть nullptr
  char* ptr = alloc.allocate(0);
  ASSERT_EQ(ptr, nullptr);
}

TEST(PoolAllocatorTest, MaxSize) {
  s21::pool_allocator<long long> alloc;

  // Проверяем, что max_size() возвращает разумное значение
  EXPECT_GT(alloc.max_size(), 0);
}

TEST(PoolAllocatorTest, BadAlloc) {
  s21::pool_allocator<int> alloc;

  // Попытка выделить слишком много памяти должна бросить исключение
  EXPECT_THROW((void*)alloc.allocate(alloc.max_size() + 1), std::bad_alloc);
}

TEST(PoolAllocatorSetTest, BasicInsert) {
  s21::set<int, std::less<int>, s21::pool_allocator<int>> set;

  for (int i = 0; i < 100; ++i) {
    set.insert(i);
  }

  EXPECT_EQ(set.size(), 100);
}

TEST(PoolAllocatorSetTest, Erase) {
  s21::set<int, std::less<int>, s21::pool_allocator<int>> set;
  set.insert(42);

  set.erase(42);
  EXPECT_TRUE(set.empty());
}

TEST(PoolAllocatorVectorTest, Insert) {
  std::vector<int, s21::pool_allocator<int>> v;
  v.reserve(10);

  EXPECT_GE(v.capacity(), 10);
}

TEST(PoolAllocatorCopyTest, CopyConstructorBasic) {
  s21::pool_allocator<int> alloc1;

  // Выделяем память в оригинальном аллокаторе
  int* ptr1 = alloc1.allocate(1);
  *ptr1 = 42;

  // Копируем аллокатор
  s21::pool_allocator<int> alloc2(alloc1);

  // Проверяем, что новый аллокатор работает
  int* ptr2 = alloc2.allocate(1);
  *ptr2 = 84;

  // Проверяем, что оригинальная память не затронута
  EXPECT_EQ(*ptr1, 42);
  EXPECT_EQ(*ptr2, 84);

  alloc1.deallocate(ptr1, 1);
  alloc2.deallocate(ptr2, 1);
}

TEST(PoolAllocatorCopyTest, CopyAssignBasic) {
  s21::pool_allocator<int> alloc1;

  // Выделяем память в оригинальном аллокаторе
  int* ptr1 = alloc1.allocate(1);
  *ptr1 = 42;

  // Копируем аллокатор
  s21::pool_allocator<int> alloc2;
  alloc2 = alloc1;

  // Проверяем, что новый аллокатор работает
  int* ptr2 = alloc2.allocate(1);
  *ptr2 = 84;

  // Проверяем, что оригинальная память не затронута
  EXPECT_EQ(*ptr1, 42);
  EXPECT_EQ(*ptr2, 84);

  alloc1.deallocate(ptr1, 1);
  alloc2.deallocate(ptr2, 1);
}

TEST(PoolAllocatorTest, RebindAndDifferentType) {
  s21::pool_allocator<int> int_alloc(256);  // Явно задаем chunk_size

  // Используем rebind через allocator_traits
  using ReboundAlloc = typename std::allocator_traits<
      s21::pool_allocator<int>>::template rebind_alloc<double>;

  // Теперь должно работать
  ReboundAlloc double_alloc(int_alloc);

  // Проверяем работу с новым типом
  double* ptr = double_alloc.allocate(1);
  double_alloc.deallocate(ptr, 1);

  // Проверяем, что chunk_size сохранился
  EXPECT_EQ(double_alloc.chunk_size(), 256);
}

TEST(PoolAllocatorTest, RebindDifferentChunkSize) {
  // Создаем аллокатор с нестандартным размером блока
  s21::pool_allocator<int> int_alloc(512);

  // Делаем rebind
  using ReboundAlloc = typename std::allocator_traits<
      s21::pool_allocator<int>>::template rebind_alloc<double>;

  ReboundAlloc double_alloc(int_alloc);

  // Проверяем, что аллокатор работает (косвенная проверка сохранения
  // chunk_size)
  double* ptr = double_alloc.allocate(2);
  double_alloc.deallocate(ptr, 2);
}
