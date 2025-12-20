#include <Arduino.h>
#include <Keyboard.h>
#include <Mouse.h>

#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "LCD_1in47.h"

// Language/Keyboard Support Configuration
#define KEYBOARD_LANGUAGE_SWE  // Swedish keyboard layout support

#define LCARS_BG       BLACK
#define LCARS_TEXT     WHITE
#define LCARS_MUTED    GRAY
#define LCARS_BAR      BROWN
#define LCARS_ACCENT_A YELLOW
#define LCARS_ACCENT_B CYAN
#define LCARS_ACCENT_C MAGENTA

UWORD *frameBuffer = NULL;

static String serialReceiveBuffer;
static String lastSerialData;
static unsigned long lastSerialRxTime = 0;

// Localized app name and message for different keyboard layouts
#ifdef KEYBOARD_LANGUAGE_SWE
  static constexpr const char *IOS_APP_NAME = "Notes";
  static constexpr const char *IOS_MESSAGE  = "I Love you ";  // Swedish: "Love you"
#else
  static constexpr const char *IOS_APP_NAME = "Notes";
  static constexpr const char *IOS_MESSAGE  = "Love you";
#endif

// Focus strategy:
// - Prefer keyboard-only focus (Cmd+N) which should create/focus a new note.
// - Optional mouse focus uses RELATIVE moves only (no absolute coordinates). You must enable the iOS pointer.
//
// Swedish Keyboard Support:
// This sketch works with Swedish keyboard layouts on iPhone. The UTF-8 encoded text
// (including special characters like Ä, Ö, Å) will be typed using the iOS HID protocol.
// Ensure your Arduino IDE is configured for UTF-8:
// 1. Save this file as UTF-8 (File > Save with Encoding > UTF-8)
// 2. Configure Tools > Board settings for RP2350/Pico
// 3. iOS will use the active keyboard layout (set in iPhone Settings > General > Keyboard)
static constexpr bool USE_NOTES_CMD_N_FOCUS = true;
static constexpr bool USE_MOUSE_CENTER_CLICK_FALLBACK = false;

// Mouse focus tuning (relative moves):
// 1) We “slam” to upper-left with big negative moves.
// 2) Move roughly toward screen center with small steps.
// These values are intentionally conservative; adjust if the click doesn’t land in the editor.
static constexpr int MOUSE_SLAM_STEPS = 40;
static constexpr int MOUSE_SLAM_DELTA = 127;
static constexpr int MOUSE_CENTER_X_STEPS = 45;
static constexpr int MOUSE_CENTER_Y_STEPS = 70;
static constexpr int MOUSE_CENTER_DX = 10;
static constexpr int MOUSE_CENTER_DY = 10;

// Timing tends to vary a lot by iPhone model/iOS version.
// If typing happens too early (goes to Spotlight instead of Notes), increase these.
static constexpr unsigned long SPOTLIGHT_OPEN_DELAY_MS = 900;
static constexpr unsigned long AFTER_ENTER_LAUNCH_DELAY_MS = 3500;

static void pressCombo(uint8_t modifierKey, uint8_t key) {
	Keyboard.press(modifierKey);
	Keyboard.press(key);
	delay(60);
	Keyboard.releaseAll();
}

static void mouseClick() {
	// Relative pointer only (no touch/coordinate tap).
	Mouse.click(MOUSE_LEFT);
}

static void mouseMoveSteps(int dx, int dy, int steps, int stepDelayMs = 6) {
	for (int i = 0; i < steps; i++) {
		Mouse.move(dx, dy, 0);
		delay(stepDelayMs);
	}
}

static void mouseCenterClick() {
	// This only works if iOS has a pointer active:
	// Settings -> Accessibility -> Touch -> AssistiveTouch -> Pointer Devices.
	mouseMoveSteps(-MOUSE_SLAM_DELTA, -MOUSE_SLAM_DELTA, MOUSE_SLAM_STEPS);
	// Move toward “center” (best effort; depends on pointer speed/acceleration).
	mouseMoveSteps(MOUSE_CENTER_DX, 0, MOUSE_CENTER_X_STEPS);
	mouseMoveSteps(0, MOUSE_CENTER_DY, MOUSE_CENTER_Y_STEPS);
	mouseClick();
}

static void pressKey(uint8_t key) {
	Keyboard.press(key);
	delay(40);
	Keyboard.releaseAll();
}

static void drawStatus(const char *line1, const char *line2) {
	const UWORD W = LCD_1IN47_WIDTH;
	const UWORD H = LCD_1IN47_HEIGHT;

	Paint_Clear(LCARS_BG);
	Paint_DrawRectangle(0, 0, 48, H - 1, LCARS_BAR, DOT_PIXEL_1X1, DRAW_FILL_FULL);
	Paint_DrawRectangle(0, 0, 48, 20, LCARS_ACCENT_A, DOT_PIXEL_1X1, DRAW_FILL_FULL);

	Paint_DrawString_EN(58, 18, line1, &Font16, LCARS_TEXT, LCARS_BG);
	Paint_DrawString_EN(58, 42, line2, &Font12, LCARS_ACCENT_A, LCARS_BG);

	LCD_1IN47_Display(frameBuffer);
}

