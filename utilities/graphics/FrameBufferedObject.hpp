#pragma once

#include <type_traits>
#include <array>
#include <utility>

template<class T>
struct FrameBufferedObject {
	using FRAMEBUFFERED_ARRAY = std::array<T, V4D_RENDERER_FRAMEBUFFERS_MAX_FRAMES>;
	FRAMEBUFFERED_ARRAY objArray;
	T& operator[](size_t i) {return objArray[i];}
	template<typename...Args> requires std::is_constructible_v<T, Args...>
	FrameBufferedObject(Args&&...args) {
		objArray.fill({std::forward<Args>(args)...});
	}
	operator const FRAMEBUFFERED_ARRAY&() const {
		return objArray;
	}
	operator FRAMEBUFFERED_ARRAY() const {
		return objArray;
	}
};
