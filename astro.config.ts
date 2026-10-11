import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { defineConfig } from 'astro/config';
import sitemap from '@astrojs/sitemap';
import tailwindcss from '@tailwindcss/vite';
import icon from 'astro-icon';
import astrowind from './vendor/integration';
import { unified } from '@astrojs/markdown-remark';
import { codeThemes } from './src/utils/code-themes';
import { accessibleMarkdown } from './src/utils/markdown';
import redirects from './src/data/redirects.json';

const root = path.dirname(fileURLToPath(import.meta.url));

export default defineConfig({
  output: 'static',
  build: { format: 'preserve' },
  integrations: [
    sitemap({
      filter: (page) => {
        const pathname = new URL(page).pathname;
        return pathname !== '/en/api' && pathname !== '/en' && pathname !== '/en/' &&
          !redirects.some(({ from }) => from.replace(/\.html$/, '') === pathname);
      },
      serialize: (item) => ({ ...item, url: new URL(item.url).pathname === '/' ? item.url : `${item.url.replace(/\/$/, '')}.html` }),
    }),
    icon({ iconDir: 'src/assets/icons', include: { tabler: ['*'] } }),
    astrowind({ config: './src/config.yaml' }),
  ],
  markdown: {
    processor: unified({ rehypePlugins: [accessibleMarkdown] }),
    shikiConfig: { themes: codeThemes, wrap: false },
  },
  vite: {
    plugins: [tailwindcss()],
    resolve: { alias: { '~': path.resolve(root, 'src') } },
  },
});
