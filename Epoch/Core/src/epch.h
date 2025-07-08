#pragma once

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <algorithm>
#include <array>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <shellapi.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>


#include <CommonUtilities/Math/Vector/Vector.h>
#include <CommonUtilities/Math/Matrix/Matrix.h>

#include <CommonUtilities/Math/Quaternion.hpp>

#include <CommonUtilities/Math/Random.h>
#include <CommonUtilities/Math/CommonMath.hpp>

#include <CommonUtilities/StringUtils.h>

#include "EpochCore/Assert.h"
#include "EpochCore/Profiler.h"
