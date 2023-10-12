#ifndef CS171_HW3_INCLUDE_TEXTURE_H_
#define CS171_HW3_INCLUDE_TEXTURE_H_
#include <vector>
#include <cstdint>
#include <string>
#include <core.h>

// optional
// you can change the structure if you want
class Texture {
public:
  Texture() = default;
  Texture(const std::string& path);

  int width, height, channel_num;
  std::vector<std::vector<vec3>> values;
};

class MaterialTexture : public Texture
{
public:
	MaterialTexture() = default;
	MaterialTexture(const std::string& path);
};


class NormalTexture : public Texture 
{
public:
  explicit NormalTexture(const std::string& path);
};


std::shared_ptr<Texture> makeMaterialTexture(const std::string& path);

std::shared_ptr<Texture> makeNormalTexture(const std::string& path);



//void loadTextureFromFile(std::vector<std::uint8_t>& data,
//    const std::string& path);
#endif