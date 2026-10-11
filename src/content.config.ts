import { defineCollection } from 'astro:content';
import { z } from 'astro/zod';
import { glob } from 'astro/loaders';

const docs = defineCollection({
  loader: glob({ pattern: '**/*.md', base: './src/content/docs' }),
  schema: z.object({
    title: z.string(),
    description: z.string(),
    route: z.string(),
    section: z.enum(['starter', 'guide', 'api', 'archive']),
    apiVersion: z.enum(['3x', '4x']).optional(),
    canonical: z.string().optional(),
    routeAlias: z.string().optional(),
  }),
});
export const collections = { docs };
