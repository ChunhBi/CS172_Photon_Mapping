#include <texture.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

MaterialTexture::MaterialTexture(const std::string& path)
{
    std::vector<std::uint8_t> data;
    stbi_uc* img = stbi_load(path.c_str(), &width, &height, &channel_num, 0);
    data.assign(img, img + width * height * channel_num);
    stbi_image_free(img);

    for (int r = 0; r < height; r++)
    {
        std::vector<vec3> temp;
        for (int c = 0; c < width; c++)
        {
            int idx = (r * width + c) * channel_num;
            temp.push_back(vec3(data[idx], data[idx + 1], data[idx + 2]));
        }
        values.push_back(temp);
    }
}

NormalTexture::NormalTexture(const std::string& path)
{
  std::vector<std::uint8_t> data;
  stbi_uc* img = stbi_load(path.c_str(), &width, &height, &channel_num, 0);
  data.assign(img, img + width * height * channel_num);
  stbi_image_free(img);

  for (int r = 0; r < height; r++)
  {
      std::vector<vec3> temp;
      for (int c = 0; c < width; c++)
      {
          int idx = (r * width + c) * channel_num;
          temp.push_back(vec3(data[idx], data[idx + 1], data[idx + 2]));
      }
      values.push_back(temp);
  }
}


std::shared_ptr<Texture> makeMaterialTexture(const std::string& path)
{
    return std::make_shared<MaterialTexture>(path);
}

std::shared_ptr<Texture> makeNormalTexture(const std::string& path)
{
    return std::make_shared<NormalTexture>(path);
}




//void loadTextureFromFile(std::vector<std::uint8_t> &data,
//                         const std::string &path) {
//  int w, h, ch;
//  stbi_uc *img = stbi_load(path.c_str(), &w, &h, &ch, 0);
//  data.assign(img, img + w * h * ch);
//  stbi_image_free(img);
//}

