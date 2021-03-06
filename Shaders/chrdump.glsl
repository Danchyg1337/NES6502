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
uniform mat3 palette;
in vec2 texPos;
uniform float width;
uniform float height;

uniform CHRrom
{
  uint val[1024];
} data;

void main () {
	float PosX = width * texPos.x; // 0.567 * 22
	float PosY = height * texPos.y; // 0.475

	uint relPosX = uint(PosX); // 12.874 -> 12
	uint relPosY = uint(PosY);

	float fractX = fract(PosX); //12.874 -> 0.874
	float fractY = fract(PosY);
	uint fractINT_X = uint(fractX * 2.);  // 0..1
	uint fractINT_Y = uint(fractY * 2.);  // 0..1

	uint int32indexX = uint(fract(fractX) * 8.0);	   // 0..7
	uint int32indexY = uint(fract(fractY * 2.) * 4.0); // 0..3

	uint finalPos = ((relPosY * 4U * uint(width)) + relPosX * 4U) + fractINT_Y;		//final position in the data array;
	uint lowByte = finalPos, highByte = finalPos + 2U;

	uint data_low = data.val[lowByte];
	data_low = (data_low >> (int32indexY * 8U));
	data_low = (data_low >> 7U - int32indexX) & 1U;

	uint data_hig = data.val[highByte];
	data_hig = (data_hig >> (int32indexY * 8U));
	data_hig = (data_hig >> 7U - int32indexX) & 1U;

	uint MSB = (data_hig << 1U) | data_low;

	Frag_color = vec4(palette[MSB - 1U], 1);
	//Frag_color = vec4(vec3(float (int32indexX) / 8., float(int32indexY) / 4., 1. ), 1);
	//Frag_color = vec4(vec3(float (fractINT_X), float(fractINT_Y), 1. ), 1);
}