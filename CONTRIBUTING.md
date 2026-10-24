# Contributing to awot.net

This branch contains the website documentation. Library bugs belong in the
[aWOT repository](https://github.com/lasselukkari/aWOT); documentation changes
should target `gh-pages`, with v4 release preparation on `docs/v4`.

## Write against the library

Use public headers, implementation behavior, focused native tests, and verified
repository examples as the authority. Current examples use `App`, explicit
`req.next()`, per-request `Request::Header` capture, `ProcessOptions`, and
caller-managed connection closure. Check boolean query/form/route results and
body read counts. Keep contexts, borrowed header values, and URL/output storage
alive for their actual use. State the board and dependency versions for complete
sketches. Do not infer memory budgets or board support from native test results.

Keep the 3.x API signatures and examples in the archive; link readers to v4 and
migration. Fix archive markup/link defects without converting the archived API.
Use Arduino Library Manager installation in current user instructions. Local
source checkouts support validation and are not the installation workflow.

## Validate changes

Follow the Node.js/Astro preview and check commands in [README.md](README.md).
Before review:

1. Run `npm run check`, build the full Astro site with `npm run build`, and run
   `python3 scripts/check-site.py dist`.
2. Compile changed complete sketches and register new partial examples in a
   `scripts/partial-*.json` manifest. Block numbers count `arduino` and `cpp`
   fences together. A `decl` wrapper compiles file-scope code; a `body` wrapper
   supplies `Request& req`, `Response& res`, `App app`, and a concrete mock client.
   `preamble_blocks` reuses earlier code blocks in the same file. Set
   `std_function: true` for member/capturing callback registrations.
3. Run native, sanitizer, and small-buffer/function-pointer configurations when
   adding or changing snippet harnesses. The workflows compile the exact page
   examples rather than separately maintained sketch copies.
4. Preview desktop and mobile navigation, API sidebar scrolling, notices, code
   highlighting, edit links, and redirects with fragment hashes.
5. Include the validation commands, library revision, board/dependency versions,
   and any release prerequisites in the review description.

The link checker verifies local links/fragments, duplicate IDs, sidebar coverage,
alias destinations, both current references, and the previously rendered 3.x
anchors recorded in `scripts/3x-anchors.txt`. External links need separate review
when changed. Keep all five historical tutorial anchors: `environment`, `setup`,
`back-end`, `front-end`, and `build`.

## Content structure and translations

Use Astro layouts and native Markdown content collections. Current pages live in
`src/content/docs/guide/` and `src/content/docs/starter/`; the expanded reference
lives in `src/content/docs/4x/api.md`. Preserve frontmatter `route`, heading IDs,
and redirect aliases in `src/data/redirects.json`. Add new current guide pages to
`src/navigation.ts`. The API sidebar is generated from headings; its rendered
wrapper declares the API version for validation and archive notices.

The native reference is shared by `/en/api.html` and `/en/4x/api.html` through
its `routeAlias`. Partial-example manifests point to the same source and count
all `cpp`/`arduino` fences, including signatures. Update block numbers when
inserting or removing a preceding fence; retain the behavior-handler checks.
The archived 3.x reference has its own expanded Markdown source and stable IDs.

For a translation, create a language-specific content directory using its ISO
639-1 code, copy the relevant current pages and versioned references, and update
the content collection loader, routes, navigation, and layout language together.
Verify translated internal links. Retain a clearly labeled archive menu wherever
archived references are translated; preserve historical URLs during updates.

Content is licensed under [CC BY-SA 3.0 US](https://creativecommons.org/licenses/by-sa/3.0/us/).
See [LICENSE.md](LICENSE.md) for the full license.
