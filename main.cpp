#include <GLFW\glfw3.h>
#include "Utils.h"
#include <thread>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include "lib/Robot/Robot.h"
static Robot::Keyboard kbd = Robot::Keyboard();
static Robot::Mouse mouse = Robot::Mouse();
#define WAIT_FOR_KEYPRESS(key) begin: if (!kbd.GetState(key)) { std::this_thread::sleep_for(std::chrono::milliseconds(50));/*<prevents tons of cpu usage*/ goto begin; }
//#define WAIT_FOR_KEYCOMBO_INIT Robot::KeyState keys;
//#define WAIT_FOR_KEYCOMBO(key) begin: if (!kbd.GetState(key)) { std::this_thread::sleep_for(std::chrono::milliseconds(50));/*<prevents tons of cpu usage*/ goto begin; }
#define WAIT_UNTIL_TRUE(condition) begin: if (!(##condition)) { std::this_thread::sleep_for(std::chrono::milliseconds(50));/*<prevents tons of cpu usage*/ goto begin; }
//this uses our g_exiting global var.. and only works in a function that should be returned out of!! bad..         and it also counts num times so can use on press instead of on down/the entire time running it
#define HOTKEY_WAIT_UNTIL_TRUE__AND_EXIT_IF_MAIN_THREAD_IS_ENDING(condition) { begin: if (g_exiting) { return; }   if (!(##condition)) { std::this_thread::sleep_for(std::chrono::milliseconds(50));/*<prevents tons of cpu usage*/ keymissCount++; goto begin; } }

static const TCHAR* APP_TITLE = _T("BrightnessChanger");
void msgBox(const char* msg) {
	Utils::msgBox((const TCHAR*)msg, APP_TITLE);
}
void msgBox_error(const char* msg) {
	static const TCHAR* APP_TITLE_WITH_ERROR = (const TCHAR*)(std::string(APP_TITLE) + ": Error").c_str();
	Utils::msgBox((const TCHAR*)msg, APP_TITLE_WITH_ERROR);
}
void msgBox_abortError(const char* msg) {
	const std::string msgPlusAbortMsg = std::string(msg) + "\n\n(Exiting program)";
	msgBox_error(msgPlusAbortMsg.c_str());
}

//returns true if the stream had been opened successfully, or false if not; AND will show error message if false.
template <typename InputOrOutputFileStream> bool checkIfOpen(const InputOrOutputFileStream& stream) {
	if (stream.is_open()) return true;
	//if we reach here, then it wasn't opened correctly, so show an error and return false
	msgBox_error("Couldn't open file!");
	return false;
}

void glfwErrorCallback(int error, const char* description)
{
	//puts(description);
	msgBox(description);
	exit(2);
}

static const char* CONFIG_FILE_NAME = "config.txt";
static const float GAMMA_BOOST_EXPONENT_DEFAULT = 2.0f; //(2.0 is the same max that dccw.exe ("Calibrate display color" in start menu search) in System32 uses; i tested it!)
//("g" for global)
float g_gammaBoostExponent; //set in main method
bool g_exiting = false; //whether or not the main thread says it's time to quit
//forward declarations//
void setHighGamma(); void setDefaultGamma();
// //
struct Monitor {
	GLFWmonitor* mon;
	int width; //"To get the current video mode of a monitor call glfwGetVideoMode."
	int height;
	int xpos; //"The position of the monitor on the virtual desktop, in screen coordinates, can be retrieved with glfwGetMonitorPos."
	int ypos;
	bool gammaToggle; //whether or not the monitor is toggled to be gamma ramped up or not
	void toggleGamma() { 
		gammaToggle = !gammaToggle;
		if (gammaToggle) setHighGamma(); else setDefaultGamma();
	}
};
std::vector<Monitor> g_monitors;
Monitor* g_currentMonitor; //in updateCurrentMonitor(), this will be set to the monitor that the user has his or her mouse over currently
void updateMonitorInfo() {
	g_monitors.clear();
	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count); // http://www.glfw.org/docs/latest/group__monitor.html#ga3fba51c8bd36491d4712aa5bd074a537
	
	//put our monitor data into the vector//
	GLFWmonitor* monitor;
	const GLFWvidmode* mode;
	int xpos, ypos;
	for (int i = 0; i < count; i++) {
		monitor = monitors[i];
		mode = glfwGetVideoMode(monitor);
		glfwGetMonitorPos(monitor, &xpos, &ypos);
		g_monitors.push_back({ monitor, mode->width, mode->height, xpos, ypos, false });
	}
	// //
}
void updateCurrentMonitor() {
	Robot::Point mousePos = mouse.GetPos();
	for (Monitor& mon : g_monitors) {
		//check if our mouse is in this monitor
		if (mousePos.X > mon.xpos &&
			mousePos.X < mon.xpos + mon.width &&
			mousePos.Y > mon.ypos &&
			mousePos.Y < mon.ypos + mon.height) { //"Negative positions are to the left and top of your main monitor." ( https://www.google.com/search?q=monitor+position&oq=monitor+position&aqs=chrome..69i57j0l5.1866j0j7&sourceid=chrome&es_sm=122&ie=UTF-8#safe=off&q=winapi+monitor+position )
			g_currentMonitor = &mon;
			break;
		}
	}
}
//"The callback function receives the handle for the monitor that has been connected or disconnected and the event that occurred." ( http://www.glfw.org/docs/latest/monitor_guide.html#monitor_event )
void glfwMonitorChangeCallback(GLFWmonitor* monitor, int event) {
	updateMonitorInfo();
}