static void drawWithSerial(const char *line1, const char *line2) {
	const UWORD W = LCD_1IN47_WIDTH;
	const UWORD H = LCD_1IN47_HEIGHT;

	Paint_Clear(LCARS_BG);
	Paint_DrawRectangle(0, 0, 48, H - 1, LCARS_BAR, DOT_PIXEL_1X1, DRAW_FILL_FULL);
	Paint_DrawRectangle(0, 0, 48, 20, LCARS_ACCENT_A, DOT_PIXEL_1X1, DRAW_FILL_FULL);

	Paint_DrawString_EN(58, 18, line1, &Font16, LCARS_TEXT, LCARS_BG);
	Paint_DrawString_EN(58, 42, line2, &Font12, LCARS_ACCENT_A, LCARS_BG);

	// Show latest serial data at bottom.
	if (lastSerialData.length() > 0) {
		String truncated = lastSerialData;
		if (truncated.length() > 40) truncated = truncated.substring(truncated.length() - 40);
		Paint_DrawString_EN(58, H - 20, truncated.c_str(), &Font8, LCARS_ACCENT_B, LCARS_BG);
	}

	LCD_1IN47_Display(frameBuffer);
}

void setup() {
	// Give USB some time to enumerate.
	delay(1500);

	Serial.begin(115200);
	serialReceiveBuffer.reserve(256);
	lastSerialData.reserve(256);

	if (DEV_Module_Init() != 0) {
		return;
	}

	DEV_SET_PWM(0);
	LCD_1IN47_Init(VERTICAL);
	LCD_1IN47_Clear(WHITE);
	DEV_SET_PWM(80);

	UDOUBLE imageSize = LCD_1IN47_HEIGHT * LCD_1IN47_WIDTH * 2;
	frameBuffer = (UWORD *)malloc(imageSize);
	if (!frameBuffer) {
		return;
	}

	Paint_NewImage((UBYTE *)frameBuffer, LCD_1IN47.WIDTH, LCD_1IN47.HEIGHT, 0, WHITE);
	Paint_SetScale(65);
	Paint_SetRotate(ROTATE_0);
	Paint_Clear(WHITE);

	drawStatus("IPHONE HID", "Focus Notes, wait...");

	// Start HID keyboard.
	Keyboard.begin();
	Mouse.begin();

	// Safety: only type once, after a short wait.
	delay(2500);

	drawStatus("CMD+SPACE", "Opening Spotlight...");
	// iOS Spotlight search: Cmd+Space (works with hardware keyboards).
	pressCombo(KEY_LEFT_GUI, ' ');

	// Give Spotlight time to appear.
	delay(SPOTLIGHT_OPEN_DELAY_MS);

	drawStatus("SEARCH", "Typing app name...");
	Keyboard.print(IOS_APP_NAME);

	// Launch the top result.
	delay(200);
	pressKey(KEY_RETURN);

	// Give Notes time to launch and focus the editor.
	delay(AFTER_ENTER_LAUNCH_DELAY_MS);

	if (USE_NOTES_CMD_N_FOCUS) {
		drawStatus("FOCUS", "Cmd+N new note...");
		// Notes shortcut: Cmd+N should open a new note and place the cursor.
		pressCombo(KEY_LEFT_GUI, 'n');
		delay(900);
	}

	if (USE_MOUSE_CENTER_CLICK_FALLBACK) {
		drawStatus("FOCUS", "Mouse center click...");
		// Best-effort: may do nothing if iOS pointer isn't active.
		mouseCenterClick();
		delay(600);
	}

	drawStatus("MESSAGE", "Typing text...");
	Keyboard.println(IOS_MESSAGE);

	delay(500);
	drawStatus("DONE", "If failed: increase AFTER_ENTER delay");
}

void loop() {
	// Listen for serial input from iPhone serial app.
	while (Serial.available()) {
		char c = (char)Serial.read();
		if (c == '\n' || c == '\r') {
			if (serialReceiveBuffer.length() > 0) {
				lastSerialData = serialReceiveBuffer;
				serialReceiveBuffer = "";
				lastSerialRxTime = millis();
				// Echo back confirmation.
				Serial.print("RX: ");
				Serial.println(lastSerialData);
			}
		} else {
			if (serialReceiveBuffer.length() < 250) {
				serialReceiveBuffer += c;
			}
		}
	}

	// Periodically redraw to show serial data with animated activity.
	static unsigned long lastRedraw = 0;
	if (millis() - lastRedraw >= 300) {
		lastRedraw = millis();
		
		// Create animated "waiting" indicator (3 dots that cycle).
		int dotCount = (millis() / 300) % 4;
		String waitingDots = "";
		for (int i = 0; i < dotCount; i++) waitingDots += ".";
		
		String statusLine = "Listening" + waitingDots;
		drawWithSerial("READY", statusLine.c_str());
	}

	DEV_Delay_ms(100);
}
