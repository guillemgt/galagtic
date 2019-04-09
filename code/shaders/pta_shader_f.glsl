R"(

uniform sampler2D u_sampler;

varying vec3 v_tex_coords;

void main(){
    gl_FragColor = vec4(vec3(1.0), v_tex_coords.z)*texture2D(u_sampler, v_tex_coords.xy);
}

)"