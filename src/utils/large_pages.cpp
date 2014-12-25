#include "large_pages.hpp"

bool LargePages::allowedToUse;
std::unordered_map<void*, size_t> LargePages::infoTable;