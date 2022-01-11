#include "Angel.h"
#include "TriMesh.h"
#include "Camera.h"
#include "MeshPainter.h"

#include <vector>
#include <string>
#include <time.h>
#include <chrono>

bool isFreeMode = false;	// 自由视角
bool cameraToBack = true;	// 后面视角
bool cameraToFront = false; // 前面视角
MeshPainter *painter = new MeshPainter();
void mouse_callback(GLFWwindow *window, double xpos, double ypos);

// 获取毫秒级时间戳
int64_t currentTimestamp()
{
	int64_t timems = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	return timems;
}

// 人物
class People
{
private:
	// 各部位的物体对象
	TriMesh *torso = new TriMesh();
	TriMesh *head = new TriMesh();
	// 调整旋转轴
	TriMesh *rightHand = new TriMesh(true);
	TriMesh *leftHand = new TriMesh(true);
	TriMesh *rightLeg = new TriMesh(true);
	TriMesh *leftLeg = new TriMesh(true);
	// 小旗子的杆和三角形
	TriMesh *rod = new TriMesh();
	TriMesh *triangle = new TriMesh();

	// mesh列表
	TriMesh *meshList[8];

	// 自由视角模式
	bool isFreeMode = false;

	// mesh名
	std::string names[8]{
		"torso",
		"head",
		"rightHand",
		"leftHand",
		"rightLeg",
		"leftLeg",
		"rod",
		"triangle"};

	MeshPainter *peoplePainter = new MeshPainter();

public:
	// 身体部位枚举
	enum
	{
		Torso,	   // 躯干
		Head,	   // 头部
		RightHand, // 右手
		LeftHand,  // 左手
		RightLeg,  //右腿
		LeftLeg,   //左腿
		Rod,	   //杆
		Triangle,  // 三角形
	};

	// 各部位旋转
	glm::vec3 BodyTheta[8] = {
		glm::vec3(0, 0, 0),	  // Torso
		glm::vec3(0, 0, 0),	  // Head
		glm::vec3(0, 0, 0),	  // RightHand
		glm::vec3(0, 0, 0),	  // LeftHand
		glm::vec3(0, 0, 0),	  // RightLeg
		glm::vec3(0, 0, 0),	  // LeftLeg
		glm::vec3(0, 25, 40), // rod
		glm::vec3(0, 0, 0),	  // triangle
	};

	// 人物初始坐标
	const glm::vec3 initPosition = glm::vec3(0, 0.258, 0);

	// 各部位相对位移
	glm::vec3 BodyTranslate[8] = {
		initPosition,				   // Torso
		glm::vec3(0, 0, 0),			   // Head
		glm::vec3(0, 0, 0),			   // RightHand
		glm::vec3(0, 0, 0),			   // LeftHand
		glm::vec3(0, 0, 0),			   // RightLeg
		glm::vec3(0, 0, 0),			   // LeftLeg
		glm::vec3(-0.03, -0.02, 0.45), // rod
		glm::vec3(0, 0, 0.47)		   // triangle
	};

	// 人物位置（实际上是身体中心的位置）
	glm::vec3 &position = BodyTranslate[Torso];

	// 运行状态
	bool isRunning = false;					// 正在运行
	bool isStopping = false;				// 运行结束了，但是手脚还没有回到原位，正在停止中
	bool leftHandRightLegIsToFront = false; // 左手右脚正在向前摆，右手和左脚运动方向与之相反
	int theta_range = 60;					// theta前后摆动幅度范围
	bool w = false;
	bool s = false;
	bool a = false;
	bool d = false;
	bool fastRun = false; // 双击w快速跑

	// 跳
	const float g = 0.04;		// 重力加速度
	float jumpInitSpeed = 0.01; // 跳的初始速度
	bool isOnTheSky = false;	// 在天上
	int64_t startTime = 0;		// 开始跳的时间戳，毫秒级

	// 行走速度与摆臂速度
	float walk_speed = 0.002f;
	float rotate_speed = 0.3f;

