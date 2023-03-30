uniform float time;
uniform sampler2D texture;

void main() {
    vec4 final_color = texture2D(texture, gl_TexCoord[0].xy);

    gl_FragColor = vec4(smoothstep(0.0, 1.0, final_color.r),
                        smoothstep(0.0, 1.0, final_color.g),
                        smoothstep(0.0, 1.0, final_color.b), 1.0);
}
