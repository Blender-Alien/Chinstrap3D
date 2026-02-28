#include "DevString.h"

const std::string_view& Chinstrap::Memory::FilePath::GetVirtualPath() const
{
    return std::string_view{virtualPath.pData};
}

const std::string_view & Chinstrap::Memory::FilePath::GetOSPath() const
{
}

Chinstrap::Memory::FilePath::FilePath(const DevString &path)
    : virtualPath(path)
{
}
