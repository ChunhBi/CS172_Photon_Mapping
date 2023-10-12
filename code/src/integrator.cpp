#include <integrator.h>
#include <brdf.h>
#include <light.h>
#include <chrono>
#include <iostream>
#define USE_DIRECTLIGHTING 1
#define USE_OPENMP 1
/**
 * Integrator class
 */
Integrator::Integrator(std::shared_ptr<Camera> camera) : camera(camera) {}

/**
 * PhongLightingIntegrator class
 */

PathIntegrator::PathIntegrator(std::shared_ptr<Camera> camera)
  : Integrator(camera), max_depth(8), spp(128) {  }


PathIntegrator::PathIntegrator(std::shared_ptr<Camera> camera, int depth, int num)
    : Integrator(camera), max_depth(depth),spp(num){  }

/**
 * render a scene
 * @param[in] the given scene
 */
void PathIntegrator::render(Scene& scene) {
    int now = 0;
    float rotate_agl = 40;
    rotate_agl = rotate_agl / 180 * PI;
    Eigen::Matrix2f rot_mtx;
    rot_mtx << cos(rotate_agl), -sin(rotate_agl), sin(rotate_agl), cos(rotate_agl);
    std::vector<vec2> samples(9);
    samples[0] = rot_mtx * vec2(-0.333, 0.333);
    samples[1] = rot_mtx * vec2(-0.333, 0);
    samples[2] = rot_mtx * vec2(-0.333, -0.333);
    samples[3] = rot_mtx * vec2(0, 0.333);
    samples[4] = rot_mtx * vec2(0, 0);
    samples[5] = rot_mtx * vec2(0, -0.333);
    samples[6] = rot_mtx * vec2(0.333, 0.333);
    samples[7] = rot_mtx * vec2(0.333, 0);
    samples[8] = rot_mtx * vec2(0.333, -0.333);

#ifdef USE_OPENMP
#pragma omp parallel for schedule(guided, 16) default(none) shared(now)
#endif
    for (int dx = 0; dx < camera->getFilm().resolution.x(); ++dx) {
#ifdef USE_OPENMP
#pragma omp atomic
#endif
        ++now;
        printf("\r%.02f%%", now * 100.0 / camera->getFilm().resolution.x());
        for (int dy = 0; dy < camera->getFilm().resolution.y(); ++dy) {
            vec3 L(0, 0, 0);

            // TODO: anti-aliasing
            //Ray ray = camera->generateRay(dx, dy);
            //L += radiance(scene, ray);
            int sample_num = spp; //128
            for (int i = 0; i < sample_num; i++)
            {
              vec3 tempL = vec3(0, 0, 0);
              for (auto& pos : samples)
              {
                Ray ray = camera->generateRay(pos[0] + dx, pos[1] + dy);
                tempL += radiance(scene, ray);
              }
              tempL = tempL / 9;
              L += tempL;
            }
            L = L / sample_num;
            camera->setPixel(dx, dy, L);
        }
    }
}


/**
 * calculate the radiance with given scene, ray and interaction
 * @param[in] scene the given scene
 * @param[in] interaction the given interaction
 * @param[in] the given ray
 */
