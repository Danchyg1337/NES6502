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
in vec2 texPos;
uniform vec3 bgColor;
uniform vec3 palettes[12];
uniform uint tilesWidth;
uniform uint tilesHeight;
uniform sampler2D bgTex;
uniform uint spriteX;
uniform uint spriteY;
uniform uint tilePalette;
uniform bool depth;
uniform bool flipH;
uniform bool flipV;
uniform bool behindBG;
uniform bool mode8x16;

uniform uint tile[8];


void main () {
	uint screenW = 32U;
	uint screenH = 30U;
	float spritePosX = screenW * 8U * texPos.x;
	float spritePosY = screenH * 8U * texPos.y;

	bool onX = (spritePosX > spriteX) && (spritePosX < spriteX + 8U);
	bool onY = (spritePosY > spriteY) && (spritePosY < spriteY + (mode8x16 ? 16U : 8U));

	uint onY2 = uint(spritePosY - spriteY) / 8U;

	Frag_color = texture(bgTex, texPos);
	if(Frag_color.rgb != bgColor && behindBG) return;

	if(onX && onY){
		uint tilePixX = uint(spritePosX - spriteX);
		uint tilePixY = uint(spritePosY - spriteY);
		
		uint side = (tilePixY / 4U) % 2U;
		
		if(mode8x16){
			if(flipV && !bool(onY2)) side+=4U;
			else if(!flipV && bool(onY2)) side += 4U;
		}
		uint shift = (flipV ? 3U - tilePixY % 4U : tilePixY % 4U) * 8U;

		
		uint LSB = tile[flipV ? 1U - side : side] >> shift;
		LSB = (LSB >> (flipH ? tilePixX : 7U - tilePixX)) & 1U;

		uint MSB = tile[(flipV ? 1U - side: side) + 2U] >> shift;
		MSB = (MSB >> (flipH ? tilePixX : 7U - tilePixX)) & 1U;
		
		uint colorNum = (MSB << 1U) | LSB;

		if(colorNum != 0U)
			Frag_color = vec4(palettes[tilePalette * 3U + colorNum - 1U] / 255, 1);
	}
}