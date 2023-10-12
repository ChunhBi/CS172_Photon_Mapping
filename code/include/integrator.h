#ifndef CS171_HW3_INCLUDE_INTEGRATOR_H_
#define CS171_HW3_INCLUDE_INTEGRATOR_H_
#include <core.h>
#include <utils.h>
#include <scene.h>
#include <camera.h>
#include <integrator.h>
#include <viewpoints.h>
/**
 * Base class of integrator
 */
class Integrator {
 public:
  explicit Integrator(std::shared_ptr<Camera> camera);
  virtual ~Integrator() = default;
  /**
   * render a scene
   * @param[in] the given scene
   */
  virtual void render(Scene &scene) = 0;
  /**
   * calculate the radiance with given scene, ray and interaction
   * @param[in] scene the given scene
   * @param[in] interaction the given interaction
   * @param[in] the given ray
   */
  virtual vec3 radiance(Scene &scene, const Ray &ray) const = 0;
 protected:
  std::shared_ptr<Camera> camera;
};

/**
 * Phong lighting integrator
 */
class PathIntegrator : public Integrator {
public:
	PathIntegrator(std::shared_ptr<Camera> camera);
  PathIntegrator(std::shared_ptr<Camera> camera, int max_depth, int spp = 1);
  void render(Scene &scene) override;
  vec3 radiance(Scene &scene, const Ray &ray) const override;
private:
	int max_depth;
	int spp;
};

class PhotonIntegrator : public Integrator {
public:
	PhotonIntegrator(std::shared_ptr<Camera> camera);
	PhotonIntegrator(std::shared_ptr<Camera> camera, int render_round, int photon_num, Float initial_radius, 
		Float re_decay, int bounceMaxDepth, int max_depth, int spp = 1);
	void render(Scene & scene);
	vec3 radiance(Scene& scene, const Ray& ray) const override;
	void setRenderround(int round);
	vec3 RayTracing(Scene& scene, const Ray& ray, double strength, int x, int y, int depth, const vec3 color);
  void PhotonTracing(Scene& scene, const Ray& ray, const int depth, const vec3 radi, const Float current_radius);
	void buildKdPointTree(std::vector<std::shared_ptr<ViewPoint>> viewpoints); // build KdPointTree from view points
private:
	int render_round;
	int photon_num;
	int max_depth;
	int bounceMaxDepth;
	std::vector<std::shared_ptr<ViewPoint>> view_points; // view points stored
	std::shared_ptr<KdPointTree> kd_point_tree; // kdpoint tree build from view points
	std::vector<vec3> pixels_data; // the rgb data for the pixels on the film. (update every round)
	Float initial_radius; //initial radius for photon tracing.
	Float re_decay; // the decay for radius and energy every round.
	int spp; //sample per pixel in ray tracing pass
};




std::shared_ptr<Integrator> makePathIntegrator(std::shared_ptr<Camera> camera);

std::shared_ptr<Integrator> makePathIntegrator(std::shared_ptr<Camera> camera, int max_depth, int spp);

std::shared_ptr<Integrator> makePhotonIntegrator(std::shared_ptr<Camera> camera);

std::shared_ptr<Integrator> makePhotonIntegrator(std::shared_ptr<Camera> camera, int render_round, 
	int photon_num, Float initial_radius, Float re_decay, int bouncemaxdepth, int max_depth, int spp = 1);

#endif  // CS171_HW3_INCLUDE_INTEGRATOR_H_
