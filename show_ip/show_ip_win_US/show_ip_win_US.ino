#include <stdint.h>
#include <Keyboard.h>
#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "LCD_1in47.h"
#include "ImageData.h"

#define DISPLAY_BG_COLOR BLACK
#define DISPLAY_TEXT_COLOR GREEN

#define LCARS_BG            BLACK
#define LCARS_TEXT          WHITE
#define LCARS_MUTED         GRAY
#define LCARS_BAR           BROWN
#define LCARS_ACCENT_A      YELLOW
#define LCARS_ACCENT_B      CYAN
#define LCARS_ACCENT_C      MAGENTA
#define TYPE_COMBO_DELAY_MS 1
#define TYPE_LINE_DELAY_MS  5

UWORD *frameBuffer = NULL;
String hostLine = "host: Empty...";
String wlanLine = "wlan0: Empty...";
String ethLine  = "eth0: Empty...";
String otherLine = "other: Empty...";
String otherLine2 = "other2: Empty...";
bool hasReceivedData = false;
unsigned long lastUpdateMs = 0;
unsigned long lastStatusRenderMs = 0;
bool scriptTriggered = false;
unsigned long lastTriggerAttempt = 0;

const char *WINDOWS_PS_SCRIPT[] = {

    "mode con: cols=24 lines=4",
    "try{iex(irm https://raw.githubusercontent.com/PierreGode/RP2350-LCD-scripts/refs/heads/main/show_ip_win/remoteps.ip.ps1)}catch{}",
    "exit"
};

const size_t WINDOWS_PS_LINES = sizeof(WINDOWS_PS_SCRIPT) / sizeof(WINDOWS_PS_SCRIPT[0]);

void typeUS(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        char c = str[i];
        
        switch(c) {
            // US keyboard: shift+1-0 produces !@#$%^&*()
            case '!':  // Shift+1
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.press('1');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '@':  // Shift+2
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.press('2');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '#':  // Shift+3
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.press('3');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '$':  // Shift+4
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.press('4');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '%':  // Shift+5
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.press('5');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '^':  // Shift+6
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.press('6');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '&':  // Shift+7
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.press('7');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '*':  // Shift+8
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.press('8');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '(':  // Shift+9
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.press('9');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case ')':  // Shift+0
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.press('0');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '_':  // Shift+minus
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.write('-');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '+':  // Shift+equals
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.write('=');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '{':  // Shift+[
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.write('[');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '}':  // Shift+]
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.write(']');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '[':  // Direct key
                Keyboard.write('[');
                break;
            case ']':  // Direct key
                Keyboard.write(']');
                break;
            case '|':  // Shift+backslash
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.write('\\');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '\\': // Direct key
                Keyboard.write('\\');
                break;
            case ':':  // Shift+semicolon
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.write(';');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case ';':  // Direct key
                Keyboard.write(';');
                break;
            case '"':  // Shift+apostrophe
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.write('\'');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '\'': // Direct key
                Keyboard.write('\'');
                break;
            case '<':  // Shift+comma
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.write(',');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '>':  // Shift+period
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.write('.');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '?':  // Shift+slash
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.write('/');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '~':  // Shift+backtick
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.write('`');
                delay(TYPE_COMBO_DELAY_MS);
                Keyboard.releaseAll();
                break;
            case '`':  // Direct key
                Keyboard.write('`');
                break;
            case '=':  // Direct key
                Keyboard.write('=');
                break;
            case '-':  // Direct key
                Keyboard.write('-');
                break;
            case '/':  // Direct key
                Keyboard.write('/');
                break;
            case '.':  // Direct key
                Keyboard.write('.');
                break;
            case ',':  // Direct key
                Keyboard.write(',');
                break;
                
            // Standard characters (letters, numbers, space)
            default:
                Keyboard.write(c);
                break;
        }
    }
}

static String ellipsize(const String &s, size_t maxLen) {
    if (s.length() <= maxLen) return s;
    if (maxLen <= 3) return s.substring(0, maxLen);
    return s.substring(0, maxLen - 3) + "...";
}

static String stripPrefix(const String &s, const char *prefix) {
    String p(prefix);
    if (s.startsWith(p)) return s.substring(p.length());
    return s;
}

static String displayValueOrDash(const String &s) {
    if (s.indexOf("Empty") >= 0) return "--";
    return s;
}

