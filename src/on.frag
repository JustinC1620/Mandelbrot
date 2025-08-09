#version 460 core
out vec4 frag_color;
	
in vec2 coords;
	
uniform sampler2D tex;
	
void main()
{             
    frag_color = texture(tex, coords);
}