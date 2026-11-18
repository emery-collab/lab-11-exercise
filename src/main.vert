#version 120

uniform sampler1D transforms;
uniform int numTransforms;

attribute vec2 position;
attribute vec2 texCoords;
attribute float spriteNumber;

varying vec2 texCoordsOut;

// Given a world position (for this lab, bottom-left is (0, 0), top-right of window is (1280, 720)),
// convert it to be in [-1, 1]^2, where y = -1 is the bottom and x = -1 is the left side.
vec2 cameraPos(vec2 worldPos) {
    return vec2(worldPos.x / 1280.0 * 2.0 - 1.0, (worldPos.y / 720.0) * 2.0 - 1.0);  // TODO
}

void main() {
    // set gl_Position and texCoordsOut...
    // TODO

    // Calculate the center of the texel belonging to this sprite.
    float transformCoord = (spriteNumber + 0.5) / float(numTransforms);

    // Retrieve scale and translation.
    vec4 transform = texture1D(transforms, transformCoord);

    // Apply the sprite transformation.
    vec2 worldPos = position * transform.xy + transform.zw;

    // Convert to clip coordinates.
    gl_Position = vec4(cameraPos(worldPos), 0.0, 1.0);

    // Pass texture coordinates to the fragment shader.
    texCoordsOut = texCoords;
}
