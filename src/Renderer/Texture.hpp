#pragma once
#include <dllapi.hpp>
#include <libs.hpp>

namespace HyperAPI {
    struct DLL_API m_Texture {
        uint32_t ID;
        int width, height, nrChannels;
        unsigned char *data;
        const char *texType;
        uint32_t slot;
        std::string texPath;
        const char *texStarterPath;
        int sharing;

        ~m_Texture();
    };

    DLL_API extern std::vector<m_Texture *> textures;
    DLL_API extern std::vector<m_Texture *> image_textures;

    class DLL_API Texture {
    public:
        std::string texPath;
        m_Texture *tex = nullptr;

        Texture(const char *texturePath, uint32_t slot,
                const char *textureType);
        Texture(unsigned char *m_Data, uint32_t slot, const char *textureType,
                const char *texturePath = "");

        ~Texture();

        void Bind(uint32_t slot = -1);
        void Unbind();
    };
} // namespace HyperAPI