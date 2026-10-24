#!/usr/bin/env python3
"""Compile partial examples and run focused behavior/native suites for v4 docs."""
import argparse
import json
from pathlib import Path
import re
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--work-dir', type=Path, default=None)
    parser.add_argument('--configuration', choices=['native', 'sanitized', 'small'], default='native')
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    library = args.library.resolve()
    tests = library/'.extras/test'
    if not tests.exists():
        tests = library/'test'
    output = args.work_dir or Path(tempfile.mkdtemp(prefix='awot-docs-partials-'))
    output.mkdir(parents=True, exist_ok=True)
    includes = '#include <aWOT.h>\n#include <WsChannel.h>\n#include <SseChannel.h>\n#include <SharedClient.h>\n#include <MockStream.h>\n'
    cases = []
    for manifest in sorted((root/'scripts').glob('partial-*.json')):
        for case in json.loads(manifest.read_text()):
            if args.configuration == 'small' and case.get('std_function'):
                continue
            fences = re.findall(r'^```(?:arduino|cpp)\n(.*?)^```\s*$', (root/case['file']).read_text(), re.M | re.S)
            code = fences[case['block']-1]
            preamble = case.get('preamble', '') + '\n'
            for block in case.get('preamble_blocks', []):
                preamble += fences[block-1] + '\n'
            if case['wrapper'] == 'body':
                preamble = 'App app;\nSharedClient client;\n' + preamble
                code = 'void snippet(Request& req, Response& res) {\n' + code + '\n}\n'
            filename = f'snippet_{len(cases)}.cpp'
            (output/filename).write_text(includes+preamble+code)
            cases.append(filename)
    if not cases:
        raise SystemExit('No snippet manifests found')
    behavior = includes + '\n#include <cassert>\n'
    # Run the documented query/parameter/authentication handlers, not copies.
    request_fences = re.findall(r'^```(?:arduino|cpp)\n(.*?)^```\s*$', (root/'src/content/docs/4x/api.md').read_text(), re.M | re.S)
    for code in request_fences:
        if any(f'void {name}(' in code for name in ('authenticate', 'search', 'item')):
            behavior += code+'\n'
    behavior += '''
static SharedClient request(const char* input) {
  SharedClient client;
  client.feed(std::string(input));
  return client;
}
void setup() {}
void loop() {}
int main(int, char**) {
  {
    App app;
    app.get("/search", search);
    SharedClient good=request("GET /search?q=Ada+Lovelace HTTP/1.0\\r\\n\\r\\n");
    app.process(&good);
    assert(good.outputString().find("Ada Lovelace") != std::string::npos);
    SharedClient missing=request("GET /search HTTP/1.0\\r\\n\\r\\n");
    app.process(&missing);
    assert(missing.outputString().find("400 Bad Request") != std::string::npos);
  }
  {
    App app;
    app.get("/items/:id", item);
    SharedClient good=request("GET /items/a+b HTTP/1.0\\r\\n\\r\\n");
    app.process(&good);
    assert(good.outputString().find("a+b") != std::string::npos);
  }
  {
    App app;
    app.use(authenticate);
    app.get("/search", search);
    char authorization[64];
    Request::Header captures[] = {{"Authorization", authorization}};
    SharedClient denied=request("GET /search?q=secret HTTP/1.0\\r\\n\\r\\n");
    app.process(&denied, captures);
    assert(denied.outputString().find("401 Unauthorized") != std::string::npos);
    assert(denied.outputString().find("secret") == std::string::npos);
    SharedClient allowed=request("GET /search?q=public HTTP/1.0\\r\\nAuthorization: Bearer example\\r\\n\\r\\n");
    app.process(&allowed, captures);
    assert(allowed.outputString().find("public") != std::string::npos);
    assert(authorization[0] == '\\0');
  }
  return 0;
}
'''
    (output/'behavior.cpp').write_text(behavior)
    definitions = ['DEBUG', 'EPOXY_CORE_ESP8266']
    flags = []
    if args.configuration == 'small':
        definitions += ['AWOT_NO_STD_FUNCTION_MIDDLEWARE', 'SERVER_URL_BUFFER_SIZE=64', 'SERVER_OUTPUT_BUFFER_SIZE=32', 'SERVER_PUSHBACK_BUFFER_SIZE=32']
        flags += ['-DAWOT_NO_STD_FUNCTION_MIDDLEWARE=ON', '-DAWOT_SMALL_BUFFERS=ON']
    if args.configuration == 'sanitized':
        flags += ['-DAWOT_ENABLE_SANITIZERS=ON']
    # Object libraries use aWOT's own native shim and target configuration.
    cmake = f'''cmake_minimum_required(VERSION 3.18)
project(awot-docs LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
enable_testing()
add_subdirectory("{tests.as_posix()}" library-tests)
add_library(documented-snippets OBJECT {' '.join(cases)})
add_executable(documented-behavior behavior.cpp)
foreach(target documented-snippets documented-behavior)
  target_include_directories(${{target}} PRIVATE "{library.as_posix()}/src" "{tests.as_posix()}/epoxy" "{tests.as_posix()}/mocks")
  target_compile_definitions(${{target}} PRIVATE {' '.join(definitions)})
  if(AWOT_ENABLE_SANITIZERS)
    target_compile_options(${{target}} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
    target_link_options(${{target}} PRIVATE -fsanitize=address,undefined -fno-sanitize-recover=all)
  endif()
endforeach()
target_link_libraries(documented-behavior PRIVATE aWOT-test-aWOT aWOT-test-EpoxyDuino)
add_test(NAME documented-behavior COMMAND documented-behavior)
'''
    (output/'CMakeLists.txt').write_text(cmake)
    build = output/'build'
    subprocess.run(['cmake','-S',str(output),'-B',str(build),*flags],check=True)
    subprocess.run(['cmake','--build',str(build),'-j','8'],check=True)
    subprocess.run(['ctest','--test-dir',str(build),'--output-on-failure'],check=True)
    print(f'Compiled {len(cases)} partial examples and passed behavior/native checks ({args.configuration}).')


if __name__ == '__main__':
    main()
