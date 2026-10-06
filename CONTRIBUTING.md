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

Follow the pinned Ruby/Bundler preview and check commands in [README.md](README.md).
Before review:

1. Build the full Jekyll site and run `scripts/check-site.py` on its output.
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

Use the established Jekyll layouts, Markdown, and Liquid includes. Keep stable
URLs, heading IDs, and redirect aliases. Add new current guide pages to
`_includes/header/header-en.html`; current API headings also need sidebar links
in `_includes/api/en/4x/menu.md`. The rendered reference declares its API version
explicitly for edit-link generation.

For a translation, create a language directory using its ISO 639-1 code, copy
the relevant current pages and versioned references, and set `lang`, `title`,
and navigation for that language. Supply corresponding header/footer and API
includes and verify translated internal links. Retain a clearly labeled archive
menu wherever archived references are translated; do not remove historical URLs
as part of a current-version update.

Content is licensed under [CC BY-SA 3.0 US](https://creativecommons.org/licenses/by-sa/3.0/us/).
See [LICENSE.md](LICENSE.md) for the full license.
