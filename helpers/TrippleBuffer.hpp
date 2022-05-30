#pragma once

namespace v4d {
	template<class Buffer>
	class TripleBuffer {
		std::mutex mu;
		Buffer back, staging, front;
		bool dirty = false;
	public:
		void SwapBack() {
			std::lock_guard lock(mu);
			back.swap(staging);
			dirty = true;
		}
		void SwapFront() {
			std::lock_guard lock(mu);
			if (dirty) {
				front.swap(staging);
				dirty = false;
			}
		}
		void Clear() {
			std::lock_guard lock(mu);
			back.clear();
			staging.clear();
			front.clear();
			dirty = false;
		}
		Buffer& Back() {return back;}
		Buffer& Front() {return front;}
		bool IsDirty() const {return dirty;}
	};
}
