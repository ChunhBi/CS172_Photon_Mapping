#include <integrator.h>
#include <geometry.h>
#include <obj_loader.h>
#include <texture.h>
#include <chrono>
#include <cstdint>

//#include <filesystem>

std::string getPath(const std::string& target, int depth = 5) {
    std::string path = target;
    for (int i = 0; i < depth; ++i) {
        FILE* file = fopen(path.c_str(), "r");
        if (file) {
            fclose(file);
            return path;
        }
        path = "../" + path;
    }
    return target;
}


std::shared_ptr<Camera> genCamera(int id = 0,
                                  const vec2i &res = vec2i(512, 512));
std::shared_ptr<Scene> genCornellBoxScene(int id = 0);

int main(int argc, const char *argv[]) {
  int sceneId = 5;
  if (argc > 1) {
    int id = std::stoi(argv[1]);
    if (0 <= id && id <= 5) sceneId = id;
  }
  auto camera = genCamera(sceneId);
  auto scene = genCornellBoxScene(sceneId);
  // uncomment this after finishing k-d tree
  //scene->buildAccel();

  // Perform Phong lighting integrator
  std::cout << "Rendering..." << std::endl;

  auto start = std::chrono::high_resolution_clock::now();
  //auto integrator = makePathIntegrator(camera,32, 256);
  auto integrator = makePhotonIntegrator(camera, 15, 200000, 0.15, 0.8, 16, 16, 1);
  integrator->render(*scene);
  auto end = std::chrono::high_resolution_clock::now();
  double timeElapsed = static_cast<double>(
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count());
  std::cout << std::endl
            << "Time elapsed: " << timeElapsed << " ms" << std::endl;

  // Output the image to a file
  //for (int dx = 0; dx < camera->getFilm().resolution.x(); dx++)
  //{
  //    for (int dy = 0; dy < camera->getFilm().resolution.y(); dy++)
  //    {
  //        camera->setPixel(dx, dy, vec3(10,10,10));
  //    }
  //}
  camera->getFilm().write("output.png");

  return 0;
}

std::shared_ptr<Camera> genCamera(int id, const vec2i &res) {
  vec3 cameraPos;
  vec3 cameraLookAt;
  Float fov = 45;
  if (id == 0 || id == 1 || id == 2 || id == 3 || id == 4) {
    cameraPos = vec3(0, 1, 6.8);
    cameraLookAt = vec3(0, 1, 0);
    fov = 19;
  }
  if (id == 5) {
      cameraPos = vec3(0, 1.5, 6.8);
      cameraLookAt = vec3(0, 1, 0);
      fov = 19;
  }
  if (id == 6)
  {
      cameraPos = vec3(0, 2, 4.8);
      cameraLookAt = vec3(0, 0, 0);
      fov = 18;
  }
  auto camera = makeCamera(cameraPos, fov, res);
  camera->lookAt(cameraLookAt, vec3(0, 1, 0));
  return camera;
}

