#!/usr/bin/env python3
"""Extract complete Arduino examples from current pages and compile their exact text."""
import argparse
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


def blocks(source):
    return re.findall(r'^```(arduino|cpp)\n(.*?)^```\s*$', source, re.M | re.S)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--fqbn', choices=['arduino:avr:uno', 'esp32:esp32:nodemcu-32s'], required=True)
    parser.add_argument('--libraries', type=Path, help='Additional Arduino library directory')
    parser.add_argument('--work-dir', type=Path, help='Retain compiler artifacts here')
    parser.add_argument('--page', action='append', help='Limit checks to these page paths (repeatable)')
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    output = args.work_dir or Path(tempfile.mkdtemp(prefix='awot-docs-sketches-'))
    output.mkdir(parents=True, exist_ok=True)
    cases = []
    for page in sorted((root/'en').rglob('*.md')):
        if args.page and page.relative_to(root).as_posix() not in args.page:
            continue
        if '3x' in page.parts:
            continue
        for index, (_, code) in enumerate(blocks(page.read_text()), 1):
            if '#include' not in code or not re.search(r'void\s+setup\s*\(', code) or not re.search(r'void\s+loop\s*\(', code):
                continue
            wifi = '#include <WiFi.h>' in code
            ethernet = '#include <Ethernet.h>' in code
            if args.fqbn.startswith('arduino:') and wifi:
                continue
            if args.fqbn.startswith('esp32:') and ethernet:
                continue
            variants = [(str(index), code)]
            if page.relative_to(root).as_posix() == 'en/guide/tutorial.md':
                fragments = [text for _, text in blocks(page.read_text())]
                include = next(text for text in fragments if text.strip() == '#include "StaticFiles.h"')
                mount = next(text for text in fragments if 'app.use(staticFiles());' in text)
                previous = '  app.get("/led", readLed);\n  app.put("/led", updateLed);\n  server.begin();'
                replacement = '\n'.join('  '+line for line in mount.strip().splitlines())
                if previous not in code:
                    raise SystemExit('Tutorial setup changed: update static-stage assembly')
                deployed = code.replace('#include <aWOT.h>', '#include <aWOT.h>\n'+include.strip()).replace(previous, replacement)
                variants.append((str(index)+'_static', deployed))
            for label, variant in variants:
                name = page.stem.replace('-', '_') + '_' + label
                sketch = output/name
                sketch.mkdir(parents=True, exist_ok=True)
                (sketch/f'{name}.ino').write_text(variant)
                if '#include "StaticFiles.h"' in variant:
                    converter = root/'scripts/fixtures/static/node_modules/awot-scripts/static/converter.js'
                    node_code = "require(process.argv[1])({sources:process.argv[2],indexFile:'index.html',sketchDir:process.argv[3]}).catch(e=>{console.error(e);process.exit(1)})"
                    subprocess.run(['node', '-e', node_code, str(converter), str(root/'scripts/fixtures/static/assets'), str(sketch)], check=True)
                    if not (sketch/'StaticFiles.h').is_file():
                        raise SystemExit('asset converter did not create StaticFiles.h')
                cases.append((page.relative_to(root), label, sketch))
    # The serial transport and member callback overloads complement the page
    # sketches with the exact repository examples on the same board coverage.
    for name in ('SerialBridge', 'MemberFunction'):
        example = args.library/'examples'/name
        if not args.page and example.exists() and (name == 'SerialBridge' or args.fqbn.startswith('esp32:')):
            cases.append((example, 0, example))
    if not cases:
        raise SystemExit('No sketches found for this board')
    failures = []
    for page, index, sketch in cases:
        print(f'Compiling {page} block {index} for {args.fqbn}', flush=True)
        command = ['arduino-cli', 'compile', '--fqbn', args.fqbn,
                   '--library', str(args.library.resolve()), '--build-path', str(output/(sketch.name+'_build'))]
        if args.libraries:
            command.extend(['--libraries', str(args.libraries.resolve())])
        command.append(str(sketch.resolve()))
        result = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        log = output/(sketch.name+'.log')
        log.write_text(result.stdout)
        if result.returncode:
            failures.append(f'{page} block {index}:\n{result.stdout}')
        else:
            print('\n'.join(line for line in result.stdout.splitlines() if line.startswith(('Sketch uses', 'Global variables'))))
    if failures:
        raise SystemExit('\n'.join(failures))
    print(f'Compiled {len(cases)} documented/repository sketches for {args.fqbn}. Logs: {output}')
    if not args.work_dir:
        shutil.rmtree(output)


if __name__ == '__main__':
    main()
