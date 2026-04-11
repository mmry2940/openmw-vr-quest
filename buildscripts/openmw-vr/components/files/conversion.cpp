
#include "conversion.hpp"

std::string Files::pathToUnicodeString(const std::filesystem::path& path)
{
    return path.string();
}

std::string Files::pathToUnicodeString(std::filesystem::path&& path)
{
    return path.string();
}

std::filesystem::path Files::pathFromUnicodeString(std::string_view path)
{
    return std::filesystem::path(std::string(path));
}

std::filesystem::path Files::pathFromUnicodeString(std::string&& path)
{
    return std::filesystem::path(std::move(path));
}

std::filesystem::path Files::pathFromUnicodeString(const char* path)
{
    return std::filesystem::path(path);
}
