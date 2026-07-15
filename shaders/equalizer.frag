// Vivace — video equalizer fragment shader (Qt 6 RHI / ShaderEffect).
// Applies brightness/contrast/saturation/gamma/hue to the video texture.
#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D source;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float brightness; // -1 .. 1  (added; 0 = neutral)
    float contrast;   //  0 .. 2  (1 = neutral)
    float saturation; //  0 .. 2  (1 = neutral)
    float gamma;      //  > 0     (1 = neutral)
    float hue;        // radians  (0 = neutral)
};

void main() {
    vec4 tex = texture(source, qt_TexCoord0);
    // Un-premultiply so adjustments act on the true colour (Qt textures are
    // premultiplied); re-premultiply at the end so transparent letterbox
    // bars stay black.
    vec3 c = tex.a > 0.0 ? tex.rgb / tex.a : tex.rgb;

    if (hue != 0.0) { // hue rotation in YIQ space
        float Y = dot(c, vec3(0.299, 0.587, 0.114));
        float I = dot(c, vec3(0.595716, -0.274453, -0.321263));
        float Q = dot(c, vec3(0.211456, -0.522591, 0.311135));
        float hc = cos(hue);
        float hs = sin(hue);
        float I2 = I * hc - Q * hs;
        float Q2 = I * hs + Q * hc;
        c = vec3(Y + 0.9563 * I2 + 0.6210 * Q2,
                 Y - 0.2721 * I2 - 0.6474 * Q2,
                 Y - 1.1070 * I2 + 1.7046 * Q2);
    }

    c += brightness;                  // brightness
    c = (c - 0.5) * contrast + 0.5;   // contrast (around mid-grey)
    c = clamp(c, 0.0, 1.0);
    c = pow(c, vec3(1.0 / gamma));    // gamma
    float l = dot(c, vec3(0.299, 0.587, 0.114));
    c = mix(vec3(l), c, saturation);  // saturation

    fragColor = vec4(clamp(c, 0.0, 1.0) * tex.a, tex.a) * qt_Opacity;
}
