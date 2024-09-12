#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <stb_image.h>
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>
#include "shader.h"
#include "utils.h"
#include "SMAA_Builder.h"
struct ray_definition {
	int sphere_size;
	int plane_size;
	int surface_size;
	int box_size;
	int torus_size;
	int ring_size;
	int light_point_size;
	int light_direct_size;
	int iterations;
	glm::vec3 ambient_color;
	glm::vec3 shadow_ambient;
};

typedef struct {
	glm::vec3 color; 
	glm::vec3 absorb;
	float __p1;
	float diffuse;
	float reflect;
	float refract;
	int specular;
	float kd;
	float ks;
	float __padding[3];
} ray_material;
typedef struct {
	ray_material material;
	glm::vec4 obj; // pos + radius
	glm::quat quat_rotation = glm::quat(1, 0, 0, 0);
	int textureNum;
	bool hollow;
	float __padding[2];
} ray_sphere;

typedef struct {
	ray_material material;
	glm::vec3 pos; float __p1;
	glm::vec3 normal; float __p2;
} ray_plane;

typedef struct {
	ray_material mat;
	glm::quat quat_rotation = glm::quat(1, 0, 0, 0);
	glm::vec3 pos; float __p1;
	glm::vec3 form;
	int textureNum;
} ray_box;

typedef struct {
	ray_material mat;
	glm::quat quat_rotation = glm::quat(1, 0, 0, 0);
	glm::vec3 pos; float __p1;
	glm::vec2 form; float __p2[2];
} ray_torus;

typedef struct {
	ray_material mat;
	glm::quat quat_rotation = glm::quat(1, 0, 0, 0);
	glm::vec3 pos; int textureNum;
	float r1, r2;
	float __p2[2];
} ray_ring;

typedef struct {
	ray_material mat;
	glm::quat quat_rotation = glm::quat(1, 0, 0, 0);
	float xMin = -FLT_MAX;
	float yMin = -FLT_MAX;
	float zMin = -FLT_MAX;
	float __p0;
	float xMax = FLT_MAX;
	float yMax = FLT_MAX;
	float zMax = FLT_MAX;
	float __p1;
	glm::vec3 pos;
	float a; // x2
	float b; // y2
	float c; // z2
	float d; // z
	float e; // y
	float f; // const

	float __padding[3];
} ray_surface;

typedef enum { sphere, light } primitiveType;

struct ray_lignt_direct {
	glm::vec3 direction; float __p1;
	glm::vec3 color;

	float intensity;
};

struct ray_lignt_point {
	glm::vec4 pos; //pos + radius
	glm::vec3 color;
	float intensity;

	float linear_k;
	float quadratic_k;
	float __padding[2];
};
class SurfaceFactory {
public:

	static ray_surface GetEllipsoid(float a, float b, float c, ray_material material)
	{
		ray_surface surface = {};
		surface.a = powf(a, -2);
		surface.b = powf(b, -2);
		surface.c = powf(c, -2);
		surface.f = -1;
		surface.mat = material;
		return surface;
	}

	static ray_surface GetEllipticParaboloid(float a, float b, ray_material material)
	{
		ray_surface surface = {};
		surface.a = powf(a, -2);
		surface.b = powf(b, -2);
		surface.d = -1;
		surface.mat = material;
		return surface;
	}

	static ray_surface GetHyperbolicParaboloid(float a, float b, ray_material material)
	{
		ray_surface surface = {};
		surface.a = powf(a, -2);
		surface.b = -powf(b, -2);
		surface.d = -1;
		surface.mat = material;
		return surface;
	}

	static ray_surface GetEllipticHyperboloidOneSheet(float a, float b, float c, ray_material material)
	{
		ray_surface surface = {};
		surface.a = powf(a, -2);
		surface.b = powf(b, -2);
		surface.c = -powf(c, -2);
		surface.f = -1;
		surface.mat = material;
		return surface;
	}

	static ray_surface GetEllipticHyperboloidTwoSheets(float a, float b, float c, ray_material material)
	{
		ray_surface surface = {};
		surface.a = powf(a, -2);
		surface.b = powf(b, -2);
		surface.c = -powf(c, -2);
		surface.f = 1;
		surface.mat = material;
		return surface;
	}

	static ray_surface GetEllipticCone(float a, float b, float c, ray_material material)
	{
		ray_surface surface = {};
		surface.a = powf(a, -2);
		surface.b = powf(b, -2);
		surface.c = -powf(c, -2);
		surface.mat = material;
		return surface;
	}

	static ray_surface GetEllipticCylinder(float a, float b, ray_material material)
	{
		ray_surface surface = {};
		surface.a = powf(a, -2);
		surface.b = powf(b, -2);
		surface.f = -1;
		surface.mat = material;
		return surface;
	}

