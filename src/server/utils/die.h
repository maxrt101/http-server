#ifndef SERVER_UTILS_DIE_H_
#define SERVER_UTILS_DIE_H_

#include <cstdlib>

namespace utils {
[[noreturn]] void Die(int exit_code = EXIT_FAILURE);
} // namespace utils

#endif
