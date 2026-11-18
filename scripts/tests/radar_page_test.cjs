const fs = require('node:fs');
const vm = require('node:vm');
const assert = require('node:assert/strict');
const header = fs.readFileSync(require('node:path').join(__dirname, '../../src/radar/Page.hpp'), 'utf8');
const script = header.match(/<script>([\s\S]*?)<\/script>/)[1]
    .replace('void loadMaps();', '').replace('void poll();', '');
const arcs = [];
const labels = [];
const drawnImages = [];
const textBounds = { width:40, actualBoundingBoxLeft:20, actualBoundingBoxRight:20, actualBoundingBoxAscent:10, actualBoundingBoxDescent:2 };
const weaponBounds = { width:60, actualBoundingBoxLeft:10, actualBoundingBoxRight:30, actualBoundingBoxAscent:8, actualBoundingBoxDescent:2 };
const ctx = {
    globalAlpha: 1,
    clearRect() { arcs.length = 0; labels.length = 0; drawnImages.length = 0; },
    drawImage(image) { drawnImages.push(image.src); },
    beginPath() {}, arc(...args) { arcs.push({ args, alpha:this.globalAlpha, color:this.fillStyle }); }, fill() {}, stroke() {},
    measureText() {
        return /RadarWeapons$/.test(this.font) ? weaponBounds : textBounds;
    },
    strokeText() {}, fillText(text, x, y) { labels.push({ text, x, y, font:this.font }); }
};
const elements = {};
for (const id of ['radar', 'map', 'status', 'notice', 'names', 'health', 'team', 'weapon', 'player', 'reload', 'view', 'fullscreen', 'floor', 'floors']) {
    elements[id] = { value: 'both', hidden: true, replaceChildren(...children) { this.children = children; }, clientWidth: 1600, clientHeight: 900, checked: true, textContent: '', style: {}, getContext() { return ctx; } };
}
let fullscreenFails = false;
const listeners = {};
const document = {
    createElement() { return {}; },
    getElementById(id) { return elements[id]; },
    addEventListener(name, handler) { listeners[name] = handler; },
    fonts: { load() { return Promise.resolve([]); } }
};
elements.view.requestFullscreen = async () => {
    if (fullscreenFails) throw new Error('blocked');
    document.fullscreenElement = elements.view;
    listeners.fullscreenchange();
};
let failImage = false;
let failSrc = '';
let defer = false;
const pending = [];
const context = vm.createContext({
    document, window: { innerHeight: 1000, addEventListener() {} },
    Image: class { decode() {
        if (failImage || this.src === failSrc) return Promise.reject(new Error('unavailable'));
        return defer ? new Promise(resolve => pending.push(resolve)) : Promise.resolve();
    } },
    AbortSignal, setTimeout() {},
    fetch() { return Promise.reject(new Error('offline')); }
});
vm.runInContext(script, context);
const run = code => vm.runInContext(code, context);
const base = run('assetBase') + 'images/radars/';
// Synthetic maps keep these checks independent of game updates and network access.
const maps = {
    test_single: { radar_info: { pos_x: 0, pos_y: 0, scale: 2 },
        radar_paths: [base + 'test_single_radar.png'] },
    test_split: { radar_info: { pos_x: 0, pos_y: 0, scale: 1,
        verticalsections: { default: { AltitudeMin: 0, AltitudeMax: 1 },
            basement: { AltitudeMin: -1, AltitudeMax: 0 } } },
        radar_paths: [base + 'test_split_basement_radar.png', base + 'test_split_radar.png'] }
};
context.fixtures = maps;
run('maps = fixtures');
async function updateState(snapshot) {
    context.fetch = async () => ({ ok:true, json:async () => snapshot });
    await run('poll()');
}
(async () => {
    assert.equal(run('JSON.stringify(worldToMap({x:0,y:0}, maps.test_single.radar_info))'), '[0,0]');
    assert.equal(run('JSON.stringify(worldToMap({x:2,y:-2}, maps.test_single.radar_info))'), '[1,1]');
    await run("loadMap('test_split')");
    assert.deepEqual(drawnImages, [base + 'test_split_radar.png', base + 'test_split_basement_radar.png']);
    assert.equal(elements.radar.width, 2048);
    assert.ok(labels.some(line => line.text === 'Upper'));
    assert.ok(labels.some(line => line.text === 'Lower'));
    run(`state = { local_team:3, players: [
        {name:'Main',team:3,x:1,y:-1,z:0,health:100},
        {name:'Lower',team:2,x:2,y:-2,z:-0.5,health:100}] }; draw()`);
    assert.equal(arcs.length, 2);
    assert.equal(arcs[0].args[0], 1);
    assert.equal(arcs[1].args[0], 1024 + 2);
    assert.ok(arcs.every(arc => arc.alpha === 1));
    run('state.players[0].z = -0.5; state.players[1].local = true; draw()');
    assert.ok(arcs.every(arc => arc.args[0] > 1024));
    assert.deepEqual(drawnImages, [base + 'test_split_radar.png', base + 'test_split_basement_radar.png']);
    run('state.players[0].z = 0; draw()');
    assert.equal(arcs[0].args[0], 1);
    failSrc = base + 'test_split_basement_radar.png';
    await run("loadMap('test_split')");
    assert.equal(run('panels.length'), 2);
    assert.equal(arcs[1].args[0], 1024 + 2);
    assert.match(elements.notice.textContent, /unavailable/);
    failSrc = '';
    await run("loadMap('test_split')");
    assert.equal(elements.floors.hidden, false);
    assert.deepEqual(elements.floor.children.map(option => option.textContent), ['Both', 'Upper', 'Lower']);
    elements.floor.value = 'default';
    elements.floor.onchange();
    assert.equal(elements.radar.width, 1024);
    assert.deepEqual(drawnImages, [base + 'test_split_radar.png']);
    assert.equal(arcs.length, 1);
    assert.equal(arcs[0].args[0], 1);
    elements.floor.value = 'basement';
    elements.floor.onchange();
    assert.deepEqual(drawnImages, [base + 'test_split_basement_radar.png']);
    assert.equal(arcs.length, 1);
    assert.equal(arcs[0].args[0], 2);
    await run("loadMap('test_split')");
    assert.equal(elements.floor.value, 'basement');
    assert.equal(arcs.length, 1);
    elements.floor.value = 'both';
    elements.floor.onchange();
    assert.equal(elements.radar.width, 2048);
    assert.equal(arcs.length, 2);
    elements.floor.value = 'basement';
    await run("loadMap('test_single')");
    assert.equal(elements.radar.width, 1024);
    assert.equal(elements.floors.hidden, true);
    assert.equal(elements.floor.value, 'both');
    const roster = [
        {id:'76561198000000001',name:'Host',team:3},
        {id:'76561198000000002',name:'You',team:2},
        {id:'76561198000000003',name:'Your teammate',team:2},
        {id:'bot-4',name:'Dead bot',team:2}
    ];
    const playing = roster.slice(0, 3).map((player, index) => ({ ...player, x:index + 1, y:-1, z:0, health:100 }));
    await updateState({in_match:true,map:'test_single',roster,players:playing});
    assert.deepEqual(elements.player.children.map(option => option.textContent),
        ['Select player', 'Host', 'You', 'Your teammate', 'Dead bot']);
    assert.ok(arcs.every(arc => arc.color === '#ff6565'));
    elements.player.value = roster[1].id;
    elements.player.onchange();
    assert.deepEqual(arcs.map(arc => arc.color), ['#ff6565', '#64b5ff', '#5dff8d']);
    const options = elements.player.children;
    await updateState({in_match:true,map:'test_single',roster,players:playing});
    assert.equal(elements.player.children, options);
    elements.team.checked = false;
    elements.team.onchange();
    assert.deepEqual(arcs.map(arc => arc.color), ['#ff6565', '#64b5ff']);
    elements.team.checked = true;
    await updateState({in_match:true,map:'test_single',roster,players:[playing[0], playing[2]]});
    assert.equal(elements.player.value, roster[1].id);
    assert.deepEqual(arcs.map(arc => arc.color), ['#ff6565', '#5dff8d']);
    await updateState({in_match:true,map:'test_single',roster:[roster[0], roster[2]],players:[playing[0], playing[2]]});
    assert.equal(elements.player.value, roster[1].id);
    assert.deepEqual(arcs.map(arc => arc.color), ['#ff6565', '#5dff8d']);
    assert.ok(elements.player.children.some(option => option.value === roster[1].id));
    const swapped = roster.map(player => ({ ...player, team:player.team === 2 ? 3 : 2 }));
    await updateState({in_match:true,map:'test_single',roster:swapped,
        players:[playing[2], playing[0], playing[1]].map(player => ({...player,team:player.team === 2 ? 3 : 2}))});
    assert.deepEqual(arcs.map(arc => arc.color), ['#5dff8d', '#ff6565', '#64b5ff']);
    const replacement = {...swapped[1], id:'76561198000000004'};
    await updateState({in_match:true,map:'test_single',roster:[swapped[0], replacement],
        players:[{...playing[1], ...replacement}]});
    assert.equal(elements.player.value, roster[1].id);
    assert.deepEqual(arcs.map(arc => arc.color), ['#5dff8d']);
    context.fetch = () => Promise.reject(new Error('offline'));
    await run('poll()');
    assert.equal(elements.player.value, roster[1].id);
    assert.equal(arcs.length, 0);
    await updateState({in_match:true,map:'test_single',roster,players:playing});
    assert.deepEqual(arcs.map(arc => arc.color), ['#ff6565', '#64b5ff', '#5dff8d']);
    elements.player.value = '';
    elements.player.onchange();
    assert.ok(arcs.every(arc => arc.color === '#ff6565'));
    run(`state = { players: [
        {id:'local',name:'Local',team:3,x:1,y:-1,z:0,health:100,weapon:'AK-47',weapon_icon:'\uE000'},
        {id:'enemy',name:'Enemy',team:2,x:2,y:-2,z:0,health:50,weapon:'AWP'},
        {id:'mate',name:'Mate',team:3,x:0,y:0,z:0,health:80,weapon:'M4A1-S'},
        {id:'outside',name:'Outside',team:2,x:Infinity,y:0,z:0,health:100,weapon:'AK-47',weapon_icon:'\uE000'}] };
        state.roster = state.players; updatePlayerList()`);
    elements.player.value = 'local';
    elements.player.onchange();
    assert.equal(arcs.length, 3);
    assert.equal(labels.length, 9);
    elements.team.checked = false;
    run('draw()');
    assert.equal(arcs.length, 2);
    for (let flags = 0; flags < 16; flags++) {
        elements.names.checked = !!(flags & 1);
        elements.health.checked = !!(flags & 2);
        elements.weapon.checked = !!(flags & 4);
        elements.team.checked = !!(flags & 8);
        run('draw()');
        assert.equal(arcs.length, elements.team.checked ? 3 : 2);
        const lines = Number(elements.names.checked) + Number(elements.health.checked) + Number(elements.weapon.checked);
        assert.equal(labels.length, lines * arcs.length);
        if (lines) {
            const stack = labels.slice(0, lines);
            assert.equal(stack.some(line => line.text === 'Local'), elements.names.checked);
            assert.equal(stack.some(line => line.text === '\u2764\uFE0F 100'), elements.health.checked);
            assert.equal(stack.some(line => line.text === '\uE000' && /RadarWeapons$/.test(line.font)), elements.weapon.checked);
            for (let i = 1; i < stack.length; i++) assert.ok(stack[i].y > stack[i - 1].y);
        }
    }
    elements.names.checked = elements.health.checked = elements.weapon.checked = true;
    run('state.players[0].y = -200; draw()');
    const stack = labels.slice(0, 3);
    const markerX = arcs[0].args[0];
    assert.equal(stack[0].x, markerX);
    assert.equal(stack[2].x + (weaponBounds.actualBoundingBoxRight - weaponBounds.actualBoundingBoxLeft) / 2, markerX);
    const nameCenter = stack[0].y + (textBounds.actualBoundingBoxDescent - textBounds.actualBoundingBoxAscent) / 2;
    const healthCenter = stack[1].y + (textBounds.actualBoundingBoxDescent - textBounds.actualBoundingBoxAscent) / 2;
    const weaponCenter = stack[2].y + (weaponBounds.actualBoundingBoxDescent - weaponBounds.actualBoundingBoxAscent) / 2;
    assert.equal(healthCenter - nameCenter, weaponCenter - healthCenter);
    assert.ok(weaponCenter + (weaponBounds.actualBoundingBoxAscent + weaponBounds.actualBoundingBoxDescent) / 2 < arcs[0].args[1] - arcs[0].args[2]);
    assert.equal(healthCenter - nameCenter, 17);
    await elements.fullscreen.onclick();
    assert.equal(elements.radar.style.width, '900px');
    await run("loadMap('test_split')");
    assert.equal(elements.radar.style.width, '1600px');
    elements.floor.value = 'basement';
    elements.floor.onchange();
    assert.equal(elements.radar.style.width, (900 * 1024 / 1056) + 'px');
    elements.floor.value = 'both';
    elements.floor.onchange();
    document.fullscreenElement = null;
    listeners.fullscreenchange();
    assert.equal(elements.radar.style.width, (800 * 2048 / 1056) + 'px');
    fullscreenFails = true;
    await elements.fullscreen.onclick();
    assert.match(elements.notice.textContent, /Fullscreen is unavailable/);
    failImage = true;
    await run("loadMap('test_split')");
    assert.equal(drawnImages.length, 0);
    assert.equal(elements.floors.hidden, true);
    assert.match(elements.notice.textContent, /Overview unavailable/);
    failImage = false;
    defer = true;
    const oldLoad = run("loadMap('test_split')");
    const newLoad = run("loadMap('test_single')");
    pending.pop()();
    await newLoad;
    for (const resolve of pending.splice(0)) resolve();
    await oldLoad;
    assert.equal(run('mapName'), 'test_single');
    assert.equal(run('map.radar_info.scale'), 2);
    await run("loadMap('unsupported')");
    assert.equal(run('map'), null);
    assert.match(elements.notice.textContent, /No calibrated overview/);
    context.fetch = () => Promise.reject(new Error('offline'));
    await run('poll()');
    assert.equal(run('state.players.length'), 0);
    assert.match(elements.status.textContent, /Disconnected/);
    console.log('Browser tests passed: player selection, consistent colours, death and missing updates, team changes, identity, reconnects, coordinates, floors, markers, labels, fullscreen, toggles, map races and disconnects');
})().catch(error => { console.error(error); process.exitCode = 1; });
