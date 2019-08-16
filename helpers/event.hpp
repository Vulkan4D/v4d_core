#pragma once

#ifdef _V4D_CORE
	#define ___EVENTEXPORT EXTERN class DLLEXPORT
#else
	#define ___EVENTEXPORT EXTERN class DLLIMPORT
#endif

#ifdef _IN_EDITOR
	#define DEFINE_EVENT_TYPE(eventName, ...) \
		namespace v4d::event { \
			___EVENTEXPORT { \
			public: \
				void operator<<(function<void(__VA_ARGS__)> func); \
				void operator()(__VA_ARGS__); \
			} eventName; \
		}
#else
	#define DEFINE_EVENT_TYPE(eventName, ...) \
		namespace v4d::event { \
			___EVENTEXPORT { \
			private: \
				std::mutex mu; \
				std::vector<std::function<void(__VA_ARGS__)>> listeners; \
			public: \
				void operator<<(std::function<void(__VA_ARGS__)> func) { \
					std::lock_guard<std::mutex> lock(mu); \
					listeners.emplace_back(func); \
				} \
				template<class... Args> \
				void operator()(Args... args) { \
					std::lock_guard<std::mutex> lock(mu); \
					for (auto ev : listeners) { \
						ev(std::forward<Args>(args)...); \
					} \
				} \
			} eventName; \
		}
#endif