	People(std::string objFilePath, std::string textureFilePath)
	{
		// 装到列表中
		meshList[Torso] = torso;
		meshList[Head] = head;
		meshList[RightHand] = rightHand;
		meshList[LeftHand] = leftHand;
		meshList[RightLeg] = rightLeg;
		meshList[LeftLeg] = leftLeg;
		meshList[Rod] = rod;
		meshList[Triangle] = triangle;

		// 读取着色器并使用
		std::string vshader, fshader;
		vshader = "shaders/vshader.glsl";
		fshader = "shaders/fshader.glsl";

		// 各个部位初始化
		for (int i = 0; i < 6; i++)
		{
			// 正则化
			meshList[i]->setNormalize(false);
			// 读各自部分obj
			meshList[i]->readObj(objFilePath, names[i]);
			// 设置初始的旋转平移缩放
			meshList[i]->setTranslation(BodyTranslate[i]);
			meshList[i]->setRotation(glm::vec3(0, 0, 0));
			meshList[i]->setScale(glm::vec3(1, 1, 1));
			// 添加到painter中
			peoplePainter->addMesh(meshList[i], names[i], textureFilePath, vshader, fshader);
			//材质
			// meshList[i]->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0));  // 环境光
			// meshList[i]->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0));  // 漫反射
			// meshList[i]->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
			// meshList[i]->setShininess(1.0);						  //高光系数
		}

		// rod与triangle初始化
		float rodHeight = 0.35, radius = 0.1;
		rod->generateCylinder(100, radius, rodHeight);
		rod->setNormalize(false);
		rod->setScale(glm::vec3(radius, radius, rodHeight));
		peoplePainter->addMesh(rod, "rod", "./assets/stone.png", vshader, fshader);

		triangle->generateTriangle(glm::vec3(1, 0, 0));
		triangle->setNormalize(false);
		triangle->setScale(glm::vec3(3, 3, 3));
		peoplePainter->addMesh(triangle, "triangle", "./assets/iron.png", vshader, fshader);
	}

	void updateJumpStatus()
	{
		if (isOnTheSky)
		{
			// 竖直上抛 vt - 1/2gt^2
			int64_t currentTime = currentTimestamp();
			float t = (currentTime - startTime) / 1000.0;
			position.y += jumpInitSpeed * t - 0.5 * g * t * t;

			if (position.y < initPosition.y)
			{
				position.y = initPosition.y;
				isOnTheSky = false;
				startTime = 0;
			}
		}
	}

