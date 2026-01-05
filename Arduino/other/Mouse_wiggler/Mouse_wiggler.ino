#include <Arduino.h>
#include <Mouse.h>

#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "LCD_1in47.h"

#define PURPLE         0x8010
#define PINK           0xFE19
#define LIGHTGRAY      0xC618
#define LCARS_BG       PURPLE
#define LCARS_TEXT     WHITE
#define LCARS_BAR      BROWN
#define LCARS_ACCENT_A YELLOW

UWORD *frameBuffer = NULL;

static void drawScreen(const char *line1, const char *line2, int lookX, int lookY, bool blink) {
	Paint_Clear(LCARS_BG);
	
	// Draw Mouse
	int cx = 160;
	int cy = 70;
	
	// Ears
	Paint_DrawCircle(cx - 45, cy - 40, 20, LIGHTGRAY, DOT_PIXEL_1X1, DRAW_FILL_FULL);
	Paint_DrawCircle(cx + 45, cy - 40, 20, LIGHTGRAY, DOT_PIXEL_1X1, DRAW_FILL_FULL);
	// Inner Ears
	Paint_DrawCircle(cx - 45, cy - 40, 10, PINK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
	Paint_DrawCircle(cx + 45, cy - 40, 10, PINK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

	// Head
	Paint_DrawCircle(cx, cy, 45, LIGHTGRAY, DOT_PIXEL_1X1, DRAW_FILL_FULL);

	// Eyes
	int eyeY = cy - 10;
	int leftEyeX = cx - 15;
	int rightEyeX = cx + 15;
	
	if (blink) {
		// Closed eyes (lines)
		Paint_DrawLine(leftEyeX - 6, eyeY, leftEyeX + 6, eyeY, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
		Paint_DrawLine(rightEyeX - 6, eyeY, rightEyeX + 6, eyeY, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
	} else {
		// Whites
		Paint_DrawCircle(leftEyeX, eyeY, 8, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
		Paint_DrawCircle(rightEyeX, eyeY, 8, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
		
		// Pupils
		int pupX = 0;
		int pupY = 0;
		if (lookX < 0) pupX = -3;
		if (lookX > 0) pupX = 3;
		if (lookY < 0) pupY = -3;
		if (lookY > 0) pupY = 3;
		
		Paint_DrawCircle(leftEyeX + pupX, eyeY + pupY, 3, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
		Paint_DrawCircle(rightEyeX + pupX, eyeY + pupY, 3, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
	}

	// Nose
	Paint_DrawCircle(cx, cy + 10, 5, PINK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

	// Whiskers
	Paint_DrawLine(cx - 10, cy + 10, cx - 30, cy + 0, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawLine(cx - 10, cy + 12, cx - 30, cy + 12, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawLine(cx - 10, cy + 14, cx - 30, cy + 24, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

	Paint_DrawLine(cx + 10, cy + 10, cx + 30, cy + 0, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawLine(cx + 10, cy + 12, cx + 30, cy + 12, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawLine(cx + 10, cy + 14, cx + 30, cy + 24, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

	int x1 = (LCD_1IN47.WIDTH - (strlen(line1) * Font20.Width)) / 2;
	int x2 = (LCD_1IN47.WIDTH - (strlen(line2) * Font20.Width)) / 2;
	
	Paint_DrawString_EN(x1 > 0 ? x1 : 0, 120, line1, &Font20, LCARS_TEXT, LCARS_BG);
	Paint_DrawString_EN(x2 > 0 ? x2 : 0, 145, line2, &Font20, LCARS_ACCENT_A, LCARS_BG);
	LCD_1IN47_Display(frameBuffer);
}

static void updateEyes(int lookX, int lookY, bool blink) {
	int cx = 160;
	int cy = 70;
	int eyeY = cy - 10;
	int leftEyeX = cx - 15;
	int rightEyeX = cx + 15;

	// Clear eye area with head color
	Paint_ClearWindows(130, 45, 190, 75, LIGHTGRAY);

	if (blink) {
		Paint_DrawLine(leftEyeX - 6, eyeY, leftEyeX + 6, eyeY, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
		Paint_DrawLine(rightEyeX - 6, eyeY, rightEyeX + 6, eyeY, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
	} else {
		Paint_DrawCircle(leftEyeX, eyeY, 8, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
		Paint_DrawCircle(rightEyeX, eyeY, 8, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
		
		int pupX = 0;
		int pupY = 0;
		if (lookX < 0) pupX = -3;
		if (lookX > 0) pupX = 3;
		if (lookY < 0) pupY = -3;
		if (lookY > 0) pupY = 3;
		
		Paint_DrawCircle(leftEyeX + pupX, eyeY + pupY, 3, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
		Paint_DrawCircle(rightEyeX + pupX, eyeY + pupY, 3, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
	}
	LCD_1IN47_DisplayWindows(130, 45, 190, 75, frameBuffer);
}

static void mouseMoveCurve(long durationMs) {
	long startTime = millis();
	long endTime = startTime + durationMs;
	
	// Random target (relative to start)
	int targetX = random(-400, 401);
	int targetY = random(-400, 401);
	
	// Control point for Bezier (randomize to create curve)
	int controlX = random(-600, 601);
	int controlY = random(-600, 601);
	
	float currentX = 0;
	float currentY = 0;
	
	int lastLookX = 0;
	int lastLookY = 0;
	
	// Initial draw
	drawScreen("", "Moving...", 0, 0, false);
	
	while (millis() < endTime) {
		unsigned long now = millis();
		float t = (float)(now - startTime) / durationMs;
		if (t > 1.0) t = 1.0;
		
		// Quadratic Bezier: B(t) = (1-t)^2 P0 + 2(1-t)t P1 + t^2 P2
		float u = 1 - t;
		float tt = t * t;
		
		float nextX = 2 * u * t * controlX + tt * targetX;
		float nextY = 2 * u * t * controlY + tt * targetY;
		
		int moveX = (int)(nextX - currentX);
		int moveY = (int)(nextY - currentY);
		
		// Clamp to char range to prevent overflow
		if (moveX > 127) moveX = 127;
		if (moveX < -128) moveX = -128;
		if (moveY > 127) moveY = 127;
		if (moveY < -128) moveY = -128;
		
		if (moveX != 0 || moveY != 0) {
			Mouse.move(moveX, moveY, 0);
			currentX += moveX;
			currentY += moveY;
		}
		
		// Calculate velocity for eyes (Derivative of Bezier)
		// B'(t) = 2(1-2t)P1 + 2tP2
		float vx = 2 * (1 - 2 * t) * controlX + 2 * t * targetX;
		float vy = 2 * (1 - 2 * t) * controlY + 2 * t * targetY;

		// Update eyes only on change
		int lookX = 0;
		if (vx < -0.1) lookX = -1;
		else if (vx > 0.1) lookX = 1;
		
		int lookY = 0;
		if (vy < -0.1) lookY = -1;
		else if (vy > 0.1) lookY = 1;
		
		if (lookX != lastLookX || lookY != lastLookY) {
			updateEyes(lookX, lookY, false);
			lastLookX = lookX;
			lastLookY = lookY;
		}
		
		delay(2);
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

	drawScreen("IOS MOUSE", "Enable iOS pointer first", 0, 0, false);

	Mouse.begin();
	delay(500);
	drawScreen("MOVING", "Pointer should move now", 0, 0, false);
}

void loop() {
	// Random mouse wiggler - all movements and pauses are randomized
	// IMPORTANT: iOS requires a pointer to be enabled (AssistiveTouch / Pointer Devices).

	// Random duration up to 3 seconds
	long moveDuration = random(500, 3001);
	
	// Perform the random curved movement
	mouseMoveCurve(moveDuration);
	
	// Random pause between movements
	int pauseTime = random(300, 20000);
	int remainingTime = pauseTime;
	
	unsigned long nextBlinkTime = millis() + random(1000, 4000);
	bool isBlinking = false;
	unsigned long blinkEndTime = 0;
	char bufTime[16];
	
	// Countdown pause with live display update
	while (remainingTime > 0) {
		unsigned long now = millis();
		
		// Handle blinking
		if (!isBlinking && now >= nextBlinkTime) {
			isBlinking = true;
			blinkEndTime = now + 150; // Blink for 150ms
		}
		if (isBlinking && now >= blinkEndTime) {
			isBlinking = false;
			nextBlinkTime = now + random(1000, 4000);
		}

		sprintf(bufTime, "Wait: %ds", (remainingTime + 999) / 1000);
		drawScreen("", bufTime, 0, 0, isBlinking);
		
		// Update in 100ms intervals for smooth countdown
		int delayAmount = (remainingTime > 100) ? 100 : remainingTime;
		delay(delayAmount);
		remainingTime -= delayAmount;
	}
}
