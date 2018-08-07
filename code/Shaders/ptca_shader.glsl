#if !OPENGL_ES

R"(
#if   defined VERTEX_SHADER

uniform mat4 u_matrix;

attribute vec3 a_position;
attribute vec2 a_tex_coords;
attribute vec4 a_color;

varying vec2 v_tex_coords;
varying vec4 v_color;

void main(){
    gl_Position = u_matrix*vec4(a_position, 1.0);
    
    v_tex_coords = a_tex_coords;
    v_color = a_color;
}

#elif defined FRAGMENT_SHADER

uniform sampler2D u_sampler;

varying vec2 v_tex_coords;
varying vec4 v_color;

void main(){
    gl_FragColor = vec4(v_color.xyz, v_color.a*texture2D(u_sampler, v_tex_coords).a);
}

#endif
)"

#else

R"(
#if   defined VERTEX_SHADER

precision mediump float;

uniform mat4 u_matrix;

attribute vec3 a_position;
attribute vec2 a_tex_coords;
attribute vec4 a_color;

varying vec2 v_tex_coords;
varying vec4 v_color;

void main(){
    gl_Position = u_matrix*vec4(a_position, 1.0);
    
    v_tex_coords = a_tex_coords;
    v_color = a_color;
}

#elif defined FRAGMENT_SHADER

precision mediump float;

uniform sampler2D u_sampler;

varying vec2 v_tex_coords;
varying vec4 v_color;

void main(){
    gl_FragColor = vec4(v_color.xyz, v_color.a*texture2D(u_sampler, v_tex_coords).a);
}

#endif
)"

#endif