vec3 PathIntegrator::radiance(Scene &scene, const Ray &ray) const {

  vec3 L(0, 0, 0);
  Ray new_ray = ray;
  vec3 beta = vec3(1.0, 1.0, 1.0);
  int depth = max_depth;//8
  for (int i = 0; i < depth; i++) {
    Interaction interaction;
    if (scene.intersect(new_ray, interaction)) {
      if (interaction.type) {
        if (interaction.type == Interaction::GEOMETRY) {
          float pdf;
          interaction.wo = -new_ray.direction;

#ifdef USE_DIRECTLIGHTING
          if (!interaction.brdf->isDelta()) {     // below is the light sampling part
            vec3 lightPos = scene.getLight()->sample(interaction, &pdf);
            interaction.wi = (lightPos - interaction.entryPoint).normalized();

            new_ray.direction = interaction.wi;
            new_ray.origin = interaction.entryPoint + 0.0001 * interaction.normal;
            vec3 L_weighted = vec3(0, 0, 0), L2_wighted = vec3(0, 0, 0);
            if (!scene.isShadowed(new_ray)) {
              vec3 tempL = interaction.brdf->eval(interaction).cwiseProduct(scene.getLight()->emission(lightPos, interaction.wi));
              tempL = tempL * interaction.normal.dot(interaction.wi) * vec3(0, -1, 0).dot(-interaction.wi) / pdf;           
              tempL = tempL / (lightPos - interaction.entryPoint).dot(lightPos - interaction.entryPoint);
              L_weighted = beta.cwiseProduct(tempL);
              if (L_weighted[0] > 0 && L_weighted[1] > 0 && L_weighted[2] > 0)
              {
                L += L_weighted;
              }
            }
          }
#endif

          pdf = interaction.brdf->sample(interaction);
          if (pdf == 0) break;
          new_ray.direction = interaction.wi;
          new_ray.origin = interaction.entryPoint + 0.0001 * new_ray.direction;

          beta = beta.cwiseProduct(interaction.brdf->eval(interaction));
        }
        else if (interaction.type == Interaction::LIGHT) {
          vec3 tempL = interaction.emission;
          L += beta.cwiseProduct(tempL);
          break;
        }
      }
    }
    else {
      break;
    }
  }
  return L;
}


/// <summary>
/// Integrator for photon mapping
/// </summary>
/// <param name="round"></param>
PhotonIntegrator::PhotonIntegrator(std::shared_ptr<Camera> camera)
    : Integrator(camera), render_round(DEFAULT_RENDER_ROUND), 
    photon_num(DEFAULT_PHOTON_NUM), initial_radius(DEFAULT_INITIAL_RAIUS), re_decay(DEFAULT_RE_DECAY), max_depth(MAX_DEPTH),
    bounceMaxDepth(BOUNCD_MAX_TIME), spp(1){}

PhotonIntegrator::PhotonIntegrator(std::shared_ptr<Camera> camera, int render_round, int photon_num, Float initial_radius,
    Float re_decay, int bounceMaxDepth, int max_depth, int spp)
    : Integrator(camera)
{
    this->render_round = render_round;
    this->photon_num = photon_num;
    this->initial_radius = initial_radius;
    this->re_decay = re_decay;
    this->bounceMaxDepth = bounceMaxDepth;
    this->max_depth = max_depth;
    this->spp = spp;
}

vec3 PhotonIntegrator::radiance(Scene& scene, const Ray& ray) const {
    return vec3::Zero();
}

void PhotonIntegrator::setRenderround(int round)
{
    render_round = round;
}

void PhotonIntegrator::buildKdPointTree(std::vector<std::shared_ptr<ViewPoint>> viewpoints)
{
    this->kd_point_tree = std::make_shared<KdPointTree>(KdPointTree(viewpoints));
}






vec3 PhotonIntegrator::RayTracing(Scene& scene, const Ray& ray, double strength, int x, int y, int depth = 0, const vec3 color = vec3(1.0,1.0,1.0))
{
  if (depth >= bounceMaxDepth) return vec3(0, 0, 0);
  Ray new_ray = ray;
  Interaction interaction;
  if (scene.intersect(new_ray, interaction)) {
    if (interaction.type) {
      interaction.wo = -new_ray.direction;
      if (interaction.type == Interaction::GEOMETRY) {
        if (strcmp(interaction.brdf->getName(), "IdealDiffusion") == 0)
        {
          std::shared_ptr<ViewPoint> new_point(new ViewPoint(interaction.entryPoint, interaction.normal, color.cwiseProduct(interaction.brdf->eval(interaction)), strength, x, y));
#pragma omp critical
          view_points.push_back(new_point);
        }
        else
        {
          Float pdf = interaction.brdf->sample(interaction);
          new_ray.direction = interaction.wi;
          new_ray.origin = interaction.entryPoint + 0.0001 * new_ray.direction;
          return RayTracing(scene, new_ray, strength, x, y, depth + 1, color.cwiseProduct(interaction.brdf->eval(interaction)));
        }
      }
      else if (interaction.type == Interaction::LIGHT) {
          vec3 tempL = interaction.emission.cwiseProduct(color);
//#pragma omp critical
        //pixels_data[x * camera->getFilm().resolution.y() + y] = tempL * strength;
          return tempL * strength;
      }
    }
  }
  return vec3(0, 0, 0);
}

