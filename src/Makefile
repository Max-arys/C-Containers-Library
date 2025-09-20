CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c++20 -lstdc++
OPTIMOIZATION = 
GCOV_CFLAGS = --coverage
TEST_CFLAGS = -lgtest -lm

TEST_UNITS = unit_tests

LIB_DIR = lib
TESTS_DIR = tests
COVERAGE_DIR = coverage

TEST_FILES = $(TESTS_DIR)/*.cc 
#TEST_FILES = $(TESTS_DIR)/allocator_test.cc $(TESTS_DIR)/testing.cc

.PHONY: test gcov_report clean

.IGNORE: test
test: $(TEST_FILES)
	$(CC) $(TEST_FILES) -o $(TEST_UNITS) \
	$(CFLAGS) $(GCOV_CFLAGS) $(TEST_CFLAGS) $(OPTIMOIZATION)
	./$(TEST_UNITS)

# Для выборочных тестов.
MY_TEST = $(TESTS_DIR)/testing.cc $(TESTS_DIR)/allocator_test.cc 
single_test: $(MY_TEST)
	$(CC) $(MY_TEST) -o $(TEST_UNITS) \
	$(CFLAGS) $(GCOV_CFLAGS) $(TEST_CFLAGS) $(OPTIMOIZATION)
	./$(TEST_UNITS)

# Сравнение скорости работы std::set и s21::set.
COMPARE_F = $(TESTS_DIR)/testing.cc $(TESTS_DIR)/*.cpp
compare_speed: $(MY_TEST)
	$(CC) $(COMPARE_F) -o $(TEST_UNITS) \
	$(CFLAGS) $(GCOV_CFLAGS) $(TEST_CFLAGS) $(OPTIMOIZATION)
	./$(TEST_UNITS)

gcov_report: test
	mkdir -p $(COVERAGE_DIR)
	lcov -q --capture --directory . --output-file $(COVERAGE_DIR)/info.info \
	--include "$(LIB_DIR)/*"
	genhtml -q $(COVERAGE_DIR)/info.info -o $(COVERAGE_DIR)/html
	echo -e "\n\nCoverage report generated at:\n\
	\033[33m Linux - $(COVERAGE_DIR)/html/index.html \033[0m\n\
	\033[33m Windows - $(COVERAGE_DIR)\html\index.html \033[0m"

my_cppcheck_all:
	cppcheck --enable=all --suppress=missingIncludeSystem --std=c++20 .

style:
	find . -name "*.cc" -exec clang-format -i {} +
	find . -name "*.h" -exec clang-format -i {} +

style_check:
	find . -name "*.cc" -exec clang-format -n {} +
	find . -name "*.h" -exec clang-format -n {} +

valgrind: test
	valgrind --tool=memcheck --leak-check=yes --quiet ./$(TEST_UNITS)

clean:
	rm -rf *.gcda
	rm -rf *.gcno
	rm -rf *.info
	rm -rf *.gcov
	rm -rf $(COVERAGE_DIR)
	rm -rf $(TEST_UNITS)