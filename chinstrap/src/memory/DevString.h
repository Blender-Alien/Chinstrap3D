#pragma once
#include "../ops/Logging.h"

namespace Chinstrap::Memory { struct StackAllocator; }

namespace Chinstrap::Memory
{
    struct DevString
    {
        typedef uint64_t DevStringID;

        const char* pData = nullptr;
        DevStringID hashID;
    };

    /* We handle filepath relative to the project working directory like this
     * '/resources/sounds/beepboop.wav' called a 'VirtualPath'
     * This can be requested as an OS specific path 'OSPath',
     * in order to load or stream the file in question
     */

    // Handle relative file paths and operating system specifics
    struct FilePath
    {
        typedef DevString::DevStringID FilePathID;

        [[nodiscard]] const std::string_view& GetVirtualPath() const;

        // Get actual path on current OS
        [[nodiscard]] const std::string_view& GetOSPath() const;

        explicit FilePath(const DevString& virtualPath);

    private:
        DevString virtualPath;
    };

}

namespace std
{ // Allow FilePath to be used as a key by std::unordered_map
    template<>
    struct hash<Chinstrap::Memory::FilePath>
    {
        size_t operator()(const Chinstrap::Memory::FilePath& path) const
        {
            return (hash<string_view>()(path.GetVirtualPath()));
        }
    };
    template<>
    struct equal_to<Chinstrap::Memory::FilePath>
    {
        size_t operator()(const Chinstrap::Memory::FilePath& lhs, const Chinstrap::Memory::FilePath& rhs) const
        {
            CHIN_LOG_WARN("std::equal_to specialization for struct FilePath was used, "
                          "this is should only happen in cases of collision in a std::unordered_map, "
                          "because it is comparing the string values directly. Compare the hashID's instead!");
            return (lhs.GetVirtualPath() == rhs.GetVirtualPath());
        }
    };
}
