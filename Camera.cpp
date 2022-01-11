#include "Camera.h"

Camera::Camera()
{
	// radius = 2.0;
	// rotateAngle = 3.0;
	// upAngle = 0.0;
	up = glm::vec3(0.0, 1.0, 0.0);
	at = glm::vec3(0.0, 0.0, 0.0);
	eye = glm::vec3(0.0, 0.0, 0.0);
	updateCameraByAngle();
};
Camera::~Camera() {}

glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(eye, at, up);
}

glm::mat4 Camera::getProjectionMatrix(bool isOrtho)
{
	if (isOrtho)
	{
		return this->ortho(-scale, scale, -scale, scale, this->near, this->far);
	}
	else
	{
		return this->perspective(fov, aspect, this->near, this->far);
	}
}

glm::mat4 Camera::ortho(const GLfloat left, const GLfloat right,
						const GLfloat bottom, const GLfloat top,
						const GLfloat zNear, const GLfloat zFar)
{
	glm::mat4 c = glm::mat4(1.0f);
	c[0][0] = 2.0 / (right - left);
	c[1][1] = 2.0 / (top - bottom);
	c[2][2] = -2.0 / (zFar - zNear);
	c[3][3] = 1.0;
	c[0][3] = -(right + left) / (right - left);
	c[1][3] = -(top + bottom) / (top - bottom);
	c[2][3] = -(zFar + zNear) / (zFar - zNear);

	c = glm::transpose(c);
	return c;
}

glm::mat4 Camera::perspective(const GLfloat fovy, const GLfloat aspect,
							  const GLfloat zNear, const GLfloat zFar)
{
	GLfloat top = tan(fovy * M_PI / 180 / 2) * zNear;
	GLfloat right = top * aspect;

	glm::mat4 c = glm::mat4(1.0f);
	c[0][0] = zNear / right;
	c[1][1] = zNear / top;
	c[2][2] = -(zFar + zNear) / (zFar - zNear);
	c[2][3] = -(2.0 * zFar * zNear) / (zFar - zNear);
	c[3][2] = -1.0;
	c[3][3] = 0.0;

	c = glm::transpose(c);
	return c;
}

glm::mat4 Camera::frustum(const GLfloat left, const GLfloat right,
						  const GLfloat bottom, const GLfloat top,
						  const GLfloat zNear, const GLfloat zFar)
{
	// 任意视锥体矩阵
	glm::mat4 c = glm::mat4(1.0f);
	c[0][0] = 2.0 * zNear / (right - left);
	c[0][2] = (right + left) / (right - left);
	c[1][1] = 2.0 * zNear / (top - bottom);
	c[1][2] = (top + bottom) / (top - bottom);
	c[2][2] = -(zFar + zNear) / (zFar - zNear);
	c[2][3] = -2.0 * zFar * zNear / (zFar - zNear);
	c[3][2] = -1.0;
	c[3][3] = 0.0;

	c = glm::transpose(c);
	return c;
}

void Camera::updateCameraByAngle()
{
	// 自由模式
	if (isFreeMode)
	{
		front.x = radius * cos(upAngle * M_PI / 180.0) * sin(rotateAngle * M_PI / 180.0);
		front.y = radius * sin(upAngle * M_PI / 180.0);
		front.z = radius * cos(upAngle * M_PI / 180.0) * cos(rotateAngle * M_PI / 180.0);
		at = eye + front;
	}
	// 第三人称模式
	else
	{
		eye.x = radius * cos(upAngle * M_PI / 180.0) * sin(rotateAngle * M_PI / 180.0) + at.x;
		eye.y = radius * sin(upAngle * M_PI / 180.0) + at.y;
		eye.z = radius * cos(upAngle * M_PI / 180.0) * cos(rotateAngle * M_PI / 180.0) + at.z;

		up = glm::vec3(0.0, 1.0, 0.0);
		if (upAngle > 90)
			up.y = -1;
		else if (upAngle < -90)
			up.y = -1;
		front = glm::normalize(at - eye);
	}
}

void Camera::updateMovingStatus()
{
	// 键盘事件处理
	// 通过按键改变相机和投影的参数
	float cameraSpeed = 0.005f;
	if (isFreeMode)
	{
		if (w)
			eye += cameraSpeed * front;
		if (s)
			eye -= cameraSpeed * front;
		if (a)
			eye -= glm::normalize(glm::cross(front, up)) * cameraSpeed;
		if (d)
			eye += glm::normalize(glm::cross(front, up)) * cameraSpeed;
		if (space)
			eye += up * cameraSpeed;
		if (shift)
			eye -= up * cameraSpeed;
	}
}

void Camera::keyboard(int key, int action, int mode)
{
	// 键盘事件处理
	// 通过按键改变相机和投影的参数
	float cameraSpeed = 0.05f;
	if (isFreeMode)
	{
		if (key == GLFW_KEY_W)
		{
			if (action == GLFW_PRESS)
				w = true;
			else if (action == GLFW_RELEASE)
				w = false;
		}
		else if (key == GLFW_KEY_S)
		{
			if (action == GLFW_PRESS)
				s = true;
			else if (action == GLFW_RELEASE)
				s = false;
		}
		else if (key == GLFW_KEY_A)
		{
			if (action == GLFW_PRESS)
				a = true;
			else if (action == GLFW_RELEASE)
				a = false;
		}
		else if (key == GLFW_KEY_D)
		{
			if (action == GLFW_PRESS)
				d = true;
			else if (action == GLFW_RELEASE)
				d = false;
		}
		else if (key == GLFW_KEY_SPACE)
		{
			if (action == GLFW_PRESS)
				space = true;
			else if (action == GLFW_RELEASE)
				space = false;
		}
		else if (key == GLFW_KEY_LEFT_SHIFT)
		{
			if (action == GLFW_PRESS)
				shift = true;
			else if (action == GLFW_RELEASE)
				shift = false;
		}
	}
}
