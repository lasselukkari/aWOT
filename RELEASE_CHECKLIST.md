# aWOT 4 documentation release

The `docs/v4` change prepares awot.net to teach aWOT **4.0.0**, using AstroWind's
design and Astro's static build with `gh-pages` hosting. Publish this change
alongside the library release, after Arduino Library Manager offers the
documented version.

## Implementation and local validation

- [x] Default and canonical v4 references share the complete native Markdown API.
- [x] The 3.x API and existing rendered anchors remain available with an archive
  notice; historical generator/static-file pages have labeled archive URLs.
- [x] Migration, starter, core, WebSocket, SSE, configuration, and static-file
  pages teach the verified v4 API and transport lifecycle.
- [x] Add the native macOS guide and carousel target. The bundled example at
  `4117ee51` builds, passes its shim/socket/HTTP tests, and returns the documented
  200 and 404 responses. Validation used an isolated source copy with port
  18080 substituted in `HelloWorld.cpp` and the Makefile to avoid the existing
  local server on 8080; no dependency download was needed.
- [x] Add the native Windows guide, navigation, carousel target, starter link,
  and `/guide/windows.html` alias beside macOS. The library port at `dbe250ff`
  cross-compiles with MinGW and passes all three socket/shim/HTTP tests under
  Wine, including concurrent request handling. Native Windows CI is configured;
  its first hosted run remains pending until the library commit is pushed.
- [x] Validate the Windows documentation additions: Astro check reports no
  errors or warnings; the production build and site checker pass for 43 pages,
  2907 local links, and 21 aliases. The exact guide program compiles and links
  with MinGW. Chrome desktop/mobile checks pass for navigation, code highlighting,
  the Windows carousel entry, and the fragment-preserving redirect, with no
  runtime errors or failed requests. Preview captures are under
  `/private/tmp/awot-windows-docs-preview`.
- [x] Add the native Linux guide, navigation, starter link, desktop-guide
  crosslinks, and `/guide/linux.html` alias. The `.extras/LinuxNative` example
  reuses the POSIX socket adapter and Arduino API through Linux client/server
  aliases, with a CMake build and socket/shim/HTTP checks.
- [x] Build and run the Linux example at `be979fae` in a Debian Linux Docker
  container with GCC 14.4 and Python 3.13: all three socket/shim/HTTP checks
  pass. The first hosted Linux CI run remains pending until the library changes
  are pushed.
- [x] Compile and link the exact Linux guide program with GCC 14.4 on Linux,
  then pass its HTTP smoke test. The documented Ninja configure/build commands
  also pass. Astro check reports no errors or warnings; the production build
  and site checker pass in an isolated copy of the staged changes for 45 pages,
  3094 local links, and 22 aliases.
- [x] Chrome checks at 1440px and 390px pass Linux navigation, code highlighting,
  edit links, layout width, and the fragment-preserving redirect. The homepage
  carousel order is Arduino, macOS, Windows, Linux, then boards, with all desktop
  platforms included in the accessible heading. No runtime errors or failed
  requests were observed.
- [x] The current generator page and its menu entry are removed. Project setup
  lives in Hello world, and both legacy generator URLs redirect there. The
  historical 3.x generator page remains labeled as an archive.
- [x] Before the Astro migration, Jekyll 3.3.1 / github-pages 110 built with
  Ruby 2.6.10 and Bundler 2.4.22; this toolchain has now been replaced.
- [x] Before the Astro migration, rendered checks passed for 38 HTML pages,
  1110 local links, 18 aliases,
  sidebar targets, both API versions, and preserved 3.x anchors.
- [x] On 2026-10-08, all 45 current partial examples compiled with standard
  callbacks and sanitizers; 43 compiled with Uno-sized buffers and
  function-pointer callbacks.
- [x] Native behavior suites pass: 24 test executables with documentation
  behavior checks in standard/sanitized builds; 23 in the small-buffer build.
- [x] The publicly available CI source revision passes the native snippet and
  behavior checks as well as the planned local library revision.
- [x] Uno Ethernet starter/configuration and SerialBridge compile with AVR
  1.8.8 and Ethernet 2.0.2.
- [x] Before the Astro migration, Chrome desktop (1440×1000) and touch-mobile
  (390×844) previews passed
  navigation, edit links, archive notices, API sidebar highlighting, resize
  behavior, Arduino/Bash/JSON/TSX highlighting, fragment redirects, and all five
  tutorial anchors. No JavaScript errors or horizontal overflow were observed.
