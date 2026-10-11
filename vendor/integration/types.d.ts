declare module 'astrowind:config' {
  // Inline `import()` types: a relative `import` statement is not allowed
  // inside an ambient module declaration.
  export const SITE: import('./utils/configBuilder').SiteConfig;
  export const I18N: import('./utils/configBuilder').I18NConfig;
  export const METADATA: import('./utils/configBuilder').MetaDataConfig;
  export const APP_BLOG: import('./utils/configBuilder').AppBlogConfig;
  export const UI: import('./utils/configBuilder').UIConfig;
  export const ANALYTICS: import('./utils/configBuilder').AnalyticsConfig;
}
