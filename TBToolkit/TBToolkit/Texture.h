#pragma once

namespace TB
{
    enum class TextureType
    {
        Color,
        Depth
    };

    enum class TextureFlags : int32_t
    {
        None,
        Target = 1 << 0,
        DepthStencil = 1 << 1,
        ShaderResource = 1 << 2
    };

    inline TextureFlags operator|(TextureFlags lhs, TextureFlags rhs)
    {
        return (TextureFlags)((int)lhs | (int)rhs);
    }
    inline TextureFlags operator|=(TextureFlags& lhs, TextureFlags rhs)
    {
        return (TextureFlags)((int)lhs | (int)rhs);
    }

    class Texture
    {
    public:
        Texture() {}
        virtual ~Texture() {}
    };
}
