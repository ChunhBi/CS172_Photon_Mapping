#include <brdf.h>
#include <utils.h>
#include <interaction.h>

/**
 * IdealDiffusion class
 */

IdealDiffusion::IdealDiffusion(vec3 diff) : reflectivity(diff) {}

vec3 IdealDiffusion::eval(const Interaction& interact) {
  if (interact.wo.dot(interact.normal) < 0) return vec3(0, 0, 0);
  return reflectivity / PI;
}

Float IdealDiffusion::pdf(const Interaction& interact) {
  return 1 / 2 * PI;
}

Float IdealDiffusion::sample(Interaction& interact) {
  // pdf is proportional to cos��, which makes monte-carlo estimator converge faster
  vec3 normal = interact.normal;
  auto s = unif(0, 1, 2);
  float s1 = s[0], s2 = s[1];

  float x1 = cos(2 * PI * s2) * sqrt(1 - s1 * s1);
  float y1 = sin(2 * PI * s2) * sqrt(1 - s1 * s1);
  float z1 = s1;
  auto deviation = unif(-0.5, 0.5, 3);
  vec3 ref_x = normal + vec3(deviation[0], deviation[1], deviation[2]);
  vec3 ref_y = ref_x.cross(normal).normalized();
  ref_x = normal.cross(ref_y).normalized();

  float s3 = sqrt(1 - s1 * s1 - s2 * s2);

  interact.wi = x1 * ref_x + y1 * ref_y + z1 * normal;
  interact.wi = interact.wi.normalized();

  return 1 / (2 * PI);
}

bool IdealDiffusion::isDelta() const { return false; }

char* IdealDiffusion::getName() { return "IdealDiffusion"; }


std::shared_ptr<BRDF> makeIdealDiffusion(const vec3& diff) {
  return std::make_shared<IdealDiffusion>(diff);
}

/**
 * IdealSpecular class
 */

IdealSpecular::IdealSpecular() {}

vec3 IdealSpecular::eval(const Interaction& interact) {
  vec3 diff = (interact.wi + interact.wo).normalized() - interact.normal;
  return (diff.dot(diff) < 0.1) * vec3(1.0, 1.0, 1.0);
}

Float IdealSpecular::pdf(const Interaction& interact) {
  vec3 diff = (interact.wi + interact.wo).normalized() - interact.normal;
  return (diff.dot(diff) < 0.1);
}

Float IdealSpecular::sample(Interaction& interact) {
  vec3 wo_ll = interact.wo.dot(interact.normal) * interact.normal;
  vec3 wo_p = interact.wo - wo_ll;
  interact.wi = wo_ll - wo_p;
  return 1;
}

char* IdealSpecular::getName() { return "IdealSpecular"; }

bool IdealSpecular::isDelta() const { return true; }

std::shared_ptr<BRDF> makeIdealSpecular() {
  return std::make_shared<IdealSpecular>();
}


/**
 * IdealTransmission class
 */

IdealTransmission::IdealTransmission() {}

vec3 IdealTransmission::eval(const Interaction& interact) {
  vec3 diff = (interact.wi + interact.wo);
  return (diff.dot(diff) < 0.00001) * vec3(1.0, 1.0, 1.0);
}

Float IdealTransmission::pdf(const Interaction& interact) {
  vec3 diff = (interact.wi + interact.wo);
  return (diff.dot(diff) < 0.00001);
}

Float IdealTransmission::sample(Interaction& interact) {
  interact.wi = -interact.wo;
  return 1;
}

char* IdealTransmission::getName() { return "IdealTransmission"; }

bool IdealTransmission::isDelta() const { return true; }

std::shared_ptr<BRDF> makeIdealTransmission() {
  return std::make_shared<IdealTransmission>();
}




/**
 * Translucent class
 */

Translucent::Translucent(float ref, vec3 c) : refraction(ref), color(c) {}

vec3 Translucent::eval(const Interaction& interact) {
  Interaction curInter = interact;
  sample(curInter);
  vec3 diff = interact.wi - curInter.wi;
  return (diff.dot(diff) < 0.0001) * color;
}

Float Translucent::pdf(const Interaction& interact) {
  Interaction curInter = interact;
  sample(curInter);
  vec3 diff = interact.wi - curInter.wi;
  return (diff.dot(diff) < 0.0001);
}
char* Translucent::getName() { return "Translucent"; }

Float Translucent::sample(Interaction& interact) {
  vec3 wIn = -interact.wo;
  float cosIn = -wIn.dot(interact.normal);
  float r = refraction;
  vec3 normal = interact.normal;
  if (cosIn < 0) {
    cosIn = -cosIn;
    r = 1 / r;
    normal = -normal;
  }
  float cosOut2 = 1.0 - r * r * (1.0 - cosIn * cosIn);
  if (cosOut2 > 0) {
    // not total reflection, use refraction
    interact.wi = r * wIn + (r * cosIn - sqrt(cosOut2)) * normal;
    interact.wi = interact.wi.normalized();
  }
  else {
    // total reflection
    vec3 wo_ll = interact.wo.dot(normal) * normal;
    vec3 wo_p = interact.wo - wo_ll;
    interact.wi = wo_ll - wo_p;
  }
  return 1;
}

