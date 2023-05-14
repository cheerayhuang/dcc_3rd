/*
 * 命令行参数定义。
 *
 * 可以使用 ——help 或者阅读 README.md 获取具体运行方法。
 */

#pragma once

#include <string>

#include "gflags/gflags.h"

DEFINE_string(seed, "seed_vec.csv", "seed vectors file path.");
DEFINE_string(dict, "dict_vec.csv", "dictionary vectors file path");
DEFINE_int32(round, 1, "the number of round of the current execution.");
