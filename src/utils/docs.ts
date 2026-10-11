import type { MarkdownHeading } from 'astro';

export function tableOfContents(body: string, headings: MarkdownHeading[]) {
  const clean = body.replace(/^```[^\n]*\n[\s\S]*?^```\s*$/gm, '');
  const items: MarkdownHeading[] = [];
  const expression = /<h([23])\b[^>]*\bid=["']([^"']+)["'][^>]*>(.*?)<\/h\1>|^(#{2,3})\s+(.+)$/gm;
  for (const match of clean.matchAll(expression)) {
    if (match[1]) {
      items.push({ depth: Number(match[1]), slug: match[2], text: match[3].replace(/<[^>]+>/g, '') });
    } else {
      const text = match[5].replace(/[`*_]/g, '').replace(/\[([^\]]+)\]\([^)]*\)/g, '$1');
      const heading = headings.find((item) => item.depth === match[4].length && item.text === text);
      if (heading) items.push(heading);
    }
  }
  return items;
}
