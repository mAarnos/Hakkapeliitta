#include "large_pages.hpp"

bool LargePages::allowedToUse;
bool LargePages::inUse;
#ifndef _WIN32
int LargePages::num;
#endif