std::shared_ptr<Scene> genCornellBoxScene(int id) {
  // Material settings
  auto leftWallMat = makeIdealDiffusion(vec3(0.630000, 0.065000, 0.050000));
  auto rightWallMat = makeIdealDiffusion(vec3(0.140000, 0.450000, 0.091000));
  auto floorMat = makeIdealDiffusion(vec3(0.725000, 0.710000, 0.680000));
  auto ceilingMat = makeIdealDiffusion(vec3(0.725000, 0.710000, 0.680000));
  auto backWallMat = makeIdealDiffusion(vec3(0.725000, 0.710000, 0.680000));
  auto shortBoxMat = makeIdealDiffusion(vec3(0.725000, 0.710000, 0.680000));
  auto tallBoxMat = makeIdealDiffusion(vec3(0.725000, 0.710000, 0.680000));
  auto mirrorMat = makeIdealSpecular();
  //auto tranparentMat = makeIdealTransmission();
  auto glassMat = makeTranslucent(0.667, vec3(1, 1, 1));
  auto iceMat = makeTranslucent(0.667, vec3(191, 239, 255) / 255.0);
  auto metalMat = makeGlossy(50);

  //std::cout << "Current path is " << std::filesystem::current_path() << '\n';
  auto woodTex = makeMaterialTexture(getPath("/assets/textures/1.jpg"));
  auto cartTex = makeMaterialTexture(getPath("/assets/textures/1.jpg"));
  auto woodMat = makeTextureMaterial(woodTex, 16.0);
  auto cartMat = makeTextureMaterial(cartTex, 16.0);

  auto makeVec2 = [](const std::vector<Float> &vec) {
    std::vector<vec2> ret;
    for (int i = 0; i < static_cast<int>(vec.size()); i += 2) {
      ret.emplace_back(vec[i], vec[i + 1]);
    }
    return ret;
  };
  auto makeVec3 = [](const std::vector<Float> &vec) {
    std::vector<vec3> ret;
    for (int i = 0; i < static_cast<int>(vec.size()); i += 3) {
      ret.emplace_back(vec[i], vec[i + 1], vec[i + 2]);
    }
    return ret;
  };
  // a box
  // clang-format off
  std::vector<int> index{0, 2, 1, 0, 3, 2, 4, 6, 5, 4, 7, 6, 8, 10, 9, 8, 11, 10, 12, 14, 13, 12, 15, 14, 16, 18, 17, 16, 19, 18, 20, 22, 21, 20, 23, 22};
  std::vector<vec3> pos = makeVec3({ -0.720444, 1.2, -0.473882, -0.720444, 0.01, -0.473882, -0.146892, 0.01, -0.673479, -0.146892, 1.2, -0.673479, -0.523986, 0.01, 0.0906493, -0.523986, 1.2, 0.0906492, 0.0495656, 1.2, -0.108948, 0.0495656, 0.01, -0.108948, -0.523986, 1.2, 0.0906492, -0.720444, 1.2, -0.473882, -0.146892, 1.2, -0.673479, 0.0495656, 1.2, -0.108948, 0.0495656, 0.01, -0.108948, -0.146892, 0.01, -0.673479, -0.720444, 0.01, -0.473882, -0.523986, 0.01, 0.0906493, -0.523986, 0.01, 0.0906493, -0.720444, 0.01, -0.473882, -0.720444, 1.2, -0.473882, -0.523986, 1.2, 0.0906492, 0.0495656, 1.2, -0.108948, -0.146892, 1.2, -0.673479, -0.146892, 0.01, -0.673479, 0.0495656, 0.01, -0.108948 });
  std::vector<vec3> normals = makeVec3({ -0.328669, -4.1283e-008, -0.944445, -0.328669, -4.1283e-008, -0.944445, -0.328669, -4.1283e-008, -0.944445, -0.328669, -4.1283e-008, -0.944445, 0.328669, 4.1283e-008, 0.944445, 0.328669, 4.1283e-008, 0.944445, 0.328669, 4.1283e-008, 0.944445, 0.328669, 4.1283e-008, 0.944445, 3.82137e-015, 1, -4.37114e-008, 3.82137e-015, 1, -4.37114e-008, 3.82137e-015, 1, -4.37114e-008, 3.82137e-015, 1, -4.37114e-008, -3.82137e-015, -1, 4.37114e-008, -3.82137e-015, -1, 4.37114e-008, -3.82137e-015, -1, 4.37114e-008, -3.82137e-015, -1, 4.37114e-008, -0.944445, 1.43666e-008, 0.328669, -0.944445, 1.43666e-008, 0.328669, -0.944445, 1.43666e-008, 0.328669, -0.944445, 1.43666e-008, 0.328669, 0.944445, -1.43666e-008, -0.328669, 0.944445, -1.43666e-008, -0.328669, 0.944445, -1.43666e-008, -0.328669, 0.944445, -1.43666e-008, -0.328669 });
  std::vector<vec2> uv = makeVec2({ 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1 });
  auto tallBox = makeTriangleMesh(index, static_cast<int>(pos.size()), pos, normals, uv, tallBoxMat);

  // shortBox
  pos = makeVec3({ -0.0460751, 0.6, 0.573007, -0.0460751, 0.01, 0.573007, 0.124253, 0.01, 0.00310463, 0.124253, 0.6, 0.00310463, 0.533009, 0.01, 0.746079, 0.533009, 0.6, 0.746079, 0.703337, 0.6, 0.176177, 0.703337, 0.01, 0.176177, 0.533009, 0.6, 0.746079, -0.0460751, 0.6, 0.573007, 0.124253, 0.6, 0.00310463, 0.703337, 0.6, 0.176177, 0.703337, 0.01, 0.176177, 0.124253, 0.01, 0.00310463, -0.0460751, 0.01, 0.573007, 0.533009, 0.01, 0.746079, 0.533009, 0.01, 0.746079, -0.0460751, 0.01, 0.573007, -0.0460751, 0.6, 0.573007, 0.533009, 0.6, 0.746079, 0.703337, 0.6, 0.176177, 0.124253, 0.6, 0.00310463, 0.124253, 0.01, 0.00310463, 0.703337, 0.01, 0.176177 });
  normals = makeVec3({ -0.958123, -4.18809e-008, -0.286357, -0.958123, -4.18809e-008, -0.286357, -0.958123, -4.18809e-008, -0.286357, -0.958123, -4.18809e-008, -0.286357, 0.958123, 4.18809e-008, 0.286357, 0.958123, 4.18809e-008, 0.286357, 0.958123, 4.18809e-008, 0.286357, 0.958123, 4.18809e-008, 0.286357, -4.37114e-008, 1, -1.91069e-015, -4.37114e-008, 1, -1.91069e-015, -4.37114e-008, 1, -1.91069e-015, -4.37114e-008, 1, -1.91069e-015, 4.37114e-008, -1, 1.91069e-015, 4.37114e-008, -1, 1.91069e-015, 4.37114e-008, -1, 1.91069e-015, 4.37114e-008, -1, 1.91069e-015, -0.286357, -1.25171e-008, 0.958123, -0.286357, -1.25171e-008, 0.958123, -0.286357, -1.25171e-008, 0.958123, -0.286357, -1.25171e-008, 0.958123, 0.286357, 1.25171e-008, -0.958123, 0.286357, 1.25171e-008, -0.958123, 0.286357, 1.25171e-008, -0.958123, 0.286357, 1.25171e-008, -0.958123 });
  auto shortBox = makeTriangleMesh(index, static_cast<int>(pos.size()), pos, normals, uv, glassMat);
  // clang-format on

  // the Cornell box geometry
  std::vector<std::shared_ptr<Geometry>> floor;
  if (id <= 5)
  {
      index = { 0, 1, 2, 0, 2, 3 };
      pos = makeVec3({ -1, 0, -1, -1, 0, 1, 1, -0, 1, 1, -0, -1 });
      normals = makeVec3({ 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 });
      uv = makeVec2({ 0, 0, 1, 0, 1, 1, 0, 1 });
      floor = makeTriangleMesh(index, static_cast<int>(pos.size()), pos,
          normals, uv, floorMat);
  }
  if (id == 6)
  {
      index = { 0, 1, 2, 0, 2, 3 };
      pos = makeVec3({ -5, 0, -5, -5, 0, 5, 5, -0, 5, 5, -0, -5 });
      normals = makeVec3({ 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 });
      uv = makeVec2({ 0, 0, 1, 0, 1, 1, 0, 1 });
      floor = makeTriangleMesh(index, static_cast<int>(pos.size()), pos,
          normals, uv, woodMat);
  }

  pos = makeVec3({1, 2, 1, -1, 2, 1, -1, 2, -1, 1, 2, -1});
  normals = makeVec3({0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0});
  auto ceiling = makeTriangleMesh(index, static_cast<int>(pos.size()), pos,
                                  normals, uv, ceilingMat);

  pos = makeVec3({-1, 0, -1, -1, 2, -1, 1, 2, -1, 1, 0, -1});
  normals = makeVec3({0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1});
  auto backWall = makeTriangleMesh(index, static_cast<int>(pos.size()), pos,
                                   normals, uv, backWallMat);

  pos = makeVec3({1, 0, -1, 1, 2, -1, 1, 2, 1, 1, 0, 1});
  normals = makeVec3({-1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0});
  auto rightWall = makeTriangleMesh(index, static_cast<int>(pos.size()), pos,
                                    normals, uv, rightWallMat);

  pos = makeVec3({-1, 0, 1, -1, 2, 1, -1, 2, -1, -1, 0, -1});
  normals = makeVec3({1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0});
  auto leftWall = makeTriangleMesh(index, static_cast<int>(pos.size()), pos,
                                   normals, uv, leftWallMat);

  std::vector<std::shared_ptr<Geometry>> bunny;
  std::vector<std::shared_ptr<Geometry>> dragon;
  std::vector<std::shared_ptr<Geometry>> trans_sphere;
  std::vector<std::shared_ptr<Geometry>> refl_sphere;
  std::vector<std::shared_ptr<Geometry>> flowerpot;
  std::vector<std::shared_ptr<Geometry>> ocean;
  std::vector<std::shared_ptr<Geometry>> decanter;
  std::vector<std::shared_ptr<Geometry>> icecream;
  std::vector<std::vector<std::shared_ptr<Geometry>>> sub;

  if (id == 1) {
    bunny = makeObjMesh("assets/stanford_bunny.obj", shortBoxMat, 4,
                        vec3(0.5, 0, 0));
    trans_sphere = makeObjMesh("assets/sphere.obj", mirrorMat, 1,
                         vec3(0, 0, 0));
  }
  if (id == 2 || id == 3 || id == 4) {
    dragon = makeObjMesh("assets/stanford_dragon.obj", shortBoxMat, 5,
                         vec3(0, 0.1, 0));
  }

  if (id == 3) {
    sub.reserve(7);
    for (int i = 0; i < 7; ++i) {
      vec3 trans(cos(radians(static_cast<float>(i) / 7.0f * 360.0f)), 0,
                 sin(radians(static_cast<float>(i) / 7.0f * 360.0f)));
      trans *= 0.6f;
      sub.emplace_back(
          makeObjMesh("assets/stanford_bunny.obj", shortBoxMat, 2, trans));
    }
  }

  if (id == 4) {
    sub.reserve(7);
    for (int i = 0; i < 7; ++i) {
      vec3 trans(cos(radians(static_cast<float>(i) / 7.0f * 360.0f)), 0,
                 sin(radians(static_cast<float>(i) / 7.0f * 360.0f)));
      trans *= 0.6f;
      sub.emplace_back(
          makeObjMesh("assets/stanford_dragon.obj", shortBoxMat, 2, trans));
    }
  }

  if (id == 5)
  {
      refl_sphere = makeObjMesh("assets/sphere.obj", mirrorMat, 1,
          vec3(-0.7, 0.3, 0));
      trans_sphere = makeObjMesh("assets/sphere.obj", glassMat, 1,
          vec3(0, 0, 0));
      ocean = makeObjMesh("assets/custom_scene/Ocean.obj", iceMat, 0.5,
        vec3(0, 0.5, 0), vec3(0, 0, 0));
  }

  if (id == 6)
  {
      flowerpot = makeObjMesh("assets/custom_scene/flowerpot.obj", iceMat, 0.05,
          vec3(-0.5, 0, 0), vec3(0,1,0));
      bunny = makeObjMesh("assets/stanford_bunny.obj", iceMat, 4,
                    vec3(-0.1, 0, 0.2));
      decanter = makeObjMesh("assets/decanter_with_wine.obj", iceMat, 0.03,
          vec3(0, 0, 0.5), vec3(0, 0, 0));
      icecream = makeObjMesh("assets/Icecream.obj", iceMat, 0.15,
          vec3(0, 0.2, 0.5), vec3(0, 1, 0));
      dragon = makeObjMesh("assets/stanford_dragon.obj", iceMat, 4,
          vec3(0, -0.1, 0.7));
  }
  // Light setting
  std::shared_ptr<Light> light;
  std::vector<std::shared_ptr<Light>> lights;
  if (id == 0)
  {
    vec3 lightPos;
    vec3 lightColor;
    vec2 lightSize;

    lightPos = vec3(-0.005, 1.98, -0.03);
    lightColor = vec3(17.0, 12.0, 4.0) * 2;
    lightSize = vec2(0.235, 0.19);
    light = makeAreaLight(lightPos, lightColor, lightSize);

    lights.push_back(light);
  }
  if (id > 0 &&id <= 5 )
  {
      vec3 lightPos;
      vec3 lightColor;
      vec2 lightSize;

      lightPos = vec3(0.505, 1.98, -0.03);
      lightColor = vec3(17.0, 12.0, 4.0) * 4;
      lightSize = vec2(0.235, 0.19);
      light = makeAreaLight(lightPos, lightColor, lightSize);

      //lights.push_back(light);

      //second light
      lightPos = vec3(0.005, 1.98, -0.03);
      lightColor = vec3(17.0, 12.0, 4.0);
      lightSize = vec2(0.47, 0.38);
      lights.push_back(makeAreaLight(lightPos, lightColor, lightSize));
  }
  if (id == 6)
  {
      vec3 lightPos;
      vec3 lightColor;

      lightPos = vec3(0.005, 1.98, -0.03);
      lightColor = vec3(17.0, 12.0, 4.0);
      light = makePointLight(lightPos, lightColor);

      //lights.push_back(light);

      //second light
      lightPos = vec3(-0.505, 0.98, -0.03);
      lightColor = vec3(17.0, 12.0, 4.0) * 2;
      lights.push_back(makePointLight(lightPos, lightColor));


  }


  // Scene setup
  std::shared_ptr<Scene> scene;
  if (lights.empty())
    scene = std::make_shared<Scene>(light);
  else
    scene = std::make_shared<Scene>(lights);
  if (id <= 5)
  {
      scene->addGeometry(backWall);
      scene->addGeometry(leftWall);
      scene->addGeometry(rightWall);
      scene->addGeometry(floor);
      scene->addGeometry(ceiling);
  }
  if (id == 6)
  {
      scene->addGeometry(floor);
  }

  if (id == 0 || id == 1) scene->addGeometry(tallBox);
  if (id == 0) scene->addGeometry(shortBox);
  if (id == 1)
  {
      //scene->addGeometry(bunny);
      scene->addGeometry(trans_sphere);
  }
  if (id == 2 || id == 3 || id == 4) scene->addGeometry(dragon);
  if (id == 3 || id == 4)
    for (auto &m : sub) scene->addGeometry(m);
  if (id == 5)
  {
      scene->addGeometry(refl_sphere);
      scene->addGeometry(trans_sphere);
      scene->addGeometry(ocean);
      //scene->addGeometry(decanter);
  }
  if (id == 6)
  {
      //scene->addGeometry(flowerpot);
      //scene->addGeometry(bunny);
      //scene->addGeometry(decanter);
      //scene->addGeometry(icecream);
      scene->addGeometry(dragon);
  }
  return scene;
}
