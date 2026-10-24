# awot.net documentation

This branch contains the Astro documentation for [awot.net](https://awot.net),
hosted from the `gh-pages` branch of
[lasselukkari/aWOT](https://github.com/lasselukkari/aWOT). The current pages
describe **aWOT 4.0.0**. The previous API stays at `/en/3x/api.html`;
`/en/api.html` and `/en/4x/api.html` render the same current reference.

The design uses [AstroWind by arthelokyo](https://github.com/arthelokyo/astrowind),
pinned to revision `14e1a691f80548dcc36370847b1a02c0d0b12821` (template
`1.0.0-beta.66`, Astro 7 and Tailwind CSS 4). Its MIT license retains the
original onWidget copyright in [licenses/ASTROWIND.md](licenses/ASTROWIND.md).
The documentation content retains its
[CC BY-SA 3.0 US license](LICENSE.md).

## Preview and build

Use **Node.js 22.22.3 or newer**. `package-lock.json` pins the site dependencies.
Run from this checkout:

```bash
npm ci
npm run dev
```

Open <http://localhost:4321>. To check and preview the production output:

```bash
npm run check
npm run build
python3 scripts/check-site.py dist
npm run preview
```

`npm run check` runs Astro’s TypeScript and component checks. Astro builds a
fully static site into `dist/`. The site checker validates local
links and fragments, duplicate IDs, current API navigation, redirect aliases,
all preserved 3.x API anchors, and the tutorial's five historical section IDs.

## Editing

Current guides and starters live in `src/content/docs/guide/` and
`src/content/docs/starter/`. The v4 reference is native Markdown in
`src/content/docs/4x/api.md`; its `routeAlias` renders the default API URL from
the same source. Archive pages live in `src/content/docs/3x/`.

Each document's frontmatter declares its stable `.html` route. Preserve the
explicit heading IDs: they keep old fragment links working. API navigation is
built from the reference headings. Site navigation lives in `src/navigation.ts`.
Historical URL redirects are listed once in `src/data/redirects.json`; the
Astro redirect page preserves fragment hashes.

Use fenced `cpp` blocks for C++, and `bash`, `json`, `typescript`, `jsx`, or
`tsx` for those languages. Astro's Shiki highlighter renders code at build
time, with `awot-light` and `awot-dark` palettes registered in `astro.config.ts`.
Customize their blue, violet, and teal colors in `src/utils/code-themes.ts`.
Code colors
follow the site's selected theme; no external highlighting script is needed.
Markdown is native Astro content, so C++ initializers do not need Liquid escapes.

## Check the documented examples

Keep a separate aWOT library checkout available. Use revision
`d6098a8d79a01d547ff96ea43bfb20d70743286f` for local validation of the current
documentation, including chunked request parsing. The script supports
native tests in either `test/` or `.extras/test/` to accommodate their relocation.

Both CI source pins in `.github/workflows/docs-check.yml` use this published
revision. Keep them aligned with the documented behavior and update them again
when publishing the final package.

The native [macOS guide](src/content/docs/guide/macos.md),
[Windows guide](src/content/docs/guide/windows.md), and
[Linux guide](src/content/docs/guide/linux.md) use a Git checkout of the
repository; their `.extras` adapters are omitted from library
packages. Windows example validation used library revision `dbe250ff`: MinGW
cross-compilation and all three CTest checks passed under Wine. The library's
Windows CI job runs the same checks on Windows once that revision is pushed.
The documentation build does not itself establish native Windows runtime support.

The Linux example at library revision `be979fae` reuses the macOS POSIX adapter
and bundled ArduinoCore API through `LinuxClient` and `LinuxServer` aliases.
Its CMake project builds the application and socket/shim/HTTP checks on Linux.
Run those checks with
`ctest --test-dir .extras/LinuxNative/build --output-on-failure` from the library
checkout after configuring and building it as described in the guide. The
Linux example built with GCC 14.4 and passed all three checks in a Debian Linux
Docker container using Python 3.13 on 2026-10-11. The library's Linux native CI
job runs the same checks; its first hosted run is pending. The exact guide
program also compiled and linked on Linux and passed its HTTP smoke test.
A documentation build or compilation on macOS does not establish Linux runtime
support.

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

Complete Arduino code blocks are compiled directly from the current Markdown
pages, including the expanded API reference. The static-file fixture uses the
same pinned converter as the tutorial; the generated header is included in the
firmware compilation. Install Arduino CLI, then prepare the pinned versions:

```bash
npm ci --ignore-scripts --prefix scripts/fixtures/static
arduino-cli config init --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install arduino:avr@1.8.8 esp32:esp32@3.3.10
arduino-cli lib install Ethernet@2.0.2 ArduinoJson@7.4.3
python3 scripts/check-sketches.py --library /path/to/aWOT --fqbn arduino:avr:uno
python3 scripts/check-sketches.py --library /path/to/aWOT --fqbn esp32:esp32:nodemcu-32s
```

Use `--page src/content/docs/guide/tutorial.md` to check one page,
`--libraries /path/to/additional/libraries` for an isolated Ethernet install,
and `--work-dir /path/to/builds` to retain logs. The checker compiles both tutorial
stages and includes the library's SerialBridge example on Uno and ESP32, and
MemberFunction on ESP32. Native tests do not establish support for additional
boards. The library's
[CompileExamples workflow](https://github.com/lasselukkari/aWOT/blob/master/.github/workflows/CompileExamples.yml)
provides its wider example/board matrix.

`.github/workflows/docs-check.yml` runs the Astro checks, production build,
site/link checks, all three native configurations, and both representative
Arduino board builds.

## GitHub Pages

`.github/workflows/deploy-pages.yml` builds and validates `dist/`, uploads that
artifact, and deploys it on a push to `gh-pages` or a manual run on that branch.
GitHub Pages must use **GitHub Actions** as its source for this Astro build.
Keep the custom domain **awot.net** in the repository's Pages settings and in
`public/CNAME`, which Astro copies into `dist/`. The site configuration uses
`https://awot.net` and a root base path, preserving existing `.html` URLs.

A local build or a push to `docs/v4` does not publish or alter the current site.
On 2026-10-11 the user authorized pushing the documentation changes and CI fix
to draft PR #170. Merging and live publication still require separate approval
and the coordinated prerequisites in [RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md). See
[CONTRIBUTING.md](CONTRIBUTING.md) for editing checks.