void setHighGamma() {
	glfwSetGamma(g_currentMonitor->mon, g_gammaBoostExponent); //sets the "gamma ramp"
	//!!!!!! GLFW SAYS this must be calledf rom the main threadnoly... but idont see any problems jappening!!!!!
}
void setDefaultGamma() {
	glfwSetGamma(g_currentMonitor->mon, 1.0f); //TODO (minor): if the user had a different value initially, this overwrites it! need to fix! (glew seems to keep track of the old gamma ramp because it restores it on glfwTerminate() (, so not that big of a deal - the user will indeed already get his or her previous gamma back on program exit)! or you can use glfwGetGammaRamp ( http://www.glfw.org/docs/latest/group__monitor.html#gab7c41deb2219bde3e1eb756ddaa9ec80 )... )
}

void hotkey_gammaToggle() {
	//Robot::KeyState keysRequired({ {Robot::Key::KeySystem, true}, {Robot::Key::KeyB, true} });
	//bool toggle = false;
	int keymissCount;
	while (true) {
		keymissCount = 0;
		HOTKEY_WAIT_UNTIL_TRUE__AND_EXIT_IF_MAIN_THREAD_IS_ENDING(kbd.GetState(Robot::Key::KeySystem) && kbd.GetState(Robot::Key::KeyG));
		if (keymissCount != 0) {
			updateCurrentMonitor();
			g_currentMonitor->toggleGamma();
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}
}
void hotkey_exit() {
	WAIT_FOR_KEYPRESS(Robot::Key::KeyEscape);
}

#define EXITING	{ g_exiting = true; hotkeyThread_winkeyPlusB.join(); }

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
#pragma region Init GLFW
	if (!glfwInit()) // http://www.glfw.org/docs/latest/intro_guide.html
	{
		// Handle initialization failure
		msgBox_abortError("Failed to init GLFW!");
		return EXIT_FAILURE;
	}
	glfwSetErrorCallback(glfwErrorCallback); //"If you wish to be notified when a monitor is connected or disconnected, set a monitor callback." ( http://www.glfw.org/docs/latest/monitor_guide.html#monitor_event )
	updateMonitorInfo(); //grab initial monitor data
	glfwSetMonitorCallback(glfwMonitorChangeCallback);
#pragma endregion

#pragma region Do stuff
	//load our g_gammaBoostExponent from file
	bool wasFileInvalid = false; //assume
	bool couldOpenFile;
	{
		g_gammaBoostExponent = GAMMA_BOOST_EXPONENT_DEFAULT; //assume
		std::string line;
		std::ifstream readingFile(CONFIG_FILE_NAME);                                  //[NVM, not using because had to open and clear if needed:]fstream allows get and put (read and write) 
		if (readingFile) { //(checks if is_open() - this will be false if the file hasn't been created yet!)
			std::getline(readingFile, line);
			try {
				g_gammaBoostExponent = std::stof(line);
			}
			catch (const std::invalid_argument& ia) { //"if no conversion could be performed" ( * http://en.cppreference.com/w/cpp/string/basic_string/stof )
				wasFileInvalid = true;
			}
			catch (const std::out_of_range& oor) { //" if the converted value would fall out of the range of the result type or if the underlying function (strtof, (since C++17)strtod or strtold) sets errno to ERANGE." (*)
				wasFileInvalid = true;
			}
			readingFile.close();
		}
	}

	//tut
	msgBox(
"Controls:\n\
 - Press {Windows Key + G} to toggle gamma (brightness)\n\
 - Press {Escape} to exit the program\n\n\
Tips:\n\
 - Edit the config.txt file to set the gamma to be used when toggling (requires restart)\n\
 - Gamma is toggled for the monitor that the mouse is currently hovering over"
	);

	std::thread hotkeyThread_winkeyPlusB(hotkey_gammaToggle);

	//GLFWmonitor* monitor = glfwGetPrimaryMonitor(); // http://www.glfw.org/docs/latest/monitor_guide.html

	//wait for escape
	hotkey_exit();
#pragma endregion

	//system("PAUSE");

#pragma region Cleanup, stop threads(... or just std::terminate(<NVM!)) , save files, etc.
	{
		std::ofstream writingFile(CONFIG_FILE_NAME, std::ios::trunc/*<removes everything in the file first*/);
		couldOpenFile = checkIfOpen(writingFile);
		if (couldOpenFile) {
			writingFile << g_gammaBoostExponent;
		}
	}


	EXITING;
	glfwTerminate(); //(<this also restores the gamma ramp to default)
#pragma endregion
	return 0;
}