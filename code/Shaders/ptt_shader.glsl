#if !OPENGL_ES

R"(
#if   defined VERTEX_SHADER

uniform mat4 u_matrix;

uniform float u_t;

attribute vec3 a_position;
attribute vec3 a_tex_coords;

varying vec2 v_tex_coords;
varying float v_t;

void main(){
    gl_Position = u_matrix*vec4(a_position, 1.0);
    
    v_tex_coords = a_tex_coords.xy;
    v_t = 1.0+0.5*sin(u_t+a_tex_coords.z); //-a_position.x+a_position.y);
}

#elif defined FRAGMENT_SHADER

uniform sampler2D u_sampler0;
uniform sampler2D u_sampler1;
uniform vec3 u_color_multiplier;

varying vec2 v_tex_coords;
varying float v_t;

void main(){
    gl_FragColor = vec4(u_color_multiplier, 1.0) * mix(texture2D(u_sampler0, v_tex_coords), texture2D(u_sampler1, v_tex_coords), v_t);
}

#endif
)"

#else

R"(
#if   defined VERTEX_SHADER
precision mediump float;

uniform mat4 u_matrix;

uniform float u_t;

attribute vec3 a_position;
attribute vec3 a_tex_coords;

varying vec2 v_tex_coords;
varying float v_t;

void main(){
    gl_Position = u_matrix*vec4(a_position, 1.0);
    
    v_tex_coords = a_tex_coords.xy;
    v_t = 1.0+0.5*sin(u_t+a_tex_coords.z); //-a_position.x+a_position.y);
}

#elif defined FRAGMENT_SHADER

precision mediump float;

uniform sampler2D u_sampler0;
uniform sampler2D u_sampler1;
uniform vec3 u_color_multiplier;

varying vec2 v_tex_coords;
varying float v_t;

void main(){
    gl_FragColor = vec4(u_color_multiplier, 1.0) * mix(texture2D(u_sampler0, v_tex_coords), texture2D(u_sampler1, v_tex_coords), v_t);
}

#endif
)"

#endif
