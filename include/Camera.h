#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Angel.h"

class Camera
{
public:
	Camera();
	~Camera();

	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix(bool isOrtho);

	glm::mat4 ortho(const GLfloat left, const GLfloat right,
					const GLfloat bottom, const GLfloat top,
					const GLfloat zNear, const GLfloat zFar);

	glm::mat4 perspective(const GLfloat fovy, const GLfloat aspect,
						  const GLfloat zNear, const GLfloat zFar);

	glm::mat4 frustum(const GLfloat left, const GLfloat right,
					  const GLfloat bottom, const GLfloat top,
					  const GLfloat zNear, const GLfloat zFar);

	void updateMovingStatus();

	// 每次更改相机参数后更新一下相关的数值
	void updateCameraByAngle();
	// 处理相机的键盘操作
	void keyboard(int key, int action, int mode);

	// 模视矩阵
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;

	// 第三人称模式下camera围绕中心旋转
	glm::vec3 at;

	// 初始参数
	float initRotationAngle = 210;
	float initUpAngle = 0;

	// 相机位置参数
	float radius = 2.0;
	float rotateAngle = initRotationAngle;
	float upAngle = initRotationAngle;
	glm::vec3 eye;
	glm::vec3 front;
	glm::vec3 up;

// 投影参数
#undef near
#undef far
	float near = 0.1;
	float far = 100.0;
	// 透视投影参数
	float fov = 45.0;
	float aspect = 1.0;
	// 正交投影参数
	float scale = 1.5;

	// 自由视角模式
	bool isFreeMode = false;

	// 自由视角时的按键按下
	bool w = false, a = false, s = false, d = false, space = false, shift = false;
};
#endif