void drawScreen() {
    const UWORD W = LCD_1IN47_WIDTH;
    const UWORD H = LCD_1IN47_HEIGHT;

    const UWORD sidebarW = 48;
    const UWORD headerH = 0;
    const UWORD footerH = 16;
    const UWORD pad = 6;

    Paint_Clear(LCARS_BG);

    // Left LCARS sidebar
    Paint_DrawRectangle(0, 0, sidebarW, H - 1, LCARS_BAR, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(0, 0, sidebarW, headerH + 8, LCARS_ACCENT_A, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(0, headerH + 14, sidebarW, headerH + 34, LCARS_ACCENT_C, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(0, headerH + 40, sidebarW, headerH + 58, LCARS_ACCENT_B, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    // Small separator gaps (black)
    Paint_DrawRectangle(0, headerH + 8, sidebarW, headerH + 12, LCARS_BG, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(0, headerH + 34, sidebarW, headerH + 38, LCARS_BG, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(0, headerH + 58, sidebarW, headerH + 62, LCARS_BG, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    const UWORD x0 = sidebarW + pad;
    const UWORD x1 = W - pad;

    // Prepare values
    String hostVal = displayValueOrDash(stripPrefix(hostLine, "host: "));
    String wifiVal = displayValueOrDash(wlanLine);
    String ethVal  = displayValueOrDash(ethLine);
    String oth1Val = displayValueOrDash(otherLine);
    String oth2Val = displayValueOrDash(otherLine2);

    hostVal = ellipsize(hostVal, 24);
    wifiVal = ellipsize(wifiVal, 30);
    ethVal  = ellipsize(ethVal, 30);
    oth1Val = ellipsize(oth1Val, 30);
    oth2Val = ellipsize(oth2Val, 30);

    // Panels
    UWORD y = pad + headerH + 10;

    // Host panel (bigger)
    Paint_DrawRectangle(x0, y, x1, y + 34, LCARS_BG, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(x0, y, x1, y + 34, LCARS_ACCENT_A, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawString_EN(x0 + 8, y + 6, "HOST", &Font12, LCARS_ACCENT_A, LCARS_BG);
    Paint_DrawString_EN(x0 + 8, y + 17, hostVal.c_str(), &Font16, LCARS_TEXT, LCARS_BG);
    y += 40;

    auto drawSmallPanel = [&](const char *label, UWORD borderColor, const String &value) {
        Paint_DrawRectangle(x0, y, x1, y + 26, LCARS_BG, DOT_PIXEL_1X1, DRAW_FILL_FULL);
        Paint_DrawRectangle(x0, y, x1, y + 26, borderColor, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
        Paint_DrawString_EN(x0 + 8, y + 5, label, &Font12, borderColor, LCARS_BG);
        Paint_DrawString_EN(x0 + 70, y + 4, value.c_str(), &Font12, LCARS_TEXT, LCARS_BG);
        y += 30;
    };

    drawSmallPanel("WIFI", LCARS_ACCENT_B, wifiVal);
    drawSmallPanel("ETH",  LCARS_ACCENT_C, ethVal);
    drawSmallPanel("AUX",  LCARS_MUTED,    oth1Val);
    drawSmallPanel("AUX2", LCARS_MUTED,    oth2Val);

    // Footer status bar
    String status;
    if (!hasReceivedData) {
        int dotCount = (millis() / 500) % 3;
        status = "LINK";
        for (int i = 0; i < dotCount + 1; i++) status += ".";
    } else {
        unsigned long elapsedMs = millis() - lastUpdateMs;
        unsigned long secs = elapsedMs / 1000;
        status = "UPDATED " + String(secs) + "s";
    }
    Paint_DrawRectangle(0, H - footerH, W - 1, H - 1, LCARS_BAR, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(8, H - footerH + 3, status.c_str(), &Font12, LCARS_BG, LCARS_BAR);

    LCD_1IN47_Display(frameBuffer);
}

String extractIp(const String &src) {
    int idx = src.indexOf("inet ");
    if (idx < 0) return "";
    idx += 5;
    // Skip any extra spaces
    while (idx < src.length() && src[idx] == ' ') {
        idx++;
    }

    String ipCandidate;
    for (int i = idx; i <= src.length(); i++) {
        char c = (i < src.length()) ? src[i] : ' '; // treat end as delimiter
        bool isIpChar = (c >= '0' && c <= '9') || c == '.';
        if (isIpChar) {
            ipCandidate += c;
        } else {
            if (ipCandidate.length() > 0) {
                if (ipCandidate.indexOf('.') >= 0) {
                    return ipCandidate; // looks like an IP
                }
                ipCandidate = "";
            }
        }
    }
    return "";
}

String extractInterface(const String &line) {
    // ip -o output looks like: "3: wlan0    inet 192.168.1.10/24 ..."
    int colon = line.indexOf(':');
    if (colon < 0) return "";
    int start = colon + 1;
    while (start < line.length() && line[start] == ' ') start++;
    int end = line.indexOf(' ', start);
    if (end < 0) end = line.length();
    return line.substring(start, end);
}

void parseIpLine(const String &line) {
    String cleaned = line;

    // Strip literal escaped ANSI sequences like "\e[31m" or "\033[0m"
    auto stripLiteralAnsi = [](String &s, const char *prefix) {
        int start = s.indexOf(prefix);
        while (start >= 0) {
            int end = s.indexOf('m', start);
            if (end < 0) break;
            s.remove(start, end - start + 1);
            start = s.indexOf(prefix);
        }
    };
    stripLiteralAnsi(cleaned, "\\e[");
    stripLiteralAnsi(cleaned, "\\033[");
    // Also strip actual ESC reset
    cleaned.replace("\x1b[0m", "");

    String noAnsi;
    bool inEsc = false;
    for (size_t i = 0; i < cleaned.length(); i++) {
        char c = cleaned[i];
        if (!inEsc) {
            if (c == 0x1B) { // ESC
                inEsc = true;
                continue;
            }
            if (c >= 32 || c == '\t' || c == ' ') { // keep printable + space/tab
                noAnsi += c;
            }
        } else {
            // End of ANSI CSI sequence when we hit a final byte (@ through ~)
            if (c >= '@' && c <= '~') {
                inEsc = false;
            }
        }
    }

    noAnsi.trim();

    // If this is a lone line with no IP, treat as hostname
    if (extractIp(noAnsi).length() == 0 && noAnsi.length() > 0) {
        hostLine = "host: " + noAnsi;
        drawScreen();
        return;
    }

    String iface = extractInterface(noAnsi);
    String ip = extractIp(noAnsi);

    // Capture hostname from lines like "1: host1: <...>" if present
    if (iface.length() > 0 && iface != "lo" && ip.length() == 0 && noAnsi.indexOf("host") >= 0) {
        hostLine = "host: " + iface;
        drawScreen();
        return;
    }

    if (ip.length() == 0) {
        return; // not an inet line
    }

    bool updated = false;
    String ifaceLower = iface;
    ifaceLower.toLowerCase();
    bool ifaceLooksWifi = ifaceLower.startsWith("wl") || ifaceLower.startsWith("wi") || ifaceLower.indexOf("wi-fi") >= 0 || ifaceLower.indexOf("wifi") >= 0;
    bool ifaceLooksEth = ifaceLower.startsWith("en") || ifaceLower.startsWith("eth") || ifaceLower.indexOf("ethernet") >= 0;

    if (ifaceLooksWifi) {
        wlanLine = iface + ":" + ip;
        updated = true;
    } else if (ifaceLooksEth) {
        ethLine = iface + ":" + ip;
        updated = true;
    } else if (line.length() > 0) {
        String otherVal = (iface.length() > 0 ? iface + ":" : "") + ip;
        if (otherLine.startsWith("other")) {
            otherLine = otherVal;
        } else if (otherLine2.startsWith("other2")) {
            otherLine2 = otherVal;
        } else {
            // Rotate updates between other slots
            otherLine = otherLine2;
            otherLine2 = otherVal;
        }
        updated = true;
    }

    if (updated) {
        hasReceivedData = true;
        lastUpdateMs = millis();
        drawScreen();
    }
}

void triggerHostScript() {
    // Minimize windows first
    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.press('m');
    delay(100);
    Keyboard.releaseAll();
    delay(300);

    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.press('r');
    delay(120);
    Keyboard.releaseAll();
    delay(200);

    typeUS("powershell -NoLogo -NoProfile");
    Keyboard.write(KEY_RETURN);
    delay(900);  // Wait for PowerShell to open

    for (size_t i = 0; i < WINDOWS_PS_LINES; i++) {
        typeUS(WINDOWS_PS_SCRIPT[i]);
        Keyboard.write(KEY_RETURN);  // Newline
        delay(TYPE_LINE_DELAY_MS);  // Small delay between lines
    }
    
    scriptTriggered = true;
    lastTriggerAttempt = millis();
}

void setup() {
    Serial.begin(115200);
    Keyboard.begin();

    delay(1500); // let USB enumerate

    if (DEV_Module_Init() != 0) {
        Serial.println("GPIO init failed");
        return;
    }
    Serial.println("LCD IP Display - Waiting for host data...");
    Serial.println("Run the PowerShell script on your host to send IP info");

    DEV_SET_PWM(0);
    LCD_1IN47_Init(VERTICAL);
    LCD_1IN47_Clear(WHITE);
    DEV_SET_PWM(80);

    UDOUBLE imageSize = LCD_1IN47_HEIGHT * LCD_1IN47_WIDTH * 2;
    frameBuffer = (UWORD *)malloc(imageSize);
    if (!frameBuffer) {
        Serial.println("Frame buffer alloc failed");
        return;
    }

    Paint_NewImage((UBYTE *)frameBuffer, LCD_1IN47.WIDTH, LCD_1IN47.HEIGHT, 0, WHITE);
    Paint_SetScale(65);
    Paint_SetRotate(ROTATE_0);
    Paint_Clear(WHITE);
    drawScreen();
    
    // Trigger script after 3 seconds
    delay(3000);
    triggerHostScript();
}

void loop() {
    // Read any lines sent back from host
    while (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();
        if (line.length() > 0) {
            parseIpLine(line);
        }
    }
    
    // Re-trigger script every 60 seconds if no data
    if (!hasReceivedData && millis() - lastTriggerAttempt > 60000) {
        triggerHostScript();
    }

    // Refresh status every second after first data arrives
    if (hasReceivedData && millis() - lastStatusRenderMs >= 1000) {
        lastStatusRenderMs = millis();
        drawScreen();
    }

    DEV_Delay_ms(200);
}
