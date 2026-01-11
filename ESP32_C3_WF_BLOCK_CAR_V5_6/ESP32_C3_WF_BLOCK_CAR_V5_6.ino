/*
  ESP32 Robotic Car Controller - BRINKO Motora
  Copyright (C) 2024 [Tu Nombre/Compania]

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// Configuracion de Pines
const int motorPins[8] = {8, 9, 6, 7, 2, 1, 4, 3}; // A1,B1,A2,B2,A3,B3,A4,B4
const int pinServo = 0;
const int pinSensorLinea = 20;

// Variables Globales
Servo direccion;
WebServer server(80);
volatile unsigned long contadorVueltas = 0;
bool ultimoEstado = HIGH;
String mensaje = "";
unsigned long tiempoMensaje = 0;
const char* password = "brinko100";

// Estados de motores
bool motorStates[4] = {false, false, false, false};
bool motorDirections[4] = {true, true, true, true};

// HTML Interfaz
const char* html = R"rawliteral(
<!DOCTYPE html><html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta charset="UTF-8">
<style>
body{margin:0;font-family:Poppins,sans-serif;background:#0f0f0f;color:#f2f2f2;text-align:center;}
h2{color:#C93287;margin:20px 0 10px;}
#tabs{display:flex;justify-content:center;gap:10px;margin:15px 0;}
.tab{padding:10px 18px;border-radius:20px;border:2px solid #C93287;background:transparent;color:#C93287;cursor:pointer;}
.tab.active{background:#C93287;color:white;box-shadow:0 0 12px rgba(201,50,135,0.6);}
.tab-content{margin-top:20px;}
#joystick{width:240px;height:240px;background:radial-gradient(circle at 30% 30%,#444,#111);
  border-radius:50%;border:3px solid #C93287;box-shadow:0 0 30px rgba(201,50,135,0.6);
  position:relative;margin:25px auto 10px;touch-action:none;}
#knob{width:70px;height:70px;background:radial-gradient(circle,#C93287,#680046);border-radius:50%;
  border:2px solid #fff;box-shadow:0 0 25px rgba(201,50,135,0.9);position:absolute;touch-action:none;transition:0.08s;left:85px;top:85px;}
#controls-dir{display:flex;justify-content:center;gap:15px;margin:20px;flex-wrap:wrap;}
.dir-btn{width:60px;height:60px;font-size:28px;border:none;border-radius:12px;background:#C93287;color:white;cursor:pointer;}
.dir-btn:active{transform:scale(0.95);}
#queue{min-height:70px;margin:20px;padding:10px;border:2px dashed #C93287;border-radius:14px;
  display:flex;justify-content:center;align-items:center;gap:10px;flex-wrap:wrap;}
.queue-item{padding:12px 16px;font-size:28px;border-radius:10px;background:orange;color:black;position:relative;min-width:60px;}
.queue-item.active{background:#00ff88 !important;}
.queue-item.stop-active{background:#f44336 !important;}
.queue-item.loop-block{background:#9C27B0;font-size:32px;}
.queue-item.motor-block{background:#FF9800;}
.loop-badge{position:absolute;top:-8px;right:-8px;background:#FF4081;color:white;border-radius:50%;width:20px;height:20px;font-size:12px;line-height:20px;}
.motor-label{font-size:10px;position:absolute;bottom:-8px;right:2px;color:white;background:rgba(0,0,0,0.7);padding:1px 4px;border-radius:3px;}
.time-label{font-size:10px;position:absolute;top:-5px;left:2px;color:#FFD700;background:rgba(0,0,0,0.7);padding:1px 4px;border-radius:3px;}
.motor-count{font-size:10px;position:absolute;top:2px;left:2px;background:#2196F3;color:white;border-radius:50%;width:16px;height:16px;line-height:16px;}
#controls-main{margin-top:15px;}
.ctrl-btn{margin:8px;padding:12px 18px;font-size:16px;border:none;border-radius:10px;background:#C93287;color:white;cursor:pointer;}
#panel{margin-top:20px;}
#mensaje{margin-top:10px;font-size:18px;color:#ffb7ff;}
.welcome-modal,.modal{position:fixed;inset:0;background:rgba(0,0,0,0.75);display:flex;justify-content:center;align-items:center;z-index:9999;}
.modal-box{background:#111;border:2px solid #C93287;border-radius:14px;padding:20px;width:340px;text-align:center;
  box-shadow:0 0 25px rgba(201,50,135,0.6);}
.hidden{display:none;}
.motor-control-row{display:flex;justify-content:center;align-items:center;gap:10px;margin:10px 0;}
.motor-number{width:40px;height:40px;border-radius:50%;background:#222;color:#C93287;display:flex;align-items:center;justify-content:center;font-size:18px;font-weight:bold;}
.motor-direction-btn{width:80px;height:35px;border:none;border-radius:6px;cursor:pointer;font-size:16px;font-weight:bold;}
.motor-forward-btn{background:#4CAF50;color:white;}
.motor-backward-btn{background:#2196F3;color:white;}
.motor-direction-btn.selected{box-shadow:0 0 0 2px white;}
.motor-direction-btn.active{opacity:0.7;}
.action-btn{width:120px;height:40px;margin:10px;border:none;border-radius:8px;cursor:pointer;font-size:14px;}
.test-btn{background:#FF9800;color:white;}
.stop-btn{background:#f44336;color:white;}
.time-input{width:80px;height:35px;font-size:16px;text-align:center;background:#222;color:white;border:1px solid #C93287;border-radius:6px;margin:5px;}
.selected-motors-info{font-size:14px;margin:15px 0;color:#4CAF50;min-height:20px;}
.control-section{margin:15px 0;}
.control-section h4{margin-bottom:8px;color:#C93287;}
.time-section{display:flex;align-items:center;justify-content:center;gap:10px;margin:15px 0;}
</style>
</head>
<body>
<div id="welcomeModal" class="welcome-modal">
  <div class="modal-box">
    <h2>&#x1F916; BRINKO Motora</h2>
    <p>Controla tu robot en modo manual o programa movimientos.</p>
    <button onclick="closeWelcome()" style="margin-top:15px;padding:10px 18px;border:none;border-radius:14px;background:#C93287;color:white;cursor:pointer;">OK</button>
  </div>
</div>

<div id="tabs">
  <button class="tab active" onclick="showTab('manual')">&#x1F579;</button>
  <button class="tab" onclick="showTab('program')">&#x1F4BB;</button>
</div>

<div id="tab-manual" class="tab-content">
  <div id="joystick"><div id="knob"></div></div>
  <div id="panel">
    <h3>LAP: <span id="vueltas">0</span></h3>
    <button class="dir-btn" onclick="resetLaps()">&#x21BA;</button>
  </div>
  <h3 id="mensaje"></h3>
</div>

<div id="tab-program" class="tab-content hidden">
  <div id="controls-dir">
    <button class="dir-btn" onclick="openMotorModal()">&#x2699;</button>
    <button class="dir-btn" onclick="openLoopModal()">&#x21BA;</button>
    <button class="dir-btn" onclick="addCommand('left')">&#x2190;</button>
    <button class="dir-btn" onclick="addCommand('right')">&#x2192;</button>
  </div>
  <div id="queue"></div>
  <div id="controls-main">
    <button class="ctrl-btn" onclick="playQueue()">&#x25B6;</button>
    <button class="ctrl-btn" onclick="pauseQueue()">&#9208;</button>
    <button class="ctrl-btn" onclick="stopQueue()">&#x23F9;</button>
    <button class="ctrl-btn" onclick="popLastCommand()">&#x232B;</button>
    <button class="ctrl-btn" onclick="clearQueue()">&#x1F5D1;</button>
  </div>
</div>

<div id="motorModal" class="modal hidden">
  <div class="modal-box">
    <h3>Control de Motores</h3>
    <div class="motor-control-row">
      <div class="motor-number">1</div>
      <button class="motor-direction-btn motor-forward-btn" onclick="selectMotor(1,'forward')">f</button>
      <button class="motor-direction-btn motor-backward-btn" onclick="selectMotor(1,'backward')">b</button>
      <div class="motor-number">2</div>
      <button class="motor-direction-btn motor-forward-btn" onclick="selectMotor(2,'forward')">f</button>
      <button class="motor-direction-btn motor-backward-btn" onclick="selectMotor(2,'backward')">b</button>
    </div>
    <div class="motor-control-row">
      <div class="motor-number">3</div>
      <button class="motor-direction-btn motor-forward-btn" onclick="selectMotor(3,'forward')">f</button>
      <button class="motor-direction-btn motor-backward-btn" onclick="selectMotor(3,'backward')">b</button>
      <div class="motor-number">4</div>
      <button class="motor-direction-btn motor-forward-btn" onclick="selectMotor(4,'forward')">f</button>
      <button class="motor-direction-btn motor-backward-btn" onclick="selectMotor(4,'backward')">b</button>
    </div>
    <div id="selectedMotorsInfo" class="selected-motors-info">Select motors</div>
    <div class="time-section">
      <span>Time (ms):</span>
      <input id="motorTime" type="number" min="100" max="10000" value="800" class="time-input">
    </div>
    <div>
      <button class="action-btn" style="background:#4CAF50;" onclick="addMotorCommand()">Add</button>
      <button class="action-btn stop-btn" onclick="addStopCommand()">Stop</button>
      <button class="action-btn test-btn" onclick="testMotors()">Test</button>
    </div>
    <div style="margin-top:20px;">
      <button onclick="clearSelections()" style="padding:8px 16px;margin:5px;border:none;border-radius:8px;background:#f44336;color:white;cursor:pointer;font-size:13px;">Clear</button>
      <button onclick="closeMotorModal()" style="padding:10px 24px;margin-top:10px;border:none;border-radius:10px;background:#C93287;color:white;cursor:pointer;">Close</button>
    </div>
  </div>
</div>

<div id="loopModal" class="modal hidden">
  <div class="modal-box">
    <h3>Repetitions</h3>
    <div class="time-section">
      <span>Times:</span>
      <input id="loopCount" type="number" min="2" max="10" value="2" class="time-input">
    </div>
    <div style="margin-top:15px;">
      <button onclick="addLoop()" style="margin:6px;padding:8px 14px;border-radius:10px;border:none;background:#4CAF50;color:white;">Add</button>
      <button onclick="closeLoopModal()" style="margin:6px;padding:8px 14px;border-radius:10px;border:none;background:#f44336;color:white;">Cancel</button>
    </div>
  </div>
</div>

<script>
let commandQueue = [];
let isPlaying = false;
let currentIndex = 0;
let motorSelections = {1:null, 2:null, 3:null, 4:null};

// Joystick
const knob = document.getElementById('knob');
const joystick = document.getElementById('joystick');
let dragging = false;
let lastSend = 0;

function updateJoystick(x, y) {
  const maxDist = 70;
  const dist = Math.sqrt(x*x + y*y);
  if(dist > maxDist) {
    x = x * maxDist / dist;
    y = y * maxDist / dist;
  }
  
  knob.style.left = (85 + x) + 'px';
  knob.style.top = (85 + y) + 'px';
  
  const normX = Math.round((x/maxDist)*100);
  const normY = Math.round((-y/maxDist)*100);
  
  const now = Date.now();
  if(now - lastSend >= 50) {
    lastSend = now;
    fetch('/control?x=' + normX + '&y=' + normY);
  }
}

knob.addEventListener('pointerdown', (e) => {
  dragging = true;
  knob.setPointerCapture(e.pointerId);
});

knob.addEventListener('pointermove', (e) => {
  if(!dragging) return;
  const rect = joystick.getBoundingClientRect();
  const x = e.clientX - rect.left - 120;
  const y = e.clientY - rect.top - 120;
  updateJoystick(x, y);
});

knob.addEventListener('pointerup', () => {
  dragging = false;
  knob.style.left = '85px';
  knob.style.top = '85px';
  fetch('/control?x=0&y=0');
});

// Motor control
function selectMotor(num, dir) {
  const btnF = document.querySelector(`button[onclick="selectMotor(${num},'forward')"]`);
  const btnB = document.querySelector(`button[onclick="selectMotor(${num},'backward')"]`);
  
  if(motorSelections[num] === dir) {
    motorSelections[num] = null;
    btnF.classList.remove('selected');
    btnB.classList.remove('selected');
  } else {
    motorSelections[num] = dir;
    btnF.classList.remove('selected');
    btnB.classList.remove('selected');
    if(dir === 'forward') btnF.classList.add('selected');
    else btnB.classList.add('selected');
  }
  updateSelectedDisplay();
}

function updateSelectedDisplay() {
  const display = document.getElementById('selectedMotorsInfo');
  const selected = [];
  for(let i=1; i<=4; i++) {
    if(motorSelections[i]) {
      selected.push(i + motorSelections[i].charAt(0));
    }
  }
  display.textContent = selected.length ? selected.join(' ') : 'Select motors';
}

function clearSelections() {
  for(let i=1; i<=4; i++) {
    motorSelections[i] = null;
    const btnF = document.querySelector(`button[onclick="selectMotor(${i},'forward')"]`);
    const btnB = document.querySelector(`button[onclick="selectMotor(${i},'backward')"]`);
    btnF.classList.remove('selected');
    btnB.classList.remove('selected');
  }
  updateSelectedDisplay();
}
function addMotorCommand() {
  if(isPlaying) return;
  const duration = parseInt(document.getElementById('motorTime').value) || 800;
  const motors = [];
  const forward = [], backward = [];
  
  for(let i=1; i<=4; i++) {
    if(motorSelections[i]) {
      motors.push(i);
      if(motorSelections[i] === 'forward') forward.push(i);
      else backward.push(i);
    }
  }
  
  if(motors.length === 0) {
    alert('Select at least one motor');
    return;
  }
  
  let cmd = 'mixedmotor_';
  if(forward.length) cmd += 'forward' + forward.join(',');
  if(backward.length) {
    if(forward.length) cmd += '_';
    cmd += 'backward' + backward.join(',');
  }
  cmd += '_time' + duration;
  
  commandQueue.push({
    type: 'mixedmotor',
    cmd: cmd,
    motors: motors,           // ← Asegurar que existe
    forward: forward,         // ← Asegurar que existe  
    backward: backward,       // ← Asegurar que existe
    duration: duration,
    symbol: 'M'
  });
  
  renderQueue();
  closeMotorModal();
}

function addStopCommand() {
  if(isPlaying) return;
  const duration = parseInt(document.getElementById('motorTime').value) || 800;
  commandQueue.push({
    type: 'stop',
    cmd: 'stop_time' + duration,
    duration: duration,
    symbol: 'S'
  });
  renderQueue();
  closeMotorModal();
}

async function testMotors() {
  const duration = parseInt(document.getElementById('motorTime').value) || 800;
  for(let i=1; i<=4; i++) {
    if(motorSelections[i]) {
      fetch('/motor?num=' + i + '&dir=' + motorSelections[i] + '&test=true');
    }
  }
  await new Promise(r => setTimeout(r, duration));
  for(let i=1; i<=4; i++) {
    if(motorSelections[i]) {
      fetch('/motor?num=' + i + '&dir=stop');
    }
  }
}

// Command queue
function addCommand(type) {
  if(isPlaying) return;
  const duration = 800; // Default
  commandQueue.push({
    type: 'movement',
    cmd: type,
    duration: duration,
    symbol: type === 'left' ? '←' : '→'
  });
  renderQueue();
}

function addLoop() {
  if(isPlaying) return;
  const count = parseInt(document.getElementById('loopCount').value) || 2;
  if(count < 2) {
    alert('Minimum 2 repetitions');
    return;
  }
  commandQueue.push({
    type: 'loop',
    cmd: 'loop' + count,
    count: count,
    symbol: '↻'
  });
  renderQueue();
  closeLoopModal();
}
function renderQueue() {
  const q = document.getElementById('queue');
  q.innerHTML = '';
  
  commandQueue.forEach((item, index) => {
    const el = document.createElement('div');
    
    if(item.type === 'loop') {
      el.className = 'queue-item loop-block';
      el.innerHTML = '↻';
      const badge = document.createElement('span');
      badge.className = 'loop-badge';
      badge.textContent = item.count;
      el.appendChild(badge);
    }
    else if(item.type === 'mixedmotor') {
      el.className = 'queue-item motor-block';
      el.textContent = 'M';
      
      const countBadge = document.createElement('span');
      countBadge.className = 'motor-count';
      countBadge.textContent = item.motors.length;
      el.appendChild(countBadge);
      
      let label = '';
      if(item.forward && item.forward.length) label += 'f' + item.forward.join(',');
      if(item.backward && item.backward.length) {
        if(label) label += ' ';
        label += 'b' + item.backward.join(',');
      }
      
      const labelEl = document.createElement('span');
      labelEl.className = 'motor-label';
      labelEl.textContent = label;
      el.appendChild(labelEl);
    }
    else if(item.type === 'stop') {
      el.className = 'queue-item';
      el.textContent = 'S';
    }
    else {
      el.className = 'queue-item';
      el.innerHTML = item.symbol || '?';
    }
    
    if(item.duration && item.duration !== 800) {
      const timeLabel = document.createElement('span');
      timeLabel.className = 'time-label';
      timeLabel.textContent = item.duration + 'ms';
      el.appendChild(timeLabel);
    }
    
    // CORRECCIÓN: Solo resaltar si está playing Y es el índice actual
    if(isPlaying && index === currentIndex) {
      el.classList.add(item.type === 'stop' ? 'stop-active' : 'active');
    }
    
    q.appendChild(el);
  });
}

// Execution
// Execution - VERSIÓN CORREGIDA
async function playQueue() {
  if(commandQueue.length === 0 || isPlaying) return;
  
  isPlaying = true;
  let executionIndex = 0;
  
  while(executionIndex < commandQueue.length && isPlaying) {
    const item = commandQueue[executionIndex];
    
    if(item.type === 'loop') {
      // Encontrar el fin del bloque del loop
      let loopEnd = executionIndex + 1;
      while(loopEnd < commandQueue.length && commandQueue[loopEnd].type !== 'loop') {
        loopEnd++;
      }
      
      // Si el loop está vacío, saltarlo
      if(executionIndex + 1 >= loopEnd) {
        executionIndex = loopEnd;
        continue;
      }
      
      const repeatCount = item.count || 2;
      console.log(`Starting loop: ${repeatCount} repetitions`);
      
      // Repetir el contenido del loop
      for(let r = 0; r < repeatCount; r++) {
        if(!isPlaying) break;
        
        console.log(`Loop iteration ${r + 1}/${repeatCount}`);
        
        // Ejecutar cada comando dentro del loop
        for(let j = executionIndex + 1; j < loopEnd; j++) {
          if(!isPlaying) break;
          
          const loopItem = commandQueue[j];
          currentIndex = j; // Actualizar índice visual
          renderQueue();
          
          console.log(`Executing: ${loopItem.cmd} for ${loopItem.duration || 800}ms`);
          
          // Enviar comando
          fetch('/control?cmd=' + encodeURIComponent(loopItem.cmd));
          
          // Esperar tiempo del comando
          await new Promise(resolve => setTimeout(resolve, loopItem.duration || 800));
          
          // Enviar stop solo si no es el último comando del último ciclo
          const isLastCommandInLoop = (j === loopEnd - 1);
          const isLastIteration = (r === repeatCount - 1);
          
          if(!isLastCommandInLoop || !isLastIteration) {
            fetch('/control?cmd=stop');
            // Breve pausa entre comandos
            await new Promise(resolve => setTimeout(resolve, 50));
          }
        }
      }
      
      // Saltar al final del bloque del loop
      executionIndex = loopEnd;
      console.log(`Loop finished, jumping to index: ${executionIndex}`);
      continue;
    }
    
    // Ejecutar comando normal (no-loop)
    currentIndex = executionIndex;
    renderQueue();
    
    console.log(`Executing normal: ${item.cmd} for ${item.duration || 800}ms`);
    
    fetch('/control?cmd=' + encodeURIComponent(item.cmd));
    await new Promise(resolve => setTimeout(resolve, item.duration || 800));
    
    // Enviar stop si no es el último comando
    if(executionIndex < commandQueue.length - 1) {
      fetch('/control?cmd=stop');
      // Breve pausa entre comandos
      await new Promise(resolve => setTimeout(resolve, 50));
    }
    
    executionIndex++;
  }
  
  stopQueue();
}

function stopQueue() {
  isPlaying = false;
  currentIndex = 0;
  fetch('/control?cmd=stop');
  renderQueue();
  console.log('Queue stopped');
}

function pauseQueue() {
  isPlaying = false;
  fetch('/control?cmd=stop');
  console.log('Queue paused');
}

function popLastCommand() {
  if(isPlaying) return;
  commandQueue.pop();
  renderQueue();
}

function clearQueue() {
  stopQueue();
  commandQueue = [];
  renderQueue();
}

// UI
function showTab(name) {
  document.getElementById('tab-manual').style.display = name === 'manual' ? 'block' : 'none';
  document.getElementById('tab-program').style.display = name === 'program' ? 'block' : 'none';
  document.querySelectorAll('.tab').forEach((b, i) => {
    b.classList.toggle('active', (name === 'manual' && i === 0) || (name === 'program' && i === 1));
  });
}

function closeWelcome() {
  document.getElementById('welcomeModal').style.display = 'none';
}

function resetLaps() {
  fetch('/reset', {method:'POST'});
  document.getElementById('mensaje').textContent = 'Reset';
  setTimeout(() => document.getElementById('mensaje').textContent = '', 1500);
}

function openMotorModal() {
  document.getElementById('motorModal').classList.remove('hidden');
}

function closeMotorModal() {
  document.getElementById('motorModal').classList.add('hidden');
}

function openLoopModal() {
  document.getElementById('loopModal').classList.remove('hidden');
}

function closeLoopModal() {
  document.getElementById('loopModal').classList.add('hidden');
}

// Status updates
setInterval(() => {
  fetch('/status').then(r => r.json()).then(data => {
    document.getElementById('vueltas').textContent = data.vueltas;
    document.getElementById('mensaje').textContent = data.mensaje;
  });
}, 500);

// Initial render
renderQueue();
</script>
</body>
</html>
)rawliteral";

// Funciones de movimiento
void setMotor(int index, bool forward) {
  digitalWrite(motorPins[index*2], forward ? HIGH : LOW);
  digitalWrite(motorPins[index*2+1], forward ? LOW : HIGH);
}

void stopMotor(int index) {
  digitalWrite(motorPins[index*2], LOW);
  digitalWrite(motorPins[index*2+1], LOW);
  motorStates[index] = false;
}

void stopAllMotors() {
  for(int i = 0; i < 8; i++) {
    digitalWrite(motorPins[i], LOW);
  }
  for(int i = 0; i < 4; i++) {
    motorStates[i] = false;
  }
}

void controlMotor(int motorNum, String command) {
  motorNum--; // 0-3
  if(motorNum < 0 || motorNum > 3) return;
  
  if(command == "forward") {
    setMotor(motorNum, true);
    motorStates[motorNum] = true;
    motorDirections[motorNum] = true;
  } 
  else if(command == "backward") {
    setMotor(motorNum, false);
    motorStates[motorNum] = true;
    motorDirections[motorNum] = false;
  }
  else if(command == "stop") {
    stopMotor(motorNum);
  }
  
  Serial.printf("Motor %d: %s\n", motorNum+1, command.c_str());
}

void controlMixedMotors(String command) {
  Serial.printf("Mixed: %s\n", command.c_str());
  
  // Extraer tiempo
  int timeIndex = command.indexOf("_time");
  unsigned long duration = 800;
  
  if(timeIndex != -1) {
    String timeStr = command.substring(timeIndex + 5);
    duration = timeStr.toInt();
    command = command.substring(0, timeIndex);
  }
  
  // Procesar forward
  int fIndex = command.indexOf("forward");
  if(fIndex != -1) {
    int start = fIndex + 7;
    int end = command.indexOf("backward");
    if(end == -1) end = command.length();
    
    String motors = command.substring(start, end);
    if(motors.endsWith("_")) motors = motors.substring(0, motors.length()-1);
    
    int last = -1;
    for(int i = 0; i <= motors.length(); i++) {
      if(i == motors.length() || motors.charAt(i) == ',') {
        if(last + 1 < i) {
          String num = motors.substring(last + 1, i);
          controlMotor(num.toInt(), "forward");
        }
        last = i;
      }
    }
  }
  
  // Procesar backward
  int bIndex = command.indexOf("backward");
  if(bIndex != -1) {
    int start = bIndex + 8;
    String motors = command.substring(start);
    
    int last = -1;
    for(int i = 0; i <= motors.length(); i++) {
      if(i == motors.length() || motors.charAt(i) == ',') {
        if(last + 1 < i) {
          String num = motors.substring(last + 1, i);
          controlMotor(num.toInt(), "backward");
        }
        last = i;
      }
    }
  }
  
  delay(duration);
  stopAllMotors();
}

void processCommand(String cmd) {
  Serial.println("Processing: " + cmd);
  
  if(cmd == "left") {
    direccion.write(55);
    delay(80);
  }
  else if(cmd == "right") {
    direccion.write(125);
    delay(80);
  }
  else if(cmd == "stop") {
    stopAllMotors();
  }
  else if(cmd.startsWith("mixedmotor_")) {
    controlMixedMotors(cmd);
  }
  else if(cmd.startsWith("motor")) {
    // Formato: motor_X_forward/backward
    int us1 = cmd.indexOf('_');
    int us2 = cmd.indexOf('_', us1+1);
    if(us1 != -1 && us2 != -1) {
      int num = cmd.substring(us1+1, us2).toInt();
      String dir = cmd.substring(us2+1);
      controlMotor(num, dir);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Configurar pines
  for(int i = 0; i < 8; i++) {
    pinMode(motorPins[i], OUTPUT);
    digitalWrite(motorPins[i], LOW);
  }
  
  pinMode(pinSensorLinea, INPUT_PULLUP);
  
  // Servo
  direccion.attach(pinServo);
  direccion.write(90);
  
  // WiFi
  String ssid = "BRINKO-" + String(ESP.getEfuseMac() >> 32, HEX);
  WiFi.softAP(ssid.c_str(), password);
  WiFi.setSleep(false); // Mejor estabilidad
  
  Serial.println("AP: " + ssid);
  Serial.println("IP: " + WiFi.softAPIP().toString());
  
  // Servidor web
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", html);
  });
  
  server.on("/control", HTTP_GET, []() {
    if(server.hasArg("cmd")) {
      processCommand(server.arg("cmd"));
    }
    else if(server.hasArg("x") && server.hasArg("y")) {
      int x = server.arg("x").toInt();
      int y = server.arg("y").toInt();
      
      // Servo
      int angle = map(x, -100, 100, 125, 55);
      direccion.write(angle);
      
      // Motores
      if(y > 30) {
        for(int i = 0; i < 4; i++) {
          setMotor(i, true);
          motorStates[i] = true;
          motorDirections[i] = true;
        }
      }
      else if(y < -30) {
        for(int i = 0; i < 4; i++) {
          setMotor(i, false);
          motorStates[i] = true;
          motorDirections[i] = false;
        }
      }
      else {
        stopAllMotors();
      }
    }
    server.send(200, "text/plain", "OK");
  });
  
  server.on("/motor", HTTP_GET, []() {
    if(server.hasArg("num") && server.hasArg("dir")) {
      int num = server.arg("num").toInt();
      String dir = server.arg("dir");
      controlMotor(num, dir);
    }
    server.send(200, "text/plain", "OK");
  });
  
  server.on("/status", HTTP_GET, []() {
    String json = "{\"vueltas\":" + String(contadorVueltas) + ",\"mensaje\":\"" + mensaje + "\"}";
    server.send(200, "application/json", json);
  });
  
  server.on("/reset", HTTP_POST, []() {
    contadorVueltas = 0;
    mensaje = "Reinicio";
    server.send(200, "text/plain", "OK");
  });
  
  server.begin();
  Serial.println("Server ready");
}

void loop() {
  server.handleClient();
  
  // Sensor de línea
  bool lectura = digitalRead(pinSensorLinea);
  if(lectura != ultimoEstado) {
    ultimoEstado = lectura;
    if(lectura == LOW) {
      contadorVueltas++;
      Serial.println("Lap: " + String(contadorVueltas));
    }
  }
  
  // Meta
  if(contadorVueltas == 5 && mensaje == "") {
    mensaje = "Meta!";
    tiempoMensaje = millis();
    contadorVueltas = 0;
  }
  
  if(mensaje != "" && millis() - tiempoMensaje > 3000) {
    mensaje = "";
  }
  
  delay(10);
}