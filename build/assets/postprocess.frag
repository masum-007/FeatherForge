uniform sampler2D texture;
uniform float time;

void main() {
    vec2 uv = gl_TexCoord[0].xy;
    vec4 color = texture2D(texture, uv);

    // 1. SOFTER VIGNETTE
    float dist = distance(uv, vec2(0.5, 0.5));
    float vignette = smoothstep(1.0, 0.3, dist); 
    color.rgb *= (vignette + 0.2); // Raised from 0.1 so corners aren't pitch black

    // 2.BLOOM (Only the brightest things glow now, saving the sky!)
    vec4 highlight = clamp(color - 0.85, 0.0, 1.0); // Raised threshold from 0.6 to 0.85
    color += highlight * 0.6;

    // 3. SHAKE TINT
    if (time > 0.01) {
        color.r += 0.05;
    }

    gl_FragColor = gl_Color * color;
}
