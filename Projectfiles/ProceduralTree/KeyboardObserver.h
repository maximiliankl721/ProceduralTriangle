#pragma once

struct GLFWwindow;

class KeyboardObserver
{
public:
	KeyboardObserver();
	KeyboardObserver(GLFWwindow* window, int key);
	void Update();

	bool m_isDown;
	bool m_wasPressed;
	bool m_wasReleased;

private:
	GLFWwindow* m_window;
	int m_key;
};