	static ray_surface GetHyperbolicCylinder(float a, float b, ray_material material)
	{
		ray_surface surface = {};
		surface.a = powf(a, -2);
		surface.b = -powf(b, -2);
		surface.f = -1;
		surface.mat = material;
		return surface;
	}

	static ray_surface GetParabolicCylinder(float a, ray_material material)
	{
		ray_surface surface = {};
		surface.a = 1;
		surface.e = 2 * a;
		surface.mat = material;
		return surface;
	}
};
typedef struct {
	glm::quat quat_camera_rotation;
	glm::vec3 camera_pos; float __p1;

	glm::vec3 bg_color;
	int canvas_width;

	int canvas_height;
	int reflect_depth;
	float __padding[2];
} ray_scene;

struct scene_container
{
	ray_scene scene;
	glm::vec3 ambient_color;
	glm::vec3 shadow_ambient;
	std::vector<ray_sphere> spheres;
	std::vector<ray_plane> planes;
	std::vector<ray_surface> surfaces;
	std::vector<ray_box> boxes;
	std::vector<ray_torus> toruses;
	std::vector<ray_ring> rings;
	std::vector<ray_lignt_point> lights_point;
	std::vector<ray_lignt_direct> lights_direct;

	ray_definition get_defines()
	{
		return { static_cast<int>(spheres.size()),
			static_cast<int>(planes.size()),
			static_cast<int>(surfaces.size()),
			static_cast<int>(boxes.size()),
			static_cast<int>(toruses.size()),
			static_cast<int>(rings.size()),
			static_cast<int>(lights_point.size()),
			static_cast<int>(lights_direct.size()),
			scene.reflect_depth, ambient_color, shadow_ambient };
	}
};
static int wind_width = 1280;
static int wind_height = 720;
class GLWrapper
{
public:
	GLWrapper(int width, int height, bool fullScreen);
	GLWrapper(bool fullScreen);
	~GLWrapper();

	int getWidth();
	int getHeight();
	GLuint getProgramId();

	bool init_window();
	void init_shaders(ray_definition& defines);
	void set_skybox(unsigned int textureId);

	void stop();
	void enable_SMAA(SMAA_PRESET preset);

	GLFWwindow* window;

	void draw();
	static GLuint load_cubemap(std::vector<std::string> faces, bool genMipmap = false);
	GLuint load_texture(int texNum, const char* name, const char* uniformName, GLuint wrapMode = GL_REPEAT);
	void init_buffer(GLuint* ubo, const char* name, int bindingPoint, size_t size, void* data) const;
	static void update_buffer(GLuint ubo, size_t size, void* data);

private:
	Shader shader, edgeShader, blendShader, neighborhoodShader;
	GLuint skyboxTex, areaTex, searchTex;
	GLuint quadVAO, quadVBO;
	GLuint fboColor, fboTexColor, fboEdge, fboTexEdge, fboBlend, fboTexBlend;
	std::vector<GLuint> textures;

	int width;
	int height;

	bool fullScreen = true;
	bool useCustomResolution = false;
	bool SMAA_enabled = false;
	SMAA_PRESET SMAA_preset;

	void gen_framebuffer(GLuint* fbo, GLuint* fboTex, GLenum internalFormat, GLenum format) const;
	
	static GLuint load_texture(char const* path, GLuint wrapMode = GL_REPEAT);
	static std::string to_string(glm::vec3 v);
};
class SceneManager
{
public:
	SceneManager(int wind_width, int wind_height, scene_container* scene, GLWrapper* wrapper);

	void init();
	void update(float frameRate);

	static ray_material create_material(glm::vec3 color, int specular, float reflect, float refract = 0.0, glm::vec3 absorb = {}, float diffuse = 0.7, float kd = 0.8, float ks = 0.2);
	static ray_sphere create_sphere(glm::vec3 center, float radius, ray_material material, bool hollow = false);
	static ray_plane create_plane(glm::vec3 normal, glm::vec3 pos, ray_material material);
	static ray_box create_box(glm::vec3 pos, glm::vec3 form, ray_material material);
	static ray_torus create_torus(glm::vec3 pos, glm::vec2 form, ray_material material);
	static ray_ring create_ring(glm::vec3 pos, float r1, float r2, ray_material material);
	static ray_lignt_point create_light_point(glm::vec4 position, glm::vec3 color, float intensity, float linear_k = 0.22f, float quadratic_k = 0.2f);
	static ray_lignt_direct create_light_direct(glm::vec3 direction, glm::vec3 color, float intensity);
	static ray_scene create_scene(int width, int height);

private:
	scene_container* scene;

	int wind_width;
	int wind_height;
	GLWrapper* wrapper;

