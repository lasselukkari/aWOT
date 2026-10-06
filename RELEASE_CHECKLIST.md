# aWOT 4 documentation release

The `docs/v4` change prepares awot.net to teach aWOT **4.0.0** while retaining the
existing Jekyll theme and `gh-pages` hosting. Publish this change alongside the
library release, after Arduino Library Manager offers the documented version.

## Implementation and local validation

- [x] Default and canonical v4 references share the complete API includes.
- [x] The 3.x API and existing rendered anchors remain available with an archive
  notice; historical generator/static-file pages have labeled archive URLs.
- [x] Migration, starter, core, WebSocket, SSE, configuration, and static-file
  pages teach the verified v4 API and transport lifecycle.
- [x] Jekyll 3.3.1 / github-pages 110 builds with Ruby 2.6.10 and Bundler 2.4.22.
- [x] Rendered checks pass for 38 HTML pages, 1164 local links, 17 aliases,
  sidebar targets, both API versions, and preserved 3.x anchors.
- [x] All 47 partial examples compile with standard callbacks and sanitizers;
  45 compile with Uno-sized buffers and function-pointer callbacks.
- [x] Native behavior suites pass: 24 test executables with documentation
  behavior checks in standard/sanitized builds; 23 in the small-buffer build.
- [x] The publicly available CI source revision passes the native snippet and
  behavior checks as well as the planned local library revision.
- [x] Uno Ethernet starter/configuration and SerialBridge compile with AVR
  1.8.8 and Ethernet 2.0.2.
- [x] Chrome desktop (1440×1000) and touch-mobile (390×844) previews pass
  navigation, edit links, archive notices, API sidebar highlighting, resize
  behavior, Arduino/Bash/JSON/TSX highlighting, fragment redirects, and all five
  tutorial anchors. No JavaScript errors or horizontal overflow were observed.
- [x] The existing tutorial and companion project validation remains applicable
  at LedBlink revision `ece7d3e1ab0a57c977742468af619a201c199959`: clean npm install,
  Vite/TypeScript build, generated asset and LED HTTP behavior, and ESP32 build.
- [ ] Complete the remaining full ESP32 documentation sketch build batch.
- [ ] Review the hosted documentation check workflow results before merging.

The local library baseline is branch `4` at `4117ee51`. CI uses the public
`bb13af5963ad950015312fe73107766056ccd936` revision, whose C++ headers and sources
match that local baseline. The later local commits reorganize development
files/examples and packaging. Update the CI pin to the released source revision
when preparing final publication. Local sketch logs are under
`/private/tmp/awot-docs-uno` and `/private/tmp/awot-docs-esp32`; native/snippet logs
are under `/private/tmp/awot-docs-partials-*`. Browser reports/screenshots are
under `/private/tmp/awot-docs-browser`, with the preview at
<http://127.0.0.1:4173> while its local server remains running.

## Publication prerequisites

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
  `v3.5.0`. The library checkout's uncommitted README work is separate from this
  documentation change.
- [ ] Wait for Arduino Library Manager to index 4.0.0, install that version in a
  clean environment, and compile the starter through that installation.
- [ ] Merge the documentation change into `gh-pages`. GitHub Pages currently
  serves the root of that branch through its existing legacy build path and
  custom domain `awot.net`; no hosting migration is required.
- [ ] Check the live home page, default and canonical v4 references, 3.x archive,
  aliases with fragments, starter and guide pages, and actual Library Manager
  installation. Keep the tutorial's five section anchors and both API menu
  versions working.

The advertised current version is staged as `4.0.0` in `_data/awot.yml`. Keep
that switch with this coordinated publication; the local preview is not a live
release or evidence that Library Manager has indexed the package.
