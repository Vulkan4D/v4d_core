#pragma once

#ifdef _V4D_CORE
	#define ___EVENTEXPORT_CLASS EXTERNCPP class DLLEXPORT
	#define ___EVENTEXPORT EXTERNCPP DLLEXPORT
#else
	#define ___EVENTEXPORT_CLASS EXTERNCPP class DLLIMPORT
	#define ___EVENTEXPORT EXTERNCPP DLLIMPORT
#endif

#define DEFINE_EXTERN_EVENT_HEADER(eventName, objType) \
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

#define DEFINE_EXTERN_EVENT_BODY(eventName, objType) \
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
	v4d::event:: ___EVENT_TYPE_ ## eventName v4d::event:: eventName; \

#define DEFINE_EVENT(eventName, objType) \
	namespace v4d::event { \
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