- [x] The existing tutorial and companion project validation remains applicable
  at LedBlink revision `ece7d3e1ab0a57c977742468af619a201c199959`: clean npm install,
  Vite/TypeScript build, generated asset and LED HTTP behavior, and ESP32 build.
- [x] All 13 ESP32 documentation/repository sketch cases compile with core
  3.3.10 and ArduinoJson 7.4.3, including both tutorial stages, middleware,
  request context/storage, keep-alive/open responses, channels, static files,
  SerialBridge, and MemberFunction. The compiler caught the old handler name
  `index` colliding with a platform overload; the corrected `home` handler
  compiled in the focused middleware rerun.
- [x] ESP32 client ownership is explicit in the connections, API, migration, and
  channel documentation. The pinned core closes a socket when its last client
  owner is destroyed; result flags do not retain it. The keep-alive sketch now
  retains six client slots with individual idle timers and processes requests
  synchronously. Both connection sketches compile in the focused ESP32 rerun;
  native checks exercised the pool with the real HTTP parser and mocked clients.
  The guide includes a two-request curl command for checking reuse on the board;
  no hardware runtime test is claimed. Logs are under
  `/private/tmp/awot-docs-connection-pool-compilation` and
  `/private/tmp/awot-pool-check`.
- [ ] Review hosted documentation check workflow results for the Astro migration
  after pushing the current changes. On 2026-10-10, both earlier Jekyll workflow
  runs ([37503121958](https://github.com/lasselukkari/aWOT/actions/runs/37503121958)
  and [37503113202](https://github.com/lasselukkari/aWOT/actions/runs/37503113202))
  passed all six jobs: site, three native configurations, Uno, and ESP32.
  Those results do not validate the pending Astro changes. The local workflow
  removes the duplicate push trigger so PR updates run once.

The current channel API and CI library baseline is the published revision
`d671a0cb466b5e3c6824e6044a20c4e7b8b7b13b`, with unified SSE/WebSocket targeting
and shared room/write helpers. Earlier validation entries used local revision
`4117ee51` and the matching public revision
`bb13af5963ad950015312fe73107766056ccd936`; those results are historical.
Update the CI pin to the released source revision when preparing final
publication. Earlier local sketch logs are under
`/private/tmp/awot-docs-uno`, `/private/tmp/awot-docs-esp32`,
`/private/tmp/awot-docs-esp32-fixed`, and
`/private/tmp/awot-docs-esp32-final-tutorial`; native/snippet logs
are under `/private/tmp/awot-docs-partials-*`. Browser reports/screenshots are
under `/private/tmp/awot-docs-browser`, with the preview at
<http://127.0.0.1:4173> while its local server remains running.

## Astro migration validation

- [x] Replace Jekyll, Liquid includes, the old logo, and Prism with AstroWind
  components and native Markdown documentation.
- [x] Update snippet manifests and Arduino extraction to read
  `src/content/docs/`, including the expanded v4 reference.
- [x] Replace the site CI job with Node.js 22.22.3, `npm ci`, Astro checks,
  production build, and rendered checks against `dist/`.
- [x] Prepare a GitHub Actions Pages workflow to build and deploy `dist/` only
  from `gh-pages`, preserving `awot.net`.
- [x] Run Astro checks and the production build: zero errors, warnings, or hints;
  41 HTML pages, 2728 local links, 20 redirect aliases, both current references,
  archived API IDs, and all five tutorial anchors pass. The sitemap omits
  redirects and the default API alias.
- [x] Revalidate documented snippets after remapping their native Markdown sources:
  on 2026-10-08, 45 exact snippets and 24 suites passed in native/sanitized
  configurations; 43 snippets and 23 suites passed with small buffers and
  function-pointer callbacks.
- [x] Compile all three Uno documentation/repository sketches from their migrated
  paths with AVR 1.8.8 and Ethernet 2.0.2. Logs are under
  `/private/tmp/awot-astro-sketches-uno`.
- [x] Compile the updated custom-buffer configuration sketch on Uno, using
  separate static URL and output arrays. Logs are under
  `/private/tmp/awot-docs-custom-buffers`.
- [x] Compile ten migrated ESP32 documentation sketches with core 3.3.10 and
  ArduinoJson 7.4.3, with no failures. The redundant full
  rerun was intentionally halted during the next sketch. At that migration
  checkpoint, the code bodies matched the earlier successful 13-case validation;
  the remaining three cases were not rerun. Subsequent connection and
  configuration changes received the focused compilation checks recorded above.
  Partial logs are under
  `/private/tmp/awot-astro-sketches-esp32`.
- [x] Preview the AstroWind design: all 12 browser checks pass for desktop/mobile,
  light/dark themes, navigation, references, syntax highlighting, fragment
  redirects, blocked local storage, and mobile browsing without JavaScript.
  Fragment targets sit 100px below the viewport top, clearing the 70px sticky
  header. No runtime errors were observed. Reports/screenshots are under `/private/tmp/awot-astro-browser`.

## Publication prerequisites

- [x] On 2026-10-11 the user authorized pushing the local Astro documentation
  changes and duplicate CI trigger fix to `docs/v4`, updating draft
  [PR #170](https://github.com/lasselukkari/aWOT/pull/170). This supersedes the
  earlier no-push instruction for those changes. Leave the draft unmerged and
  the live site unchanged until separately approved.
- [ ] Obtain explicit approval for the remote companion merge. The plan's
  [LedBlink PR #40](https://github.com/lasselukkari/LedBlink/pull/40) remains open,
  mergeable, at the verified revision above. Automatic approval review rejected
  the requested merge because it interpreted the user authorization as limited
  to the aWOT documentation update. No merge was performed.
- [ ] Merge the companion PR and review the tutorial's project link. The link
  currently targets the verified immutable revision and is valid before/after
  merging.
- [ ] Publish the aWOT `4.0.0` library package from the reviewed release revision.
  GitHub currently has no `v4.0.0` tag/release; the latest listed release is
  `v3.5.0`. The current official Arduino library index was also checked and
  does not contain aWOT 4.0.0. The library checkout's uncommitted README work is separate from this
  documentation change.
- [ ] Ensure the published library source includes `.extras/MacOSNative`,
  `.extras/WindowsNative`, `.extras/LinuxNative`, and the bundled ArduinoCore-API
  before publishing the native guides. All three guides use a Git checkout of
  the repository because release archives omit `.extras`; the earlier
  public source baseline has the macOS example under `examples/MacOSNative` and
  no Windows or Linux adapter.
- [ ] Require the library's native Windows CI job to pass at the published
  source revision; the local Wine run complements that check.
- [ ] Require the library's native Linux CI job to pass at the published source
  revision; the local Debian Linux Docker checks complement that check.
- [ ] Wait for Arduino Library Manager to index 4.0.0, install that version in a
  clean environment, and compile the starter through that installation.
- [ ] Change the repository Pages source from its legacy branch build to
  **GitHub Actions** as part of approved publication; retain custom domain
  `awot.net` and `public/CNAME`.
- [ ] Merge the documentation change into `gh-pages`. The new Pages workflow
  builds, validates, uploads, and deploys the Astro `dist/` artifact.
- [ ] Check the live home page, default and canonical v4 references, 3.x archive,
  aliases with fragments, starter and guide pages, and actual Library Manager
  installation. Keep the tutorial's five section anchors and both API menu
  versions working.

The native Markdown pages and homepage stage the current version as `4.0.0`. Keep
that switch with this coordinated publication; the local preview is not a live
release or evidence that Library Manager has indexed the package.

## Unified channel API validation

- [x] Pin native/snippet and sketch CI to the published channel API revision
  `d671a0cb466b5e3c6824e6044a20c4e7b8b7b13b`.
- [x] Document directed `send()`, room-targeted `to()`, and `broadcast()`;
  integer and named rooms; SSE client IDs and connection callbacks; room
  capacity/storage; and migration from the earlier pre-release v4 SSE helper.
- [x] Compile 45 partial examples and pass all 24 native/behavior CTest checks.
  Compile 43 partial examples and pass all 23 checks with function pointers
  and small buffers. Logs are `/private/tmp/awot-unified-docs-native.log` and
  `/private/tmp/awot-unified-docs-small.log`.
- [x] Compile the exact complete SSE and WebSocket guide sketches for ESP32.
  Logs are under `/private/tmp/awot-unified-docs-sketches`. These are compile
  checks; no hardware runtime test is claimed.
- [x] Run Astro checks with zero errors, warnings, or hints, build the production
  site, and validate 45 HTML pages, 3107 local links, 22 aliases, both API
  versions, and preserved 3.x anchors.
