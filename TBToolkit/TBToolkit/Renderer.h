#pragma once

#include <TBToolkit/Platform.h>
#include <memory>
#include <string>
#include <functional>

namespace TB
{
    enum class ShaderType
    {
        Vertex,
        Hull,
        Domain,
        Geometry,
        Pixel,
        Compute,
    };

    enum class BlendOp
    {
        Add,
        Substract,
        ReverseSubstract,
        Min,
        Max
    };

    enum class BlendFactor
    {
        Zero,
        One,
        SrcColor,
        InvSrcColor,
        SrcAlpha,
        InvSrcAlpha,
        DestAlpha,
        InvDestAlpha,
        DestColor,
        InvDestColor,
        SrcAlphaSat,
        BlendFactor,
        InvBlendFactor,
        Src1Color,
        InvSrc1Color,
        Src1Alpha,
        InvSrc1Alpha
    };

    enum class CompFunc
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
    };

    class Renderer
    {
    public:
        Renderer(const std::shared_ptr<class Canvas>& canvas) {}
        virtual ~Renderer() {}

        virtual RendererType getType() const = 0;

        virtual bool init(int multisampleLevel) = 0;
        virtual void deinit() = 0;
        virtual void render() = 0;
        virtual void update(float delta) = 0;

        virtual std::shared_ptr<class Scene> loadScene(const std::string& path) = 0;
        virtual std::shared_ptr<class Shader> loadShader(const std::string& path, const std::string& entryPoint, ShaderType type) = 0;
        virtual std::shared_ptr<class Texture> loadTexture(const std::string& path) = 0;
    };

#ifdef TB_PLATFORM_WINDOWS
    std::shared_ptr<Renderer> CreateDirectXRenderer(const std::shared_ptr<class Canvas>& canvas, const std::shared_ptr<struct IDirectXFrame>& frame);
#endif
}

