#include "ServiceLocator.h"

namespace Core {

std::unordered_map<std::type_index, std::shared_ptr<void>>& ServiceLocator::map() {
    static std::unordered_map<std::type_index, std::shared_ptr<void>> g_serviceMap;
    return g_serviceMap;
}

}  // namespace Core
