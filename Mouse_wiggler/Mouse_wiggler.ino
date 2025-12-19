#include <Arduino.h>
#include <Mouse.h>

#include "../dep/DEV_Config.h"
#include "../dep/GUI_Paint.h"
#include "../dep/LCD_1in47.h"

#define LCARS_BG       BLACK
#define LCARS_TEXT     WHITE
#define LCARS_BAR      BROWN
#define LCARS_ACCENT_A YELLOW

UWORD *frameBuffer = NULL;

static void drawStatus(const char *line1, const char *line2) {
	const UWORD H = LCD_1IN47_HEIGHT;

	Paint_Clear(LCARS_BG);
	Paint_DrawRectangle(0, 0, 48, H - 1, LCARS_BAR, DOT_PIXEL_1X1, DRAW_FILL_FULL);
	Paint_DrawRectangle(0, 0, 48, 20, LCARS_ACCENT_A, DOT_PIXEL_1X1, DRAW_FILL_FULL);
	Paint_DrawString_EN(58, 10, line1, &Font24, LCARS_TEXT, LCARS_BG);
	Paint_DrawString_EN(58, 50, line2, &Font20, LCARS_ACCENT_A, LCARS_BG);
	LCD_1IN47_Display(frameBuffer);
}

static void mouseMoveSteps(int dx, int dy, int steps, int stepDelayMs = 6) {
	for (int i = 0; i < steps; i++) {
		Mouse.move(dx, dy, 0);
		delay(stepDelayMs);
	}
}

void setup() {
	delay(1500);

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

	drawStatus("IOS MOUSE", "Enable iOS pointer first");

	Mouse.begin();
	delay(500);
	drawStatus("MOVING", "Pointer should move now");
}

void loop() {
	// Random mouse wiggler - all movements and pauses are randomized
	// IMPORTANT: iOS requires a pointer to be enabled (AssistiveTouch / Pointer Devices).

	// Generate random movement vector
	int dx = random(-8, 9);  // Random X movement: -8 to 8
	int dy = random(-8, 9);  // Random Y movement: -8 to 8
	
	// Skip if both are zero
	if (dx == 0 && dy == 0) {
		dx = random(-8, 9);
		dy = random(-8, 9);
	}
	
	// Random number of steps
	int steps = random(20, 80);
	
	// Random step delay
	int stepDelay = random(2, 15);
	
	// Perform the random movement
	mouseMoveSteps(dx, dy, steps, stepDelay);
	
	// Random pause between movements
	int pauseTime = random(300, 20000);
	int remainingTime = pauseTime;
	
	// Countdown pause with live display update
	while (remainingTime > 0) {
		char buf1[16];
		char buf2[16];
		sprintf(buf1, "dx:%d dy:%d", dx, dy);
		sprintf(buf2, "Next move:%dms", remainingTime);
		drawStatus(buf1, buf2);
		
		// Update in 100ms intervals for smooth countdown
		int delayAmount = (remainingTime > 100) ? 100 : remainingTime;
		delay(delayAmount);
		remainingTime -= delayAmount;
	}
}
