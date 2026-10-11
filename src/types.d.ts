import type { AstroComponentFactory } from 'astro/runtime/server/index.js';
import type { ImageMetadata } from 'astro';
import type { HTMLAttributes } from 'astro/types';

export interface Post {
  /** Unique ID identifying the post. */
  id: string;
  /** URL-friendly slug derived from the post name. */
  slug: string;
  /** Fully resolved permalink, computed from the configured pattern. */
  permalink: string;

  publishDate: Date;
  updateDate?: Date;

  title: string;
  /** Optional summary of post content. */
  excerpt?: string;
  image?: ImageMetadata | string;
  /** Alternative text for the cover image. */
  imageAlt?: string;

  category?: Taxonomy;
  tags?: Taxonomy[];
  author?: string;

  metadata?: MetaData;

  draft?: boolean;

  /** Rendered Astro component factory for the post body. */
  Content?: AstroComponentFactory;

  /** Estimated reading time in minutes. */
  readingTime?: number;
}

export interface Taxonomy {
  slug: string;
  title: string;
}

export interface MetaData {
  title?: string;
  ignoreTitleTemplate?: boolean;

  canonical?: string;

  robots?: MetaDataRobots;

  description?: string;

  openGraph?: MetaDataOpenGraph;
  twitter?: MetaDataTwitter;
}

export interface MetaDataRobots {
  index?: boolean;
  follow?: boolean;
}

export interface MetaDataImage {
  url: string;
  width?: number;
  height?: number;
}

export interface MetaDataOpenGraph {
  url?: string;
  siteName?: string;
  images?: Array<MetaDataImage>;
  locale?: string;
  type?: string;
  /** Open Graph `article:*` properties, used when `type` is `article`. */
  article?: {
    publishedTime?: string;
    modifiedTime?: string;
    expirationTime?: string;
    authors?: Array<string>;
    section?: string;
    tags?: Array<string>;
  };
}

export interface MetaDataTwitter {
  handle?: string;
  site?: string;
  cardType?: string;
}

export interface Image {
  src: string;
  alt?: string;
  /** Optional link (used by Brands logos). */
  href?: string;
}

export interface Widget {
  id?: string;
  isDark?: boolean;
  bg?: string;
  classes?: Record<string, string | Record<string, string>>;
}

export interface Headline {
  title?: string;
  subtitle?: string;
  tagline?: string;
  classes?: Record<string, string>;
}

export interface Stat {
  amount?: number | string;
  title?: string;
  icon?: string;
}

export interface Item {
  title?: string;
  description?: string;
  icon?: string;
  classes?: Record<string, string>;
  callToAction?: CallToAction;
  image?: Image;
  /** Makes the whole item a link (used by Features2 cards). */
  href?: string;
}

export type PriceAmount = number | string | { monthly: number | string; yearly: number | string };

export interface Price {
  title?: string;
  subtitle?: string;
  description?: string;
  /** A single amount, or `{ monthly, yearly }` to enable the billing toggle. */
  price?: PriceAmount;
  period?: string | { monthly: string; yearly: string };
  /** Small line under the price, e.g. "2 months free". */
  note?: string;
  items?: Array<Item>;
  callToAction?: CallToAction;
  hasRibbon?: boolean;
  ribbonTitle?: string;
  /** Primary border around the recommended plan. */
  highlight?: boolean;
}

export interface Testimonial {
  title?: string;
  testimonial?: string;
  name?: string;
  job?: string;
  image?: string | unknown;
  /** 1–5 stars. */
  rating?: number;
  /** Company logo: an icon name (`tabler:…`) or an image. */
  logo?: string | Image;
}

export interface Input {
  type: astroHTML.JSX.HTMLInputTypeAttribute;
  name: string;
  label?: string;
  autocomplete?: string;
  placeholder?: string;
}

export interface Textarea {
  label?: string;
  name?: string;
  placeholder?: string;
  rows?: number;
}

export interface Disclaimer {
  label?: string;
}

// COMPONENTS
export interface CallToAction extends Omit<HTMLAttributes<'a'>, 'slot'> {
  variant?: 'primary' | 'secondary' | 'tertiary' | 'link';
  text?: string;
  icon?: string;
  classes?: Record<string, string>;
  type?: 'button' | 'submit' | 'reset';
}

export interface Form {
  inputs?: Array<Input>;
  textarea?: Textarea;
  disclaimer?: Disclaimer;
  button?: string;
  description?: string;
}

// WIDGETS
export interface HeroImage extends Image {
  /** `cover` (default) crops the picture to the hero's frame; `auto` shows it whole at its natural proportions, for illustrations with a transparent background. */
  aspect?: 'cover' | 'auto';
  /** Any other attribute is forwarded to the <img>. */
  [attribute: string]: unknown;
}

