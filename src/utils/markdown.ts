interface Node { type: string; tagName?: string; properties?: Record<string, unknown>; children?: Node[]; }

export function accessibleMarkdown() {
  return (tree: Node) => {
    function visit(node: Node) {
      if (!node.children) return;
      node.children = node.children.map((child) => {
        visit(child);
        if (child.tagName === 'pre') {
          child.properties = { ...child.properties, tabindex: 0, 'aria-label': 'Code example' };
        }
        if (child.tagName === 'table') {
          return { type: 'element', tagName: 'div', properties: { className: ['table-scroll'], tabindex: 0, role: 'region', 'aria-label': 'Documentation table' }, children: [child] };
        }
        return child;
      });
    }
    visit(tree);
  };
}
