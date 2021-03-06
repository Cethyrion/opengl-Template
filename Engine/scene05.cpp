#include "stdafx.h"
#include "renderer.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "scene05.h"
#include "timer.h"
#include "image.h"
#include "light.h"
#include "input.h"
#include "meshLoader.h"

namespace
{
	const float PI = 3.14159265358f;
}

Scene05::Scene05(Engine* engine) : Scene(engine)
{
}


Scene05::~Scene05()
{
}

//#define SPECULAR
//#define MULTITEXTURE

bool Scene05::Initialize()
{

#if defined (SPECULAR)

	m_shader.CompileShader("..\\Resources\\Shaders\\texture_phong_specular.vs", GL_VERTEX_SHADER);
	m_shader.CompileShader("..\\Resources\\Shaders\\texture_phong_specular.fs", GL_FRAGMENT_SHADER);

#elif defined (MULTITEXTURE)

	m_shader.CompileShader("..\\Resources\\Shaders\\multi_texture_phong.vs", GL_VERTEX_SHADER);
	m_shader.CompileShader("..\\Resources\\Shaders\\multi_texture_phong.fs", GL_FRAGMENT_SHADER);


#else

	m_shader.CompileShader("..\\Resources\\Shaders\\texture_phong.vs", GL_VERTEX_SHADER);
	m_shader.CompileShader("..\\Resources\\Shaders\\texture_phong.fs", GL_FRAGMENT_SHADER);

#endif // SPECULAR

	m_shader.Link();
	m_shader.Use();

	MeshLoader::LoadMesh("..\\Resources\\Meshes\\puma.obj", m_vertices, m_normals, m_uvs);

	m_numVertices = m_vertices.size();

	glGenBuffers(3, m_vboHandle);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[0]);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), m_vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[1]);
	glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3), m_normals.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboHandle[2]);
	glBufferData(GL_ARRAY_BUFFER, m_uvs.size() * sizeof(glm::vec2), m_uvs.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &m_vaoHandle);
	glBindVertexArray(m_vaoHandle);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindVertexBuffer(0, m_vboHandle[0], 0, sizeof(glm::vec3));
	glBindVertexBuffer(1, m_vboHandle[1], 0, sizeof(glm::vec3));
	glBindVertexBuffer(2, m_vboHandle[2], 0, sizeof(glm::vec2));
	glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(0, 0);
	glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(1, 1);
	glVertexAttribFormat(2, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(2, 2);

	m_material.SetMaterial(glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), 10.0f);
	m_material.LoadTexture2D("..\\Resources\\Textures\\MetalTexture.bmp", /*"crate",*/ GL_TEXTURE0);
	m_material.LoadTexture2D("..\\Resources\\Textures\\MetalSpecular.bmp", /*"crate_specular",*/ GL_TEXTURE1);
	m_material.LoadTexture2D("..\\Resources\\Textures\\grass.bmp", /*"crate_specular",*/ GL_TEXTURE2);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	m_engine->Get<Input>()->AddButton("application quit", Input::eButtonType::KEYBOARD, GLFW_KEY_ESCAPE);

	Camera* camera = new Camera("camera", this);
	camera->Initialize(glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

	AddObject(camera);

	Light* light = new Light("light", this);

	light->m_ambient = glm::vec3(0.75f, 0.85f, 1.0f);
	light->m_diffuse = glm::vec3(0.75f, 0.85f, 1.0f);
	light->m_specular = glm::vec3(0.75f, 0.85f, 1.0f);

	Transform lightPosition;

	lightPosition.position = glm::vec3(-10.0f, 10.0f, 10.0f);
	light->m_transform = lightPosition;

	AddObject(light);

	return 1;
}

void Scene05::Update()
{
	if (m_engine->Get<Input>()->GetButton("application quit") == Input::eButtonState::DOWN)
	{
		glfwSetWindowShouldClose(m_engine->Get<Renderer>()->m_window, GLFW_TRUE);
	}

	//auto objects = GetObjects<Object>();
	for (auto object : m_objects)
	{
		object->Update();
	}

	m_shader.SetUniform("material.ambient", m_material.m_ambient);
	m_shader.SetUniform("material.diffuse", m_material.m_diffuse);
	m_shader.SetUniform("material.specular", m_material.m_specular);
	m_shader.SetUniform("material.shininess", m_material.m_shininess);

	glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

	m_rotation = m_rotation + m_engine->Get<Timer>()->FrameTime() / 8;
	glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), m_rotation, glm::vec3(1.0f, 0.0f, 1.0f));
	glm::mat4 mxModel = translate * rotate;

	Camera* camera = GetObject<Camera>("camera");

	glm::mat4 mxView = camera->GetView();
	glm::mat4 mxProjection = camera->GetProjection();

	glm::mat4 mxModelView = mxView * mxModel;
	m_shader.SetUniform("mxModelView", mxModelView);

	glm::mat4 mxMVP = mxProjection * mxView * mxModel;
	m_shader.SetUniform("mxMVP", mxMVP);

	glm::mat3 mxNormal = glm::mat3(mxModelView);
	mxNormal = glm::inverse(mxNormal);
	mxNormal = glm::transpose(mxNormal);
	m_shader.SetUniform("mxNormal", mxNormal);

	Light* light = GetObject<Light>("light");

	m_shader.SetUniform("light.ambient", light->m_ambient);
	m_shader.SetUniform("light.diffuse", light->m_diffuse);
	m_shader.SetUniform("light.specular", light->m_specular);

	glm::vec3 lightPosition = mxView * glm::vec4(light->m_transform.position, 1.0f);
	m_shader.SetUniform("light.position", lightPosition);

}

void Scene05::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	std::vector<Renderable*> renderables = GetObjects<Renderable>();
	for (auto renderable : renderables)
	{
		renderable->Render();
	}

	glfwSwapBuffers(m_engine->Get<Renderer>()->m_window);
}

void Scene05::Shutdown()
{
}