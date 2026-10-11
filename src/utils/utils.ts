import { I18N } from 'astrowind:config';

export const formatter: Intl.DateTimeFormat = new Intl.DateTimeFormat(I18N?.language, {
  year: 'numeric',
  month: 'short',
  day: 'numeric',
  timeZone: 'UTC',
});

export const getFormattedDate = (date: Date): string => (date ? formatter.format(date) : '');

/**
 * Shortens text at a word boundary so it fits a limit (meta descriptions are
 * cut by search engines at about 155 characters). Adds an ellipsis when cut.
 */
export const truncateAtWord = (text = '', limit = 155): string => {
  if (text.length <= limit) return text;
  const cut = text.slice(0, limit - 1);
  const lastSpace = cut.lastIndexOf(' ');
  return `${(lastSpace > limit / 2 ? cut.slice(0, lastSpace) : cut).replace(/[\s,;:.]+$/, '')}…`;
};

export const trim = (str = '', ch?: string) => {
  let start = 0,
    end = str.length || 0;
  while (start < end && str[start] === ch) ++start;
  while (end > start && str[end - 1] === ch) --end;
  return start > 0 || end < str.length ? str.substring(start, end) : str;
};

/** Grid column classes for the `columns` prop shared by several widgets. */
export const getColumnsClass = (columns?: number, fallback = ''): string =>
  columns === 4
    ? 'lg:grid-cols-4 md:grid-cols-3 sm:grid-cols-2'
    : columns === 3
      ? 'lg:grid-cols-3 sm:grid-cols-2'
      : columns === 2
        ? 'sm:grid-cols-2'
        : fallback;
