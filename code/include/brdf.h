#ifndef CS171_HW4_INCLUDE_BRDF_H_
#define CS171_HW4_INCLUDE_BRDF_H_
#include <core.h>
#include <texture.h>
/**
 * Base class of BRDFs
 */
class BRDF {
public:
  BRDF() = default;

  /**
   * Evaluate the BRDF, namely, return the BRDF value at the given interaction
   */
  virtual vec3 eval(const Interaction& interact) = 0;
  /**
   * Compute the PDF of the given BRDF sample at the specified interaction
   * You may need to use the Interaction.wi and Interaction.wo
   */
  virtual Float pdf(const Interaction& interact) = 0;
  /**
   * Sample a direction according to the BRDF
   * Store the sampled direction in the given interaction
   * Also, the PDF of this sample should be returned
   */
  virtual Float sample(Interaction& interact) = 0;


  virtual char* getName() = 0;
  /**
   * Return if the BRDF is delta (specular or transmission)
   */
  [[nodiscard]] virtual bool isDelta() const = 0;
};

/**
 * Ideal diffusion BRDFs
 */

class IdealDiffusion : public BRDF {
protected:
  vec3 reflectivity;

public:
  explicit IdealDiffusion(vec3 diff);
  vec3 eval(const Interaction& interact) override;
  Float pdf(const Interaction& interact) override;
  Float sample(Interaction& interact) override;
  char* getName() override;
  [[nodiscard]] bool isDelta() const override;
};

std::shared_ptr<BRDF> makeIdealDiffusion(const vec3& diff);

// optional
/**
 * Ideal specular BRDFs
 */
class IdealSpecular : public BRDF {
public:
  explicit IdealSpecular();
  vec3 eval(const Interaction& interact) override;
  Float pdf(const Interaction& interact) override;
  Float sample(Interaction& interact) override;
  char* getName() override;
  [[nodiscard]] bool isDelta() const override;
};

std::shared_ptr<BRDF> makeIdealSpecular();

/**
 * Ideal IdealTransmission BRDFs
 */
class IdealTransmission : public BRDF {
public:
  explicit IdealTransmission();
  vec3 eval(const Interaction& interact) override;
  Float pdf(const Interaction& interact) override;
  Float sample(Interaction& interact) override;
  char* getName() override;
  [[nodiscard]] bool isDelta() const override;
};

std::shared_ptr<BRDF> makeIdealTransmission();

/**
 * Ideal translucent BRDFs
 */
class Translucent : public BRDF {
  float refraction;
  vec3 color;
public:
  explicit Translucent(float ref, vec3 color);
  vec3 eval(const Interaction& interact) override;
  Float pdf(const Interaction& interact) override;
  Float sample(Interaction& interact) override;
  char* getName() override;
  [[nodiscard]] bool isDelta() const override;
};

std::shared_ptr<BRDF> makeTranslucent(const float& ref, const vec3 color);

/**
 * TextureMaterial BRDFs
 */
class TextureMaterial : public BRDF {
  // optional TODO: finish this
protected:
  std::shared_ptr<Texture> texture;
  Float shininess;
  bool is_normalmap;

public:
  explicit TextureMaterial(std::shared_ptr<Texture> tex, Float sh);
  vec3 eval(const Interaction& interact) override;
  Float pdf(const Interaction& interact) override;
  Float sample(Interaction& interact) override;
  char* getName() override;
  [[nodiscard]] bool isDelta() const override;
  //vec3 getNormal(const Interaction& in);
};
std::shared_ptr<BRDF> makeTextureMaterial(std::shared_ptr<Texture> tex, Float sh);


class Glossy : public BRDF {
  float alpha;
public:
  explicit Glossy(float alpha);
  vec3 eval(const Interaction& interact) override;
  Float pdf(const Interaction& interact) override;
  Float sample(Interaction& interact) override;
  char* getName() override;
  [[nodiscard]] bool isDelta() const override;
};

std::shared_ptr<BRDF> makeGlossy(float alpha);


#endif  // CS171_HW4_INCLUDE_BRDF_H_
