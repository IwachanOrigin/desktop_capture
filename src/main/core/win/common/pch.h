
#ifndef PCH_H_
#define PCH_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

// Windows.h
#include <Windows.h>

// Windows SDK support
#include <Unknwn.h>
#include <inspectable.h>

// Needs to come before C++/WinRT headers
#include <wil/cppwinrt.h>

// WinRT
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.Metadata.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.DirectX.Direct3d11.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Security.Authorization.AppCapabilityAccess.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.UI.Popups.h>

#include <DispatcherQueue.h>

// STL
#include <atomic>
#include <memory>
#include <algorithm>
#include <unordered_set>
#include <vector>
#include <optional>
#include <future>
#include <mutex>
#include <iostream>
#include <string>
#include <cassert>


// D3D
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <d2d1_3.h>
#include <wincodec.h>

// DWM
#include <dwmapi.h>

// WIL
#include <wil/resource.h>

// wrl
#include <wrl/client.h>

#endif // PCH_H_
