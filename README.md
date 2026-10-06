# awot.net documentation

This branch contains the Jekyll documentation hosted at [awot.net](https://awot.net)
from the `gh-pages` branch of [lasselukkari/aWOT](https://github.com/lasselukkari/aWOT).
The current pages describe **aWOT 4.0.0**. The previous API stays at
`/en/3x/api.html`; `/en/api.html` and `/en/4x/api.html` share the v4 includes.

## Preview and build

The retained Gemfile pins `github-pages` to **110**, including Jekyll **3.3.1**.
Use **Ruby 2.6.10** and **Bundler 2.4.22**, the tested local/CI combination.
`Gemfile.lock` records the transitive dependencies and macOS/Linux platforms.
This pin preserves the existing site toolchain; it does not track the latest
GitHub Pages gem release.

With Ruby 2.6.10 active, run from this checkout:

```bash
gem install bundler --version 2.4.22
bundle _2.4.22_ install
bundle _2.4.22_ exec jekyll serve --host 127.0.0.1
```

Open <http://127.0.0.1:4000>. For a static preview and link checks:

```bash
bundle _2.4.22_ exec jekyll build --destination .docs-build/site
python3 scripts/check-site.py .docs-build/site
```

If the system Ruby gem directories are read-only, set `GEM_HOME` to a writable
user or temporary directory, add its `bin` directory to `PATH`, and use
`bundle config set --local path /a/writable/gem-directory`. The validated macOS
preview keeps the Ruby gems and build output under `/private/tmp`; no upgrade
to the Gemfile is needed. The older Jekyll dependencies can print RubyGems,
Logger, and optional Faraday retry warnings; a successful build and the checks
below are the validation results.

## Check the documented examples

Keep a separate aWOT library checkout available. Local validation used library
branch `4` at `4117ee51`. CI pins the publicly available revision
`bb13af5963ad950015312fe73107766056ccd936`; its C++ sources and headers are identical
to that local baseline. The script supports the native tests in either `test/`
or `.extras/test/` to accommodate their relocation in the local checkout.
Update the CI source revision when releasing the final library package.

Partial examples are extracted from the Markdown using `scripts/partial-*.json`,
compiled against the actual headers, and wrapped in representative native
contexts. The same run executes aWOT's behavior tests and checks the documented
authentication, decoded query, and route-parameter handlers with real HTTP input.
Python 3, CMake 3.18+, and a C++11 compiler are required.

```bash
python3 scripts/check-snippets.py --library /path/to/aWOT --configuration native
python3 scripts/check-snippets.py --library /path/to/aWOT --configuration sanitized
python3 scripts/check-snippets.py --library /path/to/aWOT --configuration small
```

`small` sets function-pointer callbacks and Uno-sized URL/output/pushback buffers
consistently in all translation units; it excludes member-binding examples.
`sanitized` enables AddressSanitizer and UndefinedBehaviorSanitizer.

Complete Arduino code blocks are compiled directly from the current English
pages. The static-file fixture uses the same pinned converter as the tutorial;
the generated header is included in the firmware compilation. Install Node.js
22.12+ and Arduino CLI, then prepare the pinned board/dependency versions:

```bash
npm ci --ignore-scripts --prefix scripts/fixtures/static
arduino-cli config init --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install arduino:avr@1.8.8 esp32:esp32@3.3.10
arduino-cli lib install Ethernet@2.0.2 ArduinoJson@7.4.3
python3 scripts/check-sketches.py --library /path/to/aWOT --fqbn arduino:avr:uno
python3 scripts/check-sketches.py --library /path/to/aWOT --fqbn esp32:esp32:nodemcu-32s
```

Use `--libraries /path/to/additional/libraries` for an isolated Ethernet install,
and `--work-dir /path/to/builds` to retain logs. The checker compiles both tutorial stages and includes the library's
SerialBridge example on Uno and ESP32, and MemberFunction on ESP32. Native tests
do not establish support for additional boards. The library's
[CompileExamples workflow](https://github.com/lasselukkari/aWOT/blob/4/.github/workflows/CompileExamples.yml)
provides its wider example/board matrix.

`.github/workflows/docs-check.yml` runs the site/link checks, all three native
configurations, and both representative Arduino board builds. The historical
`npm test` command only lints JavaScript in Markdown with old ESLint; it is not
a firmware or site validation command.

## Editing

Shared pages live in `en/starter/` and `en/guide/`. Current API content lives in
`_includes/api/en/4x/`; both current reference pages include `reference.md`.
Preserve IDs in headings and add matching sidebar links in `menu.md`. Archive
content lives in `_includes/api/en/3x/` and `en/3x/`.

Aliases are explicit pages under `redirects/` with a `permalink` and
`redirect_target`, using `_layouts/redirect.html`. This preserves the URL hash
for old method and tutorial section links. The pinned `jekyll-redirect-from`
plugin remains installed; version 0.11's generated redirects do not preserve
fragments. Give each alias exactly one destination.

Use fenced `arduino` or `cpp` blocks for C++, and `bash`, `json`, `typescript`,
`jsx`, or `tsx` for those languages. Prism 1.15.0 includes matching grammars.
Jekyll processes Liquid inside code fences: avoid adjacent `{{` in C++
initializers (use `{ { ... } }` spacing) or protect literal text with Liquid raw
blocks and account for them in the snippet extractor.

See [CONTRIBUTING.md](CONTRIBUTING.md) for editing checks and
[RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md) for the coordinated v4 release.
