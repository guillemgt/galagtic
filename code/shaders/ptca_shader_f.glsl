R"(

uniform sampler2D u_sampler;

varying vec2 v_tex_coords;
varying vec4 v_color;

void main(){
    gl_FragColor = vec4(v_color.xyz, v_color.a*texture2D(u_sampler, v_tex_coords).a);
}

)"