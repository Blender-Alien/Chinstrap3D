#pragma once

namespace Chinstrap::Serialization
{
    bool IsLineField(const std::string& line);

    std::tuple<uint32_t, uint32_t> GetFieldContent(const std::string& field);
}