	// 更新运动状态
	// 当前行走方向与x轴的夹角弧度制: currentDirection
	void updateMovingStatus(float currentDirection)
	{
		// 正在运动中
		if (w || s || a || d)
		{
			isRunning = true;
			float cos_d = glm::cos(currentDirection), sin_d = glm::sin(currentDirection);

			// 如果存在侧面走动，则不旋转旋转轴，让手脚侧着动
			if (a || d)
			{
				rightHand->needRotate = false;
				leftHand->needRotate = false;
				rightLeg->needRotate = false;
				leftLeg->needRotate = false;
			}
			else
			{
				rightHand->needRotate = true;
				leftHand->needRotate = true;
				rightLeg->needRotate = true;
				leftLeg->needRotate = true;
			}

			// 人物位置移动：前后左右
			if (w)
			{
				position.x += walk_speed * cos_d;
				position.z -= walk_speed * sin_d;
			}
			if (s)
			{
				position.x -= walk_speed * cos_d;
				position.z += walk_speed * sin_d;
			}
			if (d)
			{
				position.x += walk_speed * sin_d;
				position.z += walk_speed * cos_d;
			}
			if (a)
			{
				position.x -= walk_speed * sin_d;
				position.z -= walk_speed * cos_d;
			}

			// 手脚运动
			// 左手和右脚在前，右手和左脚在后
			if (leftHandRightLegIsToFront)
			{
				// 左手和右脚往前旋转
				BodyTheta[LeftHand].x -= rotate_speed;
				BodyTheta[RightLeg].x -= rotate_speed;
				// 达到一定幅度就反向
				if (BodyTheta[LeftHand].x < -theta_range || BodyTheta[RightLeg].x < -theta_range)
				{
					leftHandRightLegIsToFront = false;
					BodyTheta[LeftHand].x += 2 * rotate_speed;
					BodyTheta[RightLeg].x += 2 * rotate_speed;
				}

				// 右手和左脚往后旋转
				BodyTheta[RightHand].x += rotate_speed;
				BodyTheta[LeftLeg].x += rotate_speed;
				// 达到一定幅度就反向
				if (BodyTheta[RightHand].x > theta_range || BodyTheta[LeftLeg].x > theta_range)
				{
					leftHandRightLegIsToFront = false;
					BodyTheta[RightHand].x -= 2 * rotate_speed;
					BodyTheta[LeftLeg].x -= 2 * rotate_speed;
				}
			}
			// 左手和右脚在后，右手和左脚在前
			else
			{
				BodyTheta[LeftHand].x += rotate_speed;
				BodyTheta[RightLeg].x += rotate_speed;
				// 达到一定幅度就反向
				if (BodyTheta[LeftHand].x > theta_range || BodyTheta[RightLeg].x > theta_range)
				{
					leftHandRightLegIsToFront = true;
					BodyTheta[LeftHand].x -= 2 * rotate_speed;
					BodyTheta[RightLeg].x -= 2 * rotate_speed;
				}

				// 右手和左脚往后旋转
				BodyTheta[RightHand].x -= rotate_speed;
				BodyTheta[LeftLeg].x -= rotate_speed;
				// 达到一定幅度就反向
				if (BodyTheta[RightHand].x < -theta_range || BodyTheta[LeftLeg].x < -theta_range)
				{
					leftHandRightLegIsToFront = true;
					BodyTheta[RightHand].x += 2 * rotate_speed;
					BodyTheta[LeftLeg].x += 2 * rotate_speed;
				}
			}
		}
		// 没有任何一个行走按键被按下
		else
		{
			// 从正在运动切换为停止运动
			if (isRunning)
			{
				isRunning = false;
				isStopping = true;
			}

			// 手脚还没归位，正在停止中
			if (isStopping)
			{
				std::vector<float *> tmp{
					&BodyTheta[LeftHand].x,
					&BodyTheta[RightHand].x,
					&BodyTheta[LeftLeg].x,
					&BodyTheta[RightLeg].x,
				};

				// 恢复原位
				for (int i = 0; i < 4; i++)
				{
					float *theta = tmp[i];
					// 已经恢复原位
					if (abs(*theta) < 1e-6)
					{
						*theta = 0;
						isStopping = false;
					}
					else if (*theta > 0)
					{
						*theta -= rotate_speed;
					}
					else
					{
						*theta += rotate_speed;
					}
				}
			}
		}
	}

	// 渲染该人物
	void render(Light *light, Camera *camera)
	{
		// 更新跳的状态
		updateJumpStatus();

		// 更新角度和坐标信息
		for (int i = 0; i < 8; i++)
		{
			meshList[i]->setRotation(BodyTheta[i]);
			meshList[i]->setTranslation(BodyTranslate[i]);
		}

		// 三角形旋转
		BodyTheta[Triangle].z += 1;

		// 层级建模下各部位的旋转角
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = torso->calculateAndSetModelMatrixByBase(modelMatrix);

		// 叠加
		head->calculateAndSetModelMatrixByBase(modelMatrix);
		leftHand->calculateAndSetModelMatrixByBase(modelMatrix, PointfRotation::UpFaceCenter);
		rightLeg->calculateAndSetModelMatrixByBase(modelMatrix, PointfRotation::UpFaceCenter);
		leftLeg->calculateAndSetModelMatrixByBase(modelMatrix, PointfRotation::UpFaceCenter);
		// 左手拿着小旗子
		glm::mat4 secondModelMatrix = rightHand->calculateAndSetModelMatrixByBase(modelMatrix, PointfRotation::UpFaceCenter);
		glm::mat4 thirdModelMatrix = rod->calculateAndSetModelMatrixByBase(secondModelMatrix);
		triangle->calculateAndSetModelMatrixByBase(thirdModelMatrix);

		// 绘制
		peoplePainter->drawMeshes(light, camera);
	}

