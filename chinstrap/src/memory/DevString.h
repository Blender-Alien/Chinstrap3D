#pragma once
#include "../ops/Logging.h"

namespace Chinstrap::Memory
{
    struct StringMap;

    struct DevString
    {
        typedef std::size_t HashIDType; // std::hash returns a value of type std::size_t

        [[nodiscard]] std::optional<HashIDType> GetHashID() const
        {
            return hashID;
        }

        void Hash(const std::string_view& virtualFilePath)
        {
            ENSURE_OR_RETURN((!hashID.has_value()));
            hashID.emplace(std::hash<std::string_view>()(virtualFilePath));
        }
    private:
        friend StringMap;
        std::optional<HashIDType> hashID;
    };
}
