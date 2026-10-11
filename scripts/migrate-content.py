#!/usr/bin/env python3
"""Convert the legacy English Jekyll documentation to Astro Markdown.

Run with --source pointing to a checkout of the former gh-pages branch. The
generated Markdown is the editable source after migration; this utility is kept
to make the original conversion reproducible, not as a site build dependency.
"""

import argparse
import json
from pathlib import Path
import re


DESCRIPTIONS = {
    'starter/hello-world.md': 'Run your first aWOT web server on an ESP32 or an Arduino with Ethernet.',
    'starter/basic-routing.md': 'Register HTTP routes and continue through multiple handlers with aWOT 4.',
    'starter/static-files.md': 'Build and embed a browser application as compressed static files in your Arduino firmware.',
    'guide/routing.md': 'Match HTTP methods and paths, read route parameters, and mount routers in aWOT.',
    'guide/middleware.md': 'Write middleware for logging, authentication, and ordered request handling.',
    'guide/request-data.md': 'Read request headers, query values, route parameters, and HTTP request bodies.',
    'guide/json.md': 'Read and validate JSON requests and send JSON responses with ArduinoJson and aWOT.',
    'guide/connections.md': 'Manage connection reuse, open responses, request errors, timeouts, and buffered writes.',
    'guide/configuration.md': 'Configure aWOT memory buffers, HTTP defaults, callbacks, and channel capacities.',
    'guide/websockets.md': 'Build a WebSocket server with aWOT 4 and manage messages, rooms, and connection ownership.',
    'guide/server-sent-events.md': 'Stream server-sent events from an Arduino and manage clients, heartbeat, and event replay.',
    'guide/migrating-to-v4.md': 'Update aWOT 3 projects for the API, middleware, headers, and connection behavior in aWOT 4.',
    'guide/tutorial.md': 'Build a React and TypeScript LED controller with Vite and serve it from an ESP32 using aWOT.',
    '4x/api.md': 'The aWOT 4 API reference for App, Router, Request, Response, WebSockets, server-sent events, and configuration.',
    '3x/api.md': 'Archived aWOT 3 API reference for Application, Request, Response, and Router.',
    '3x/starter/generator.md': 'Archived instructions for generating an aWOT 3 application.',
    '3x/starter/static-files.md': 'Archived instructions for serving static files with aWOT 3.',
}

INCLUDE = re.compile(r'^[ \t]*\{%\s*include\s+(.+?)\s*%\}[ \t]*$', re.M)
FENCE = re.compile(r'^([ \t]*)(`{3,}|~{3,})(.*)$')


def frontmatter(text):
    """Read the scalar fields used by the legacy pages without a YAML dependency."""
    match = re.match(r'\A---\n(.*?)\n---\n(.*)\Z', text, re.S)
    if not match:
        raise ValueError('Expected YAML frontmatter')
    fields = {}
    for line in match[1].splitlines():
        if ':' not in line or line.lstrip().startswith('#'):
            continue
        key, value = line.split(':', 1)
        value = value.strip()
        if len(value) >= 2 and value[0] == value[-1] and value[0] in ('"', "'"):
            value = value[1:-1]
        fields[key.strip()] = value
    return fields, match[2]


def expand_includes(body, source, stack=()):
    def expand(match):
        name = match[1].replace('{{ page.lang }}', 'en')
        if name in stack:
            raise ValueError(f'Recursive include: {name}')
        path = source / '_includes' / name
        content = path.read_text()
        if name == 'archive-notice.html':
            # Markdown blockquotes use the site's theme without Jekyll's
            # markdown="1" extension or the old doc-box color classes.
            content = re.sub(r'^<div[^>]*>\n?', '', content)
            content = re.sub(r'\n?</div>\s*$', '', content)
            return '\n'.join('> ' + line for line in content.splitlines())
        return expand_includes(content, source, stack + (name,)).rstrip('\n')

    return INCLUDE.sub(expand, body)


