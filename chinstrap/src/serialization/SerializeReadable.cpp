#include "SerializeReadable.h"

bool Chinstrap::Serialization::IsLineField(const std::string& line)
{
    // "; stuff" is our syntax, so any less than 3 characters doesn't make sense
    if (line.length() < 3)
    {
        return false;
    }
    if (line.at(0) == ';' && line.at(1) == ' ')
    {
        return true;
    }
    return false;
}

std::tuple<uint32_t, uint32_t> Chinstrap::Serialization::GetFieldContent(const std::string& field)
{
    // "; stuff #fieldAnnotation"

    const auto annotationPos = field.find_last_of('#');
    return std::make_tuple(3, annotationPos - 1);
}