	bool w_pressed = false;
	bool a_pressed = false;
	bool s_pressed = false;
	bool d_pressed = false;
	bool ctrl_pressed = false;
	bool shift_pressed = false;
	bool space_pressed = false;
	bool alt_pressed = false;

	float lastX = 0;
	float lastY = 0;

	// Camera Attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 world_up = glm::vec3(0, 1, 0);
	// Euler Angles
	float yaw = 0;
	float pitch = 0;

	GLuint sceneUbo = 0;
	GLuint sphereUbo = 0;
	GLuint planeUbo = 0;
	GLuint surfaceUbo = 0;
	GLuint boxUbo = 0;
	GLuint torusUbo = 0;
	GLuint ringUbo = 0;
	GLuint lightPointUbo = 0;
	GLuint lightDirectUbo = 0;

	void update_scene(float deltaTime);
	void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void glfw_framebuffer_size_callback(GLFWwindow* wind, int width, int height);
	void glfw_mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void init_buffers();
	void update_buffers() const;
	glm::vec3 get_color(float r, float g, float b);

	template<typename T>
	void init_buffer(GLuint* ubo, const char* name, int bindingPoint, std::vector<T>& v);
	template<typename T>
	void update_buffer(GLuint ubo, std::vector<T>& v) const;
};
struct ray_definition;


void update_scene(scene_container& scene, float delta, float time);

namespace update {
	int jupiter = -1,
		saturn = -1,
		saturn_rings = -1,
		mars = -1,
		box = -1,
		torus = -1,
		baseball= -1;
}

const glm::quat saturn_pitch = glm::quat(glm::vec3(glm::radians(15.f), 0, 0));

