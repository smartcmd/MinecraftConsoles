#pragma comment(lib, "legacy_stdio_definitions.lib")

namespace std {
    char const* _Winerror_map(int) { return nullptr; }
}