export interface Hero extends Omit<Headline, 'classes'>, Omit<Widget, 'isDark' | 'classes'> {
  /** Small pill above the tagline, e.g. "New" or "Beta". */
  badge?: string;
  content?: string;
  actions?: string | CallToAction[];
  image?: string | HeroImage;
}

export interface Stats extends Omit<Headline, 'classes'>, Widget {
  stats?: Array<Stat>;
  /** Animate numeric amounts from 0 when they scroll into view (off with reduced motion). */
  countUp?: boolean;
}

export interface Pricing extends Omit<Headline, 'classes'>, Widget {
  prices?: Array<Price>;
  currency?: string;
  billingLabels?: { monthly: string; yearly: string };
}

export interface Testimonials extends Omit<Headline, 'classes'>, Widget {
  testimonials?: Array<Testimonial>;
  callToAction?: CallToAction;
  /** `grid` (equal rows) or `masonry` (packed "wall"). */
  layout?: 'grid' | 'masonry';
  columns?: number;
}

export interface Brands extends Omit<Headline, 'classes'>, Widget {
  icons?: Array<string>;
  images?: Array<Image>;
  /** Static row (`grid`) or an infinite scrolling row (`marquee`, pauses on hover and with reduced motion). */
  variant?: 'grid' | 'marquee';
  /** Monochrome logos that reveal their colours on hover. */
  grayscale?: boolean;
  /** Light card behind each logo. */
  boxed?: boolean;
}

export interface Features extends Omit<Headline, 'classes'>, Widget {
  image?: string | unknown;
  items?: Array<Item>;
  columns?: number;
  defaultIcon?: string;
  isBeforeContent?: boolean;
  isAfterContent?: boolean;
}

export interface Faqs extends Omit<Headline, 'classes'>, Widget {
  items?: Array<Item>;
  columns?: number;
  /** Render questions as a native accordion (`<details>`); `false` keeps every answer visible. */
  collapsible?: boolean;
  /** Index of the question open by default when collapsible (-1 for none). */
  defaultOpen?: number;
  /** Emit FAQPage structured data (JSON-LD) built from the items. */
  schema?: boolean;
}

export interface Steps extends Omit<Headline, 'classes'>, Widget {
  items?: Array<Item>;
  callToAction?: string | CallToAction;
  image?: string | Image;
  isReversed?: boolean;
}

export interface ContentImage extends Image {
  /** Crop of the picture: `1/1` (default), `4/3`, `3/4` for portraits, or `auto` to keep the natural height without cropping. */
  aspect?: '4/3' | '1/1' | '3/4' | 'auto';
  /** Any other attribute is forwarded to the <img> (e.g. `loading`, `fetchpriority`). */
  [attribute: string]: unknown;
}

export interface Content extends Omit<Headline, 'classes'>, Widget {
  content?: string;
  image?: string | ContentImage;
  items?: Array<Item>;
  columns?: number;
  isReversed?: boolean;
  isAfterContent?: boolean;
  callToAction?: CallToAction;
}

export interface Contact extends Omit<Headline, 'classes'>, Form, Widget {}

export interface ComparisonColumn {
  title: string;
  subtitle?: string;
  /** Highlighted column (the recommended plan, or "us" in versus mode). */
  highlight?: boolean;
  callToAction?: CallToAction;
}

export interface ComparisonRow {
  label: string;
  description?: string;
  /** One value per column: `true`/`false` render as check/cross, strings as text. */
  values: Array<boolean | string | undefined>;
  /** Rows with the same group are listed under a group heading. */
  group?: string;
}

export interface Comparison extends Omit<Headline, 'classes'>, Widget {
  columns?: Array<ComparisonColumn>;
  rows?: Array<ComparisonRow>;
  /** `features`: plan matrix. `versus`: your product against alternatives. */
  mode?: 'features' | 'versus';
  firstColumnLabel?: string;
}

export interface FeatureTab extends Item {
  image?: Image;
}

export interface FeatureTabs extends Omit<Headline, 'classes'>, Widget {
  items?: Array<FeatureTab>;
  /** `vertical`: tabs beside the image. `horizontal`: tabs above it. */
  orientation?: 'vertical' | 'horizontal';
  isReversed?: boolean;
}

export interface BentoItem extends Item {
  /** Columns the card spans (1–3). */
  span?: 1 | 2 | 3;
  rowSpan?: 1 | 2;
}

export interface Bento extends Omit<Headline, 'classes'>, Widget {
  items?: Array<BentoItem>;
  columns?: 3 | 4;
}

