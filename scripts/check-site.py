#!/usr/bin/env python3
"""Check the rendered site, including redirect destinations and fragment targets."""
import argparse
from collections import Counter
from html.parser import HTMLParser
from pathlib import Path
import re
from urllib.parse import unquote, urljoin, urlsplit


class Document(HTMLParser):
    def __init__(self, text):
        super().__init__()
        self.ids = []
        self.links = []
        self.headings = []
        self.redirect = None
        self.api_version = None
        self.feed(text)

    def handle_starttag(self, tag, attributes):
        attrs = dict(attributes)
        if attrs.get('id'):
            self.ids.append(attrs['id'])
            if tag in ('h2', 'h3'):
                self.headings.append(attrs['id'])
        if tag == 'div' and attrs.get('id') == 'api-doc':
            self.api_version = attrs.get('data-api-version')
        for name in ('href', 'src'):
            if attrs.get(name):
                self.links.append(attrs[name])
        if tag == 'meta' and attrs.get('http-equiv', '').lower() == 'refresh':
            match = re.search(r'url=(.*)', attrs.get('content', ''), re.I)
            if match:
                self.redirect = match[1].strip()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('site', type=Path)
    args = parser.parse_args()
    root = args.site.resolve()
    documents = {p.relative_to(root).as_posix(): Document(p.read_text())
                 for p in root.rglob('*.html')}
    errors = []
    checked = 0
    for path, doc in documents.items():
        duplicates = [key for key, count in Counter(doc.ids).items() if count > 1]
        if duplicates:
            errors.append(f'{path}: duplicate IDs: {duplicates}')
        for link in doc.links:
            parts = urlsplit(urljoin('https://awot.net/' + path, link))
            if parts.scheme not in ('http', 'https') or parts.netloc not in ('awot.net', 'localhost:4000', '127.0.0.1:4000'):
                continue
            target = unquote(parts.path).lstrip('/') or 'index.html'
            if target.endswith('/'):
                target += 'index.html'
            if not (root / target).is_file():
                errors.append(f'{path}: missing target {link}')
                continue
            checked += 1
            seen = set()
            while target in documents and documents[target].redirect:
                if target in seen:
                    errors.append(f'{path}: redirect cycle at {target}')
                    break
                seen.add(target)
                target = urlsplit(urljoin('https://awot.net/' + target, documents[target].redirect)).path.lstrip('/')
            if parts.fragment and target in documents and unquote(parts.fragment) not in documents[target].ids:
                errors.append(f'{path}: missing fragment {link}')
        if doc.api_version == '4x':
            sidebar = {link[1:] for link in doc.links if link.startswith('#')}
            for heading in doc.headings:
                if heading not in sidebar:
                    errors.append(f'{path}: API heading missing from sidebar: {heading}')
    for path in ('en/api.html', 'en/4x/api.html'):
        if documents.get(path) is None or documents[path].api_version != '4x':
            errors.append(f'{path}: default/current API must be v4')
        else:
            for anchor in ('app', 'req', 'res', 'router', 'app-process', 'app-header', 'req-stream', 'ws', 'wsmessage', 'sse', 'storage', 'configuration'):
                if anchor not in documents[path].ids:
                    errors.append(f'{path}: missing required API anchor {anchor}')
    archive = documents.get('en/3x/api.html')
    baseline = Path(__file__).with_name('3x-anchors.txt').read_text().splitlines()
    if archive is None or archive.api_version != '3x':
        errors.append('en/3x/api.html: archive must remain v3')
    elif set(baseline) - set(archive.ids):
        errors.append(f'3.x archive lost anchors: {set(baseline) - set(archive.ids)}')
    aliases = {}
    source_root = Path(__file__).resolve().parent.parent
    for source in source_root.glob('en/**/*.md'):
        front = source.read_text().split('---', 2)[1]
        match = re.search(r'^redirect_from:\s*[\'"]([^\'"]+)', front, re.M)
        if match:
            alias = match[1].lstrip('/')
            if alias in aliases:
                errors.append(f'duplicate redirect source: {alias}')
            aliases[alias] = source.relative_to(source_root).with_suffix('.html').as_posix()
    for source in (source_root/'redirects').glob('*.html'):
        front = source.read_text().split('---', 2)[1]
        alias = re.search(r'^permalink:\s*[\'"]?([^\'"\n]+)', front, re.M)[1].lstrip('/')
        destination = re.search(r'^redirect_target:\s*[\'"]?([^\'"\n]+)', front, re.M)[1].lstrip('/')
        if alias in aliases:
            errors.append(f'duplicate redirect source: {alias}')
        aliases[alias] = destination
    for alias, destination in aliases.items():
        if alias not in documents or not documents[alias].redirect:
            errors.append(f'{alias}: missing redirect')
        elif urlsplit(documents[alias].redirect).path.lstrip('/') != destination:
            errors.append(f'{alias}: incorrect redirect destination')
    if errors:
        raise SystemExit('\n'.join(errors))
    print(f'Checked {len(documents)} HTML pages, {checked} local links, {len(aliases)} aliases, both API versions, and preserved 3.x anchors.')


if __name__ == '__main__':
    main()
