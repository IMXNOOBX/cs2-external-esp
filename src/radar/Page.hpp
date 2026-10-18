#pragma once

inline constexpr char radar_page[] = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>cs2-external-esp radar</title>
<style>
    @font-face { font-family: RadarWeapons; src: url('/weapon-font.ttf') format('truetype'); }
    :root { color-scheme: dark; }
    body { margin: 20px; background: #181818; color: #ddd; font: 14px Consolas, monospace; }
    header, nav { display: flex; align-items: center; flex-wrap: wrap; gap: 18px; margin-bottom: 12px; }
    h1 { font-size: 18px; margin: 0; }
    button, select { font: inherit; padding: 5px 10px; }
    #status, #notice, footer { color: #aaa; }
    #notice { min-height: 20px; }
    #view { width: 100%; }
    #view:fullscreen { display: flex; align-items: center; justify-content: center; height: 100%; background: #181818; }
    canvas { display: block; max-width: 100%; height: auto; background: #222; }
    footer { margin-top: 12px; font-size: 12px; }
    #reload { margin-left: 14px; padding: 0; border: 0; background: none; color: inherit; text-decoration: underline; cursor: pointer; }
    a { color: #8bbcff; }
</style>
</head>
<body>
<header><h1>cs2-external-esp radar</h1><span id="map">No map</span><span id="status">Connecting...</span></header>
<nav>
    <label><input id="names" type="checkbox" checked> Names</label>
    <label><input id="health" type="checkbox" checked> Health</label>
    <label><input id="team" type="checkbox" checked> Teammates</label>
    <label><input id="weapon" type="checkbox"> Weapon</label>
    <label id="floors" hidden>Floor <select id="floor"><option value="both">Both</option></select></label>
    <button id="fullscreen">Fullscreen</button>
</nav>
<p id="notice" role="status"></p>
<div id="view"><canvas id="radar" width="1024" height="1024" aria-label="Live player positions on the current map"></canvas></div>
<footer>Map assets &copy; Valve, served by <a href="https://github.com/MurkyYT/cs2-map-icons" target="_blank" rel="noreferrer">cs2-map-icons</a>. <button id="reload" title="Retry downloading map images and calibration">Reload maps</button></footer>
<script>
'use strict';
const view = document.getElementById('view');
const canvas = document.getElementById('radar');
const ctx = canvas.getContext('2d');
const status = document.getElementById('status');
const notice = document.getElementById('notice');
const names = document.getElementById('names');
const health = document.getElementById('health');
const teammates = document.getElementById('team');
const weapon = document.getElementById('weapon');
const floor = document.getElementById('floor');
const floors = document.getElementById('floors');
const assetBase = 'https://raw.githubusercontent.com/MurkyYT/cs2-map-icons/main/';
let maps = {};
let state = { players: [] };
let mapName = '';
let map = null;
let panels = [];
let generation = 0;
let loading = false;
let weaponFontReady = false;

document.fonts.load('24px RadarWeapons').then(() => {
    weaponFontReady = true;
    draw();
}).catch(() => {});

function panelForHeight(z) {
    const sections = map?.radar_info?.verticalsections;
    return panels.find(panel => {
        const range = sections?.[panel.section];
        return range && z >= range.AltitudeMin && z < range.AltitudeMax;
    }) || panels.find(panel => panel.section === 'default') || panels[0];
}

function worldToMap(player, info) {
    return [(player.x - info.pos_x) / info.scale, (info.pos_y - player.y) / info.scale];
}

function visible(player) {
    return player.local || teammates.checked || player.team !== state.local_team;
}

function draw() {
    const shown = panels.length > 1 && floor.value !== 'both' ?
        panels.filter(panel => panel.section === floor.value) : panels;
    const titleHeight = panels.length > 1 ? 32 : 0;
    const width = Math.max(1, shown.length) * 1024;
    const height = 1024 + titleHeight;
    if (canvas.width !== width) canvas.width = width;
    if (canvas.height !== height) canvas.height = height;
    const availableHeight = document.fullscreenElement === view ? view.clientHeight : Math.max(1, window.innerHeight - 200);
    const displayWidth = Math.min(view.clientWidth, availableHeight * width / height) + 'px';
    if (canvas.style.width !== displayWidth) canvas.style.width = displayWidth;
    ctx.clearRect(0, 0, width, height);
    if (!map || !panels.length) return;
    ctx.textAlign = 'center';
    for (const [index, panel] of shown.entries()) {
        if (panel.image) ctx.drawImage(panel.image, index * 1024, titleHeight, 1024, 1024);
        ctx.font = '18px Consolas, monospace';
        ctx.fillStyle = '#ddd';
        if (titleHeight) ctx.fillText(panel.title, index * 1024 + 512, 23);
        if (!panel.image) ctx.fillText('Overview unavailable', index * 1024 + 512, height / 2);
    }

    for (const player of state.players) {
        if (!visible(player)) continue;
        const [px, py] = worldToMap(player, map.radar_info);
        if (!Number.isFinite(px) || !Number.isFinite(py) || px < 0 || py < 0 || px > 1024 || py > 1024) continue;
        const panelIndex = shown.indexOf(panelForHeight(player.z));
        if (panelIndex < 0) continue;
        const x = px + panelIndex * 1024;
        const y = py + titleHeight;
        ctx.fillStyle = player.local ? '#64b5ff' :
            (state.local_team ? (player.team === state.local_team ? '#5dff8d' : '#ff6565') :
                (player.team === 3 ? '#64b5ff' : '#ffcc66'));
        const markerRadius = 6;
        ctx.beginPath();
        ctx.arc(x, y, markerRadius, 0, Math.PI * 2);
        ctx.fill();
        ctx.strokeStyle = '#111';
        ctx.lineWidth = 2;
        ctx.stroke();
        const details = [];
        if (names.checked) details.push({ text: player.name });
        if (health.checked) details.push({ text: `\u2764\uFE0F ${player.health}` });
        if (weapon.checked && player.weapon) {
            const icon = weaponFontReady && player.weapon_icon && player.weapon_icon !== '?';
            details.push({ text: icon ? player.weapon_icon : player.weapon, icon });
        }
        const rowHeight = 17;
        const top = Math.max(titleHeight + rowHeight / 2, y - markerRadius - rowHeight / 2 -
            (details.length - 1) * rowHeight - 9);
        for (let i = 0; i < details.length; i++) {
            const line = details[i];
            ctx.font = line.icon ? '24px RadarWeapons' : '15px Consolas, monospace';
            const bounds = ctx.measureText(line.text);
            const widthScale = bounds.width ? Math.min(1, 220 / bounds.width) : 1;
            const labelX = x + (bounds.actualBoundingBoxLeft - bounds.actualBoundingBoxRight) * widthScale / 2;
            const labelY = top + i * rowHeight +
                (bounds.actualBoundingBoxAscent - bounds.actualBoundingBoxDescent) / 2;
            ctx.lineWidth = 4;
            ctx.strokeText(line.text, labelX, labelY, 220);
            ctx.fillStyle = '#fff';
            ctx.fillText(line.text, labelX, labelY, 220);
        }
    }
}

async function loadMap(name) {
    const version = ++generation;
    if (name !== mapName) floor.value = 'both';
    floors.hidden = true;
    mapName = name;
    map = null;
    panels = [];
    document.getElementById('map').textContent = name || 'No map';
    notice.textContent = '';
    draw();
    if (!name) return;
    const entry = maps[name];
    const info = entry?.radar_info;
    if (!info || ![info.pos_x, info.pos_y, info.scale].every(Number.isFinite) || info.scale <= 0) {
        notice.textContent = loading ? 'Loading map list...' : 'No calibrated overview available for this map.';
        return;
    }
    notice.textContent = 'Loading overview...';
    const ranges = info.verticalsections || {};
    const sections = Object.keys(ranges).length ? Object.keys(ranges) : ['default'];
    sections.sort((a, b) => (ranges[b]?.AltitudeMin ?? 0) - (ranges[a]?.AltitudeMin ?? 0));
    const loaded = await Promise.all(sections.map(async (section, index) => {
        const title = sections.length === 1 ? '' : index === 0 ? 'Upper' : index === sections.length - 1 ? 'Lower' : section;
        const panel = { section, title, image: null };
        const prefix = name + (section === 'default' ? '' : '_' + section) + '_radar';
        const url = entry.radar_paths?.filter(path => path.split('/').pop().startsWith(prefix + '_') ||
            path.endsWith('/' + prefix + '.png')).sort((a, b) => a.length - b.length)[0];
        if (!url?.startsWith(assetBase + 'images/radars/')) return panel;
        const image = new Image();
        image.referrerPolicy = 'no-referrer';
        image.src = url;
        try {
            await image.decode();
            panel.image = image;
        } catch {}
        return panel;
    }));
    if (version !== generation) return;
    if (!loaded.some(panel => panel.image)) {
        notice.textContent = 'Overview unavailable. Check your connection and reload maps.';
        return;
    }
    map = entry;
    panels = loaded;
    const selection = floor.value;
    floor.replaceChildren(...[{ section: 'both', title: 'Both' }, ...panels].map(panel => {
        const option = document.createElement('option');
        option.value = panel.section;
        option.textContent = panel.title;
        return option;
    }));
    floor.value = panels.some(panel => panel.section === selection) ? selection : 'both';
    floors.hidden = panels.length < 2;
    notice.textContent = loaded.every(panel => panel.image) ? '' : 'Some floor images are unavailable. Reload maps to retry.';
    draw();
}

async function loadMaps() {
    if (loading) return;
    loading = true;
    try {
        const response = await fetch(assetBase + 'data/available.json', { signal: AbortSignal.timeout(10000) });
        if (!response.ok) throw new Error('Map list unavailable');
        maps = (await response.json()).maps || {};
        loading = false;
        await loadMap(mapName);
    } catch {
        notice.textContent = 'Could not load map assets. Check your connection and reload maps.';
    } finally {
        loading = false;
    }
}

async function poll() {
    try {
        const response = await fetch('/state', { cache: 'no-store', signal: AbortSignal.timeout(2000) });
        if (!response.ok) throw new Error('Disconnected');
        state = await response.json();
        const name = state.in_match ? state.map.replaceAll('\\', '/').split('/').pop().replace(/\.vpk$/i, '') : '';
        if (name !== mapName) void loadMap(name);
        status.textContent = state.in_match ? `${state.players.length} players` : 'Waiting for a match...';
        draw();
    } catch {
        state = { players: [] };
        status.textContent = 'Disconnected. Retrying...';
        draw();
    } finally {
        setTimeout(poll, 50);
    }
}

names.onchange = health.onchange = teammates.onchange = weapon.onchange = floor.onchange = draw;
document.getElementById('reload').onclick = loadMaps;
document.getElementById('fullscreen').onclick = async () => {
    try {
        await view.requestFullscreen();
    } catch {
        notice.textContent = 'Fullscreen is unavailable in this browser.';
    }
};
document.addEventListener('fullscreenchange', draw);
window.addEventListener('resize', draw);
void loadMaps();
void poll();
</script>
</body>
</html>)HTML";
