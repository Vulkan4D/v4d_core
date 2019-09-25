#include <v4d.h>

namespace v4d::tests {
	int SHA() {
		if (v4d::crypto::SHA1("Vulkan4D") == "9a42354fb0481698635f43844eba09605c67908e" 
		&& v4d::crypto::SHA256("Vulkan4D") == "150740b0e225678164ac5580f46a5faba663c323c5306362c1e61e45c12a5415"
		&& v4d::crypto::SHA256("Vulkan4C") != "150740b0e225678164ac5580f46a5faba663c323c5306362c1e61e45c12a5415"
		&& v4d::crypto::SHA256("Vulkan4D") != "250740b0e265678164ac5580f46a5faba663c323c5306362c1e61e45c12a5419"
		) {
			return 0;
		} else {
			return -1;
		}
	}
}
