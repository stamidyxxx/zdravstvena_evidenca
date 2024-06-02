#include <mariadb/conncpp.hpp>
#include <iostream>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <cstdio>
#include <format>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>
#include <functional>
#include <chrono>
#include <thread>


#include "imgui/imgui.h"
#include "imgui/impl/imgui_impl_win32.h"
#include "imgui/impl/imgui_impl_dx12.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"
#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif
#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

#define NUM_FRAMES_IN_FLIGHT 3
#define NUM_BACK_BUFFERS 3

using namespace std;
using namespace std::chrono_literals;