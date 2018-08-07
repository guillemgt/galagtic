#if !OPENGL_ES

R"(
#if   defined VERTEX_SHADER

uniform mat4 u_matrix;

attribute vec3 a_position;
attribute vec3 a_tex_coords;

varying vec3 v_tex_coords;

void main(){
    gl_Position = u_matrix*vec4(a_position, 1.0);
    
    v_tex_coords = a_tex_coords;
}

#elif defined FRAGMENT_SHADER

uniform sampler2D u_sampler;

varying vec3 v_tex_coords;

void main(){
    gl_FragColor = vec4(vec3(1.0), v_tex_coords.z)*texture2D(u_sampler, v_tex_coords.xy);
}

#endif
)"

#else

R"(
#if   defined VERTEX_SHADER
precision mediump float;

uniform mat4 u_matrix;

attribute vec3 a_position;
attribute vec3 a_tex_coords;

varying vec3 v_tex_coords;

void main(){
    gl_Position = u_matrix*vec4(a_position, 1.0);
    
    v_tex_coords = a_tex_coords;
}

#elif defined FRAGMENT_SHADER

precision mediump float;

uniform sampler2D u_sampler;

varying vec3 v_tex_coords;

void main(){
    gl_FragColor = vec4(vec3(1.0), v_tex_coords.z)*texture2D(u_sampler, v_tex_coords.xy);
}

#endif
)"

#endif
