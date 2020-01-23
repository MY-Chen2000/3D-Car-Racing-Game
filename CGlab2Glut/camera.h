#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	ROTATE
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Camera Attributes
	glm::vec3 Center;
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 lastRight;
	glm::vec3 WorldUp;
	int firsttime = 0;
	// Euler Angles
	float Yaw;
	float Pitch;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// Constructor with vectors
	Camera(glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Center = center;
		WorldUp = up;
		Yaw = yaw;

		Pitch = pitch;
		Position = glm::vec3(0.0, 60.0, 5.0);
		Front = Center - Position;
		updateCameraVectors();
	}
	// Constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Center = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		Position = glm::vec3(1.0, 1.0, 3.0);
		updateCameraVectors();
	}

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT) {
			//Position -= Right * velocity;
			Yaw -= velocity * 10;
			updateCameraVectors();
		}
		if (direction == RIGHT) {
			//Position += Right * velocity;
			Yaw += velocity * 10;

			updateCameraVectors();
		}
		if (direction == ROTATE) {
			updateCameraVectorsRotate(velocity);
		}
	}




	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		//glm::vec3 upup = glm::vec3(0.0f, 1.0f, 0.0f);
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		if (xoffset > 0) {
			float velocity = MovementSpeed * xoffset / 100;
			Position -= Right * velocity;
		}
		if (xoffset < 0) {
			float velocity = MovementSpeed * xoffset / 100;
			Position -= Right * velocity;
		}
		if (yoffset > 0) {
			float velocity = MovementSpeed * yoffset / 100;
			Position -= Up * velocity;
		}
		if (yoffset < 0) {
			float velocity = MovementSpeed * yoffset / 100;
			Position -= Up * velocity;
		}


		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		//if (constrainPitch)
		//{
		//	if (Pitch > 89.0f)
		//		Pitch = 89.0f;
		//	if (Pitch < -89.0f)
		//		Pitch = -89.0f;
		//}

		// Update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovementRight(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		float len = sqrtf(powf(Position.x - Center.x, 2) + powf(Position.y - Center.y, 2) + powf(Position.z - Center.z, 2));
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		float Yawbf = Yaw;
		float Pitchbf = Pitch;
		Yaw += xoffset;
		Pitch += yoffset;

		Position.y = (len* sin(glm::radians(Pitch)) + Center.y);
		Position.z = (len * cos(glm::radians(Pitch))*sin(glm::radians(Yaw)) + Center.z);
		Position.x = (len * cos(glm::radians(Pitch))*cos(glm::radians(Yaw)) + Center.x);



		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		//if (constrainPitch)
		//{
		//	if (Pitch > 89.0f)
		//		Pitch = 89.0f;
		//	if (Pitch < -89.0f)
		//		Pitch = -89.0f;
		//}

		// Update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectorsRight();
	}


	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;
	}

private:
	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		//front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		//front.y = sin(glm::radians(Pitch));
		//front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		//Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		lastRight = Right;
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		if (glm::dot(lastRight, Right) < 0) {
			Right = -Right;
		}  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}

	void updateCameraVectorsRight()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = -Position.x + Center.x;// cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = -Position.y + Center.y; //sin(glm::radians(Pitch));
		front.z = -Position.z + Center.z;//sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// Also re-calculate the Right and Up 
		lastRight = Right;
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		if (glm::dot(lastRight, Right) < 0 && firsttime != 0) {
			Right = -Right;
		}
		if (firsttime == 0) {
			firsttime = 1;
		}

		Up = glm::normalize(glm::cross(Right, Front));

	}
	void updateCameraVectorsRotate(float velocity)
	{

		// Calculate the new Front vector
		//glm::vec3 front;
		//front.x = -Position.x + Center.x;// cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		//front.y = -Position.y + Center.y; //sin(glm::radians(Pitch));
		//front.z = -Position.z + Center.z;//sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		//Front = glm::normalize(front);
		// Also re-calculate the Right and Up 
		//lastRight = Right;
		float len1 = sqrtf(powf(Position.x - Center.x, 2) + powf(Position.y - Center.y, 2) + powf(Position.z - Center.z, 2));
		float lenth = len1 / sin(glm::radians(Pitch));
		glm::vec3 tmp = Position - Center - lenth * glm::normalize(WorldUp);
		//float begin =glm::acos(glm::dot(glm::normalize(Right), glm::normalize(tmp)));
		float angle = glm::radians(velocity);

		glm::vec3 n = glm::normalize(glm::cross(WorldUp, glm::vec3(Position.x - Center.x, 0, Position.z - Center.z)));

		Right.y = -cos(angle)*cos(glm::radians(Pitch));
		Right.x = Center.x + (1 + cos(angle)*sin(glm::radians(Pitch)))*(Position.x - Center.x) + n.x*sin(angle) - Position.x;
		Right.z = Center.z + (1 + cos(angle)*sin(glm::radians(Pitch)))*(Position.z - Center.z) + n.z*sin(angle) - Position.z;

		Right = glm::normalize(Right);
		//Front = -glm::normalize(glm::cross(Right, tmp));
		//Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		//if (glm::dot(lastRight, Right) < 0) {
		//	Right = -Right;
	//	}

		Up = glm::normalize(glm::cross(Right, Front));

	}
};
#endif