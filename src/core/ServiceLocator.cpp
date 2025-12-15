#include "ServiceLocator.h"

namespace Core {
inline static std::unordered_map<std::type_index, std::shared_ptr<void>> g_serviceMap;

std::unordered_map<std::type_index, std::shared_ptr<void>>& ServiceLocator::map() { return g_serviceMap; }

}  // namespace Core
