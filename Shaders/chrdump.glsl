#shader vertex
#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texturePos;

out vec2 texPos;
void main(){
	texPos = texturePos;
	gl_Position = vec4(position, 0.0, 1.0);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 Frag_color;
uniform int textureWidth;
uniform int textureHeight;
uniform int dataSize;
uniform uint data[dataSize];
in vec2 texPos;	

void main(){
	Frag_color = vec4(1, .3, .5, 1);
}