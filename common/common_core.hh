#include <limits>
#include <vector>
#include <condition_variable>
#include <math.h>

// OpenSSL
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/rand.h>

// Vulkan
#include <vulkan/vulkan.hpp>

// GLFW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// GLM
// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtx/hash.hpp>