int main()
{
	GLWrapper glWrapper(wind_width, wind_height, false);
	// fullscreen
	//GLWrapper glWrapper(true);
	// window with monitor resolution
	//GLWrapper glWrapper(false);

	// set SMAA quality preset
	glWrapper.enable_SMAA(ULTRA);
	
	glWrapper.init_window();
	glfwSwapInterval(1); // vsync
	wind_width = glWrapper.getWidth();
	wind_height = glWrapper.getHeight();

	// fix ray direction issues
	if (wind_width % 2 == 1) wind_width++;
	if (wind_height % 2 == 1) wind_height++;

	scene_container scene = {};

	scene.scene = SceneManager::create_scene(wind_width, wind_height);
	scene.scene.camera_pos = { 0, 0, -5 };
	scene.shadow_ambient = glm::vec3{ 0.1, 0.1, 0.1 };
	scene.ambient_color = glm::vec3{ 0.025, 0.025, 0.025 };

	// lights
	scene.lights_point.push_back(SceneManager::create_light_point({ 3, 5, 0, 0.1 }, { 1, 1, 1 }, 25.5));
	scene.lights_direct.push_back(SceneManager::create_light_direct({ 3, -1, 1 }, { 1, 1, 1 }, 1.5));

	// baseball
	ray_sphere baseball = SceneManager::create_sphere({ 2, 0, 6 }, 10,
		SceneManager::create_material({}, 0, 0.0f));
	baseball.textureNum = 6;
	scene.spheres.push_back(baseball);
	update::baseball = scene.spheres.size() - 1;
	//ball
	scene.spheres.push_back(SceneManager::create_sphere({ -2, 0, 2 }, 1,
		SceneManager::create_material({ 1, 0, 0 }, 100, 0.1), true));
	scene.spheres.push_back(SceneManager::create_sphere({ -4, 0, 2 }, 1,
		SceneManager::create_material({ 0, 1, 0 }, 100, 0.1), true));
	scene.spheres.push_back(SceneManager::create_sphere({ -6, 0, 2 }, 1,
		SceneManager::create_material({ 0, 0, 1 }, 100, 0.1), true));
	scene.spheres.push_back(SceneManager::create_sphere({ -8, 0, 2 }, 1,
		SceneManager::create_material({ 0, 0, 0 }, 100, 0.1), true));
	//reflect bsll
	scene.spheres.push_back(SceneManager::create_sphere({ 0, 0, 2 }, 1,
		SceneManager::create_material({ 0.5, 0.5, 0.5 }, 200, 0.1, 1.125, { 1, 0, 2 }, 1), true));
		
	scene.spheres.push_back(SceneManager::create_sphere({ 2, 0, 2 }, 1,
		SceneManager::create_material({ 0.1, 0.1, 0.1 }, 200, 0.1, 1.125, { 1, 0, 2 }, 1), true));
		
	scene.spheres.push_back(SceneManager::create_sphere({ 4, 0, 2 }, 1,
		SceneManager::create_material({ 1, 1, 1 }, 200, 0.1, 1.125, { 1, 0, 2 }, 1), true));
		
	scene.spheres.push_back(SceneManager::create_sphere({ 6, 0, 2 }, 1,
		SceneManager::create_material({ 1, 1, 1 }, 200, 0.1, 1.125, { 1, 0, 2 }, 1), true));
	
	// box
	ray_box box = SceneManager::create_box({ 1, 2, 6 }, { 1, 1, 1 },
		SceneManager::create_material({ 0.8,0.7,0 }, 50, 0.0));
	box.textureNum = 5;
	scene.boxes.push_back(box);
	update::box = scene.boxes.size() - 1;

	
	// jupiter
	ray_sphere jupiter = SceneManager::create_sphere({}, 5000,
		SceneManager::create_material({}, 0, 0.0f));
	jupiter.textureNum = 1;
	scene.spheres.push_back(jupiter);
	update::jupiter = scene.spheres.size() - 1;


	// mars
	ray_sphere mars = SceneManager::create_sphere({}, 500,SceneManager::create_material({}, 0, 0.0f));
	mars.textureNum = 3;
	scene.spheres.push_back(mars);
	update::mars = scene.spheres.size() - 1;


	// saturn
	const int saturnRadius = 4150;
	ray_sphere saturn = SceneManager::create_sphere({}, saturnRadius,
		SceneManager::create_material({}, 0, 0.0f));
	saturn.textureNum = 2;
	saturn.quat_rotation = saturn_pitch;
	scene.spheres.push_back(saturn);
	update::saturn = scene.spheres.size() - 1;
	// ring
	{
		ray_ring ring = SceneManager::create_ring({}, saturnRadius * 1.1166, saturnRadius * 2.35,
			SceneManager::create_material({}, 0, 0));
		ring.textureNum = 4;
		ring.quat_rotation = glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0)) * saturn_pitch;
		scene.rings.push_back(ring);
		update::saturn_rings = scene.rings.size() - 1;
	}

	// floor
	scene.boxes.push_back(SceneManager::create_box({ 0, -1.2, 6 }, { 10, 0.1, 10 },
		SceneManager::create_material({ 0.6, 0.3, 0.2 }, 150, 0.05)));
	

	// torus
	ray_torus torus = SceneManager::create_torus({ 0, 1, 1 }, { 1.0, 0.5 },
		SceneManager::create_material({ 0.7, 0.3, 0.8 }, 200, 0.2));
	torus.quat_rotation = glm::quat(glm::vec3(glm::radians(45.f), 0, 0));
	scene.toruses.push_back(torus);
	update::torus = scene.toruses.size() - 1;

	// cone
	ray_material coneMaterial = SceneManager::create_material({ 255 / 255.0f, 255 / 255.0f, 255 / 255.0f }, 200, 0.2);
	ray_surface cone = SurfaceFactory::GetEllipticCone(1 / 3.0f, 1 / 3.0f, 1, coneMaterial);
	cone.pos = { 5, 4, 6 };
	cone.quat_rotation = glm::quat(glm::vec3(glm::radians(90.f), 0, 0));
	cone.yMin = -1;
	cone.yMax = 2.5;
	scene.surfaces.push_back(cone);

	// cylinder
	ray_material cylinderMaterial = SceneManager::create_material({ 0, 255 / 255.0f, 255 / 255.0f }, 200, 0.2);
	ray_surface cylinder = SurfaceFactory::GetEllipticCylinder(1 / 2.0f, 1 / 2.0f, cylinderMaterial);
	cylinder.pos = { -3, 0, 5 };
	cylinder.quat_rotation = glm::quat(glm::vec3(glm::radians(90.f), 0, 0));
	cylinder.yMin = -1;
	cylinder.yMax = 1;
	scene.surfaces.push_back(cylinder);

	ray_definition defines = scene.get_defines();
	glWrapper.init_shaders(defines);

	

	auto jupiterTex = glWrapper.load_texture(1, "ear.jpg", "texture_sphere_1");
	auto saturnTex = glWrapper.load_texture(2, "8k_saturn.jpg", "texture_sphere_2");
	auto marsTex = glWrapper.load_texture(3, "2k_mars.jpg", "texture_sphere_3");
	auto ringTex = glWrapper.load_texture(4, "8k_saturn_ring_alpha.png", "texture_ring");
	auto boxTex = glWrapper.load_texture(5, "metal.png", "texture_box");
	auto baseballTex = glWrapper.load_texture(6, "baseball.jpg", "baseball");
	SceneManager scene_manager(wind_width, wind_height, &scene, &glWrapper);
	scene_manager.init();

	float currentTime = static_cast<float>(glfwGetTime());
	float lastFramesPrint = currentTime;
	float framesCount = 0;
	std::vector<std::string> faces =
	{
		ASSETS_DIR "/textures/background/darkblue.jpg",
		ASSETS_DIR "/textures/background/darkblue.jpg",
		ASSETS_DIR "/textures/background/darkblue.jpg",
		ASSETS_DIR "/textures/background/darkblue.jpg",
		ASSETS_DIR "/textures/background/darkblue.jpg",
		ASSETS_DIR "/textures/background/darkblue.jpg",
	};

	glWrapper.set_skybox(GLWrapper::load_cubemap(faces, false));
	while (!glfwWindowShouldClose(glWrapper.window))
	{
		framesCount++;
		float newTime = static_cast<float>(glfwGetTime());
		float deltaTime = newTime - currentTime;
		currentTime = newTime;

		if (newTime - lastFramesPrint > 1.0f)
		{
			//std::cout << "FPS: " << framesCount << std::endl;
			lastFramesPrint = newTime;
			framesCount = 0;
		}

		update_scene(scene, deltaTime, newTime);
		scene_manager.update(deltaTime);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, jupiterTex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, saturnTex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, marsTex);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, ringTex);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, boxTex);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, baseballTex);

		glWrapper.draw();
		glfwSwapBuffers(glWrapper.window);
		glfwPollEvents();
	}

	glWrapper.stop(); 
	return 0;
}

