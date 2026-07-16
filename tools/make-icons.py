#!/usr/bin/env python3
"""Regenerate the whole Whatly icon set from code: the app icons (16-512), the
tray/notification icons with their red count badges, the Windows .ico, and the
scalable + symbolic SVGs. Writes straight into the repo's icon locations.

Usage: python3 tools/make-icons.py

Change the C1/C2 palette below to re-skin the whole identity. Requires Pillow
and a DejaVu Sans Bold font.
"""
import os
from PIL import Image, ImageDraw, ImageFont

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Brand palette (teal gradient). Re-skin the identity here.
C1 = (13, 148, 136)     # #0d9488
C2 = (45, 212, 191)     # #2dd4bf
RIM = (243, 243, 243)
WHITE = (255, 255, 255)
BADGE = (225, 29, 29)    # red count badge

def _bold(size):
    for p in ("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
              "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf"):
        if os.path.exists(p):
            return ImageFont.truetype(p, size)
    return ImageFont.load_default()

def lerp(a, b, t):
    return tuple(round(a[i] + (b[i] - a[i]) * t) for i in range(3))

def bubble(size, ss=8):
    """The Whatly speech-bubble + W, transparent background, at `size` px."""
    S = size * ss
    im = Image.new("RGBA", (S, S), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    cx, cy, R = S*0.5, S*0.47, S*0.40
    rim_w = S*0.028
    def shape(r, col, dr):
        dr.polygon([(cx-r*0.62, cy+r*0.45), (cx-r*1.02, cy+r*0.98), (cx-r*0.10, cy+r*0.80)], fill=col)
        dr.ellipse([cx-r, cy-r, cx+r, cy+r], fill=col)
    shape(R, RIM+(255,), d)
    rb = R - rim_w
    grad = Image.new("RGBA", (S, S), (0, 0, 0, 0)); gd = ImageDraw.Draw(grad)
    top, bot = int(cy-rb), int(cy+rb*2.0)
    for y in range(top, bot):
        gd.line([(0, y), (S, y)], fill=lerp(C1, C2, (y-top)/max(1, bot-top))+(255,))
    mask = Image.new("L", (S, S), 0); shape(rb, 255, ImageDraw.Draw(mask))
    im.paste(grad, (0, 0), mask)
    d = ImageDraw.Draw(im)
    ww, wh = R*1.02, R*0.62
    x0, y0, y1 = cx-ww/2, cy-wh/2, cy+wh/2
    pts = [(x0, y0), (x0+ww*0.24, y1), (x0+ww*0.5, y0+wh*0.34), (x0+ww*0.76, y1), (x0+ww, y0)]
    lw = int(R*0.17)
    d.line(pts, fill=WHITE, width=lw, joint="curve")
    for px, py in pts:
        d.ellipse([px-lw/2, py-lw/2, px+lw/2, py+lw/2], fill=WHITE)
    return im.resize((size, size), Image.LANCZOS)

def with_badge(text):
    S = 64*8
    big = bubble(64, ss=8).resize((S, S), Image.LANCZOS)
    d = ImageDraw.Draw(big)
    bw, bh = S*0.52, S*0.46
    d.rounded_rectangle([S-bw, S-bh, S, S], radius=int(S*0.10), fill=BADGE+(255,))
    f = _bold(int(bh*(0.82 if len(text) == 1 else 0.66)))
    tb = d.textbbox((0, 0), text, font=f)
    d.text(((S-bw)+(bw-(tb[2]-tb[0]))/2-tb[0], (S-bh)+(bh-(tb[3]-tb[1]))/2-tb[1]), text, font=f, fill=WHITE)
    return big.resize((64, 64), Image.LANCZOS)

SVG_APP = '''<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg xmlns="http://www.w3.org/2000/svg" width="256" height="256" viewBox="0 0 256 256">
  <defs>
    <linearGradient id="whatlyTeal" gradientUnits="userSpaceOnUse" x1="128" y1="25.09" x2="128" y2="310.78">
      <stop offset="0" stop-color="#0d9488"/>
      <stop offset="1" stop-color="#2dd4bf"/>
    </linearGradient>
  </defs>
  <g fill="#f3f3f3">
    <path d="M64.51,166.40 L23.55,220.67 L117.76,202.24 Z"/>
    <circle cx="128" cy="120.32" r="102.4"/>
  </g>
  <g fill="url(#whatlyTeal)">
    <path d="M68.96,163.17 L30.87,213.65 L118.48,196.50 Z"/>
    <circle cx="128" cy="120.32" r="95.23"/>
  </g>
  <path d="M75.8,88.6 L100.9,152.1 L128,110.2 L155.1,152.1 L180.2,88.6"
        fill="none" stroke="#ffffff" stroke-width="17.4" stroke-linecap="round" stroke-linejoin="round"/>
</svg>
'''

SVG_SYMBOLIC = '''<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg xmlns="http://www.w3.org/2000/svg" width="64" height="64" viewBox="0 0 64 64"
     style="fill-rule:evenodd;clip-rule:evenodd">
  <path style="fill:#000000" fill-rule="evenodd" d="M 32 5 C 17.098857 5 5 16.969621 5 31.710938 C 5 37.047389 6.5860348 42.020918 9.3144531 46.193359 L 5 59 L 18.179688 54.65625 C 22.223999 57.046126 26.949584 58.419922 32 58.419922 C 46.901143 58.419922 59 46.452254 59 31.710938 C 59 16.969621 46.901143 5 32 5 z M 32 10.308594 C 43.939672 10.308594 53.634766 19.899327 53.634766 31.710938 C 53.634766 43.522547 43.939672 53.109375 32 53.109375 C 27.481058 53.109375 23.28555 51.738327 19.8125 49.390625 L 11.679688 52.072266 L 14.365234 44.101562 C 11.847131 40.603908 10.365234 36.327615 10.365234 31.710938 C 10.365234 19.899327 20.060328 10.308594 32 10.308594 z"/>
  <path d="M 18.95 23.76 L 25.22 39.64 L 32 29.16 L 38.79 39.64 L 45.06 23.76"
        fill="none" stroke="#000000" stroke-width="4.6" stroke-linecap="round" stroke-linejoin="round"/>
</svg>
'''

def w(path, data):
    full = os.path.join(REPO, path)
    os.makedirs(os.path.dirname(full), exist_ok=True)
    if isinstance(data, str):
        open(full, "w").write(data)
    else:
        data.save(full)
    print("  wrote", path)

def main():
    apps = {n: bubble(n) for n in (16, 32, 48, 64, 128, 256, 512)}
    # in-app icon set
    for n in (16, 32, 48, 64, 128, 256, 512):
        w("src/icons/app/icon-%d.png" % n, apps[n])
    w("src/icons/whatly.png", apps[512])
    # notification / tray icons
    w("src/icons/app/notification/whatly.png", apps[64])
    w("src/icons/app/notification/whatly-notify.png", apps[64])
    for n in range(1, 10):
        w("src/icons/app/notification/whatly-notify-%d.png" % n, with_badge(str(n)))
    w("src/icons/app/notification/whatly-notify-10.png", with_badge("+"))
    # desktop hicolor set
    for n in (16, 32, 64, 128, 256):
        w("dist/linux/hicolor/%dx%d/apps/net.shakaran.whatly.png" % (n, n), apps[n])
    w("dist/linux/hicolor/scalable/apps/net.shakaran.whatly.svg", SVG_APP)
    w("dist/linux/hicolor/symbolic/apps/net.shakaran.whatly-symbolic.svg", SVG_SYMBOLIC)
    w("src/icons/app/whatly-symbolic.svg", SVG_SYMBOLIC)
    # debian packaging + snap + windows
    w("debianpkg/data/net.shakaran.whatly.png", apps[256])
    w("debianpkg/data/net.shakaran.whatly-symbolic.svg", SVG_SYMBOLIC)
    w("snap/gui/icon.png", apps[512])
    w("snap/gui/icon.svg", SVG_APP)
    apps[256].save(os.path.join(REPO, "dist/windows/whatly.ico"),
                   sizes=[(16, 16), (32, 32), (64, 64), (128, 128), (256, 256)])
    print("  wrote dist/windows/whatly.ico")

if __name__ == "__main__":
    main()
