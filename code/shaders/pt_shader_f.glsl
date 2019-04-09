R"(

uniform sampler2D u_sampler;
uniform vec3 u_color_multiplier;

varying vec2 v_tex_coords;

void main(){
    gl_FragColor = vec4(u_color_multiplier, 1.0)*texture2D(u_sampler, v_tex_coords);
}

)"