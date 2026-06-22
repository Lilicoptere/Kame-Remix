#pragma once

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="fr">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
  <title>Kame Remix</title>
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
    }

    * { box-sizing: border-box; -webkit-tap-highlight-color: transparent; }

    body {
      margin: 0;
      min-height: 100vh;
      background: var(--bg);
      color: var(--text);
      font-family: system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      touch-action: manipulation;
      user-select: none;
    }

    main {
      width: min(960px, 100%);
      margin: 0 auto;
      padding: 18px;
      display: grid;
      gap: 16px;
    }

    header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 14px;
    }

    h1 {
      margin: 0;
      font-size: clamp(1.7rem, 6vw, 3rem);
      line-height: 1;
      letter-spacing: 0;
    }

    .status {
      min-width: 104px;
      padding: 10px 12px;
      border: 1px solid var(--line);
      border-radius: 8px;
      background: var(--panel);
      text-align: center;
      color: var(--muted);
      font-weight: 700;
    }

    .status strong {
      display: block;
      color: var(--accent);
      font-size: 0.92rem;
      margin-top: 2px;
    }

    .remote {
      display: grid;
      grid-template-columns: minmax(250px, 1.1fr) minmax(240px, 0.9fr);
      gap: 16px;
      align-items: stretch;
    }

    .pad, .side {
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: 8px;
      padding: 14px;
    }

    .dpad {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      grid-template-rows: repeat(3, minmax(82px, 1fr));
      gap: 10px;
      aspect-ratio: 1 / 1;
      max-height: 520px;
    }

    button {
      border: 0;
      border-radius: 8px;
      color: var(--text);
      background: var(--panel-2);
      font: inherit;
      font-weight: 800;
      cursor: pointer;
      box-shadow: inset 0 0 0 1px var(--line);
    }

    button:active, button.active {
      transform: translateY(1px);
      background: var(--accent-2);
      color: #061018;
      box-shadow: inset 0 0 0 1px rgba(255,255,255,.22);
    }

    .drive {
      font-size: clamp(2.2rem, 10vw, 4.8rem);
      line-height: 1;
    }

    .forward { grid-column: 2; grid-row: 1; }
    .left { grid-column: 1; grid-row: 2; }
    .stop { grid-column: 2; grid-row: 2; background: #3b2027; color: #ffdce1; }
    .right { grid-column: 3; grid-row: 2; }
    .backward { grid-column: 2; grid-row: 3; }

    .side {
      display: grid;
      align-content: start;
      gap: 16px;
    }

    .speedBox {
      display: grid;
      gap: 10px;
    }

    .speedRow {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 12px;
      color: var(--muted);
      font-weight: 700;
    }

    input[type="range"] {
      width: 100%;
      accent-color: var(--accent);
    }

    .actions {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 10px;
    }

    .actions button {
      min-height: 58px;
      padding: 0 12px;
      background: #223024;
      color: #defde8;
    }

    .actions button[data-action="home"] {
      background: #243140;
      color: #e8f2ff;
    }

    .hint {
      margin: 0;
      color: var(--muted);
      line-height: 1.45;
      font-size: 0.95rem;
    }

    @media (max-width: 760px) {
      main { padding: 14px; }
      .remote { grid-template-columns: 1fr; }
      .dpad { grid-template-rows: repeat(3, minmax(74px, 1fr)); }
      header { align-items: flex-start; }
    }
  </style>
</head>
<body>
  <main>
    <header>
      <h1>Kame Remix</h1>
      <div class="status">mode<strong id="mode">stop</strong></div>
    </header>

    <section class="remote">
      <div class="pad">
        <div class="dpad" aria-label="Controle directionnel">
          <button class="drive forward" data-drive="forward" aria-label="Avancer">↑</button>
          <button class="drive left" data-drive="left" aria-label="Tourner a gauche">←</button>
          <button class="drive stop" data-drive="stop" aria-label="Stop">■</button>
          <button class="drive right" data-drive="right" aria-label="Tourner a droite">→</button>
          <button class="drive backward" data-drive="backward" aria-label="Reculer">↓</button>
        </div>
      </div>

      <aside class="side">
        <div class="speedBox">
          <div class="speedRow">
            <span>Vitesse</span>
            <span><strong id="speedValue">55</strong>%</span>
          </div>
          <input id="speed" type="range" min="20" max="100" value="55">
        </div>

        <div class="actions">
          <button data-action="home">Home</button>
          <button data-action="hello">Hello</button>
          <button data-action="dance">Dance</button>
          <button data-action="pushup">Push-up</button>
          <button data-action="moonwalk">Moonwalk</button>
        </div>

        <p class="hint">Garde un bouton directionnel appuye pour rouler. Relache pour stopper. Les fleches clavier et WASD marchent aussi.</p>
      </aside>
    </section>
  </main>

  <script>
    const modeEl = document.getElementById('mode');
    const speedEl = document.getElementById('speed');
    const speedValueEl = document.getElementById('speedValue');
    const driveButtons = [...document.querySelectorAll('[data-drive]')];
    let repeatTimer = null;
    let activeButton = null;
    let activeCommand = 'stop';

    function speed() {
      return Number(speedEl.value || 55);
    }

    async function request(path, options = {}) {
      try {
        const response = await fetch(path, { cache: 'no-store', ...options });
        if (!response.ok) throw new Error(response.statusText);
        return response.json();
      } catch (error) {
        modeEl.textContent = 'offline';
        return null;
      }
    }

    async function drive(command) {
      activeCommand = command;
      const data = await request(`/api/drive?cmd=${encodeURIComponent(command)}&speed=${speed()}`, { method: 'POST' });
      if (data && data.mode) modeEl.textContent = data.mode;
    }

    function clearActiveButton() {
      if (activeButton) activeButton.classList.remove('active');
      activeButton = null;
    }

    function startDrive(button, command) {
      stopRepeat(false);
      activeButton = button;
      activeButton.classList.add('active');
      drive(command);
      if (command !== 'stop') {
        repeatTimer = setInterval(() => drive(command), 210);
      }
    }

    function stopRepeat(sendStop = true) {
      if (repeatTimer) clearInterval(repeatTimer);
      repeatTimer = null;
      clearActiveButton();
      if (sendStop && activeCommand !== 'stop') {
        drive('stop');
      }
      activeCommand = 'stop';
    }

    driveButtons.forEach(button => {
      const command = button.dataset.drive;
      button.addEventListener('pointerdown', event => {
        event.preventDefault();
        button.setPointerCapture(event.pointerId);
        startDrive(button, command);
      });
      button.addEventListener('pointerup', () => stopRepeat(command !== 'stop'));
      button.addEventListener('pointercancel', () => stopRepeat(true));
      button.addEventListener('contextmenu', event => event.preventDefault());
    });

    document.querySelectorAll('[data-action]').forEach(button => {
      button.addEventListener('click', async () => {
        stopRepeat(false);
        const action = button.dataset.action;
        const data = await request(`/api/action?name=${encodeURIComponent(action)}`, { method: 'POST' });
        if (data && data.mode) modeEl.textContent = data.mode;
      });
    });

    const keyMap = {
      ArrowUp: 'forward',
      KeyW: 'forward',
      ArrowDown: 'backward',
      KeyS: 'backward',
      ArrowLeft: 'left',
      KeyA: 'left',
      ArrowRight: 'right',
      KeyD: 'right',
      Space: 'stop'
    };
    const heldKeys = new Set();

    window.addEventListener('keydown', event => {
      const command = keyMap[event.code];
      if (!command || heldKeys.has(event.code)) return;
      event.preventDefault();
      heldKeys.add(event.code);
      const button = document.querySelector(`[data-drive="${command}"]`);
      startDrive(button, command);
    });

    window.addEventListener('keyup', event => {
      if (!heldKeys.has(event.code)) return;
      heldKeys.delete(event.code);
      stopRepeat(true);
    });

    speedEl.addEventListener('input', () => {
      speedValueEl.textContent = speed();
      if (activeCommand !== 'stop') drive(activeCommand);
    });

    document.addEventListener('visibilitychange', () => {
      if (document.hidden) stopRepeat(true);
    });

    window.addEventListener('beforeunload', () => {
      navigator.sendBeacon('/api/drive?cmd=stop');
    });

    setInterval(async () => {
      const data = await request('/api/state');
      if (data && data.mode) {
        modeEl.textContent = data.mode;
      }
    }, 1200);
  </script>
</body>
</html>
)HTML";
