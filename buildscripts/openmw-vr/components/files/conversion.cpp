
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
<<<<<<< HEAD
    return std::filesystem::path(std::string(path));
=======
    return std::filesystem::path(path);
>>>>>>> 3ecc687e950b13580a4e709d17a2dd7170894a4e
}

std::filesystem::path Files::pathFromUnicodeString(std::string&& path)
{
    return std::filesystem::path(std::move(path));
}

std::filesystem::path Files::pathFromUnicodeString(const char* path)
{
    return std::filesystem::path(path);
}
