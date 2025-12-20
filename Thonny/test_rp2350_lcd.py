"""
RP2350 LCD 1.47" Test Script for Thonny MicroPython
Tests LCD display, GPIO, and basic display functions
"""

import machine
import time
from machine import Pin, SPI, I2C

# =====================
# LCD Configuration
# =====================
# Adjust these pins based on your wiring
LCD_WIDTH = 172
LCD_HEIGHT = 320
LCD_RST = Pin(12, Pin.OUT)
LCD_DC = Pin(8, Pin.OUT)
LCD_CS = Pin(9, Pin.OUT)
LCD_BL = Pin(13, Pin.OUT)  # Backlight

# SPI Configuration
SPI_ID = 0
SPI_BAUD = 10_000_000  # 10 MHz
SPI_SCK = Pin(6, Pin.OUT)
SPI_MOSI = Pin(7, Pin.OUT)
SPI_MISO = Pin(4, Pin.IN)

# =====================
# Color Definitions
# =====================
BLACK = 0x0000
WHITE = 0xFFFF
RED = 0xF800
GREEN = 0x07E0
BLUE = 0x001F
YELLOW = 0xFFE0
CYAN = 0x07FF
MAGENTA = 0xF81F

# =====================
# Helper Functions
# =====================

def init_spi():
    """Initialize SPI for LCD communication"""
    spi = SPI(SPI_ID, baudrate=SPI_BAUD, polarity=0, phase=0,
              sck=SPI_SCK, mosi=SPI_MOSI, miso=SPI_MISO)
    print("[LCD] SPI initialized")
    return spi

def reset_lcd():
    """Reset LCD display"""
    LCD_RST.value(1)
    time.sleep_ms(100)
    LCD_RST.value(0)
    time.sleep_ms(100)
    LCD_RST.value(1)
    time.sleep_ms(100)
    print("[LCD] Reset complete")

def write_command(spi, cmd):
    """Send command to LCD"""
    LCD_DC.value(0)  # DC = 0 for command
    LCD_CS.value(0)
    spi.write(bytes([cmd]))
    LCD_CS.value(1)

def write_data(spi, data):
    """Send data to LCD"""
    LCD_DC.value(1)  # DC = 1 for data
    LCD_CS.value(0)
    if isinstance(data, int):
        spi.write(bytes([data]))
    else:
        spi.write(data)
    LCD_CS.value(1)

def init_lcd(spi):
    """Initialize LCD display - ST7789 driver"""
    print("[LCD] Initializing display...")
    
    # Soft reset
    write_command(spi, 0x01)
    time.sleep_ms(150)
    
    # Sleep out
    write_command(spi, 0x11)
    time.sleep_ms(500)
    
    # Set color mode to 16-bit
    write_command(spi, 0x3A)
    write_data(spi, 0x05)
    time.sleep_ms(10)
    
    # MADCTL - Memory data access control
    write_command(spi, 0x36)
    write_data(spi, 0x00)
    time.sleep_ms(10)
    
    # Display on
    write_command(spi, 0x29)
    time.sleep_ms(500)
    
    # Enable backlight
    LCD_BL.value(1)
    
    print("[LCD] Display initialized")

def fill_screen(spi, color):
    """Fill entire screen with a color"""
    # Set column address
    write_command(spi, 0x2A)
    write_data(spi, 0x00)
    write_data(spi, 0x00)
    write_data(spi, (LCD_WIDTH >> 8) & 0xFF)
    write_data(spi, LCD_WIDTH & 0xFF)
    
    # Set row address
    write_command(spi, 0x2B)
    write_data(spi, 0x00)
    write_data(spi, 0x00)
    write_data(spi, (LCD_HEIGHT >> 8) & 0xFF)
    write_data(spi, LCD_HEIGHT & 0xFF)
    
    # Write memory
    write_command(spi, 0x2C)
    LCD_DC.value(1)
    LCD_CS.value(0)
    
    # Convert color to bytes (16-bit big-endian)
    color_bytes = bytes([(color >> 8) & 0xFF, color & 0xFF])
    
    # Fill with color
    for _ in range(LCD_WIDTH * LCD_HEIGHT):
        spi.write(color_bytes)
    
    LCD_CS.value(1)