	~People()
	{
		for (int i = 0; i < 8; i++)
			delete meshList[i];
	}
};

int WIDTH = 600;
int HEIGHT = 600;

int mainWindow;

Camera *camera = new Camera();
Light *light = new Light();

// mesh对象
People *currentUsedPeople = NULL; // 正在操控的人物
People *steve = NULL;			  // steve原画
People *alex = NULL;			  // alex原画
People *nikeMan = NULL;			  // 穿耐克的人
TriMesh *skyCube = NULL;		  // 天空盒
TriMesh *ground = NULL;			  // 不同纹理的地面
TriMesh *ground2 = NULL;		  // 不同纹理的地面
TriMesh *ground3 = NULL;		  // 不同纹理的地面
TriMesh *ground4 = NULL;		  // 不同纹理的地面
TriMesh *ground5 = NULL;		  // 不同纹理的地面
TriMesh *ground6 = NULL;		  // 不同纹理的地面
TriMesh *manWithSword = NULL;	  // 拿剑的人

// 鼠标属性，用于实现画面随着鼠标旋转
namespace MouseAttribute
{
	float lastX = 300;
	float lastY = 300;
	float sensitivity = 0.05f;
	bool firstMouse = true;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void init()
{
	// 设置光源位置
	light->setTranslation(glm::vec3(0, 8.0, 0));
	light->setAmbient(glm::vec4(1.0, 1.0, 1.0, 1.0));  // 环境光
	light->setDiffuse(glm::vec4(1.0, 1.0, 1.0, 1.0));  // 漫反射
	light->setSpecular(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 镜面反射
	light->setAttenuation(1.0, 0.045, 0.0075);		   // 衰减系数

	// 天空盒
	skyCube = new TriMesh();
	skyCube->generateCube();
	skyCube->setScale(glm::vec3(100, 100, 100));
	skyCube->setRotation(glm::vec3(0, 0, 0));
	skyCube->setNormalize(true);

	// 读取着色器并使用
	std::string cube_vshader = "shaders/cube_vshader.glsl";
	std::string cube_fshader = "shaders/cube_fshader.glsl";
	painter->addSkyCube(skyCube,
						"skyCube",
						std::vector<std::string>{
							"./assets/skybox/right.jpg",
							"./assets/skybox/left.jpg",
							"./assets/skybox/top.jpg",
							"./assets/skybox/bottom.jpg",
							"./assets/skybox/front.jpg",
							"./assets/skybox/back.jpg"},
						cube_vshader, cube_fshader);

	// 人体各个部位
	steve = new People("./assets/steve-obj/steve.obj", "./assets/steve-obj/Steve.png");
	alex = new People("./assets/steve-obj/steve.obj", "./assets/steve-obj/Alex2.png");
	nikeMan = new People("./assets/steve-obj/steve.obj", "./assets/steve-obj/nike.png");
	alex->position = glm::vec3(5, 0.258, 5);
	nikeMan->position = glm::vec3(3, 0.258, 5);
	currentUsedPeople = steve;
	currentUsedPeople->BodyTheta[currentUsedPeople->Torso].y = camera->initRotationAngle;

	// 地板1、2、3、4
	ground = new TriMesh();
	ground2 = new TriMesh();
	ground3 = new TriMesh();
	ground4 = new TriMesh();
	ground5 = new TriMesh();
	ground6 = new TriMesh();
	// 稍微往下移动一点点，不与阴影投影平面重叠
	ground->setTranslation(glm::vec3(0, -0.0001, 0));
	ground2->setTranslation(glm::vec3(-5, -0.0001, -5));
	ground3->setTranslation(glm::vec3(0, -0.0001, -5));
	ground4->setTranslation(glm::vec3(-5, -0.0001, 0));
	ground5->setTranslation(glm::vec3(5, -0.0001, -5));
	ground6->setTranslation(glm::vec3(-5, -0.0001, 5));
	bool first = true;
	for (TriMesh *g : std::vector<TriMesh *>{ground, ground2, ground3, ground4, ground5, ground6})
	{
		g->setNormalize(false);
		// 草方块面积大一点
		if (first)
		{
			g->generateSquare(10);
			first = false;
		}
		else
		{
			g->generateSquare(5);
		}
		g->setScale(glm::vec3(1, 1, 1));
		g->setRotation(glm::vec3(0, 0, 0));
		g->calculateAndSetModelMatrixByBase(glm::mat4(1.0f));
	}

	std::string vshader = "shaders/vshader.glsl";
	std::string fshader = "shaders/fshader.glsl";
	painter->addMesh(ground, "ground", "./assets/grass2d.jpg", vshader, fshader);
	painter->addMesh(ground2, "ground", "./assets/stone.png", vshader, fshader);
	painter->addMesh(ground3, "ground", "./assets/wood.png", vshader, fshader);
	painter->addMesh(ground4, "ground", "./assets/diamond.png", vshader, fshader);
	painter->addMesh(ground5, "ground", "./assets/brick.png", vshader, fshader);
	painter->addMesh(ground6, "ground", "./assets/ice.png", vshader, fshader);

	// 拿剑的人
	manWithSword = new TriMesh();
	manWithSword->setRotation(glm::vec3(0, 0, 0));
	manWithSword->setTranslation(glm::vec3(3, -1, 3));
	manWithSword->setScale(glm::vec3(1, 1, 1));
	manWithSword->setNormalize(false);
	manWithSword->readObj("./assets/chr_sword/chr_sword.obj", "chr_sword");
	manWithSword->calculateAndSetModelMatrixByBase(glm::mat4(1));
	painter->addMesh(manWithSword, "manWithSword", "./assets/chr_sword/chr_sword.png", vshader, fshader);

	// 刷新视角
	mouse_callback(NULL, MouseAttribute::lastX, MouseAttribute::lastY);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	// glClearColor(0.0, 0.0, 0.0, 1.0);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 更新摄像机
	camera->isFreeMode = isFreeMode;
	if (!isFreeMode)
		camera->at = currentUsedPeople->position;						// 第三人称模式
	mouse_callback(NULL, MouseAttribute::lastX, MouseAttribute::lastY); // 刷新视角
	camera->updateMovingStatus();

	// 天空和的位置连同史蒂夫一起移动
	skyCube->setTranslation(currentUsedPeople->position);
	skyCube->calculateAndSetModelMatrixByBase(glm::mat4(1.0f));

	// 更新正在操控的人物的运动状态
	float currentFront = camera->rotateAngle + 90;
	currentUsedPeople->updateMovingStatus(glm::radians(currentFront));

	// 渲染除了人以外的其他物体
	painter->drawMeshes(light, camera);

	// 渲染人
	steve->render(light, camera);
	alex->render(light, camera);
	nikeMan->render(light, camera);
}

void printHelp()
{
	std::cout << "================================================" << std::endl
			  << "Use mouse to controll the direction." << std::endl
			  << "================================================" << std::endl
			  << "Keyboard Usage" << std::endl
			  << "[Window]" << std::endl
			  << "ESC:		Exit" << std::endl
			  << "h:		Print help message" << std::endl
			  << "[People]" << std::endl
			  << "w:		Go front" << std::endl
			  << "s:		Go back" << std::endl
			  << "a:		Go left" << std::endl
			  << "d:		Go right" << std::endl
			  << "SPACE:		" << std::endl
			  << "[Camera]:	Jump" << std::endl
			  << "F5:		Switch to front/back/free mode" << std::endl
			  << "m:		Switch to character steve/alex/nikeman" << std::endl
			  << std::endl;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	// 移动
	if (!isFreeMode)
	{
		switch (key)
		{
		case GLFW_KEY_W:
			if (action == GLFW_RELEASE)
				currentUsedPeople->w = false;
			else if (action == GLFW_PRESS)
				currentUsedPeople->w = true;
			break;
		case GLFW_KEY_S:
			if (action == GLFW_RELEASE)
				currentUsedPeople->s = false;
			else if (action == GLFW_PRESS)
				currentUsedPeople->s = true;
			break;
		case GLFW_KEY_A:
			if (action == GLFW_RELEASE)
				currentUsedPeople->a = false;
			else if (action == GLFW_PRESS)
				currentUsedPeople->a = true;
			break;
		case GLFW_KEY_D:
			if (action == GLFW_RELEASE)
				currentUsedPeople->d = false;
			else if (action == GLFW_PRESS)
				currentUsedPeople->d = true;
			break;
		case GLFW_KEY_SPACE:
			if (action == GLFW_PRESS)
			{
				if (currentUsedPeople->isOnTheSky == false)
				{
					currentUsedPeople->isOnTheSky = true;
					currentUsedPeople->startTime = currentTimestamp();
				}
			}
			break;
		default:
			break;
		}
	}
	// 自由视角
	else
	{
		switch (key)
		{
		case GLFW_KEY_W:
		case GLFW_KEY_S:
		case GLFW_KEY_A:
		case GLFW_KEY_D:
		case GLFW_KEY_LEFT_SHIFT:
		case GLFW_KEY_SPACE:
			camera->keyboard(key, action, mode);
			return;
		default:
			break;
		}
	}

	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			exit(EXIT_SUCCESS);
			break;
		case GLFW_KEY_H:
			printHelp();
			break;
		// 切换三种不同的视角
		case GLFW_KEY_F5:
		{
			if (isFreeMode)
			{
				isFreeMode = false;
				cameraToBack = true;
				camera->initRotationAngle = 210;
				camera->rotateAngle += 185;
				break;
			}
			else if (cameraToBack)
			{
				cameraToBack = false;
				cameraToFront = true;
				camera->initRotationAngle = 25;
				camera->rotateAngle -= 185;
			}
			else if (cameraToFront)
			{
				cameraToFront = false;
				isFreeMode = true;
			}
			break;
		}
		// 切换人物
		case GLFW_KEY_M:
		{
			if (currentUsedPeople == steve)
				currentUsedPeople = alex;
			else if (currentUsedPeople == alex)
				currentUsedPeople = nikeMan;
			else if (currentUsedPeople == nikeMan)
				currentUsedPeople = steve;
			break;
		}
		default:
			camera->keyboard(key, action, mode);
			break;
		}
	}
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	// 第一帧
	if (MouseAttribute::firstMouse)
	{
		MouseAttribute::lastX = xpos;
		MouseAttribute::lastY = ypos;
		MouseAttribute::firstMouse = false;
	}

