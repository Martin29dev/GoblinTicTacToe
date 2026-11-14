"""Tworzy ikone gry z Twojego obrazka assets/Icon/source.jpg:
  1. wycina srodkowa (srednia) wersje ikony,
  2. usuwa tlo-szachownice (przezroczystosc),
  3. zapisuje assets/Icon/app.ico (wiele rozmiarow) i assets/Png/Icon.png.
Wymaga:  pip install pillow numpy scipy      Uruchom:  python tools/generate_icon.py
"""
import os
import numpy as np
from PIL import Image
from scipy import ndimage

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
SOURCE = os.path.join(ROOT, "assets", "Icon", "source.jpg")
CROP_BOX = (1590, 500, 2050, 940)          # gdzie w obrazku lezy ikona (lewo, gora, prawo, dol)
ICO_SIZES = [(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]

def remove_checkerboard(img):
    """Szachownica = jasnoszare, prawie bezbarwne piksele. Obiekt ma czarny kontur, wiec
    wszystko szare polaczone z brzegiem (i dziura w kolku O) to tlo."""
    rgb = np.asarray(img.convert("RGB")).astype(int)
    grayish = (rgb.max(axis=2) - rgb.min(axis=2) < 22) & (rgb.min(axis=2) > 120)
    grayish = ndimage.binary_closing(grayish, iterations=1)

    labels, count = ndimage.label(grayish)
    background = np.zeros_like(grayish)
    edge_labels = set(labels[0]) | set(labels[-1]) | set(labels[:, 0]) | set(labels[:, -1])
    sizes = ndimage.sum(grayish, labels, index=range(1, count + 1))
    for lab in range(1, count + 1):
        if lab in edge_labels or sizes[lab - 1] > 1200:      # tlo przy brzegu albo duza "dziura" (O)
            background |= labels == lab

    alpha = ~ndimage.binary_dilation(background, iterations=1)            # zjedz 1 px szarej obwodki
    alpha = ndimage.gaussian_filter(alpha.astype(float), 0.9)             # wygladz krawedz
    # wyrzuc drobne pylki (male oderwane plamki), zostaw tylko duze elementy ikony
    parts, n = ndimage.label(alpha > 0.03)
    areas = ndimage.sum(alpha > 0.03, parts, index=range(1, n + 1))
    keep = np.isin(parts, [i + 1 for i, a in enumerate(areas) if a > 800])
    alpha = alpha * keep
    out = img.convert("RGBA")
    out.putalpha(Image.fromarray((np.clip(alpha, 0, 1) * 255).astype(np.uint8)))
    return out

def to_square(img, pad=0.04):
    box = img.getchannel("A").point(lambda a: 255 if a > 8 else 0).getbbox()
    img = img.crop(box)
    side = int(max(img.size) * (1 + 2 * pad))
    canvas = Image.new("RGBA", (side, side), (0, 0, 0, 0))
    canvas.alpha_composite(img, ((side - img.width) // 2, (side - img.height) // 2))
    return canvas

if __name__ == "__main__":
    icon = to_square(remove_checkerboard(Image.open(SOURCE).crop(CROP_BOX)))
    icon = icon.resize((256, 256), Image.LANCZOS)
    os.makedirs(os.path.join(ROOT, "assets", "Png"), exist_ok=True)
    icon.save(os.path.join(ROOT, "assets", "Png", "Icon.png"))
    icon.save(os.path.join(ROOT, "assets", "Icon", "app.ico"), sizes=ICO_SIZES)
    # podglad: na jasnym i ciemnym tle + male rozmiary
    sheet = Image.new("RGBA", (780, 300), (255, 255, 255, 255))
    sheet.paste(Image.new("RGBA", (390, 300), (30, 30, 40, 255)), (390, 0))
    sheet.alpha_composite(icon, (67, 22)); sheet.alpha_composite(icon, (457, 22))
    x = 340
    ico = Image.open(os.path.join(ROOT, "assets", "Icon", "app.ico"))
    for s in (48, 32, 16):
        ico.size = (s, s); sheet.alpha_composite(ico.copy().convert("RGBA"), (x, 230)); x += s + 12
    sheet.save("/home/claude/icon_preview.png")
    print("zapisano assets/Icon/app.ico i assets/Png/Icon.png")