def draw_rectangle(spi, x, y, width, height, color):
    """Draw filled rectangle"""
    # Set column address
    write_command(spi, 0x2A)
    write_data(spi, (x >> 8) & 0xFF)
    write_data(spi, x & 0xFF)
    write_data(spi, ((x + width - 1) >> 8) & 0xFF)
    write_data(spi, (x + width - 1) & 0xFF)
    
    # Set row address
    write_command(spi, 0x2B)
    write_data(spi, (y >> 8) & 0xFF)
    write_data(spi, y & 0xFF)
    write_data(spi, ((y + height - 1) >> 8) & 0xFF)
    write_data(spi, (y + height - 1) & 0xFF)
    
    # Write memory
    write_command(spi, 0x2C)
    LCD_DC.value(1)
    LCD_CS.value(0)
    
    color_bytes = bytes([(color >> 8) & 0xFF, color & 0xFF])
    
    for _ in range(width * height):
        spi.write(color_bytes)
    
    LCD_CS.value(1)

# =====================
# Test Functions
# =====================

def test_basic_colors(spi):
    """Test basic colors"""
    colors = [
        (BLACK, "BLACK"),
        (RED, "RED"),
        (GREEN, "GREEN"),
        (BLUE, "BLUE"),
        (YELLOW, "YELLOW"),
        (CYAN, "CYAN"),
        (MAGENTA, "MAGENTA"),
        (WHITE, "WHITE"),
    ]
    
    print("\n[TEST] Color Test")
    for color, name in colors:
        print(f"  Displaying {name}...")
        fill_screen(spi, color)
        time.sleep(1)

def test_pattern(spi):
    """Test pattern drawing"""
    print("\n[TEST] Pattern Test")
    
    fill_screen(spi, BLACK)
    time.sleep(0.5)
    
    # Draw colored squares
    colors = [RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE]
    rect_size = 30
    
    for i, color in enumerate(colors):
        x = (i % 4) * 40
        y = (i // 4) * 100
        draw_rectangle(spi, x, y, rect_size, rect_size, color)
    
    time.sleep(2)

def test_diagonal_lines(spi):
    """Test diagonal lines using rectangles"""
    print("\n[TEST] Line Pattern Test")
    
    fill_screen(spi, BLACK)
    
    # Draw diagonal pattern
    for i in range(0, LCD_WIDTH, 10):
        draw_rectangle(spi, i, 0, 2, LCD_HEIGHT, WHITE)
    
    time.sleep(2)

def test_gradient(spi):
    """Test gradient effect"""
    print("\n[TEST] Gradient Test")
    
    fill_screen(spi, BLACK)
    
    # Simple horizontal color gradient simulation
    for x in range(0, LCD_WIDTH, 10):
        color = ((x * 0xF8) // LCD_WIDTH) << 11  # Red gradient
        draw_rectangle(spi, x, 0, 10, LCD_HEIGHT, color)

def test_gpio():
    """Test GPIO pins"""
    print("\n[TEST] GPIO Test")
    
    test_pin = Pin(14, Pin.OUT)
    
    print("  GPIO toggle test (5 times)...")
    for i in range(5):
        test_pin.value(1)
        time.sleep_ms(200)
        test_pin.value(0)
        time.sleep_ms(200)
    
    print("  GPIO test complete")

# =====================
# Main Test Sequence
# =====================

def main():
    """Run all tests"""
    print("=" * 50)
    print("RP2350 LCD Test Script for Thonny")
    print("=" * 50)
    
    try:
        print("\n[INIT] Starting initialization...")
        
        # Initialize SPI
        spi = init_spi()
        time.sleep(0.5)
        
        # Reset LCD
        reset_lcd()
        time.sleep(0.5)
        
        # Initialize LCD
        init_lcd(spi)
        time.sleep(1)
        
        # Run tests
        print("\n" + "=" * 50)
        print("Running Tests")
        print("=" * 50)
        
        test_basic_colors(spi)
        time.sleep(1)
        
        test_pattern(spi)
        time.sleep(1)
        
        test_diagonal_lines(spi)
        time.sleep(1)
        
        test_gradient(spi)
        time.sleep(1)
        
        test_gpio()
        time.sleep(1)
        
        # Final screen
        print("\n[RESULT] All tests completed successfully!")
        fill_screen(spi, GREEN)
        time.sleep(2)
        fill_screen(spi, BLACK)
        
    except Exception as e:
        print(f"\n[ERROR] Test failed: {e}")
        import traceback
        traceback.print_exc()
        # Try to turn screen red on error
        try:
            fill_screen(spi, RED)
        except:
            pass

if __name__ == "__main__":
    main()
