#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <mutex>

namespace Core {

inline std::unordered_map<std::type_index, std::shared_ptr<void>> gServiceMap{};
inline std::mutex gServiceMtx{};

class ServiceLocator {
public:
	template<typename T>
	static void registerService(std::shared_ptr<T> instance)
	{
		std::scoped_lock lock(mtx());
		map()[std::type_index(typeid(T))] = std::static_pointer_cast<void>(instance);
	}

	template<typename T>
	static T* get()
	{
		std::scoped_lock lock(mtx());
		auto it = map().find(std::type_index(typeid(T)));
		if (it == map().end()) return nullptr;
		return static_cast<T*>(it->second.get());
	}

	static void clear()
	{
		std::scoped_lock lock(mtx());
		map().clear();
	}

private:
	static std::unordered_map<std::type_index, std::shared_ptr<void>>& map();
	static std::mutex& mtx();
};

} // namespace Core