void update_scene(scene_container& scene, float deltaTime, float time)
{	
	
	if (update::jupiter != -1) {
		ray_sphere* jupiter = &scene.spheres[update::jupiter];
		const float jupiterSpeed = 0.02;
		jupiter->obj.x = cos(time * jupiterSpeed) * 20000;
		jupiter->obj.z = sin(time * jupiterSpeed) * 20000;
		jupiter->obj.y = 5000;
		jupiter->quat_rotation *= glm::angleAxis(deltaTime / 15, glm::vec3(0, 1, 0));
	}

	if (update::saturn != -1 && update::saturn_rings != -1) {
		ray_sphere* saturn = &scene.spheres[update::saturn];
		ray_ring* ring = &scene.rings[update::saturn_rings];
		const float speed = 0.0082;
		const float dist = 35000;
		const float offset = 1;

		saturn->obj.x = cos(time * speed + offset) * dist;
		saturn->obj.z = sin(time * speed + offset) * dist;
		saturn->obj.y = 9000;
		glm::vec3 axis = glm::vec3(0, 1, 0) * saturn_pitch;
		saturn->quat_rotation *= glm::angleAxis(deltaTime / 10, axis);

		ring->pos.x = cos(time * speed + offset) * dist;
		ring->pos.z = sin(time * speed + offset) * dist;
		ring->pos.y = 9000+sin(time * speed*50 + offset)*3000;
	}	

	if (update::mars != -1) {
		ray_sphere* mars = &scene.spheres[update::mars];
		const float marsSpeed = 0.1;
		mars->obj.x = cos(time * marsSpeed + 0.5f) * 10000;
		mars->obj.z = sin(time * marsSpeed + 0.5f) * 10000;
		mars->obj.y = -cos(time * marsSpeed) * 1000;
		mars->quat_rotation *= glm::angleAxis(deltaTime / 5, glm::vec3(0, 1, 0));
	}

	if (update::box != -1)
	{
		ray_box* box = &scene.boxes[update::box];
		box->quat_rotation *= glm::angleAxis(deltaTime, glm::vec3(0.5774, 0.5774, 0.5774));;
	}

	if (update::torus != -1)
	{
		ray_torus* torus = &scene.toruses[update::torus];
		torus->quat_rotation *= glm::angleAxis(deltaTime, glm::vec3(0, 1, 0));
	}
}
static void glfw_error_callback(int error, const char * desc)
{
	fputs(desc, stderr);
}

GLWrapper::GLWrapper(int width, int height, bool fullScreen)
{
	this->width = width;
	this->height = height;
	this->fullScreen = fullScreen;
	this->useCustomResolution = true;
}

GLWrapper::GLWrapper(bool fullScreen)
{
	this->fullScreen = fullScreen;
}

GLWrapper::~GLWrapper()
{
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);

	if (SMAA_enabled)
	{
		glDeleteFramebuffers(1, &fboColor);
		glDeleteTextures(1, &fboTexColor);

		glDeleteFramebuffers(1, &fboEdge);
		glDeleteTextures(1, &fboTexEdge);

		glDeleteFramebuffers(1, &fboBlend);
		glDeleteTextures(1, &fboTexBlend);
	}
	
	glDeleteTextures(1, &skyboxTex);
	glDeleteTextures(textures.size(), textures.data());
}

int GLWrapper::getWidth()
{
	return width;
}

int GLWrapper::getHeight()
{
	return height;
}

GLuint GLWrapper::getProgramId()
{
	return shader.ID;
}

