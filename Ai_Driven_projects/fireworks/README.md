Diwali Fireworks — Interactive Canvas Demo

Files
- index.html — the demo page
- style.css — styles and UI
- fireworks.js — main animation logic

How to run
1. Open `index.html` in a modern browser (Chrome, Edge, Firefox). No server required.
2. Click or tap anywhere to launch a firework aiming at the horizontal position of your click.
3. Use the UI to change pattern (Peony, Willow, Crackle), particle count, and gravity.

Configuration
- Edit values in the UI, or change defaults in `fireworks.js` under `config`.
- Sounds are optional and use the WebAudio API — your browser might require a user gesture to enable audio.

Notes
- The animation uses a single `<canvas>` for performance.
- The code is intentionally small and dependency-free. Feel free to extend it with more patterns, tighter physics, or preloaded audio files for richer sound.

Enjoy and Happy Diwali!