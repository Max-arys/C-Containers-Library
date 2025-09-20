#ifndef TESTING_H
#define TESTING_H

#include <gtest/gtest.h>
#include <sys/resource.h>  // для теста bad_alloc
#include <valgrind/valgrind.h>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "../lib/s21_allocator.h"
#include "../lib/s21_helpers.h"
#include "../lib/s21_map.h"
#include "../lib/s21_multiset.h"
#include "../lib/s21_red_black_tree.h"
#include "../lib/s21_set.h"

#endif  // TESTING_H