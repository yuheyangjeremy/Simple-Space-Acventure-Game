#pragma once
#include <vector>

class Texture 
{
public:
    void loadCubemap(std::vector<const char*> faces);
	void setupTexture(const char* texturePath);
	void bind(unsigned int slot) const;
    void bind_cubemap(unsigned int slot) const;
	void unbind() const;

private:
	unsigned int ID = 0;
	int Width = 0, Height = 0, BPP = 0;
};