void PhotonIntegrator::PhotonTracing(Scene& scene, const Ray& ray, const int depth, const vec3 radi, const Float current_radius) {
  if (depth > max_depth) return;
  Interaction interact;
  scene.intersect(ray, interact);
  if (interact.type != Interaction::GEOMETRY) return;
  interact.wo = -ray.direction;
  if (strcmp(interact.brdf->getName(), "IdealDiffusion") != 0)
  {
    interact.brdf->sample(interact);
    Ray newray = Ray(interact.entryPoint + 0.0001 * interact.wi, interact.wi);
    vec3 tmp_color = interact.brdf->eval(interact);
    tmp_color = tmp_color.cwiseProduct(radi).cwiseMax(vec3::Zero());
    PhotonTracing(scene, newray, depth + 1, tmp_color, current_radius);
  }
  else
  {
    std::vector<std::shared_ptr<ViewPoint>> tmpViewPoint;
    kd_point_tree->search(tmpViewPoint, interact.entryPoint, current_radius);
    for (auto& v : tmpViewPoint) {
      if (v->N.dot(ray.direction) < 0)
      {
        Float r = current_radius;
        vec3 res = v->color.cwiseProduct(radi).cwiseMax(vec3::Zero()) / (PI * r * r) * v->strength;
#pragma omp critical
        pixels_data[v->x * camera->getFilm().resolution.y() + v->y] += res;

      }
    }
    float pdf = interact.brdf->sample(interact);
    Ray newray = Ray(interact.entryPoint + 0.0000001 * interact.wi, interact.wi);
    vec3 tmp_color = interact.brdf->eval(interact);
    tmp_color = tmp_color.cwiseProduct(radi).cwiseMax(vec3::Zero()) * PI;

    PhotonTracing(scene, newray, depth + 1, tmp_color, current_radius);
  }
}


