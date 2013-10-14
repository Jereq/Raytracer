#include "GLWindow.h"

#include <sstream>
#include <string>

void GLWindow::initOpenGL(const std::string& _title)
{
	if (!glfwInit())
	{
		throw std::exception("Failed to initialize GLFW.");
	}

	window = glfwCreateWindow(width, height, _title.c_str(), nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		throw std::exception("Failed to create window.");
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err)
	{
		glfwTerminate();

		std::ostringstream oss;
		oss << "Failed to initialize GLEW: (" << err << ") " << glewGetErrorString(err) << std::endl;
		throw std::exception(oss.str().c_str());
	}

	if (!shader.compileShaderFromFile("simple.vert", GLSLShaderType::VERTEX)
		|| !shader.compileShaderFromFile("simple.frag", GLSLShaderType::FRAGMENT)
		|| !shader.link())
	{
		std::cout << shader.log();
	}

	const static glm::vec2 rectPos[] = 
	{
		glm::vec2(-1.f, -1.f),
		glm::vec2(-1.f,  1.f),
		glm::vec2( 1.f,  1.f),
		glm::vec2(-1.f, -1.f),
		glm::vec2( 1.f, -1.f),
		glm::vec2( 1.f,  1.f)
	};

	glGenBuffers(1, &rectVBO);
	glGenVertexArrays(1, &rectVAO);

	glBindVertexArray(rectVAO);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);

	glBufferData(GL_ARRAY_BUFFER, sizeof(rectPos), rectPos, GL_STATIC_DRAW);
}

GLWindow::GLWindow(const std::string& _title, int _width, int _height)
	: window(nullptr),
	  width(_width),
	  height(_height),
	  framebuffer(0),
	  framebufferTexture(0),
	  framebufferWidth(0),
	  framebufferHeight(0)
{
	initOpenGL(_title);
}

GLWindow::~GLWindow()
{
	glDeleteBuffers(1, &rectVBO);
	glDeleteVertexArrays(1, &rectVAO);
	destroyFramebuffer();
	glfwTerminate();
}

bool GLWindow::createFramebuffer(GLuint _framebufferWidth, GLuint _framebufferHeight)
{
	framebufferWidth = _framebufferWidth;
	framebufferHeight = _framebufferHeight;

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenTextures(1, &framebufferTexture);
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_FLOAT, nullptr);

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
	
	shader.use();
	shader.setUniform("tex", 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void GLWindow::destroyFramebuffer()
{
	if (framebuffer)
	{
		glDeleteFramebuffers(1, &framebuffer);
		framebuffer = 0;
	}

	if (framebufferTexture)
	{
		glDeleteTextures(1, &framebufferTexture);
		framebufferTexture = 0;
	}
}

void GLWindow::blitFramebuffer()
{
	glm::vec4 data(1.f, 1.f, 1.f, 1.f);
	GLuint t;
	glGenTextures(1, &t);
	glBindTexture(GL_TEXTURE_2D, t);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, &data[0]);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shader.use();
	shader.setUniform("tex", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, t);

	glBindVertexArray(rectVAO);

	glDrawArrays(GL_TRIANGLES, 0, 2);

	//glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	//glBlitFramebuffer(0, 0, framebufferWidth, framebufferHeight, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLWindow::clearFramebuffer(float _red, float _green, float _blue)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glClearColor(_red, _green, _blue, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLWindow::clearBackbuffer(float _red, float _green, float _blue)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(_red, _green, _blue, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void GLWindow::drawFramebuffer()
{
	//glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

	clearBackbuffer(1.f, 0.f, 1.f);
	blitFramebuffer();

	glfwSwapBuffers(window);
	glfwPollEvents();
}

GLuint GLWindow::getFramebufferTexture() const
{
	return framebufferTexture;
}

GLuint GLWindow::getFramebufferWidth() const
{
	return framebufferWidth;
}

GLuint GLWindow::getFramebufferHeight() const
{
	return framebufferHeight;
}

void GLWindow::setKeyCallback(GLFWkeyfun _func)
{
	glfwSetKeyCallback(window, _func);
}

void GLWindow::setMouseCallback(GLFWcursorposfun _func)
{
	glfwSetCursorPosCallback(window, _func);
}

void GLWindow::setTitle(const std::string& _title)
{
	glfwSetWindowTitle(window, _title.c_str());
}

bool GLWindow::shouldClose() const
{
	return glfwWindowShouldClose(window) || (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS);
}
