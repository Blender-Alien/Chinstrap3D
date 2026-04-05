#pragma once
#include "../ops/Logging.h"

namespace Chinstrap::Renderer
{
    struct Material;
}

namespace Chinstrap::Resourcer
{
    struct ResourceManager;

    // Identify resources by given name and associated hashID
    typedef std::size_t ResourceID;

    enum class ResourceType
    {
        MATERIAL
    };

    // A ResourceRef can be created at any time by a user,
    // it will by default be "invalid" or not refer to anything.
    // A resource can be made valid by giving it to the ResourceManager
    // alongside some arguments that identify the resource you want a reference to.
    // If a resource is unloaded manually or deleted, this ResourceRef becomes
    // 'invalidated' which can only happen in a non shipping build.
    // A valid resource has
    struct ResourceRef
    {
        [[nodiscard]] std::byte* GetData() const
        {
            if (callbackContext == nullptr)
            {
                return nullptr;
            }
            return getResourcePtr(resourceID, callbackContext);
        }

        ResourceRef (const ResourceRef& other)
        {
            resourceType = other.resourceType;
            *this = other;
        }
        ResourceRef(ResourceRef&& other) noexcept
        {
            resourceType = other.resourceType;
            *this = other;
        }
        ResourceRef& operator=(const ResourceRef& other)
        {
            assert(this->resourceType == other.resourceType);
            if (this != &other)
            {
                if (other.callbackContext != nullptr)
                { // Very important, we created a new valid Ref!
                    this->resourceID = other.resourceID;
                    this->callbackContext = other.callbackContext;
                    this->unloadCallback = other.unloadCallback;
                    this->getResourcePtr = other.getResourcePtr;
                    this->referenceCount = other.referenceCount;
#ifndef CHIN_SHIPPING_BUILD
                    this->ptrResourceDeleted = other.ptrResourceDeleted;
#endif
                    ++(*this->referenceCount);
                }
                else
                {
                    CHIN_LOG_WARN("Invalid ResourceRef was copied!");
                }
                return *this;
            }
            return *this;
        }

        explicit ResourceRef(ResourceType resourceType_arg)
            : resourceType(resourceType_arg) {}
        ~ResourceRef()
        {
#ifndef CHIN_SHIPPING_BUILD
            if (*ptrResourceDeleted == true)
            {
                return;
            }
#endif
            assert(*referenceCount > 0);

            --(*referenceCount);
            if (*referenceCount == 0)
            {
                unloadCallback(resourceID, resourceType, callbackContext);
            }
        }

    private:
        friend ResourceManager;

        ResourceID resourceID = NULL;
        ResourceType resourceType;

        uint32_t* referenceCount = nullptr;
#ifndef CHIN_SHIPPING_BUILD
        bool* ptrResourceDeleted = nullptr;
#endif
        void (*unloadCallback)(ResourceID resourceId, ResourceType resourceType, ResourceManager* callbackContext) = nullptr;
        std::byte* (*getResourcePtr)(ResourceID resourceID, ResourceManager* callbackContext) = nullptr;
        ResourceManager* callbackContext = nullptr;
    };
}
