// Blue and violet syntax colors follow AstroWind's palette in both modes.
// Code blocks are highlighted at build time; no browser highlighter is needed.
const scopes = {
  comment: ['comment', 'punctuation.definition.comment'],
  keyword: ['keyword', 'storage.type', 'storage.modifier'],
  string: ['string', 'punctuation.definition.string'],
  number: ['constant.numeric', 'constant.language', 'constant.character'],
  type: ['entity.name.type', 'support.type', 'support.class'],
  function: ['entity.name.function', 'support.function'],
  property: ['variable.other.property', 'entity.other.attribute-name'],
  tag: ['entity.name.tag'],
};

function theme(name: string, type: 'light' | 'dark', colors: Record<keyof typeof scopes | 'foreground' | 'background', string>) {
  return {
    name,
    type,
    colors: { 'editor.foreground': colors.foreground, 'editor.background': colors.background },
    tokenColors: Object.entries(scopes).map(([key, scope]) => ({
      scope,
      settings: { foreground: colors[key as keyof typeof scopes] },
    })),
  };
}

export const codeThemes = {
  light: theme('awot-light', 'light', {
    foreground: '#243047', background: '#f8fafc', comment: '#64748b',
    keyword: '#6d28d9', string: '#0154cf', number: '#6d28d9',
    type: '#0f766e', function: '#0154cf', property: '#6d28d9', tag: '#0154cf',
  }),
  dark: theme('awot-dark', 'dark', {
    foreground: '#e5ecf6', background: '#111827', comment: '#94a3b8',
    keyword: '#c4b5fd', string: '#7dd3fc', number: '#c4b5fd',
    type: '#5eead4', function: '#93c5fd', property: '#c4b5fd', tag: '#93c5fd',
  }),
};
