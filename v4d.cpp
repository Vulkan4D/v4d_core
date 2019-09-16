#include "v4d.h"
#ifdef _V4D_CORE

	//////////////////////////////////////////////////////////
	// V4D global events

	DEFINE_CORE_EVENT_BODY(V4D_CORE_INIT, CoreInitEvent&)
	DEFINE_CORE_EVENT_BODY(V4D_CORE_DESTROY, CoreDestroyEvent&)


	//////////////////////////////////////////////////////////
	// V4D global functions

	const std::string v4d::GetCoreBuildVersion() noexcept {
		return V4D_VERSION;
	}

	//////////////////////////////////////////////////////////
	// V4D Core Instance

	std::shared_ptr<v4d::io::Logger> v4d::Core::coreLogger = v4d::io::Logger::ConsoleInstance();
	
	bool v4d::Core::Init() {
		return Init(nullptr);
	}

	bool v4d::Core::Init(std::shared_ptr<v4d::io::Logger> coreLogger) {
		CoreInitEvent e;
		if (coreLogger) v4d::Core::coreLogger = coreLogger;
		//...
		v4d::event::V4D_CORE_INIT(e);
		return true;
	}

	void v4d::Core::Destroy() {
		CoreDestroyEvent e;
		v4d::event::V4D_CORE_DESTROY(e);
		//...
	}

	void v4d::Core::SetProjectName(std::string projectName) {
		this->projectName = projectName;
	}
	std::string v4d::Core::GetProjectName() const {
		return this->projectName;
	}

	v4d::io::SystemsLoader* v4d::Core::GetSystemsLoader() {
		static v4d::io::SystemsLoader systemsLoader(this);
		return &systemsLoader;
	}
	v4d::io::SystemInstance* v4d::Core::GetSystem(const std::string& name) {
		return GetSystemsLoader()->Load(name);
	}

#endif
