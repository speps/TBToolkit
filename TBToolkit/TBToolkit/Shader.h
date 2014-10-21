#pragma once

#include <TBToolkit/Renderer.h>

namespace TB
{
    class Shader
    {
    public:
        Shader(const std::string& entryPoint, ShaderType type) {}
        virtual ~Shader() {}
    };
}