def convert_body(body):
    """Remove Jekyll Markdown wrappers and normalize code language labels."""
    converted = []
    active_fence = None
    for line in body.splitlines():
        match = FENCE.match(line)
        if match:
            marker = match[2]
            if active_fence:
                if marker[0] == active_fence[0] and len(marker) >= len(active_fence) and not match[3].strip():
                    active_fence = None
            else:
                active_fence = marker
                language = match[3].strip()
                languages = {'arduino': 'cpp', 'JavaScript': 'javascript'}
                if language in languages:
                    line = match[1] + marker + languages[language]
            converted.append(line)
            continue
        if not active_fence:
            if re.fullmatch(r'\s*<section\s+markdown=[\'"]1[\'"]\s*>\s*', line):
                converted.append('')
                continue
            if re.fullmatch(r'\s*</section>\s*', line):
                converted.append('')
                continue
            if re.fullmatch(r'\s*<div\s+id=[\'"]api-doc[\'"][^>]*>\s*', line):
                converted.append('')
                continue
            if re.fullmatch(r'\s*</div>\s*', line):
                converted.append('')
                continue
        converted.append(line)
        if not active_fence and re.fullmatch(r'<h([1-6])\b[^>]*>.*</h\1>', line):
            # CommonMark treats prose immediately after a raw HTML heading as
            # part of that HTML block. Separate it so Markdown links and inline
            # formatting continue to render as they did under Kramdown.
            converted.append('')
    result = '\n'.join(converted).strip() + '\n'
    result = re.sub(r'\n{4,}', '\n\n\n', result)
    if re.search(r'\{%|\{\{|markdown=[\'"]1', result):
        raise ValueError('Unconverted Liquid or Jekyll Markdown markup')
    return result


def migrate(source, destination):
    docs = destination / 'src/content/docs'
    count = 0
    for path in sorted((source / 'en').rglob('*.md')):
        relative = path.relative_to(source / 'en').as_posix()
        if relative == 'api.md':
            # The current and default API routes render one content entry.
            _, default_body = frontmatter(path.read_text())
            _, current_body = frontmatter((source / 'en/4x/api.md').read_text())
            if default_body != current_body:
                raise ValueError('Default API content differs from the current v4 API')
            continue
        fields, body = frontmatter(path.read_text())
        route = '/' + path.relative_to(source).with_suffix('.html').as_posix()
        metadata = {
            'title': fields['title'],
            'description': DESCRIPTIONS[relative],
            'route': route,
            'section': 'archive' if relative.startswith('3x/') else ('api' if relative.startswith('4x/') else relative.split('/')[0]),
        }
        if relative in ('3x/api.md', '4x/api.md'):
            metadata['apiVersion'] = relative.split('/')[0]
        if relative == '4x/api.md':
            metadata['routeAlias'] = '/en/api.html'
            metadata['canonical'] = route
        body = convert_body(expand_includes(body, source))
        target = docs / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        serialized = '\n'.join(f'{key}: {json.dumps(value, ensure_ascii=False)}' for key, value in metadata.items())
        target.write_text(f'---\n{serialized}\n---\n\n{body}')
        count += 1

    redirects = [{'from': '/en/index.html', 'to': '/'}]
    for path in sorted((source / 'redirects').glob('*.html')):
        fields, _ = frontmatter(path.read_text())
        redirects.append({'from': fields['permalink'], 'to': fields['redirect_target']})
    for path in sorted((source / 'en').rglob('*.md')):
        fields, _ = frontmatter(path.read_text())
        if 'redirect_from' in fields:
            redirects.append({'from': fields['redirect_from'], 'to': '/' + path.relative_to(source).with_suffix('.html').as_posix()})
    redirects.sort(key=lambda item: item['from'])
    if len({item['from'] for item in redirects}) != len(redirects):
        raise ValueError('Duplicate legacy redirect routes')
    target = destination / 'src/data/redirects.json'
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps(redirects, indent=2) + '\n')
    print(f'Migrated {count} documentation entries, the default v4 API alias, and {len(redirects)} redirects.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True, help='Legacy Jekyll gh-pages checkout')
    parser.add_argument('--destination', type=Path, default=Path(__file__).resolve().parent.parent)
    args = parser.parse_args()
    migrate(args.source.resolve(), args.destination.resolve())


if __name__ == '__main__':
    main()
