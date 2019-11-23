#pragma once
#include <v4d.h>

#ifdef _V4D_CORE
	#define ___EVENTEXPORT_CLASS EXTERNCPP class DLLEXPORT
	#define ___EVENTEXPORT EXTERNCPP DLLEXPORT
#else
	#define ___EVENTEXPORT_CLASS EXTERNCPP class DLLIMPORT
	#define ___EVENTEXPORT EXTERNCPP DLLIMPORT
#endif

#ifdef EVENT_DEFINITIONS_VARIADIC
	#define DEFINE_CORE_EVENT_HEADER(eventName, ...) \
		namespace v4d::event { \
			___EVENTEXPORT_CLASS ___EVENT_TYPE_ ## eventName { \
			private: \
				std::mutex mu; \
				std::vector<std::function<void(__VA_ARGS__)>> listeners; \
			public: \
				void operator<<(std::function<void(__VA_ARGS__)>); \
				template<class... Args> \
				void operator()(Args... args) { \
					std::lock_guard<std::mutex> lock(mu); \
					for (auto ev : listeners) { \
						ev(std::forward<Args>(args)...); \
					} \
				} \
			}; \
			___EVENTEXPORT ___EVENT_TYPE_ ## eventName eventName; \
		}
	#ifdef _V4D_CORE
		#define DEFINE_CORE_EVENT_BODY(eventName, ...) \
			void v4d::event:: ___EVENT_TYPE_ ## eventName ::operator<<(std::function<void(__VA_ARGS__)> func) { \
				std::lock_guard<std::mutex> lock(mu); \
				listeners.emplace_back(func); \
			} \
			v4d::event:: ___EVENT_TYPE_ ## eventName v4d::event:: eventName;
	#endif
	#define DEFINE_EVENT(nameSpace, eventName, ...) \
		namespace nameSpace::event { \
			class { \
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
#else
	#define DEFINE_CORE_EVENT_HEADER(eventName, objType) \
		namespace v4d::event { \
			___EVENTEXPORT_CLASS ___EVENT_TYPE_ ## eventName { \
			private: \
				std::mutex mu; \
				std::vector<std::function<void(objType)>> listeners; \
			public: \
				void operator<<(std::function<void(objType)>); \
				void operator()(objType); \
			}; \
			___EVENTEXPORT ___EVENT_TYPE_ ## eventName eventName; \
		}
	#ifdef _V4D_CORE
		#define DEFINE_CORE_EVENT_BODY(eventName, objType) \
			void v4d::event:: ___EVENT_TYPE_ ## eventName ::operator<<(std::function<void(objType)> func) { \
				std::lock_guard<std::mutex> lock(mu); \
				listeners.emplace_back(func); \
			} \
			void v4d::event:: ___EVENT_TYPE_ ## eventName ::operator()(objType obj) { \
				std::lock_guard<std::mutex> lock(mu); \
				for (auto ev : listeners) { \
					ev(std::forward<objType>(obj)); \
				} \
			} \
			v4d::event:: ___EVENT_TYPE_ ## eventName v4d::event:: eventName;
	#endif
	#define DEFINE_EVENT(nameSpace, eventName, objType) \
		namespace nameSpace::event { \
			class { \
			private: \
				std::mutex mu; \
				std::vector<std::function<void(objType)>> listeners; \
			public: \
				void operator<<(std::function<void(objType)> func) { \
					std::lock_guard<std::mutex> lock(mu); \
					listeners.emplace_back(func); \
				} \
				void operator()(objType obj) { \
					std::lock_guard<std::mutex> lock(mu); \
					for (auto ev : listeners) { \
						ev(std::forward<objType>(obj)); \
					} \
				} \
			} eventName; \
		}
#endif
