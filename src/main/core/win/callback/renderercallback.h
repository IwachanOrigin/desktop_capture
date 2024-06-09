
#ifndef RENDERER_CALLBACK_H_
#define RENDERER_CALLBACK_H_

#include "pch.h"
#include <functional>

namespace core
{

// std::function<return value type(argument type)>
using RendererCallback = std::function<bool(const uint8_t*, const size_t)>;

} // core

#endif // RENDERER_CALLBACK_H_