	// 计算相对上一帧的偏移
	float xoffset = MouseAttribute::lastX - xpos;
	float yoffset = MouseAttribute::lastY - ypos; // y坐标从底部往顶部依次增大的

	if (!isFreeMode)
		yoffset = -yoffset;

	// 更新last
	MouseAttribute::lastX = xpos;
	MouseAttribute::lastY = ypos;

	// 乘以鼠标灵敏度
	xoffset *= MouseAttribute::sensitivity;
	yoffset *= MouseAttribute::sensitivity;

	// 更新俯仰角和偏航角
	camera->rotateAngle += xoffset;
	camera->upAngle += yoffset;

	// 史蒂夫的方向和摄像头同步旋转
	if (!isFreeMode)
	{
		glm::vec3 &torso_theta = currentUsedPeople->BodyTheta[currentUsedPeople->Torso];
		torso_theta.y = camera->rotateAngle - camera->initRotationAngle;
	}

	// 超过90°
	if (camera->upAngle > 89.0f)
		camera->upAngle = 89.0f;
	if (camera->upAngle < -89.0f)
		camera->upAngle = -89.0f;
}

void cleanData()
{
	delete camera;
	camera = NULL;

	delete light;
	light = NULL;

	delete steve;
	steve = NULL;
}

int main(int argc, char **argv)
{
	// 初始化GLFW库，必须是应用程序调用的第一个GLFW函数
	glfwInit();

	// 配置GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// 配置窗口属性
	GLFWwindow *window = glfwCreateWindow(600, 600, "main", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Init mesh, shaders, buffer
	init();
	// 输出帮助信息
	printHelp();
	// 启用深度测试
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	cleanData();

	return 0;
}