import os
import sys

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    print("Error: Pillow is not installed.")
    print("Please run: pip install Pillow")
    sys.exit(1)

# Configuration
THEME_NAME = "cyberpunk"
OUTPUT_DIR = f"examples/themes/{THEME_NAME}"
WIDTH = 240
HEIGHT = 135

# Cyberpunk Palette (RGB)
COLOR_BG = (0, 0, 0)
COLOR_GRID = (0, 50, 0)
COLOR_TEXT = (0, 255, 0)
COLOR_ACCENT = (255, 0, 128)

PAGES = [
    "boot",
    "identity",
    "resources",
    "status",
    "sd",
    "thermal",
    "network"
]

def ensure_dir(path):
    if not os.path.exists(path):
        os.makedirs(path)

def draw_grid(draw, width, height, spacing=20):
    for x in range(0, width, spacing):
        draw.line([(x, 0), (x, height)], fill=COLOR_GRID, width=1)
    for y in range(0, height, spacing):
        draw.line([(0, y), (width, y)], fill=COLOR_GRID, width=1)

def generate_image(name):
    img = Image.new('RGB', (WIDTH, HEIGHT), color=COLOR_BG)
    draw = ImageDraw.Draw(img)
    
    draw_grid(draw, WIDTH, HEIGHT)
    
    # Draw Borders
    draw.rectangle([(0, 0), (WIDTH-1, HEIGHT-1)], outline=COLOR_ACCENT, width=2)
    
    # Draw Title
    text = name.upper()
    
    # Simple centered text
    # We don't have a guaranteed font, so we use default or try to load one
    try:
        # Try to load a "tech" font if available, else default
        font = ImageFont.truetype("DejaVuSans-Bold.ttf", 20)
    except IOError:
        font = ImageFont.load_default()

    # Calculate text size (rudimentary for default font)
    bbox = draw.textbbox((0, 0), text, font=font)
    text_w = bbox[2] - bbox[0]
    text_h = bbox[3] - bbox[1]
    
    x = (WIDTH - text_w) / 2
    y = (HEIGHT - text_h) / 2
    
    draw.text((x, y), text, font=font, fill=COLOR_TEXT)
    
    # Save
    filename = f"{name}.jpg"
    path = os.path.join(OUTPUT_DIR, filename)
    img.save(path, "JPEG", quality=90)
    print(f"Generated: {path}")

def main():
    ensure_dir(OUTPUT_DIR)
    print(f"Generating '{THEME_NAME}' theme assets in {OUTPUT_DIR}...")
    
    for page in PAGES:
        generate_image(page)
        
    print("Done. Copy the folder to your SD card under /themes/")

if __name__ == "__main__":
    main()