bool GLWrapper::init_window()
{
	if (!glfwInit())
		return false;

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (!useCustomResolution)
	{
		width = mode->width;
		height = mode->height;
	}

	glfwSetErrorCallback(glfw_error_callback);

	window = glfwCreateWindow(width, height, "RayTracing", fullScreen ? monitor : NULL, NULL);
	glfwGetWindowSize(window, &width, &height);

	if (!window) {
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGL()) {
		printf("gladLoadGL failed!\n");
		return false;
	}
	printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);

	float quadVertices[] = 
	{
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,

		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f
	};

	// quad VAO
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);

	// SMAA framebuffers
	if (SMAA_enabled)
	{
		gen_framebuffer(&fboColor, &fboTexColor, GL_RGBA, GL_RGBA);
		gen_framebuffer(&fboEdge, &fboTexEdge, GL_RG, GL_RG);
		gen_framebuffer(&fboBlend, &fboTexBlend, GL_RGBA, GL_RGBA);
	}

	return true;
}

void GLWrapper::set_skybox(unsigned textureId)
{
	skyboxTex = textureId;
	shader.setInt("skybox", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
}

void GLWrapper::stop()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void GLWrapper::enable_SMAA(SMAA_PRESET preset)
{
	SMAA_enabled = true;
	SMAA_preset = preset;
}

void GLWrapper::draw()
{
	shader.use();
	glBindVertexArray(quadVAO);
	if (SMAA_enabled) 
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboColor);
	}
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	checkGlErrors("Draw raytraced image");

	if (!SMAA_enabled)
	{
		return;
	}
	
	edgeShader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fboTexColor);
	glBindFramebuffer(GL_FRAMEBUFFER, fboEdge);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	checkGlErrors("Draw edge");
	
	blendShader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fboTexEdge);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, areaTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, searchTex);
	glBindFramebuffer(GL_FRAMEBUFFER, fboBlend);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	checkGlErrors("Draw blend");

	neighborhoodShader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fboTexColor);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fboTexBlend);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	checkGlErrors("Draw screen");

	shader.use();
}

void GLWrapper::gen_framebuffer(GLuint* fbo, GLuint* fboTex, GLenum internalFormat, GLenum format) const
{
	glGenFramebuffers(1, fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, *fbo);

	glGenTextures(1, fboTex);
	glBindTexture(GL_TEXTURE_2D, *fboTex);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *fboTex, 0);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		exit(1);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLWrapper::init_shaders(ray_definition& defines)
{
	const std::string vertexShaderSrc = readStringFromFile(ASSETS_DIR "/shaders/quad.vert");
	std::string fragmentShaderSrc = readStringFromFile(ASSETS_DIR "/shaders/rt.frag");
	
	replace(fragmentShaderSrc, "{SPHERE_SIZE}", std::to_string(defines.sphere_size));
	replace(fragmentShaderSrc, "{PLANE_SIZE}", std::to_string(defines.plane_size));
	replace(fragmentShaderSrc, "{SURFACE_SIZE}", std::to_string(defines.surface_size));
	replace(fragmentShaderSrc, "{BOX_SIZE}", std::to_string(defines.box_size));
	replace(fragmentShaderSrc, "{TORUS_SIZE}", std::to_string(defines.torus_size));
	replace(fragmentShaderSrc, "{RING_SIZE}", std::to_string(defines.ring_size));
	replace(fragmentShaderSrc, "{LIGHT_POINT_SIZE}", std::to_string(defines.light_point_size));
	replace(fragmentShaderSrc, "{LIGHT_DIRECT_SIZE}", std::to_string(defines.light_direct_size));
	replace(fragmentShaderSrc, "{ITERATIONS}", std::to_string(defines.iterations));
	replace(fragmentShaderSrc, "{AMBIENT_COLOR}", to_string(defines.ambient_color));
	replace(fragmentShaderSrc, "{SHADOW_AMBIENT}", to_string(defines.shadow_ambient));

	shader.initFromSrc(vertexShaderSrc.c_str(), fragmentShaderSrc.c_str());

	if (SMAA_enabled)
	{
		SMAA_Builder smaaBuilder(width, height, SMAA_preset);
		smaaBuilder.init_edge_shader(edgeShader);
		smaaBuilder.init_blend_shader(blendShader);
		smaaBuilder.init_neighborhood_shader(neighborhoodShader);

		edgeShader.use();
		edgeShader.setInt("color_tex", 0);

		blendShader.use();
		blendShader.setInt("edge_tex", 0);
		blendShader.setInt("area_tex", 1);
		blendShader.setInt("search_tex", 2);

		neighborhoodShader.use();
		neighborhoodShader.setInt("color_tex", 0);
		neighborhoodShader.setInt("blend_tex", 1);

		areaTex = smaaBuilder.load_area_texture();
		searchTex = smaaBuilder.load_search_texture();
	}

	shader.use();

	checkGlErrors("Shader creation");
}

