#!/usr/bin/env python3
"""Check the rendered site, including redirect destinations and fragment targets."""
import argparse
from collections import Counter
from html.parser import HTMLParser
import json
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
        if attrs.get('id') == 'api-doc':
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
            if parts.scheme not in ('http', 'https') or parts.netloc not in ('awot.net', 'localhost:4321', '127.0.0.1:4321', 'localhost:4173', '127.0.0.1:4173'):
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
                target = unquote(urlsplit(urljoin('https://awot.net/' + target, documents[target].redirect)).path).lstrip('/') or 'index.html'
                if target.endswith('/'):
                    target += 'index.html'
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
    for source in (source_root/'src/content/docs').rglob('*.md'):
        front = source.read_text().split('---', 2)[1]
        route_match = re.search(r'^route:\s*[\'"]?([^\'"\n]+)', front, re.M)
        if not route_match:
            errors.append(f'{source.relative_to(source_root)}: missing route')
            continue
        destination = route_match[1].strip().lstrip('/')
        if destination not in documents:
            errors.append(f'{source.relative_to(source_root)}: missing rendered route {destination}')
        match = re.search(r'^routeAlias:\s*[\'"]?([^\'"\n]+)', front, re.M)
        if match:
            alias = match[1].strip().lstrip('/')
            if alias not in documents:
                errors.append(f'{source.relative_to(source_root)}: missing rendered alias {alias}')
    for redirect in json.loads((source_root/'src/data/redirects.json').read_text()):
        alias = redirect['from'].lstrip('/')
        destination = redirect['to'].lstrip('/') or 'index.html'
        if alias in aliases:
            errors.append(f'duplicate redirect source: {alias}')
        aliases[alias] = destination
    for alias, destination in aliases.items():
        if alias not in documents or not documents[alias].redirect:
            errors.append(f'{alias}: missing redirect')
        elif (urlsplit(documents[alias].redirect).path.lstrip('/') or 'index.html') != destination:
            errors.append(f'{alias}: incorrect redirect destination')
    tutorial = documents.get('en/guide/tutorial.html')
    for anchor in ('environment', 'setup', 'back-end', 'front-end', 'build'):
        if tutorial is None or anchor not in tutorial.ids:
            errors.append(f'en/guide/tutorial.html: missing historical anchor {anchor}')
    if errors:
        raise SystemExit('\n'.join(errors))
    print(f'Checked {len(documents)} HTML pages, {checked} local links, {len(aliases)} aliases, both API versions, and preserved 3.x anchors.')


if __name__ == '__main__':
    main()
