#if !OPENGL_ES

R"(
#if   defined VERTEX_SHADER

uniform mat4 u_matrix;

attribute vec3 a_position;
attribute vec2 a_tex_coords;

varying vec2 v_tex_coords;

void main(){
    gl_Position = u_matrix*vec4(a_position, 1.0);
    
    v_tex_coords = a_tex_coords;
}

#elif defined FRAGMENT_SHADER

uniform sampler2D u_sampler;
uniform vec3 u_color_multiplier;

varying vec2 v_tex_coords;

void main(){
    gl_FragColor = vec4(u_color_multiplier, 1.0)*texture2D(u_sampler, v_tex_coords);
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

varying vec2 v_tex_coords;

void main(){
    gl_Position = u_matrix*vec4(a_position, 1.0);
    
    v_tex_coords = a_tex_coords;
}

#elif defined FRAGMENT_SHADER

precision mediump float;

uniform sampler2D u_sampler;
uniform vec3 u_color_multiplier;

varying vec2 v_tex_coords;

void main(){
    vec4 color = texture2D(u_sampler, v_tex_coords);
    if(color.a <= 0.1)
        discard;
    gl_FragColor = vec4(u_color_multiplier, 1.0)*color;
}

#endif
)"

#endif
