#pragma once

#include <TBToolkit/Platform.h>
#include <TBToolkit/Geometry.h>
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

    enum class FilterMode
    {
        MinMagMipPoint,
        MinMagPointMipLinear,
        MinPointMagLinearMipPoint,
        MinPointMagMipLinear,
        MinLinearMagMipPoint,
        MinLinearMagPointMipLinear,
        MinMagLinearMipPoint,
        MinMagMipLinear,
        Anisotropic,
        ComparisonMinMagMipPoint,
        ComparisonMinMagPointMipLinear,
        ComparisonMinPointMagLinearMipPoint,
        ComparisonMinPointMagMipLinear,
        ComparisonMinLinearMagMipPoint,
        ComparisonMinLinearMagPointMipLinear,
        ComparisonMinMagLinearMipPoint,
        ComparisonMinMagMipLinear,
        ComparisonAnisotropic,
        MinimumMinMagMipPoint,
        MinimumMinMagPointMipLinear,
        MinimumMinPointMagLinearMipPoint,
        MinimumMinPointMagMipLinear,
        MinimumMinLinearMagMipPoint,
        MinimumMinLinearMagPointMipLinear,
        MinimumMinMagLinearMipPoint,
        MinimumMinMagMipLinear,
        MinimumAnisotropic,
        MaximumMinMagMipPoint,
        MaximumMinMagPointMipLinear,
        MaximumMinPointMagLinearMipPoint,
        MaximumMinPointMagMipLinear,
        MaximumMinLinearMagMipPoint,
        MaximumMinLinearMagPointMipLinear,
        MaximumMinMagLinearMipPoint,
        MaximumMinMagMipLinear,
        MaximumAnisotropic
    };

    enum class AddressMode
    {
        Wrap,
        Mirror,
        Clamp,
        Border,
        MirrorOnce
    };

    enum class FillMode
    {
        Wireframe,
        Solid
    };

    enum class CullMode
    {
        None,
        Front,
        Back
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
        virtual std::shared_ptr<class Scene> loadScene(const std::string& path, const MeshModifierCallback& meshModifier) = 0;
        virtual std::shared_ptr<class Shader> loadShader(const std::string& path, const std::string& entryPoint, ShaderType type) = 0;
        virtual std::shared_ptr<class Texture> loadTexture(const std::string& path) = 0;
    };

#ifdef TB_PLATFORM_WINDOWS
    std::shared_ptr<Renderer> CreateDirectXRenderer(const std::shared_ptr<class Canvas>& canvas, const std::shared_ptr<struct IDirectXFrame>& frame);
#endif
}

