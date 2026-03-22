#pragma once

namespace Chinstrap::Resourcer
{
    struct ResourceManager;

    // Identify resources by given name and associated hashID
    typedef std::size_t resourceIDType;

    enum class ResourceType
    {
        MATERIAL, SHADER
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
        resourceIDType resourceID = NULL;

        ResourceType resourceType;

        ResourceRef& operator=(const ResourceRef& other)
        {
            assert(this->resourceType == other.resourceType);
            if (this != &other)
            {
                this->resourceID = other.resourceID;
                if (other.referenceCount != nullptr)
                { // Very important, we created a new valid Ref!
                    this->referenceCount = other.referenceCount;
                    this->unloadCallback = other.unloadCallback;
                    this->callbackContext = other.callbackContext;
#ifndef CHIN_SHIPPING_BUILD
                    this->ptrResourceDeleted = other.ptrResourceDeleted;
#endif
                    ++(*this->referenceCount);
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
        uint32_t* referenceCount = nullptr;
#ifndef CHIN_SHIPPING_BUILD
        bool* ptrResourceDeleted = nullptr;
#endif
        void (*unloadCallback)(resourceIDType resourceId, ResourceType resourceType, ResourceManager* callbackContext) = nullptr;
        ResourceManager* callbackContext = nullptr;
    };
}
