#pragma once

#include <ostream>

#if (defined __apple_build_version__) && (__apple_build_version__ >= 12000000)
// defined in /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/../include/c++/v1/ostream:223:20
#else
inline std::ostream& operator << (std::ostream& out, const std::nullptr_t &np) { return out << "nullptr"; }
#endif
