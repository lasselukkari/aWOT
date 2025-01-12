#pragma once

#include <ostream>

#if (defined __apple_build_version__) && (__apple_build_version__ >= 12000000)
#else
inline std::ostream& operator << (std::ostream& out, const std::nullptr_t &np) { return out << "nullptr"; }
#endif
