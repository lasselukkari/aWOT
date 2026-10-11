export const docsNavigation = [
  { title: 'Get started', links: [
    { text: 'Hello world', href: '/en/starter/hello-world.html' },
    { text: 'Basic routing', href: '/en/starter/basic-routing.html' },
    { text: 'Serving static files', href: '/en/starter/static-files.html' },
  ] },
  { title: 'Guides', links: [
    { text: 'Routing', href: '/en/guide/routing.html' },
    { text: 'Middleware', href: '/en/guide/middleware.html' },
    { text: 'Request data', href: '/en/guide/request-data.html' },
    { text: 'JSON', href: '/en/guide/json.html' },
    { text: 'Connections', href: '/en/guide/connections.html' },
    { text: 'WebSockets', href: '/en/guide/websockets.html' },
    { text: 'Server Sent Events', href: '/en/guide/server-sent-events.html' },
    { text: 'Configuration', href: '/en/guide/configuration.html' },
    { text: 'macOS', href: '/en/guide/macos.html' },
    { text: 'Windows', href: '/en/guide/windows.html' },
    { text: 'Linux', href: '/en/guide/linux.html' },
    { text: 'Full stack tutorial', href: '/en/guide/tutorial.html' },
    { text: 'Migrating to v4', href: '/en/guide/migrating-to-v4.html' },
  ] },
  { title: 'Reference', links: [
    { text: 'aWOT 4 API', href: '/en/4x/api.html' },
    { text: 'aWOT 3.x archive', href: '/en/3x/api.html' },
  ] },
];

export const headerData = {
  links: [
    { text: 'Get started', links: docsNavigation[0].links },
    { text: 'Guides', links: docsNavigation[1].links.filter(({ href }) => href !== '/en/guide/tutorial.html') },
    { text: 'API', links: docsNavigation[2].links },
    { text: 'Tutorial', href: '/en/guide/tutorial.html' },
  ],
  actions: [{ text: 'GitHub', href: 'https://github.com/lasselukkari/aWOT', icon: 'tabler:brand-github' }],
};

export const footerData = {
  links: [docsNavigation[0], { title: 'Explore', links: [
    { text: 'Full stack tutorial', href: '/en/guide/tutorial.html' },
    { text: 'aWOT 4 API', href: '/en/4x/api.html' },
    { text: 'Migrating to v4', href: '/en/guide/migrating-to-v4.html' },
  ] }, { title: 'Project', links: [
    { text: 'Source code', href: 'https://github.com/lasselukkari/aWOT' },
    { text: 'Examples', href: 'https://github.com/lasselukkari/aWOT/tree/master/examples' },
    { text: '3.x archive', href: '/en/3x/api.html' },
  ] }],
  secondaryLinks: [{ text: 'Arduino web server library', href: '/' }],
  socialLinks: [],
  footNote: 'aWOT is open source under the MIT license. Built with <a href="https://astro.build">Astro</a> and <a href="https://github.com/arthelokyo/astrowind">AstroWind</a>.',
};
