---
format: 1920x1080
message: "WhatsApp Web, as a real desktop app — themed, private, and on every Linux desktop"
arc: Logo hook → Hero app → Signature feature → Feature montage → Available everywhere
audience: Linux desktop users browsing the Snap Store
mode: autonomous
music: upbeat
---

## Video direction

Brand-forward, upbeat, fast. Teal gradient grounds everywhere (#0d9488 → #064e46),
accent #2dd4bf, ink white. Every card is a rounded, shadowed surface that enters
with real motion — never a static cut to a still. Titles are kinetic (slam/wipe
on the beat), set in the display font. Consistent motion grammar: content pushes
in from depth or wipes in directionally, holds briefly, then hands off with a
crossfade/wipe. Keep 12–16px of breathing room; nothing touches the frame edge.
No narration — cut and reveal to the music's pulse (~120 BPM feel).

## Frame 1 — Logo hook

- scene: The teal Whatly logo springs in with a glow, the "Whatly" wordmark wipes in beside it, tagline fades under
- duration: 2.6s
- poster: 2s
- transition_in: cut
- status: animated
- asset_candidates: logo.png
- src: compositions/frames/01-logo.html

Blueprint: logo-assemble-lockup. Shots: [0.0–0.9] logo scales from 0.6→1.0 with spring-pop-entrance + ambient-glow-bloom accent halo; [0.7–1.4] "Whatly" wordmark wipes in left→right beside it; [1.3–2.0] tagline "A feature-rich desktop client for WhatsApp Web" fades up beneath; [2.0–2.6] gentle hold, halo breathes (sine-wave-loop).

## Frame 2 — The app, dark

- scene: The full app window (dark theme) flies in from depth with a slight 3D tilt, then squares to camera
- duration: 3s
- poster: 2.4s
- transition_in: crossfade
- status: animated
- asset_candidates: card-main-dark.png
- src: compositions/frames/02-hero-dark.html

Blueprint: device-surface-showcase. Shots: [0.0–0.9] card-main-dark enters via orbit-3d-entry (Z push from far, −12° Y-rotation) settling flat with 3d-text-depth-layers; [0.8–1.4] kinetic title "A native desktop window" slams in (kinetic-beat-slam) top-left; [1.4–3.0] slow 1.0→1.04 push (multi-phase-camera), subtle depth-of-field-blur on the ground.

## Frame 3 — The app, light

- scene: The window cross-dissolves to the light theme; a "light or dark" caption snaps in
- duration: 2.8s
- poster: 2.2s
- transition_in: wipe
- status: animated
- asset_candidates: card-main-light.png
- src: compositions/frames/03-hero-light.html

Blueprint: device-surface-showcase. Shots: [0.0–0.7] horizontal wipe reveals card-main-light (scale-swap-transition from the dark hero); [0.7–1.3] caption "Light or dark — or follow your desktop" wipes in; [1.3–2.8] gentle drift + hold.

## Frame 4 — Scheduled messages

- scene: The Scheduled-messages dialog rises, a row highlights, a bold "Schedule messages" title punches in
- duration: 3.4s
- poster: 2.6s
- transition_in: crossfade
- status: animated
- asset_candidates: card-scheduled.png
- src: compositions/frames/04-scheduled.html

Blueprint: device-surface-showcase + kinetic-type-beats. Shots: [0.0–0.8] card-scheduled rises from lower third with vertical-spring-ticker ease + spring-pop-entrance; [0.8–1.5] title "Schedule messages" slams in (kinetic-beat-slam); [1.5–2.2] subtitle "sent even if the app was closed" reveals (discrete-text-sequence); [2.2–3.4] soft push-in hold.

## Frame 5 — Themes & privacy

- scene: Two cards (chat themes, privacy blur) parallax past each other
- duration: 3s
- poster: 2.2s
- transition_in: cut
- status: animated
- asset_candidates: card-themes.png, card-chat.png
- src: compositions/frames/05-themes.html

Blueprint: spatial-pan-stations. Shots: [0.0–1.0] card-themes and card-chat enter on split-tilt-cards parallax (different depth/speed, ±6° tilt); [0.8–1.4] title "Make it yours — 14 themes, privacy blur" wipes in; [1.4–3.0] slow lateral pan (motion-blur-streak on entry only).

## Frame 6 — Accounts & lock

- scene: card-accounts and card-lock stack and rotate in
- duration: 3s
- poster: 2.2s
- transition_in: crossfade
- status: animated
- asset_candidates: card-accounts.png, card-lock.png
- src: compositions/frames/06-accounts.html

Blueprint: grid-card-assemble. Shots: [0.0–0.9] card-accounts pushes in (orbit-3d-entry), card-lock overlaps behind with a +8° tilt (split-tilt-cards); [0.9–1.5] title "Multiple accounts, locked down" slams in; [1.5–3.0] hold with a slight parallax settle.

## Frame 7 — Spell-check & shortcuts

- scene: card-spellcheck and card-shortcuts swipe through
- duration: 3s
- poster: 2.2s
- transition_in: wipe
- status: animated
- asset_candidates: card-spellcheck.png, card-shortcuts.png
- src: compositions/frames/07-power.html

Blueprint: spatial-pan-stations. Shots: [0.0–0.8] card-spellcheck swipes in from right (scale-swap-transition); [0.8–1.6] card-shortcuts swaps in from left; [1.0–1.6] title "Spell-check in 31 languages · every shortcut" wipes in; [1.6–3.0] hold.

## Frame 8 — Tray & notifications

- scene: card-tray and card-notifications settle side by side
- duration: 3s
- poster: 2.2s
- transition_in: cut
- status: animated
- asset_candidates: card-tray.png, card-notifications.png
- src: compositions/frames/08-desktop.html

Blueprint: grid-card-assemble. Shots: [0.0–1.0] card-tray and card-notifications ease in together (depth-scatter-assemble into a two-up); [1.0–1.6] title "A real desktop citizen — tray, notifications, watchdog" slams in; [1.6–3.0] gentle push-in hold.

## Frame 9 — Available everywhere

- scene: card-installers rises; logo + "snap install whatly" CTA lock in; final hold
- duration: 4s
- poster: 3s
- transition_in: crossfade
- status: animated
- asset_candidates: card-installers.png, logo.png
- src: compositions/frames/09-outro.html

Blueprint: cta-morph-press + logo-assemble-lockup. Shots: [0.0–1.0] card-installers rises and presents the formats (spring-pop-entrance); [1.0–2.0] card recedes as the logo + "Whatly" wordmark return center (logo-assemble-lockup); [2.0–2.8] CTA "snap install whatly" slams in (kinetic-beat-slam) with "snapcraft.io/whatly" beneath; [2.8–4.0] confident final hold, accent underline draws (svg-path-draw).
