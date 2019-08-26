#version 330 core
out vec4 FragColor;

in vec2 texCoord;

// If you do not explicitly create sampler objects through OpenGL function calls,
// Each texture object, for convenience, will already contain a built-in sampler object.
uniform sampler2D ourTexture;

void main()
{
	// Texture function will sample a texel from a sampler given a texture coordinate.
	// The return value is a vector containing the sampled texture data.
    FragColor = texture(ourTexture, texCoord);
}