#include "Texture.h"
#include <Importers/Bmp/BmpParser.h>
#include <Importers/PPM/PpmParser.h>
#include <Importers/TGA/TgaParser.h>
#include <Importers/JPG/JpgParser.h>
#include <Importers/PNG/PngParser.h>
#include <Importers/TIF/TifParser.h>
#include <Importers/GIF/GifParser.h>
#include <Importers/HEIF/HeifParser.h>
#include <Importers/PGM/PgmParser.h>
#include <Importers/PBM/PbmParser.h>
#include <Importers/PNM/PnmParser.h>
#include <Importers/WebP/WebpParser.h>
//#include "stb_image/stb_image.h"
Texture::Texture(const std::string& path) :m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0) {
	//stbi_set_flip_vertically_on_load(1);
	//m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);
	//m_LocalBuffer = LoadBMP(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);
	m_LocalBuffer = LoadWebP(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);
	if (m_LocalBuffer == nullptr)
	{
		std::cerr << "Error loading file" << std::endl;
	}
	else
	{
		GLCall(glGenTextures(1, &m_RendererID));
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::cerr << "Error generating texture: " << gluErrorString(error) << std::endl;
		}

		GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::cerr << "Error binding texture: " << gluErrorString(error) << std::endl;
		}

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::cerr << "Error setting texture parameter: " << gluErrorString(error) << std::endl;
		}

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::cerr << "Error setting texture parameter: " << gluErrorString(error) << std::endl;
		}

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::cerr << "Error setting texture parameter: " << gluErrorString(error) << std::endl;
		}

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::cerr << "Error setting texture parameter: " << gluErrorString(error) << std::endl;
		}

		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer));
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::cerr << "Error uploading texture data: " << gluErrorString(error) << std::endl;
		}
	}
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
	if (m_LocalBuffer)
		return;
		//stbi_image_free(m_LocalBuffer);
}
Texture::~Texture() {
	GLCall(glDeleteTextures(1, &m_RendererID));
}
void Texture::Bind(unsigned int slot) const {
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}
void Texture::Unbind() const {
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}