# Branding & screenshot tools

Scripts that regenerate Whatly's icons and the README images, so the visuals can
be rebuilt from source instead of hand-edited.

## Dependencies

```bash
sudo apt-get install imagemagick fonts-dejavu-core kde-spectacle
pip install pillow websockets
```

## Icons

Regenerate the whole icon set (app icons 16–512, tray/notification icons with
count badges, the Windows `.ico`, and the scalable + symbolic SVGs) from the
teal design in code:

```bash
python3 tools/make-icons.py
```

Change the `C1`/`C2` palette at the top of `make-icons.py` to re-skin the
identity, then rebuild (`cmake --build build`).

## README images

The README uses composed "feature cards": a teal-gradient background + a
rounded, shadowed screenshot + the logo + a bold title.

```bash
# Settings + About cards (safe: a disposable profile, no personal data)
tools/make-screenshots.sh

# Also the chat card, from your real logged-in session (privacy blur on,
# extra regions blurred, settings restored afterwards)
tools/make-screenshots.sh --with-chat
```

Notes:
- **WhatsApp Web renders in your account language** (set on your phone), not the
  app's. Switch your phone's WhatsApp to English first for an English chat shot.
- Always **eyeball `tools/shots/chat.png`** before publishing — confirm every
  name, avatar and label pill is blurred.
- To recompose the cards from existing screenshots without recapturing:
  `python3 tools/make-cards.py`

## Pieces

| File | What it does |
|------|--------------|
| `make-icons.py` | Draws the full icon set + SVGs into the repo's icon locations |
| `make-cards.py` | Composes `tools/shots/*.png` into `docs/img/*` feature cards |
| `make-screenshots.sh` | Captures Settings/About (throwaway) and the chat (real session, blurred) |
| `cdp-screenshot.py` | Screenshots the WhatsApp Web content over CDP, with optional blur regions |
| `cdp-eval.py` | Evaluates a JS expression in the page (used to wait for the chat list) |