bool Translucent::isDelta() const { return true; }

std::shared_ptr<BRDF> makeTranslucent(const float& ref, const vec3 color = vec3(1.0, 1.0, 1.0)) {
  return std::make_shared<Translucent>(ref,color);
}


/**
 * Glossy class
 */

Glossy::Glossy(float a) { alpha = a; }

vec3 Glossy::eval(const Interaction& interact) {
  if (interact.normal.dot(interact.wi) < 0) return vec3(0, 0, 0);
  vec3 wo_ll = interact.wo.dot(interact.normal) * interact.normal;
  vec3 wo_p = interact.wo - wo_ll;
  vec3 idealIn = wo_ll - wo_p;

  return pow(idealIn.dot(interact.wi), alpha) * vec3(1.0, 1.0, 1.0);
}

Float Glossy::pdf(const Interaction& interact) {
  if (interact.normal.dot(interact.wi) < 0) return 0;
  vec3 wo_ll = interact.wo.dot(interact.normal) * interact.normal;
  vec3 wo_p = interact.wo - wo_ll;
  vec3 idealIn = wo_ll - wo_p;
  return pow(idealIn.dot(interact.wi), alpha) * (alpha + 1) / 2 / PI;
}

Float Glossy::sample(Interaction& interact) {
  vec3 wo_ll = interact.wo.dot(interact.normal) * interact.normal;
  vec3 wo_p = interact.wo - wo_ll;
  vec3 idealIn = wo_ll - wo_p;

  vec3 normal = idealIn;
  auto s = unif(0, 1, 2);
  float s1 = s[0], s2 = s[1];

  auto delta_x = sqrt(1 - pow(1 - s1, 2 / (alpha + 1))) * cos(2 * PI * s2);
  auto delta_y = sqrt(1 - pow(1 - s1, 2 / (alpha + 1))) * sin(2 * PI * s2);
  auto delta_z = pow(1 - s1, 1 / (alpha + 1));

  auto deviation = unif(-0.5, 0.5, 3);
  vec3 ref_x = idealIn + vec3(deviation[0], deviation[1], deviation[2]);
  vec3 ref_y = ref_x.cross(normal).normalized();
  ref_x = normal.cross(ref_y).normalized();

  interact.wi = delta_x * ref_x + delta_y * ref_y + delta_z * idealIn;
  if (interact.normal.dot(interact.wi) < 0) return 0;
  interact.wi = interact.wi.normalized();
  return pow(idealIn.dot(interact.wi), alpha) * (alpha + 1) / 2 / PI;
}

char* Glossy::getName() { return "Glossy"; }


bool Glossy::isDelta() const { return false; }

std::shared_ptr<BRDF> makeGlossy(float a) {
  return std::make_shared<Glossy>(a);
}


/**
 * TextureMaterial class
 */

TextureMaterial::TextureMaterial(std::shared_ptr<Texture> tex, Float sh)
{
  texture = tex;
  shininess = sh;
}

vec3 TextureMaterial::eval(const Interaction& interact) {
  Interaction m;
  float w = texture->width, h = texture->height, ch = texture->channel_num;
  float u = interact.uv[0] * w, v = (1 - interact.uv[1]) * h;

  return texture->values[int(v)][int(u)] / 255;
}

Float TextureMaterial::pdf(const Interaction& interact) {
  return interact.normal.dot(interact.wi) / PI;
}

Float TextureMaterial::sample(Interaction& interact) {
  // pdf is proportional to cos��, which makes monte-carlo estimator converge faster
  vec3 normal = interact.normal;
  auto s = unif(0, 1, 2);
  float s1 = s[0], s2 = s[1];

  float x1 = cos(2 * PI * s2) * sqrt(1 - s1 * s1);
  float y1 = sin(2 * PI * s2) * sqrt(1 - s1 * s1);
  float z1 = s1;
  auto deviation = unif(-0.5, 0.5, 3);
  vec3 ref_x = normal + vec3(deviation[0], deviation[1], deviation[2]);
  vec3 ref_y = ref_x.cross(normal).normalized();
  ref_x = normal.cross(ref_y).normalized();

  float s3 = sqrt(1 - s1 * s1 - s2 * s2);

  interact.wi = x1 * ref_x + y1 * ref_y + z1 * normal;
  interact.wi = interact.wi.normalized();

  return 1 / (2 * PI);
}

bool TextureMaterial::isDelta() const { return false; }

char* TextureMaterial::getName() { return "IdealDiffusion"; }

std::shared_ptr<BRDF> makeTextureMaterial(std::shared_ptr<Texture> tex, Float sh)
{
  return std::make_shared<TextureMaterial>(tex, sh);
}



