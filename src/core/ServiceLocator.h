#pragma once

#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>

namespace Core {

class ServiceLocator {
   public:
    template <typename T>
    static void registerService(std::shared_ptr<T> instance) {
        map()[std::type_index(typeid(T))] = std::static_pointer_cast<void>(instance);
    }

    template <typename T>
    static T* get() {
        auto it = map().find(std::type_index(typeid(T)));
        if (it == map().end()) return nullptr;
        return static_cast<T*>(it->second.get());
    }

    static void clear() { map().clear(); }

   private:
    static std::unordered_map<std::type_index, std::shared_ptr<void>>& map();
};

}  // namespace Core