std::string GLWrapper::to_string(glm::vec3 v)
{
	return std::string().append("vec3(").append(std::to_string(v.x)).append(",").append(std::to_string(v.y)).append(",").append(std::to_string(v.z)).append(")");
}

unsigned int GLWrapper::load_cubemap(std::vector<std::string> faces, bool genMipmap)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	if (genMipmap) {
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, genMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

unsigned int GLWrapper::load_texture(char const* path, GLuint wrapMode)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

GLuint GLWrapper::load_texture(int texNum, const char* name, const char* uniformName, GLuint wrapMode)
{
	const std::string path = ASSETS_DIR "/textures/" + std::string(name);
	const unsigned int tex = load_texture(path.c_str(), wrapMode);
	shader.setInt(uniformName, texNum);
	textures.push_back(tex);
	return tex;
}

void GLWrapper::init_buffer(GLuint* ubo, const char* name, int bindingPoint, size_t size, void* data) const
{
	glGenBuffers(1, ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, *ubo);
	glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
	GLuint blockIndex = glGetUniformBlockIndex(shader.ID, name);
	if (blockIndex == 0xffffffff)
	{
		fprintf(stderr, "Invalid ubo block name '%s'", name);
		exit(1);
	}
	glUniformBlockBinding(shader.ID, blockIndex, bindingPoint);
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, *ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GLWrapper::update_buffer(GLuint ubo, size_t size, void* data)
{
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
#define PI_F 3.14159265358979f

SceneManager::SceneManager(int wind_width, int wind_height, scene_container* scene, GLWrapper* wrapper)
{
	this->wind_width = wind_width;
	this->wind_height = wind_height;
	this->wrapper = wrapper;
	this->scene = scene;
	this->position = scene->scene.camera_pos;
}

void SceneManager::init()
{
	glfwSetWindowUserPointer(wrapper->window, this);

	auto mouseFunc = [](GLFWwindow* w, double x, double y)
	{
		static_cast<SceneManager*>(glfwGetWindowUserPointer(w))->glfw_mouse_callback(w, x, y);
	};
	auto keyFunc = [](GLFWwindow* w, int a, int b, int c, int d)
	{
		static_cast<SceneManager*>(glfwGetWindowUserPointer(w))->glfw_key_callback(w, a, b, c, d);
	};

	glfwSetInputMode(wrapper->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(wrapper->window, mouseFunc);
	glfwSetKeyCallback(wrapper->window, keyFunc);
	glfwSetFramebufferSizeCallback(wrapper->window, glfw_framebuffer_size_callback);

	init_buffers();
}

void SceneManager::update(float deltaTime)
{
	update_scene(deltaTime);
	update_buffers();
}

void SceneManager::update_scene(float deltaTime)
{
	front.x = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);
	right = glm::normalize(glm::cross(-front, world_up));
	scene->scene.quat_camera_rotation = glm::quat(glm::vec3(glm::radians(-pitch), glm::radians(yaw), 0));

	auto speed = deltaTime * 3;
	if (shift_pressed)
		speed *= 3;
	if (alt_pressed)
		speed /= 6;

	const glm::vec3 aq = front * speed * 1e3f;

	if (w_pressed)
		position += front * speed;
	if (a_pressed)
		position -= right * speed;
	if (s_pressed)
		position -= front * speed;
	if (d_pressed)
		position += right * speed;
	if (space_pressed)
		position += world_up * speed;
	if (ctrl_pressed)
		position -= world_up * speed;

	scene->scene.camera_pos = position;
}

void SceneManager::glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_RELEASE) {
		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, GL_TRUE);

		bool pressed = action == GLFW_PRESS;

		if (key == GLFW_KEY_W)
			w_pressed = pressed;
		else if (key == GLFW_KEY_S)
			s_pressed = pressed;
		else if (key == GLFW_KEY_A)
			a_pressed = pressed;
		else if (key == GLFW_KEY_D)
			d_pressed = pressed;
		else if (key == GLFW_KEY_SPACE)
			space_pressed = pressed;
		else if (key == GLFW_KEY_LEFT_CONTROL)
			ctrl_pressed = pressed;
		else if (key == GLFW_KEY_LEFT_SHIFT)
			shift_pressed = pressed;
		else if (key == GLFW_KEY_LEFT_ALT)
			alt_pressed = pressed;
	}
}

void SceneManager::glfw_framebuffer_size_callback(GLFWwindow* wind, int width, int height)
{
	glViewport(0, 0, width, height);
}

bool firstMouse = true;

