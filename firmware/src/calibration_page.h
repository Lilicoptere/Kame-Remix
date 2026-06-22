#pragma once

#include <Arduino.h>

const char CALIBRATION_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="fr">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
  <title>Kame Calibration</title>
  <style>
    :root {
      color-scheme: dark;
      --bg: #101317;
      --panel: #181d24;
      --panel-2: #202833;
      --text: #f5f7fb;
      --muted: #aeb8c7;
      --accent: #45d19f;
      --accent-2: #49a7ff;
      --danger: #ff5a67;
      --line: #313b4a;
      --warn: #ffd27a;
    }

    * { box-sizing: border-box; -webkit-tap-highlight-color: transparent; }

    body {
      margin: 0;
      min-height: 100vh;
      background: var(--bg);
      color: var(--text);
      font-family: system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      user-select: none;
    }

    main {
      width: min(1040px, 100%);
      margin: 0 auto;
      padding: 18px;
      display: grid;
      gap: 16px;
    }

    header {
      display: flex;
      align-items: flex-start;
      justify-content: space-between;
      gap: 14px;
    }

    h1 {
      margin: 0;
      font-size: clamp(1.7rem, 6vw, 3rem);
      line-height: 1;
      letter-spacing: 0;
    }

    .pill {
      min-width: 118px;
      padding: 10px 12px;
      border: 1px solid var(--line);
      border-radius: 8px;
      background: var(--panel);
      color: var(--muted);
      text-align: center;
      font-weight: 800;
    }

    .pill strong {
      display: block;
      color: var(--accent);
      margin-top: 2px;
      font-size: .94rem;
    }

    .toolbar, .notice, .servo {
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: 8px;
      padding: 14px;
    }

    .toolbar {
      display: grid;
      grid-template-columns: repeat(5, minmax(0, 1fr));
      gap: 10px;
    }

    button, a.button {
      min-height: 48px;
      border: 0;
      border-radius: 8px;
      padding: 0 12px;
      color: var(--text);
      background: var(--panel-2);
      font: inherit;
      font-weight: 850;
      text-decoration: none;
      display: inline-grid;
      place-items: center;
      cursor: pointer;
      box-shadow: inset 0 0 0 1px var(--line);
    }

    button:active, a.button:active {
      transform: translateY(1px);
      background: var(--accent-2);
      color: #061018;
    }

    button.save { background: #203729; color: #defde8; }
    button.reset { background: #3b2027; color: #ffdce1; }
    a.button { background: #243140; color: #e8f2ff; }

    .notice {
      color: var(--muted);
      line-height: 1.45;
    }

    .notice strong { color: var(--warn); }

    .grid {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 12px;
    }

    .servo {
      display: grid;
      gap: 12px;
    }

    .servoHeader {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 12px;
    }

    .servoHeader h2 {
      margin: 0;
      font-size: 1.12rem;
      letter-spacing: 0;
    }

    .valueBadge {
      min-width: 76px;
      padding: 7px 10px;
      border-radius: 8px;
      background: #111820;
      border: 1px solid var(--line);
      color: var(--accent);
      text-align: center;
      font-weight: 850;
    }

    .row {
      display: grid;
      grid-template-columns: 1fr 78px;
      gap: 10px;
      align-items: center;
    }

    input[type="range"] {
      width: 100%;
      accent-color: var(--accent);
    }

    input[type="number"] {
      width: 78px;
      min-height: 42px;
      border: 1px solid var(--line);
      border-radius: 8px;
      padding: 0 8px;
      color: var(--text);
      background: #111820;
      font: inherit;
      font-weight: 800;
      text-align: center;
    }

    label.toggle {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 12px;
      color: var(--muted);
      font-weight: 800;
    }

    input[type="checkbox"] {
      width: 26px;
      height: 26px;
      accent-color: var(--accent);
    }

    .tests {
      display: grid;
      grid-template-columns: repeat(4, minmax(0, 1fr));
      gap: 8px;
    }

    .tests button {
      min-height: 42px;
      font-size: .94rem;
      background: #223024;
      color: #defde8;
    }

    .toast {
      position: fixed;
      left: 50%;
      bottom: 18px;
      transform: translateX(-50%);
      max-width: calc(100% - 28px);
      padding: 12px 14px;
      border: 1px solid var(--line);
      border-radius: 8px;
      background: #111820;
      color: var(--text);
      font-weight: 800;
      opacity: 0;
      pointer-events: none;
      transition: opacity .18s ease;
    }

    .toast.show { opacity: 1; }

    @media (max-width: 760px) {
      main { padding: 14px; }
      header { display: grid; }
      .toolbar { grid-template-columns: repeat(2, minmax(0, 1fr)); }
      .grid { grid-template-columns: 1fr; }
      .tests { grid-template-columns: repeat(2, minmax(0, 1fr)); }
    }
  </style>
</head>
<body>
  <main>
    <header>
      <h1>Kame Calibration</h1>
      <div class="pill">memoire<strong id="memory">...</strong></div>
    </header>

    <section class="toolbar">
      <button id="neutral">Pose neutre</button>
      <button id="save" class="save">Sauvegarder</button>
      <button id="reset" class="reset">Reset live</button>
      <button id="reload">Recharger</button>
      <a class="button" href="/" id="remote">Telecommande</a>
    </section>

    <section class="notice">
      <strong>Calibre robot pose au sol, pattes libres.</strong>
      Regle un servo a la fois. Les offsets sont en degres et ne seront persistants qu'apres <strong>Sauvegarder</strong>.
      Si une patte part dans le mauvais sens, active <strong>Inverser</strong> sur le servo concerne.
    </section>

    <section class="grid" id="servos"></section>
  </main>

  <div class="toast" id="toast">OK</div>

  <script>
    const SERVO_COUNT = 8;
    const servosEl = document.getElementById('servos');
    const memoryEl = document.getElementById('memory');
    const toastEl = document.getElementById('toast');
    const timers = {};

    function showToast(message) {
      toastEl.textContent = message;
      toastEl.classList.add('show');
      clearTimeout(showToast.timer);
      showToast.timer = setTimeout(() => toastEl.classList.remove('show'), 1700);
    }

    async function api(path, options = {}) {
      const response = await fetch(path, { cache: 'no-store', ...options });
      if (!response.ok) throw new Error(response.statusText);
      return response.json();
    }

    function servoCard(id) {
      return `
        <article class="servo" data-servo="${id}">
          <div class="servoHeader">
            <h2>Servo S${id}</h2>
            <div class="valueBadge"><span data-role="badge">0</span> deg</div>
          </div>
          <div class="row">
            <input data-role="trim" type="range" min="-30" max="30" value="0">
            <input data-role="number" type="number" min="-30" max="30" value="0">
          </div>
          <label class="toggle">
            <span>Inverser le sens</span>
            <input data-role="reverse" type="checkbox">
          </label>
          <div class="tests">
            <button data-test="60">60 deg</button>
            <button data-test="90">90 deg</button>
            <button data-test="120">120 deg</button>
            <button data-test="home">Repos</button>
          </div>
        </article>
      `;
    }

    function renderCards() {
      servosEl.innerHTML = Array.from({ length: SERVO_COUNT }, (_, id) => servoCard(id)).join('');
    }

    function clampTrim(value) {
      value = Number(value || 0);
      if (value < -30) return -30;
      if (value > 30) return 30;
      return Math.round(value);
    }

    function updateCard(id, trim, reversed) {
      const card = servosEl.querySelector(`[data-servo="${id}"]`);
      if (!card) return;
      card.querySelector('[data-role="trim"]').value = trim;
      card.querySelector('[data-role="number"]').value = trim;
      card.querySelector('[data-role="badge"]').textContent = trim;
      card.querySelector('[data-role="reverse"]').checked = !!reversed;
    }

    async function loadState() {
      const data = await api('/api/calibration/state');
      memoryEl.textContent = data.loaded ? 'chargee' : 'defaut';
      data.servos.forEach(servo => updateCard(servo.id, servo.trim, servo.reversed));
    }

    function scheduleSet(id) {
      clearTimeout(timers[id]);
      timers[id] = setTimeout(() => setServo(id), 120);
    }

    async function setServo(id) {
      const card = servosEl.querySelector(`[data-servo="${id}"]`);
      const trim = clampTrim(card.querySelector('[data-role="trim"]').value);
      const reversed = card.querySelector('[data-role="reverse"]').checked ? 1 : 0;
      updateCard(id, trim, reversed);
      await api(`/api/calibration/set?id=${id}&trim=${trim}&reverse=${reversed}`, { method: 'POST' });
    }

    async function testServo(id, angle) {
      await setServo(id);
      const path = angle === 'home'
        ? '/api/calibration/neutral'
        : `/api/calibration/test?id=${id}&angle=${encodeURIComponent(angle)}`;
      await api(path, { method: 'POST' });
    }

    function bindCards() {
      servosEl.addEventListener('input', event => {
        const card = event.target.closest('[data-servo]');
        if (!card) return;
        const id = Number(card.dataset.servo);
        if (event.target.dataset.role === 'trim' || event.target.dataset.role === 'number') {
          const value = clampTrim(event.target.value);
          updateCard(id, value, card.querySelector('[data-role="reverse"]').checked);
          scheduleSet(id);
        }
      });

      servosEl.addEventListener('change', event => {
        const card = event.target.closest('[data-servo]');
        if (!card) return;
        const id = Number(card.dataset.servo);
        if (event.target.dataset.role === 'reverse') {
          setServo(id);
        }
      });

      servosEl.addEventListener('click', event => {
        const button = event.target.closest('[data-test]');
        if (!button) return;
        const id = Number(button.closest('[data-servo]').dataset.servo);
        testServo(id, button.dataset.test);
      });
    }

    document.getElementById('neutral').addEventListener('click', async () => {
      await api('/api/calibration/neutral', { method: 'POST' });
      showToast('Pose neutre envoyee');
    });

    document.getElementById('save').addEventListener('click', async () => {
      const data = await api('/api/calibration/save', { method: 'POST' });
      memoryEl.textContent = data.saved ? 'sauvee' : 'erreur';
      showToast(data.saved ? 'Offsets sauvegardes en EEPROM' : 'Sauvegarde impossible');
    });

    document.getElementById('reset').addEventListener('click', async () => {
      const data = await api('/api/calibration/reset', { method: 'POST' });
      data.servos.forEach(servo => updateCard(servo.id, servo.trim, servo.reversed));
      memoryEl.textContent = 'defaut';
      showToast('Offsets remis a zero. Sauvegarde si tu veux garder ce reset.');
    });

    document.getElementById('reload').addEventListener('click', loadState);

    document.getElementById('remote').addEventListener('click', () => {
      navigator.sendBeacon('/api/calibration/exit');
    });

    renderCards();
    bindCards();
    loadState().catch(() => showToast('Robot hors ligne'));
  </script>
</body>
</html>
)HTML";
