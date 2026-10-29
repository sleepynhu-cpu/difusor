// Guarda o app no celular para abrir sem internet.
// Ao mudar arquivos do app, aumente a VERSAO para os celulares baixarem de novo.
const VERSAO = 'difusor-v4';
const ARQUIVOS = [
  './',
  './index.html',
  './manifest.json',
  './carro.jpg?v=3',
  './icon.png',
  './icon-192.png',
  './icon-512.png',
  './icon-maskable-512.png',
];

self.addEventListener('install', ev => {
  ev.waitUntil(caches.open(VERSAO).then(c => c.addAll(ARQUIVOS)).then(() => self.skipWaiting()));
});

self.addEventListener('activate', ev => {
  ev.waitUntil(
    caches.keys()
      .then(nomes => Promise.all(nomes.filter(n => n !== VERSAO).map(n => caches.delete(n))))
      .then(() => self.clients.claim())
  );
});

self.addEventListener('fetch', ev => {
  const req = ev.request;
  if (req.method !== 'GET') return;

  // Pagina: tenta a internet (para pegar atualizacoes) e cai no salvo se estiver offline
  if (req.mode === 'navigate') {
    ev.respondWith(
      fetch(req)
        .then(resp => {
          const copia = resp.clone();
          caches.open(VERSAO).then(c => c.put('./', copia));
          return resp;
        })
        .catch(() => caches.match('./'))
    );
    return;
  }

  // Resto (foto, icones, fontes): usa o salvo e guarda o que vier da internet
  ev.respondWith(
    caches.match(req).then(salvo => salvo || fetch(req).then(resp => {
      if (resp.ok || resp.type === 'opaque') {
        const copia = resp.clone();
        caches.open(VERSAO).then(c => c.put(req, copia));
      }
      return resp;
    }))
  );
});
