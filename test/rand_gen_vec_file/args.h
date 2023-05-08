#include <gflags/gflags.h>

DEFINE_string(seed_file, "seed_vec.csv", "Specify the file to store seed vectors.");
DEFINE_string(dict_file, "dict_vec.csv", "Specify the file to store dictionary vectors.");
DEFINE_int32(dimetion, 256, "Specify the dimetion of vector.");
DEFINE_int32(seed_total, 1000, "Specify the total of seed vectors.");
DEFINE_int32(dict_total, 1000*1000, "Specify the total of dictionary vectors.");
