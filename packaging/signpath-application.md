# SignPath Foundation — OSS code-signing application

Ready-to-paste answers for the SignPath Foundation free open-source code-signing
application (<https://about.signpath.io/product/open-source>, "Apply now").
Signing itself, once approved, is documented in [windows-signing.md](windows-signing.md).

> Replace the **(you)** placeholders with your own details before submitting.
> Everything else is factual project information.

---

## Eligibility (why Whatly qualifies)

- **OSI-approved license:** MIT.
- **Public source code:** <https://github.com/shakaran/whatly>.
- **Builds from CI, not a developer's machine:** GitHub Actions builds the Windows
  binary in a clean `windows-2022` runner on every tagged release
  (`.github/workflows/release-artifacts.yml`, job `windows`).
- **Non-commercial / free:** the app is free, has no paid tier and no ads.
- **Tagged, versioned releases:** v6.0.0 → v6.2.0, published on GitHub Releases
  and the Snap Store.

---

## Form answers

**Project name**
> Whatly

**Project website**
> https://shakaran.github.io/whatly/

**Source code repository**
> https://github.com/shakaran/whatly

**License**
> MIT (OSI-approved)

**Short description**
> Whatly is a free, MIT-licensed desktop client for WhatsApp Web, built with Qt 6
> and Qt WebEngine. It wraps WhatsApp Web in a native window and adds chat themes,
> a privacy blur, scheduled messages, multiple accounts, a multi-language spell
> checker, an app lock, a system-tray integration and more. It runs on Linux and
> Windows. It is an actively maintained, independent fork of WhatSie.

**Programming language / platform**
> C++ with Qt 6 (Qt WebEngine), built with CMake. Targets: Linux and Windows
> (x64).

**What do you need to sign?**
> The Windows 64-bit application executable (`whatly.exe`), distributed as a
> `.zip` in GitHub Releases. (No installer/MSI at present.)

**Which operating systems / signing type?**
> Windows Authenticode code signing.

**Build / CI system**
> GitHub Actions. The Windows artifact is produced by the `windows` job in
> `.github/workflows/release-artifacts.yml`, triggered on version tags (`v*`),
> on a clean `windows-2022` runner. No manual/local build steps are involved.

**Link to the build pipeline that produces the artifact**
> https://github.com/shakaran/whatly/blob/main/.github/workflows/release-artifacts.yml

**How are releases published?**
> Git tag → GitHub Actions builds and attaches the Windows zip (and Linux Flatpak
> and AppImage) to a GitHub Release. The snap is published to the Snap Store
> (https://snapcraft.io/whatly). Release cadence: several releases in the current
> cycle (6.0.0, 6.1.0, 6.1.1, 6.2.0).

**Number of users / downloads (approximate)**
> (you) — report your current Snap Store install count / GitHub release download
> totals. Be honest; the Foundation accepts small but genuine OSS projects.

**Is the project commercial? Any monetisation?**
> No. It is free and open source (MIT). Optional donations only
> (Ko-fi / Wise / GitHub Sponsors); no paid features, no advertising.

**Maintainer / responsible person**
> Ángel Guzmán Maeso — https://shakaran.net
> Contact e-mail: (you)

**Team / contributors**
> Primarily maintained by Ángel Guzmán Maeso. Fork of WhatSie by Keshav Bhatt
> (MIT). Contributions are accepted via GitHub pull requests; interface strings
> ship translated into 15 languages.

**Relationship to WhatsApp / trademarks (if asked)**
> Whatly is an unofficial, independent client. It is not affiliated with,
> endorsed by, or connected to WhatsApp or Meta. "WhatsApp" is a trademark of its
> respective owner. The app name and icon are Whatly's own.

**Anything else / notes to reviewers (optional)**
> The repository already contains a secret-gated SignPath signing step
> (`.github/workflows/release-artifacts.yml`) and setup notes
> (`packaging/windows-signing.md`); it activates the moment the SignPath
> credentials are configured, so no pipeline changes are needed after approval.

---

## After you submit

Once approved, follow [windows-signing.md](windows-signing.md):
create the `whatly` project, a `release-signing` policy, then add the
`SIGNPATH_API_TOKEN` secret and `SIGNPATH_ORGANIZATION_ID` variable to the GitHub
repo. The next tagged release will be signed automatically.

## Tips

- If the form asks for a **specific commit or release to sign as a sample**, point
  it at the latest tag (currently `v6.2.0`).
- If it asks whether the build is **reproducible / deterministic**, answer that it
  is produced solely by the public CI workflow from a tagged commit — no local
  artifacts are ever uploaded.
- Keep the description consistent with the repository's `README.md` and the
  AppStream metadata (`dist/linux/net.shakaran.whatly.appdata.xml`).
