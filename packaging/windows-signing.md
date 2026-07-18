# Windows code signing (SignPath Foundation)

Unsigned Windows builds trigger a SmartScreen "unknown publisher" warning on
first run. [SignPath Foundation](https://signpath.org/) runs a **free code
signing programme for open-source projects** that removes it. This is issue #325.

Signing is **optional and off by default**: the release workflow's signing steps
are skipped until the two settings below are present, so nothing here affects a
build until you deliberately enable it.

## One-time application

1. Apply to the SignPath Foundation OSS programme:
   <https://about.signpath.io/product/open-source>. Whatly qualifies (OSI/MIT
   licensed, public repository, tagged releases).
2. Once approved, in the SignPath web console create — or have the Foundation
   provision — for this project:
   - a **project** with slug `whatly`;
   - a **signing policy** with slug `release-signing`;
   - an **artifact configuration** matching a single `.exe` (or a zip containing
     it), if your policy requires one.
3. Note your **organization ID** (a GUID shown in the console).
4. Create a **CI user / API token** scoped to submit signing requests.

## Wire it into GitHub

In the repository's **Settings → Secrets and variables → Actions** add:

| Kind     | Name                        | Value                              |
| -------- | --------------------------- | ---------------------------------- |
| Secret   | `SIGNPATH_API_TOKEN`        | the SignPath CI API token          |
| Variable | `SIGNPATH_ORGANIZATION_ID`  | your SignPath organization GUID    |

That is all. On the next tagged release the `windows` job in
`.github/workflows/release-artifacts.yml` will:

1. upload the freshly built `whatly.exe`,
2. submit it to SignPath and wait for the signed result,
3. write the signed `.exe` back over `build\Release\whatly.exe`,
4. package and attach it to the GitHub release as usual.

If the token is ever removed, the job silently falls back to shipping an unsigned
build — no workflow edit needed.

## Notes

- SignPath signs in the cloud with an HSM-held certificate; there is no
  certificate file to download or store, which is why signing runs in CI rather
  than locally.
- The slugs above (`whatly`, `release-signing`) must match what you create in the
  SignPath console. If you choose different names, update them in the two
  `Code-sign with SignPath` / `signing-policy-slug` fields of the workflow.