export interface Newsletter extends Omit<Headline, 'classes'>, Widget {
  placeholder?: string;
  button?: string;
  disclaimer?: string;
  /** Where to POST the email. Without it the form does nothing (see configure-contact-form.md). */
  action?: string;
  method?: 'post' | 'get';
  name?: string;
  layout?: 'band' | 'card';
}

export interface Quote extends Widget {
  quote?: string;
  name?: string;
  job?: string;
  image?: string | Image;
  logo?: string | Image;
  rating?: number;
}

export interface TeamMember {
  name: string;
  role?: string;
  image?: string | Image;
  bio?: string;
  socials?: Array<{ icon: string; href: string; ariaLabel?: string }>;
}

export interface Team extends Omit<Headline, 'classes'>, Widget {
  members?: Array<TeamMember>;
  columns?: number;
}

export interface TimelineItem {
  date?: string;
  title?: string;
  description?: string;
  icon?: string;
  image?: string | Image;
  /** Filled marker for the current or most important entry. */
  highlight?: boolean;
  /** Faded entry with a dashed connector: a gap, something uncertain, or what comes next. */
  ghost?: boolean;
}

export interface Timeline extends Omit<Headline, 'classes'>, Widget {
  items?: Array<TimelineItem>;
  /** `vertical`: single column. `alternate`: entries alternate around a centre line on large screens. */
  layout?: 'vertical' | 'alternate';
}

export interface GalleryImage extends Image {
  caption?: string;
}

export interface Gallery extends Omit<Headline, 'classes'>, Widget {
  images?: Array<GalleryImage>;
  columns?: 2 | 3 | 4;
  /** Open images in a native <dialog> viewer. */
  lightbox?: boolean;
}

export interface Project {
  title?: string;
  description?: string;
  image?: string | Image;
  tags?: Array<string>;
  href?: string;
  linkText?: string;
  /** A measurable outcome shown with a trend icon, e.g. "+38 % conversions". */
  result?: string;
}

export interface Projects extends Omit<Headline, 'classes'>, Widget {
  items?: Array<Project>;
  columns?: number;
  callToAction?: CallToAction;
}

export interface QuickStartCommand {
  command: string;
  /** Shown above the command as a `# comment`. */
  comment?: string;
}

export interface QuickStart extends Omit<Headline, 'classes'>, Widget {
  /** One or more shell commands, rendered in a terminal-like block with a copy button. */
  commands?: Array<string | QuickStartCommand>;
  /** Prompt character shown before each command (`$` by default). */
  prompt?: string;
  copyLabel?: string;
  copiedLabel?: string;
  /** Two or three facts to read before running the command. */
  items?: Array<Item>;
  actions?: Array<CallToAction>;
  note?: string;
}

export interface Countdown extends Omit<Headline, 'classes'>, Widget {
  /** Target date/time in ISO 8601, e.g. "2026-10-15T09:00:00Z". */
  date?: string;
  labels?: { days: string; hours: string; minutes: string; seconds: string };
  expiredText?: string;
  callToAction?: CallToAction;
}

export interface SocialProofItem {
  /** The headline figure: "4.9/5", "12,000+", "#1". */
  value: string;
  label?: string;
  icon?: string;
  image?: string | Image;
  href?: string;
  /** 1–5, rendered as stars under the value. */
  rating?: number;
  /** Tailwind colour class for the icon, e.g. `text-amber-500` (defaults to the primary colour). */
  color?: string;
}

export interface SocialProof extends Widget {
  items?: Array<SocialProofItem>;
  /** `strip`: a row of figures with dividers (default). `badges`: small rounded chips. */
  layout?: 'strip' | 'badges';
}

export interface Video extends Omit<Headline, 'classes'>, Widget {
  /** A self-hosted video file. Ignored when `youtube` or `vimeo` is set. */
  src?: string;
  /** YouTube video id. */
  youtube?: string;
  /** Vimeo video id. */
  vimeo?: string;
  poster?: string | Image;
  caption?: string;
  /** Accessible name of the player. */
  videoTitle?: string;
}

export interface StickyCTA {
  id?: string;
  text: string;
  href: string;
  /** Short line shown next to the button. */
  label?: string;
  target?: string;
  icon?: string;
  /** Pixels scrolled before the bar slides in. */
  showAfter?: number;
  /** Hide on md screens and up (default true). */
  mobileOnly?: boolean;
}

export interface Integration {
  name: string;
  description?: string;
  icon?: string;
  image?: string | Image;
  href?: string;
}

export interface Integrations extends Omit<Headline, 'classes'>, Widget {
  items?: Array<Integration>;
  columns?: number;
}
