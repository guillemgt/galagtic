R"(
uniform mat4 u_matrix;

attribute vec3 a_position;
attribute vec4 a_color;

varying vec4 v_color;

void main(){
    gl_Position = u_matrix*vec4(a_position, 1.0);
    
    v_color = a_color;
}
)"