void SceneManager::glfw_mouse_callback(GLFWwindow* window, double xpos, double ypos) {

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
}

ray_material SceneManager::create_material(glm::vec3 color, int specular, float reflect, float refract, glm::vec3 absorb, float diffuse, float kd, float ks)
{
	ray_material material = {};

	material.color = color;
	material.absorb = absorb;
	material.specular = specular;
	material.reflect = reflect;
	material.refract = refract;
	material.diffuse = diffuse;
	material.kd = kd;
	material.ks = ks;

	return material;
}

ray_sphere SceneManager::create_sphere(glm::vec3 center, float radius, ray_material material, bool hollow)
{
	ray_sphere sphere = {};
	sphere.obj = glm::vec4(center, radius);
	sphere.hollow = hollow;
	sphere.material = material;

	return sphere;
}

ray_plane SceneManager::create_plane(glm::vec3 normal, glm::vec3 pos, ray_material material)
{
	ray_plane plane = {};
	plane.normal = normal;
	plane.pos = pos;
	plane.material = material;
	return plane;
}

ray_box SceneManager::create_box(glm::vec3 pos, glm::vec3 form, ray_material material)
{
	ray_box box = {};
	box.form = form;
	box.pos = pos;
	box.mat = material;
	return box;
}

ray_torus SceneManager::create_torus(glm::vec3 pos, glm::vec2 form, ray_material material)
{
	ray_torus torus = {};
	torus.form = form;
	torus.pos = pos;
	torus.mat = material;
	return torus;
}

ray_ring SceneManager::create_ring(glm::vec3 pos, float r1, float r2, ray_material material)
{
	ray_ring ring = {};
	ring.pos = pos;
	ring.mat = material;
	ring.r1 = r1 * r1;
	ring.r2 = r2 * r2;
	return ring;
}

ray_lignt_point SceneManager::create_light_point(glm::vec4 position, glm::vec3 color, float intensity, float linear_k,
	float quadratic_k)
{
	ray_lignt_point light = {};

	light.intensity = intensity;
	light.pos = position;
	light.color = color;
	light.linear_k = linear_k;
	light.quadratic_k = quadratic_k;

	return light;
}

ray_lignt_direct SceneManager::create_light_direct(glm::vec3 direction, glm::vec3 color, float intensity)
{
	ray_lignt_direct light = {};

	light.intensity = intensity;
	light.direction = direction;
	light.color = color;

	return light;
}

ray_scene SceneManager::create_scene(int width, int height)
{
	ray_scene scene = {};

	scene.camera_pos = {};
	scene.canvas_height = height;
	scene.canvas_width = width;
	scene.bg_color = { 0,0.0,0.0 };
	scene.reflect_depth = 5;

	return scene;
}

template<typename T>
void SceneManager::init_buffer(GLuint* ubo, const char* name, int bindingPoint, std::vector<T>& v)
{
	wrapper->init_buffer(ubo, name, bindingPoint, sizeof(T) * v.size(), v.data());
}

void SceneManager::init_buffers()
{
	wrapper->init_buffer(&sceneUbo, "scene_buf", 0, sizeof(ray_scene), nullptr);
	init_buffer(&sphereUbo, "spheres_buf", 1, scene->spheres);
	init_buffer(&planeUbo, "planes_buf", 2, scene->planes);
	init_buffer(&surfaceUbo, "surfaces_buf", 3, scene->surfaces);
	init_buffer(&boxUbo, "boxes_buf", 4, scene->boxes);
	init_buffer(&torusUbo, "toruses_buf", 5, scene->toruses);
	init_buffer(&ringUbo, "rings_buf", 6, scene->rings);
	init_buffer(&lightPointUbo, "lights_point_buf", 7, scene->lights_point);
	init_buffer(&lightDirectUbo, "lights_direct_buf", 8, scene->lights_direct);
}

template<typename T>
void SceneManager::update_buffer(GLuint ubo, std::vector<T>& v) const
{
	if (!v.empty())
	{
		wrapper->update_buffer(ubo, sizeof(T) * v.size(), v.data());
	}
}

void SceneManager::update_buffers() const
{
	wrapper->update_buffer(sceneUbo, sizeof(ray_scene), &scene->scene);
	update_buffer(sphereUbo, scene->spheres);
	update_buffer(planeUbo, scene->planes);
	update_buffer(surfaceUbo, scene->surfaces);
	update_buffer(boxUbo, scene->boxes);
	update_buffer(torusUbo, scene->toruses);
	update_buffer(ringUbo, scene->rings);
	update_buffer(lightPointUbo, scene->lights_point);
}
glm::vec3 SceneManager::get_color(float r, float g, float b)
{
	return glm::vec3(r / 255, g / 255, b / 255);
}