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
#define dataSize 256
uniform uint data[dataSize];
uniform mat3 palette;
in vec2 texPos;	

void main () {
	/// [0 0 0 0] [0 0 0 0] [0 0 0 0] [0 0 0 0] * 22
	float PosX = 22. * texPos.x; // 0.567 * 22
	float PosY = 24. * texPos.y; // 0.475

	uint relPosX = uint(PosX); // 12.874 -> 12
	uint relPosY = uint(PosY);

	float fractX = float(PosX - float(relPosX)); //12.874 - 12 -> 0.874
	float fractY = float(PosY - float(relPosY));
	uint fractINT_X = uint(fractX * 2.);  // 0..1   1
	uint fractINT_Y = uint(fractY * 2.);  // 0..1   0

	uint int32indexX = uint((fractX * 10.0) - float (uint(fractX * 10.0)) * 7.0); // 0..7  0.74
	uint int32indexY = uint((fractY * 10.0) - float (uint(fractY * 10.0)) * 3.0); // 0..3

	uint lowByte, highByte;
	
	bool left = (float (relPosX) + .5) > (PosX);

	uint a[] = { 61440U, 3840U, 240U, 15U };
	if (left) {
		lowByte = ((relPosY * 4U * 22U) + relPosX * 4U) + fractINT_X + fractINT_Y;
		highByte = ((relPosY * 4U * 22U) + relPosX * 4U) + fractINT_X + fractINT_Y + 2U;
	}
	else {
		highByte = ((relPosY * 4U * 22U) + relPosX * 4U) + fractINT_X + fractINT_Y;
		lowByte = ((relPosY * 4U * 22U) + relPosX * 4U) + fractINT_X + fractINT_Y - 2U;

	}
	uint data_low = data[lowByte];
	data_low = (data_low & a[int32indexY]) >> ((3U - int32indexY) * 4U); //& (0b1 << int32indexX));
	data_low = (data_low & (1U << int32indexX)) >> int32indexX;

	uint data_hig = data[highByte];
	data_hig = (data_hig & a[int32indexY]) >> ((3U - int32indexY) * 4U); //& (0b1 << int32indexX));
	data_hig = (data_hig & (1U << int32indexX)) >> int32indexX;

	//uint MSB = data_hig;//(data_hig << 4) | data_low;
	uint MSB = (data_hig << 4) | data_low;
	//uint res = line & 3U;

	Frag_color = vec4(palette[MSB], 1);
}