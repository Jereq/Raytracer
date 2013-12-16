#pragma once

#include <GL/glew.h>
#include <GL/wglew.h>

#include <GLFW/glfw3.h>

#include <iostream>

class GLWindow
{
private:
	GLFWwindow* window;
	int width;
	int height;

	GLuint framebuffer;
	GLuint renderbuffer;

	void initOpenGL(const std::string& _title);

public:
	GLWindow(const std::string& _title, int _width, int _height);
	~GLWindow();

	bool createFramebuffer(GLuint _framebufferWidth, GLuint _framebufferHeight);
	void updateFramebuffer(GLuint _framebufferWidth, GLuint _framebufferHeight);
	void destroyFramebuffer();
	void blitFramebuffer();
	void clearFramebuffer(float _red, float _green, float _blue);
	void clearBackbuffer(float _red,  float _green, float _blue);
	void drawFramebuffer();
	GLuint getRenderbuffer() const;
	GLuint getFramebufferWidth() const;
	GLuint getFramebufferHeight() const;

	void setKeyCallback(GLFWkeyfun _func);
	void setMouseCallback(GLFWcursorposfun _func);
	void setFramebufferSizeCallback(GLFWframebuffersizefun _func);

	void setTitle(const std::string& _title);

	bool shouldClose() const;
};
