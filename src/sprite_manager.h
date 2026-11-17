#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H

#include "basic_transform.h"
#include "texture_atlas.h"
#include "vector_util.h"
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#define GL_SILENCE_DEPRECATION  // silences macOS OpenGL deprecation warnings

#define glCheck(expr)                                                                              \
    expr;                                                                                          \
    if (int e = glGetError()) {                                                                    \
        std::cerr << "OpenGL error: " << e << " in file " __FILE__ << ", line number " << __LINE__ \
                  << '\n';                                                                         \
        abort();                                                                                   \
    }

using SpriteID = size_t;

class SpriteManager {
public:
    SpriteManager(const sf::Vector2u& atlasSize, uint padding)
        : mVertices(sf::PrimitiveType::Triangles),
          mNumSprites{0},
          mAtlas(atlasSize, padding),
          mShader(std::filesystem::path{"src/main_sfml.vert"}, std::filesystem::path{"src/main.frag"}) {
        // TODO
    }

    // TODO
    TextureID registerTexture(const std::string& filepath) { return mAtlas.registerTexture(filepath); }

    // TODO
    SpriteID createSprite(TextureID textureID) {
        // Get the dimensions and UV coordinates of the texture.
        sf::Vector2u size = mAtlas.getSizePixels(textureID);
        sf::FloatRect uv = mAtlas.getBoundsUV(textureID);

        float width = static_cast<float>(size.x);
        float height = static_cast<float>(size.y);

        // Calculate UV boundaries.
        float left = uv.position.x;
        float bottom = uv.position.y;
        float right = left + uv.size.x;
        float top = bottom + uv.size.y;

        // Define the six vertices of the sprite (two triangles).
        std::array<sf::Vertex, 6> vertices = {
            sf::Vertex{{0.f, 0.f}, sf::Color::White, {left, bottom}},
            sf::Vertex{{width, 0.f}, sf::Color::White, {right, bottom}},
            sf::Vertex{{width, height}, sf::Color::White, {right, top}},

            sf::Vertex{{0.f, 0.f}, sf::Color::White, {left, bottom}},
            sf::Vertex{{width, height}, sf::Color::White, {right, top}},
            sf::Vertex{{0.f, height}, sf::Color::White, {left, top}}
        };

        // Add the six vertices to the vertex array.
        for (const auto& vertex : vertices) {
            mVertices.append(vertex);
        }

        // Every sprite gets a default transformation.
        mTransforms.emplace_back();

        // Assign a unique ID to this sprite.
        return mNumSprites++;
    }

    // TODO
    void setTexture(SpriteID sprite, TextureID newTexture) {    
        // Get the new texture dimensions and UV coordinates.
        sf::Vector2u size = mAtlas.getSizePixels(newTexture);
        sf::FloatRect uv = mAtlas.getBoundsUV(newTexture);

        float width = static_cast<float>(size.x);
        float height = static_cast<float>(size.y);

        float left = uv.position.x;
        float bottom = uv.position.y;
        float right = left + uv.size.x;
        float top = bottom + uv.size.y;

        // Find the first vertex belonging to this sprite.
        size_t start = sprite * 6;

        // Update positions and UV coordinates.
        mVertices[start + 0] = sf::Vertex{{0.f, 0.f}, sf::Color::White, {left, bottom}};
        mVertices[start + 1] = sf::Vertex{{width, 0.f}, sf::Color::White, {right, bottom}};
        mVertices[start + 2] = sf::Vertex{{width, height}, sf::Color::White, {right, top}};

        mVertices[start + 3] = sf::Vertex{{0.f, 0.f}, sf::Color::White, {left, bottom}};
        mVertices[start + 4] = sf::Vertex{{width, height}, sf::Color::White, {right, top}};
        mVertices[start + 5] = sf::Vertex{{0.f, height}, sf::Color::White, {left, top}};
    }

    // TODO
    void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        // Create a temporary vertex array for transformed vertices.
        sf::VertexArray transformed(sf::PrimitiveType::Triangles);

        // Copy the original vertices and apply each sprite's transformation.
        for (size_t i = 0; i < mNumSprites; ++i) {
            const BasicTransform& transform = mTransforms[i];

            // Each sprite has exactly six vertices.
            for (size_t j = 0; j < 6; ++j) {
                sf::Vertex vertex = mVertices[i * 6 + j];

                // Apply scale and translation to the vertex position.
                vertex.position = transform.applyPoint(vertex.position);

                transformed.append(vertex);
            }
        }

        // Tell SFML which texture and shader to use.
        states.texture = &mAtlas.getTexture();
        states.shader = &mShader;

        // The shader should sample the currently bound texture.
        mShader.setUniform("texture", sf::Shader::CurrentTexture);

        // Draw every sprite together.
        target.draw(transformed, states);
    }

    // TODO
    void drawGL() {};

    // TODO
    BasicTransform& getTransform(SpriteID id) {
        return mTransforms.at(id); //use .at rather than [] to check if valid
    }

    // TODO
    const TextureAtlas& getAtlas() const {
        return mAtlas;
    }

private:
    sf::VertexArray mVertices;
    std::vector<BasicTransform> mTransforms;
    size_t mNumSprites;
    TextureAtlas mAtlas;
    mutable sf::Shader mShader;

    // Use the below only in Batched rendering (part 2):
    std::vector<float> mSpriteNumbers;

    GLint mTextureUniformLocation;
    GLint mTransformsUniformLocation;
    GLint mNumTransformsUniformLocation;

    GLint mPositionAttribLocation;
    GLint mTexCoordsAttribLocation;
    GLint mSpriteNumberAttribLocation;

    GLuint mVerticesVBO;
    GLuint mSpriteNumberVBO;
    GLuint mTransformsTexture;
};

#endif  // SPRITE_MANAGER_H
