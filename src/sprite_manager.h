#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H

#include "basic_transform.h"
#include "texture_atlas.h"
#include "vector_util.h"
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <cstddef>
#include <vector>

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
          mShader(std::filesystem::path{"src/main.vert"}, std::filesystem::path{"src/main.frag"}),
        // TODO
        mTextureUniformLocation{-1},
        mTransformsUniformLocation{-1},
        mNumTransformsUniformLocation{-1},
        mPositionAttribLocation{-1},
        mTexCoordsAttribLocation{-1},
        mSpriteNumberAttribLocation{-1},
        mVerticesVBO{0},
        mSpriteNumberVBO{0},
        mTransformsTexture{0} {

            // Activate the shader program.
            glUseProgram(mShader.getNativeHandle());

            // Obtain vertex attribute locations.
            mPositionAttribLocation = glGetAttribLocation(mShader.getNativeHandle(), "position");

            mTexCoordsAttribLocation = glGetAttribLocation(mShader.getNativeHandle(), "texCoords");

            mSpriteNumberAttribLocation = glGetAttribLocation(mShader.getNativeHandle(), "spriteNumber");

            // Obtain uniform locations.
            mTextureUniformLocation = glGetUniformLocation(mShader.getNativeHandle(), "texture");

            mTransformsUniformLocation = glGetUniformLocation(mShader.getNativeHandle(), "transforms");

            mNumTransformsUniformLocation = glGetUniformLocation(mShader.getNativeHandle(), "numTransforms");

            // Generate GPU buffers.
            glGenBuffers(1, &mVerticesVBO);
            glGenBuffers(1, &mSpriteNumberVBO);

            // Generate the transformation texture.
            glGenTextures(1, &mTransformsTexture);
        }

    // Added a destructor for part 2
    ~SpriteManager() {
        glDeleteBuffers(1, &mVerticesVBO);
        glDeleteBuffers(1, &mSpriteNumberVBO);
        glDeleteTextures(1, &mTransformsTexture);
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

            // Store this sprite's ID for this vertex.
            mSpriteNumbers.push_back(static_cast<float>(mNumSprites));
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
    void drawGL() {
        // Use our shader.
        glUseProgram(mShader.getNativeHandle());

        const GLsizei vertexCount = static_cast<GLsizei>(mVertices.getVertexCount());

        // Upload vertex data. 1
        glBindBuffer(GL_ARRAY_BUFFER, mVerticesVBO);
        glBufferData(GL_ARRAY_BUFFER,  static_cast<GLsizeiptr>(vertexCount * sizeof(sf::Vertex)), vertexCount > 0 ? &mVertices[0] : nullptr, GL_DYNAMIC_DRAW);

        // Position attribute.
        glEnableVertexAttribArray(mPositionAttribLocation);

        glVertexAttribPointer(mPositionAttribLocation, 2, GL_FLOAT, GL_FALSE, sizeof(sf::Vertex), reinterpret_cast<const void*>(offsetof(sf::Vertex, position)));

        // Texture coordinate attribute.
        glEnableVertexAttribArray(mTexCoordsAttribLocation);
        glVertexAttribPointer(mTexCoordsAttribLocation, 2, GL_FLOAT, GL_FALSE, sizeof(sf::Vertex), reinterpret_cast<const void*>(offsetof(sf::Vertex, texCoords)));

        // Upload sprite number data. 2
        glBindBuffer(GL_ARRAY_BUFFER, mSpriteNumberVBO);

        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(mSpriteNumbers.size() * sizeof(float)), mSpriteNumbers.empty() ? nullptr : mSpriteNumbers.data(), GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(mSpriteNumberAttribLocation);
        glVertexAttribPointer(mSpriteNumberAttribLocation, 1, GL_FLOAT, GL_FALSE, sizeof(float), nullptr);

        // Upload transformation data as a 1D texture. 3
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, mTransformsTexture);

        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri( GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);

        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F_ARB, static_cast<GLsizei>(mTransforms.size()), 0, GL_RGBA, GL_FLOAT, mTransforms.data());

        // Bind the atlas texture. 4
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mAtlas.getTexture().getNativeHandle());

        // Set shader uniforms. 5
        glUniform1i(mTextureUniformLocation, 0);
        glUniform1i(mTransformsUniformLocation, 1);
        glUniform1i(mNumTransformsUniformLocation, static_cast<GLint>(mTransforms.size()));

        // Draw everything in one draw call. 6
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);

        // Clean up vertex attribute state.
        glDisableVertexAttribArray(mPositionAttribLocation);
        glDisableVertexAttribArray(mTexCoordsAttribLocation);
        glDisableVertexAttribArray(mSpriteNumberAttribLocation);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

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