void PhotonIntegrator::render(Scene& scene) {
    //initialize for render process
    scene.buildAccel();
    int now = 0;
    int film_x = camera->getFilm().resolution.x();
    int film_y = camera->getFilm().resolution.y();

    pixels_data.resize(film_x * film_y);//initialize pixels data
    
    Float current_radius = initial_radius; //initialize radius for photon_tracing
    //Float current_energy = 1.0f / log(render_round); //initialize energy for photon tracing
    Float current_energy = (1.0f - re_decay) / (1 -  pow(re_decay,render_round)); //initialize energy for photon tracing

    for (int dx = 0; dx < camera->getFilm().resolution.x(); ++dx)
    {
        for (int dy = 0; dy < camera->getFilm().resolution.y(); ++dy)
        {
            camera->setPixel(dx, dy, vec3::Zero());
        }
    }

    // start rendering
    for (int iter = 0; iter < render_round; iter++)
    {
        auto start = std::chrono::high_resolution_clock::now();
        // clear pixels_data every round
        view_points.clear(); //clear last round view points
        pixels_data.assign(pixels_data.size(), vec3::Zero()); 
        

#ifdef USE_OPENMP
#pragma omp parallel for schedule(guided, 16) default(none) shared(now)
#endif
        for (int dx = 0; dx < camera->getFilm().resolution.x(); ++dx)
        {
#ifdef USE_OPENMP
#pragma omp atomic
#endif
            ++now;
            printf("\r%.02f%%", now * 100.0 / camera->getFilm().resolution.x());
            for (int dy = 0; dy < camera->getFilm().resolution.y(); ++dy)
            {
              vec3 L = vec3(0, 0, 0);
                for (int i = 0; i < this->spp; i++)
                {
                    Float _dx = dx + (unif(0.0, 1.0, 1)[0] * 1.0 - .5) * 1;
                    Float _dy = dy + (unif(0.0, 1.0, 1)[0] * 1.0 - .5) * 1; // add random interruption every round
                    Ray cam_ray = camera->generateRay(_dx, _dy);
                    L += RayTracing(scene, cam_ray, current_energy/this->spp, dx, dy);
                }
                if (L != vec3(0, 0, 0))
                {
                    pixels_data[dx * camera->getFilm().resolution.y() + dy] = L / spp;
                }

            }
        }
        buildKdPointTree(view_points);
        printf("\nPhoton rendering...");

        int photon_now = 0;
        if (scene.getLights().empty())
        {
#ifdef USE_OPENMP
#pragma omp parallel for schedule(guided, 16) default(none) shared(photon_now)
#endif
            for (int i = 0; i < photon_num; i++)
            {
                printf("\r%.02f%%", photon_now * 100.0 / photon_num);
                vec3 light_energy;
                Ray light_ray = scene.getLight()->generateRay(light_energy); // randomly generate a ray from light
                vec3 radi = light_energy / photon_num;
                //vec3 radi = scene.getLight()->getRadiance()/ photon_num;
                PhotonTracing(scene, light_ray, 1, radi, current_radius); // todo
#ifdef USE_OPENMP
#pragma omp atomic
#endif
                photon_now++;
            }
        }
        else
        {
            int light_emit_num = (int)photon_num / scene.getLights().size();
            for (auto& lt : scene.getLights())
            {
#ifdef USE_OPENMP
#pragma omp parallel for schedule(guided, 16) default(none) shared(photon_now)
#endif
                for (int i = 0; i < light_emit_num; i++)
                {
                    printf("\r%.02f%%", photon_now * 100.0 / photon_num);
                    vec3 light_energy;
                    Ray light_ray = lt->generateRay(light_energy); // randomly generate a ray from light
                    vec3 radi = light_energy / photon_num;
                    //vec3 radi = scene.getLight()->getRadiance()/ photon_num;
                    PhotonTracing(scene, light_ray, 1, radi, current_radius); // todo
#ifdef USE_OPENMP
#pragma omp atomic
#endif
                    photon_now++;
                }
            }
        }



        for (int dx = 0; dx < camera->getFilm().resolution.x(); ++dx)
        {
            for (int dy = 0; dy < camera->getFilm().resolution.y(); ++dy)
            {
                camera->updatePixel(dx, dy, pixels_data[dx * film_y + dy]); //update pixel every time
            }
        }
        std::string file_name = "output_round";
        file_name += std::to_string(iter);
        file_name += ".png";
        camera->getFilm().write(file_name);
        current_radius *= re_decay; //the radius decreases every round
        current_energy *= re_decay; //the accumulated energy increases every round

        auto end = std::chrono::high_resolution_clock::now();
        double timeElapsed = static_cast<double>(
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count());
        std::cout
            << "\nRound " << iter << " takes " << timeElapsed << " ms" << std::endl;
    }

}


std::shared_ptr<Integrator> makePathIntegrator(std::shared_ptr<Camera> camera) {
  return std::make_shared<PathIntegrator>(camera);
}

std::shared_ptr<Integrator> makePathIntegrator(std::shared_ptr<Camera> camera, int max_depth, int spp) {
  return std::make_shared<PathIntegrator>(camera, max_depth, spp);
}

std::shared_ptr<Integrator> makePhotonIntegrator(std::shared_ptr<Camera> camera) {
    return std::make_shared<PhotonIntegrator>(camera);
}

std::shared_ptr<Integrator> makePhotonIntegrator(std::shared_ptr<Camera> camera, int render_round, int photon_num, Float initial_radius, 
        Float re_decay, int bouncemaxdepth, int max_depth, int spp) {
    return std::make_shared<PhotonIntegrator>(camera, render_round, photon_num, initial_radius, re_decay, bouncemaxdepth, max_depth, spp);
}

