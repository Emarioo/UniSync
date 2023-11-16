#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>
#include <random>
#include <functional>
#include <mutex>
#include <thread>
#include <filesystem>

// #include "glm/glm.hpp"
// #include "glm/gtc/type_ptr.hpp"
// #include "glm/gtx/transform.hpp"
// #include "glm/gtx/intersect.hpp"
// #include "glm/gtx/matrix_decompose.hpp"
// #include "glm/gtc/quaternion.hpp"
// #include "glm/gtc/matrix_transform.hpp"

#include <time.h>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#undef APIENTRY // collision with minwindef

#include "Engone/Typedefs.h"