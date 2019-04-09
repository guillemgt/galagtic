R"(
uniform mat4 u_matrix;

attribute vec3 a_position;
attribute vec2 a_tex_coords;

varying vec2 v_tex_coords;

void main(){
    gl_Position = u_matrix*vec4(a_position, 1.0);
    
    v_tex_coords = a_tex_coords;
}
)"