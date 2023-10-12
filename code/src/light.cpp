#include <light.h>
#include <utils.h>

/**
 * Light class
 */
Light::Light(const vec3 &position, const vec3 &radiance)
    : position(position), radiance(radiance) {}

vec3 Light::getPosition() const { return position; }

vec3 Light::getRadiance() const { return radiance; }

/**
 * AreaLight class
 */
AreaLight::AreaLight(const vec3 &position, const vec3 &color, const vec2 &size)
    : Light(position, color),
      areaSize(size),
      geoms(makeParallelogram(position - vec3(size[0], 0, size[1]) / 2,
                              vec3(size[0], 0, 0), vec3(0, 0, size[1]),
                              vec3(0, -1, 0), nullptr)) {}

vec3 AreaLight::emission(vec3 pos, vec3 dir) {
    auto geom = geoms[0];
    if (dir.dot(geom.get()->getNormal()) < 0.0f)
        return radiance;
    else
        return vec3::Zero();
    //return std::max(dir.dot(geom.get()->getNormal()), 0.0f) * radiance;
}

Float AreaLight::pdf(const Interaction& ref_it, vec3 pos) {
    auto geom = geoms[0];
    Float cos_theta1 = abs(geom.get()->getNormal().dot(ref_it.wi));
    Float distance = (pos - ref_it.entryPoint).norm();
    return cos_theta1 / (distance * distance); // solid angle of the tiny surface
}

vec3 AreaLight::sample(Interaction& refIt, Float* pdf) {
    auto random_sample = unif(0.0, 1.0, 2);
    Float x1 = random_sample[0];
    Float z1 = random_sample[1];
    vec3 sample_position = position + ((x1 - 0.5) * areaSize[0] * vec3(1, 0, 0)) + ((z1 - 0.5) * areaSize[1] * vec3(0, 0, 1));//uniformly ramdom choose a point on arealight
    refIt.wi = (refIt.entryPoint - sample_position).normalized();// incoming radiance direction
    *pdf = 1 / (areaSize[0] * areaSize[1]); //pdf_light = 1/A
    return sample_position;
}

bool AreaLight::intersect(Interaction &interaction, const Ray &ray) {
  bool intersection = false;
  for (auto &i : geoms)
    intersection = intersection || i->intersect(interaction, ray);
  interaction.type = Interaction::Type::LIGHT;
  if (intersection)
      interaction.emission = this->emission(interaction.entryPoint, ray.direction);
  return intersection;
}

Ray AreaLight::generateRay(vec3& light_energy)
{
    auto random_sample = unif(0.0, 1.0, 2);
    Float x1 = random_sample[0];
    Float z1 = random_sample[1];
    vec3 sample_position = position + ((x1 - 0.5) * areaSize[0] * vec3(1, 0, 0)) + ((z1 - 0.5) * areaSize[1] * vec3(0, 0, 1));//uniformly ramdom choose a point on arealight
    
    
    //auto random_samples = unif(0.0, 1.0, 2);
    ////sampling on a disk
    //x1 = random_samples[0];
    //Float y1 = random_samples[1];

    ////cosine-weighted hemisphere sampling
    //Float sin_theta = sqrt(x1);
    //Float cos_theta = sqrt(1 - x1);
    //Float phi = 2 * PI * y1;
    //vec3 tmp_wi = vec3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);

    auto s = unif(0, 1, 2);
    float s1 = s[0], s2 = s[1];

    float x = cos(2 * PI * s2) * sqrt(1 - s1 * s1);
    float y = sin(2 * PI * s2) * sqrt(1 - s1 * s1);
    float z = s1;
    vec3 tmp_wi = vec3(x, y, z);

    Eigen::Matrix3f rotation_matrix = Eigen::Quaternionf::FromTwoVectors(vec3(0, 0, 1), normal).toRotationMatrix();
    vec3 direction = (rotation_matrix * tmp_wi).normalized(); // from (0,0,1) system to world coordinate
    light_energy = this->getRadiance() * PI * areaSize[0] * areaSize[1];
    return Ray(sample_position+0.0001*direction, direction);

}

PointLight::PointLight(const vec3& position, const vec3& color) : Light(position, color) {}


vec3 PointLight::emission(vec3 pos, vec3 dir)
{
    return radiance;
}


Float PointLight::pdf(const Interaction& ref_it, vec3 pos) {
    return 1.0f;
}

vec3 PointLight::sample(Interaction& refIt, Float* pdf) {
    return vec3::Zero();
}

Ray PointLight::generateRay(vec3& light_energy)
{
    vec3 origin = this->position;
    
    auto s = unif(0, 1, 2);
    Float s1 = s[0], s2 = s[1];

    Float theta = s1 * 2 * PI;
    Float phi = s2 * 2 * PI - PI;

    vec3 direction = vec3(cos(phi) * sin(theta), cos(phi) * cos(theta), sin(phi)).normalized();
    light_energy = radiance;
    return Ray(origin + 0.0001 * direction, direction);

}

bool PointLight::intersect(Interaction& interaction, const Ray& ray) {
    bool intersection = false;
    interaction.type = Interaction::Type::NONE;
    return intersection;
}



std::shared_ptr<Light> makeAreaLight(const vec3 &position, const vec3 &color,
                                     const vec2 &size) {
  return std::make_shared<AreaLight>(position, color, size);
}

std::shared_ptr<Light> makePointLight(const vec3& position, const vec3& color) {
    return std::make_shared<PointLight>(position, color);
}
