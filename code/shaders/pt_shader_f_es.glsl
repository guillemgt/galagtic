R"(
precision lowp float;

uniform sampler2D u_sampler;

varying vec2 v_tex_coords;

void main(){
    gl_FragColor = texture2D(u_sampler, v_tex_coords);
}
)"