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
<head><meta name="viewport" content="width=device-width, initial-scale=1">
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
</style></head>
<body>
<div id="welcomeModal" class="welcome-modal">
  <div class="modal-box"><h2>&#x1F916; BRINKO Motora</h2><p>Controla tu robot en modo manual o programa movimientos.</p>
  <button onclick="closeWelcome()" style="margin-top:15px;padding:10px 18px;border:none;border-radius:14px;background:#C93287;color:white;cursor:pointer;">OK</button></div>
</div>
<div id="tabs"><button class="tab active" onclick="showTab('manual')">&#x1F579;</button>
<button class="tab" onclick="showTab('program')">&#x1F4BB;</button></div>

<!-- MODO MANUAL -->
<div id="tab-manual" class="tab-content">
  <div id="joystick"><div id="knob"></div></div>
  <div id="panel"><h3>LAP: <span id="vueltas">0</span></h3>
  <button class="dir-btn" onclick="resetLaps()">&#x21BA;</button></div>
  <h3 id="mensaje"></h3>
</div>

<!-- MODO PROGRAMACION -->
<div id="tab-program" class="tab-content hidden">
  <div id="controls-dir">
    <button class="dir-btn" onclick="openMotorModal()">&#x2699;</button>
    <button class="dir-btn" onclick="openLoopModal()">&#x21BA;</button>
    <button class="dir-btn" onclick="addCommand('&#x2190;','left',800)">&#x2190;</button>
    <button class="dir-btn" onclick="addCommand('&#x2192;','right',800)">&#x2192;</button>
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

<!-- MODAL PARA CONTROL DE MOTORES -->
<div id="motorModal" class="modal hidden">
  <div class="modal-box">
    <h3>Motor Control</h3>
    
    <!-- Fila 1: Motores 1 y 2 -->
    <div class="motor-control-row">
      <div class="motor-number">1</div>
      <button class="motor-direction-btn motor-forward-btn" onclick="selectMotorDirection(1,'forward')">f</button>
      <button class="motor-direction-btn motor-backward-btn" onclick="selectMotorDirection(1,'backward')">b</button>
      <div class="motor-number">2</div>
      <button class="motor-direction-btn motor-forward-btn" onclick="selectMotorDirection(2,'forward')">f</button>
      <button class="motor-direction-btn motor-backward-btn" onclick="selectMotorDirection(2,'backward')">b</button>
    </div>
    
    <!-- Fila 2: Motores 3 y 4 -->
    <div class="motor-control-row">
      <div class="motor-number">3</div>
      <button class="motor-direction-btn motor-forward-btn" onclick="selectMotorDirection(3,'forward')">f</button>
      <button class="motor-direction-btn motor-backward-btn" onclick="selectMotorDirection(3,'backward')">b</button>
      <div class="motor-number">4</div>
      <button class="motor-direction-btn motor-forward-btn" onclick="selectMotorDirection(4,'forward')">f</button>
      <button class="motor-direction-btn motor-backward-btn" onclick="selectMotorDirection(4,'backward')">b</button>
    </div>
    
    <div id="selectedMotorsInfo" class="selected-motors-info"></div>
    
    <div class="control-section">
      <div class="time-section">
        <span>Time (ms):</span>
        <input id="motorTime" type="number" min="100" max="10000" value="800" class="time-input">
      </div>
    </div>
    
    <div class="control-section">
      <button class="action-btn" style="background:#4CAF50;" onclick="addMotorCommand()">Add</button>
      <button class="action-btn stop-btn" onclick="addStopCommand()">Stop</button>
      <button class="action-btn test-btn" onclick="testSelectedMotors()">Test</button>
    </div>
    
    <div style="margin-top:20px;">
      <button onclick="clearAllSelections()" style="padding:8px 16px;margin:5px;border:none;border-radius:8px;background:#f44336;color:white;cursor:pointer;font-size:13px;">Clear</button>
      <button onclick="closeMotorModal()" style="padding:10px 24px;margin-top:10px;border:none;border-radius:10px;background:#C93287;color:white;cursor:pointer;">Close</button>
    </div>
  </div>
</div>

<!-- MODAL PARA BUCLES -->
<div id="loopModal" class="modal hidden">
  <div class="modal-box">
    <h3>Repetitions</h3>
    <div class="time-section">
      <span>Times:</span>
      <input id="loopCount" type="number" min="2" max="10" value="2" class="time-input">
    </div>
    <div style="margin-top:15px;">
      <button onclick="confirmLoop()" style="margin:6px;padding:8px 14px;border-radius:10px;border:none;background:#4CAF50;color:white;">Add</button>
      <button onclick="closeLoopModal()" style="margin:6px;padding:8px 14px;border-radius:10px;border:none;background:#f44336;color:white;">Cancel</button>
    </div>
  </div>
</div>

<script>
let commandQueue=[],isPlaying=false,currentIndex=0;
const DEFAULT_STEP_TIME=800,minInterval=50;
let lastSend=0,dragging=false;

// Estado de seleccion de motores
let motorSelections={
  1:{selected:false,direction:null},
  2:{selected:false,direction:null},
  3:{selected:false,direction:null},
  4:{selected:false,direction:null}
};

// ========== JOYSTICK ==========
const knob=document.getElementById("knob"),joystick=document.getElementById("joystick");
let centerX=joystick.offsetWidth/2,centerY=joystick.offsetHeight/2;

function updateJoystick(dx,dy){
  const maxDist=70,dist=Math.sqrt(dx*dx+dy*dy);
  if(dist>maxDist){dx*=maxDist/dist;dy*=maxDist/dist;}
  const knobRadius=35;
  knob.style.left=(centerX+dx-knobRadius)+"px";
  knob.style.top=(centerY+dy-knobRadius)+"px";
  
  const normX=Math.round((dx/maxDist)*100),normY=Math.round((-dy/maxDist)*100);
  const now=Date.now();
  if(now-lastSend>=minInterval){
    lastSend=now;
    fetch(`/control?x=${normX}&y=${normY}`);
  }
}

knob.addEventListener("pointerdown",(e)=>{
  dragging=true;
  knob.setPointerCapture(e.pointerId);
  const rect=joystick.getBoundingClientRect();
  centerX=rect.width/2;
  centerY=rect.height/2;
});

knob.addEventListener("pointermove",(e)=>{
  if(!dragging) return;
  const rect=joystick.getBoundingClientRect();
  const x=e.clientX-rect.left-centerX;
  const y=e.clientY-rect.top-centerY;
  requestAnimationFrame(()=>updateJoystick(x,y));
});

knob.addEventListener("pointerup",()=>{
  dragging=false;
  knob.style.left="85px";
  knob.style.top="85px";
  fetch("/control?x=0&y=0");
});

// ========== CONTROL DE MOTORES ==========
function selectMotorDirection(motorNum,direction){
  const forwardBtn=document.querySelector(`button[onclick="selectMotorDirection(${motorNum},'forward')"]`);
  const backwardBtn=document.querySelector(`button[onclick="selectMotorDirection(${motorNum},'backward')"]`);
  
  if(motorSelections[motorNum].selected && motorSelections[motorNum].direction===direction){
    motorSelections[motorNum]={selected:false,direction:null};
    forwardBtn.classList.remove('selected');
    backwardBtn.classList.remove('selected');
  }else{
    motorSelections[motorNum]={selected:true,direction:direction};
    
    forwardBtn.classList.remove('selected');
    backwardBtn.classList.remove('selected');
    
    if(direction==='forward'){
      forwardBtn.classList.add('selected');
    }else{
      backwardBtn.classList.add('selected');
    }
  }
  
  updateSelectedMotorsDisplay();
}

function updateSelectedMotorsDisplay(){
  const display=document.getElementById('selectedMotorsInfo');
  const selectedMotors=[];
  
  for(let i=1;i<=4;i++){
    if(motorSelections[i].selected){
      const directionSymbol=motorSelections[i].direction==='forward'?'f':'b';
      selectedMotors.push(`${i}${directionSymbol}`);
    }
  }
  
  if(selectedMotors.length===0){
    display.textContent='Select motors or add stop';
  }else{
    display.textContent=selectedMotors.join(' ');
  }
}

function addMotorCommand(){
  if(isPlaying) return;
  
  const selectedMotors=[];
  const motorsByDirection={forward:[],backward:[]};
  const duration=parseInt(document.getElementById('motorTime').value)||DEFAULT_STEP_TIME;
  
  for(let i=1;i<=4;i++){
    if(motorSelections[i].selected){
      selectedMotors.push(i);
      if(motorSelections[i].direction==='forward'){
        motorsByDirection.forward.push(i);
      }else{
        motorsByDirection.backward.push(i);
      }
    }
  }
  
  if(selectedMotors.length===0){
    alert('Select at least one motor or add stop');
    return;
  }
  
  let cmd='mixedmotor_';
  
  if(motorsByDirection.forward.length>0){
    cmd+=`forward${motorsByDirection.forward.join(',')}`;
  }
  
  if(motorsByDirection.backward.length>0){
    if(motorsByDirection.forward.length>0){
      cmd+='_';
    }
    cmd+=`backward${motorsByDirection.backward.join(',')}`;
  }
  
  cmd+=`_time${duration}`;
  
  commandQueue.push({
    symbol:'M',
    cmd:cmd,
    type:'mixedmotor',
    motorsByDirection:JSON.parse(JSON.stringify(motorsByDirection)),
    motorCount:selectedMotors.length,
    duration:duration
  });
  
  renderQueue();
  closeMotorModal();
}

function addStopCommand(){
  if(isPlaying) return;
  
  const duration=parseInt(document.getElementById('motorTime').value)||DEFAULT_STEP_TIME;
  
  commandQueue.push({
    symbol:'S',
    cmd:`stop_time${duration}`,
    type:'stop',
    duration:duration
  });
  
  renderQueue();
  closeMotorModal();
}

async function testSelectedMotors(){
  const selectedMotors=[];
  const motorsByDirection={forward:[],backward:[]};
  const duration=parseInt(document.getElementById('motorTime').value)||DEFAULT_STEP_TIME;
  
  for(let i=1;i<=4;i++){
    if(motorSelections[i].selected){
      selectedMotors.push(i);
      if(motorSelections[i].direction==='forward'){
        motorsByDirection.forward.push(i);
      }else{
        motorsByDirection.backward.push(i);
      }
    }
  }
  
  if(selectedMotors.length===0){
    alert('Select at least one motor to test');
    return;
  }
  
  if(motorsByDirection.forward.length>0){
    motorsByDirection.forward.forEach(motorNum=>{
      fetch(`/motor?num=${motorNum}&dir=forward&test=true`);
    });
  }
  
  if(motorsByDirection.backward.length>0){
    motorsByDirection.backward.forEach(motorNum=>{
      fetch(`/motor?num=${motorNum}&dir=backward&test=true`);
    });
  }
  
  await new Promise(r=>setTimeout(r,duration));
  
  selectedMotors.forEach(motorNum=>{
    fetch(`/motor?num=${motorNum}&dir=stop`);
  });
}

function clearAllSelections(){
  for(let i=1;i<=4;i++){
    motorSelections[i]={selected:false,direction:null};
    const forwardBtn=document.querySelector(`button[onclick="selectMotorDirection(${i},'forward')"]`);
    const backwardBtn=document.querySelector(`button[onclick="selectMotorDirection(${i},'backward')"]`);
    if(forwardBtn) forwardBtn.classList.remove('selected');
    if(backwardBtn) backwardBtn.classList.remove('selected');
  }
  updateSelectedMotorsDisplay();
}

// ========== PROGRAMACION ==========
function addCommand(symbol,cmd,duration){
  if(isPlaying) return;
  commandQueue.push({symbol:symbol,cmd:cmd,type:'movement',duration:duration||DEFAULT_STEP_TIME});
  renderQueue();
}

function renderQueue(activeIndex=-1){
  const q=document.getElementById("queue");
  q.innerHTML="";
  
  commandQueue.forEach((item,index)=>{
    const el=document.createElement("div");
    
    // Determinar clase basada en tipo
    if(item.type==='loop'){
      el.className="queue-item loop-block";
      el.innerHTML='&#x21BA;';
      const badge=document.createElement("span");
      badge.className="loop-badge";
      badge.textContent=item.count;
      el.appendChild(badge);
    }
    else if(item.type==='mixedmotor'){
      el.className="queue-item motor-block";
      el.textContent='M';
      
      const countBadge=document.createElement("span");
      countBadge.className="motor-count";
      countBadge.textContent=item.motorCount;
      el.appendChild(countBadge);
      
      const forwardMotors=item.motorsByDirection.forward||[];
      const backwardMotors=item.motorsByDirection.backward||[];
      let labelText='';
      
      if(forwardMotors.length>0){
        labelText+=`f${forwardMotors.join(',')}`;
      }
      if(backwardMotors.length>0){
        if(labelText!=='') labelText+=' ';
        labelText+=`b${backwardMotors.join(',')}`;
      }
      
      const label=document.createElement("span");
      label.className="motor-label";
      label.textContent=labelText;
      el.appendChild(label);
    }
    else if(item.type==='stop'){
      el.className="queue-item";
      el.textContent='S';
    }
    else{
      el.className="queue-item";
      el.innerHTML=item.symbol;
    }
    
    // Agregar etiqueta de tiempo si es diferente al default
    if(item.duration && item.duration!==DEFAULT_STEP_TIME){
      const timeLabel=document.createElement("span");
      timeLabel.className="time-label";
      timeLabel.textContent=`${item.duration}ms`;
      el.appendChild(timeLabel);
    }
    
    // Resaltar si está activo - SIN ANIMACIONES
    if(index===activeIndex){
      if(item.type==='stop'){
        el.classList.add("stop-active");
      }else{
        el.classList.add("active");
      }
    }
    
    q.appendChild(el);
  });
}

async function playQueue() {
  if (commandQueue.length === 0 || isPlaying) return;
  
  isPlaying = true;
  
  let executionIndex = 0;
  
  while (executionIndex < commandQueue.length && isPlaying) {
    const item = commandQueue[executionIndex];

    if (item.type === 'loop') {
      // Encontrar el fin del bloque del loop
      let loopEnd = executionIndex + 1;
      while (loopEnd < commandQueue.length && commandQueue[loopEnd].type !== 'loop') {
        loopEnd++;
      }

      // Si no hay comandos dentro del loop, saltar
      if (executionIndex + 1 >= loopEnd) {
        executionIndex = loopEnd;
        continue;
      }

      const repeatCount = item.count || 2;
      
      for (let r = 0; r < repeatCount; r++) {
        if (!isPlaying) break;
        
        for (let j = executionIndex + 1; j < loopEnd; j++) {
          if (!isPlaying) break;

          const loopItem = commandQueue[j];
          
          // 1. CAMBIAR COLOR DE FONDO INMEDIATAMENTE
          renderQueue(j);
          
          // 2. ENVIAR COMANDO INMEDIATAMENTE
          const duration = loopItem.duration || DEFAULT_STEP_TIME;
          fetch(`/control?cmd=${loopItem.cmd}`);
          
          // 3. ESPERAR TIEMPO EXACTO del comando
          await new Promise(r => setTimeout(r, duration));
          
          // 4. SI NO ES EL ULTIMO comando del ULTIMO ciclo
          if (j < loopEnd - 1 || r < repeatCount - 1) {
            fetch("/control?cmd=stop");
            // SIN PAUSA - el cambio de color es inmediato
          }
        }
      }

      executionIndex = loopEnd;
      continue;
    }

    // Ejecutar comando normal (no-loop)
    
    // 1. CAMBIAR COLOR DE FONDO INMEDIATAMENTE
    renderQueue(executionIndex);
    
    // 2. ENVIAR COMANDO INMEDIATAMENTE
    const duration = item.duration || DEFAULT_STEP_TIME;
    fetch(`/control?cmd=${item.cmd}`);
    
    // 3. ESPERAR TIEMPO EXACTO del comando
    await new Promise(r => setTimeout(r, duration));
    
    // 4. SI NO ES EL ULTIMO comando
    if (executionIndex < commandQueue.length - 1) {
      fetch("/control?cmd=stop");
      // SIN PAUSA - el cambio de color es inmediato
    }

    executionIndex++;
  }

  stopQueue();
}

function stopQueue() {
  isPlaying = false;
  currentIndex = 0;
  fetch("/control?cmd=stop");
  renderQueue(); // Quitar todos los colores
}

function pauseQueue() {
  isPlaying = false;
  fetch("/control?cmd=stop");
}

function popLastCommand() {
  if (isPlaying) return;
  commandQueue.pop();
  renderQueue();
}

function clearQueue() {
  stopQueue();
  commandQueue = [];
  renderQueue();
}

// ========== BUCLES ==========
function openLoopModal() {
  document.getElementById("loopModal").classList.remove("hidden");
}

function closeLoopModal() {
  document.getElementById("loopModal").classList.add("hidden");
}

function confirmLoop() {
  const count = parseInt(document.getElementById("loopCount").value);
  
  if (count < 2) {
    alert("Loop must repeat at least 2 times");
    return;
  }
  
  if (commandQueue.length === 0) {
    alert("Add commands before loop");
    return;
  }
  
  commandQueue.push({
    symbol: '&#x21BA;',
    cmd: `loop${count}`,
    type: 'loop',
    count: count
  });
  
  closeLoopModal();
  renderQueue();
}

// ========== UI ==========
function showTab(name) {
  document.getElementById("tab-manual").style.display = name === "manual" ? "block" : "none";
  document.getElementById("tab-program").style.display = name === "program" ? "block" : "none";
  document.querySelectorAll(".tab").forEach(b => b.classList.remove("active"));
  document.querySelectorAll(".tab")[name === "manual" ? 0 : 1].classList.add("active");
}

function closeWelcome() {
  document.getElementById("welcomeModal").style.display = "none";
}

function resetLaps() {
  fetch("/reset", { method: "POST" }).then(() => {
    document.getElementById("mensaje").innerHTML = "Reset";
    setTimeout(() => {
      document.getElementById("mensaje").textContent = "";
    }, 1500);
  });
}

function openMotorModal() {
  document.getElementById("motorModal").classList.remove("hidden");
  updateSelectedMotorsDisplay();
}

function closeMotorModal() {
  document.getElementById("motorModal").classList.add("hidden");
}

// ========== ESTADO EN TIEMPO REAL ==========
setInterval(() => {
  fetch("/status").then(r => r.json()).then(data => {
    document.getElementById("vueltas").textContent = data.vueltas;
    document.getElementById("mensaje").textContent = data.mensaje;
  });
}, 500);
</script></body></html>
)rawliteral";

// Funciones de Movimiento
void setMotor(int index, bool forward) {
  digitalWrite(motorPins[index*2], forward?HIGH:LOW);
  digitalWrite(motorPins[index*2+1], forward?LOW:HIGH);
}

void controlMotorIndividual(int motorNum, String command) {
  motorNum--; // Convertir a indice 0-3
  
  if(motorNum >= 0 && motorNum < 4) {
    if(command == "forward") {
      digitalWrite(motorPins[motorNum*2], HIGH);
      digitalWrite(motorPins[motorNum*2+1], LOW);
      motorStates[motorNum] = true;
      motorDirections[motorNum] = true;
    } 
    else if(command == "backward") {
      digitalWrite(motorPins[motorNum*2], LOW);
      digitalWrite(motorPins[motorNum*2+1], HIGH);
      motorStates[motorNum] = true;
      motorDirections[motorNum] = false;
    }
    else if(command == "stop") {
      digitalWrite(motorPins[motorNum*2], LOW);
      digitalWrite(motorPins[motorNum*2+1], LOW);
      motorStates[motorNum] = false;
    }
    
    Serial.printf("Motor %d: %s\n", motorNum+1, command.c_str());
  }
}

void controlMultipleMotors(String command) {
  int underscore1 = command.indexOf('_');
  int underscore2 = command.indexOf('_', underscore1 + 1);
  
  if(underscore1 > 0 && underscore2 > 0) {
    String direction = command.substring(underscore1 + 1, underscore2);
    String motorsStr = command.substring(underscore2 + 1);
    
    int lastComma = -1;
    for(int i = 0; i <= motorsStr.length(); i++) {
      if(i == motorsStr.length() || motorsStr.charAt(i) == ',') {
        if(lastComma + 1 < i) {
          String motorStr = motorsStr.substring(lastComma + 1, i);
          int motorNum = motorStr.toInt();
          controlMotorIndividual(motorNum, direction);
        }
        lastComma = i;
      }
    }
    
    Serial.printf("MultiMotor: %s - Motores: %s\n", direction.c_str(), motorsStr.c_str());
  }
}

void controlMixedMotors(String command) {
  // Formato: mixedmotor_forward1,3_backward2,4_time1000
  // O: mixedmotor_forward1,2,3,4_time500
  // O: mixedmotor_backward2,4_time2000
  
  Serial.printf("MixedMotor command: %s\n", command.c_str());
  
  // Separar tiempo
  int timeIndex = command.indexOf("_time");
  unsigned long duration = 800; // Default 800ms
  
  if(timeIndex != -1) {
    String timeStr = command.substring(timeIndex + 5);
    duration = timeStr.toInt();
    command = command.substring(0, timeIndex);
  }
  
  // Eliminar "mixedmotor_" del inicio
  command = command.substring(11);
  
  int forwardIndex = command.indexOf("forward");
  int backwardIndex = command.indexOf("backward");
  
  // Procesar motores adelante
  if(forwardIndex != -1) {
    int start = forwardIndex + 7; // "forward" tiene 7 letras
    int end = backwardIndex != -1 && backwardIndex > forwardIndex ? backwardIndex : command.length();
    String forwardMotors = command.substring(start, end);
    
    // Eliminar _ si existe al final
    if(forwardMotors.endsWith("_")) {
      forwardMotors = forwardMotors.substring(0, forwardMotors.length() - 1);
    }
    
    Serial.printf("Forward motors: %s\n", forwardMotors.c_str());
    
    int lastComma = -1;
    for(int i = 0; i <= forwardMotors.length(); i++) {
      if(i == forwardMotors.length() || forwardMotors.charAt(i) == ',') {
        if(lastComma + 1 < i) {
          String motorStr = forwardMotors.substring(lastComma + 1, i);
          int motorNum = motorStr.toInt();
          controlMotorIndividual(motorNum, "forward");
        }
        lastComma = i;
      }
    }
  }
  
  // Procesar motores atrás
  if(backwardIndex != -1) {
    int start = backwardIndex + 8; // "backward" tiene 8 letras
    String backwardMotors = command.substring(start);
    
    Serial.printf("Backward motors: %s\n", backwardMotors.c_str());
    
    int lastComma = -1;
    for(int i = 0; i <= backwardMotors.length(); i++) {
      if(i == backwardMotors.length() || backwardMotors.charAt(i) == ',') {
        if(lastComma + 1 < i) {
          String motorStr = backwardMotors.substring(lastComma + 1, i);
          int motorNum = motorStr.toInt();
          controlMotorIndividual(motorNum, "backward");
        }
        lastComma = i;
      }
    }
  }
  
  // Mantener los motores encendidos por el tiempo especificado
  delay(duration);
  
  // Apagar todos los motores después del tiempo
  detener();
  
  Serial.printf("Motores ejecutados por %lu ms\n", duration);
}

void procesarComandoConTiempo(String cmd) {
  // Verificar si tiene tiempo
  int timeIndex = cmd.indexOf("_time");
  unsigned long duration = 800; // Default
  
  if(timeIndex != -1) {
    String timeStr = cmd.substring(timeIndex + 5);
    duration = timeStr.toInt();
    cmd = cmd.substring(0, timeIndex);
  }
  
  // Procesar comando
  if(cmd == "left") {
    izquierda();
  }
  else if(cmd == "right") {
    derecha();
  }
  else if(cmd == "stop") {
    detener();
  }
  else if(cmd.startsWith("loop")) {
    // Comando de loop - solo para compatibilidad
    detener();
  }
  else if(cmd.startsWith("motor")) {
    int underscorePos = cmd.indexOf('_');
    if(underscorePos > 0) {
      String motorPart = cmd.substring(5, underscorePos);
      int motorNum = motorPart.toInt();
      String action = cmd.substring(underscorePos + 1);
      controlMotorIndividual(motorNum, action);
    }
  }
  else if(cmd.startsWith("multimotor_")) {
    controlMultipleMotors(cmd);
  }
  else if(cmd.startsWith("mixedmotor_")) {
    controlMixedMotors(cmd + "_time" + String(duration));
    return; // Ya incluye delay en la función
  }
  
  // Esperar el tiempo especificado
  delay(duration);
  
  // Detener después del tiempo (excepto para mixedmotor que ya lo hace)
  if(!cmd.startsWith("mixedmotor_")) {
    detener();
  }
}

void aplicarEstadoMotores() {
  for(int i=0; i<4; i++) {
    if(motorStates[i]) {
      if(motorDirections[i]) {
        digitalWrite(motorPins[i*2], HIGH);
        digitalWrite(motorPins[i*2+1], LOW);
      } else {
        digitalWrite(motorPins[i*2], LOW);
        digitalWrite(motorPins[i*2+1], HIGH);
      }
    } else {
      digitalWrite(motorPins[i*2], LOW);
      digitalWrite(motorPins[i*2+1], LOW);
    }
  }
}

void adelante() { 
  detener();
  delay(10);
  
  for(int i=0;i<4;i++) {
    motorStates[i] = true;
    motorDirections[i] = true;
  }
  aplicarEstadoMotores();
}

void atras() { 
  detener();
  delay(10);
  
  for(int i=0;i<4;i++) {
    motorStates[i] = true;
    motorDirections[i] = false;
  }
  aplicarEstadoMotores();
}

void detener() { 
  for(int i=0;i<4;i++) {
    motorStates[i] = false;
  }
  for(int i=0;i<8;i++) digitalWrite(motorPins[i],LOW); 
}

void izquierda() { 
  moverServo(80); 
  delay(80); 
}

void derecha() { 
  moverServo(-80); 
  delay(80); 
}

void moverServo(int pos) { 
  int angulo = map(pos, -100, 100, 125, 55);
  direccion.write(angulo); 
}

void setup() {
  Serial.begin(115200);
  
  // Configurar pines
  for(int i=0; i<8; i++) pinMode(motorPins[i], OUTPUT);
  pinMode(pinSensorLinea, INPUT_PULLUP);
  
  // Servo
  direccion.attach(pinServo);
  direccion.write(90);
  
  // Inicializar estados de motores
  for(int i=0; i<4; i++) {
    motorStates[i] = false;
    motorDirections[i] = true;
  }
  
  // WiFi AP dinamico
  uint64_t chipid = ESP.getEfuseMac();
  String ssidFinal = "BRINKO-" + String((uint32_t)(chipid >> 32), HEX);
  WiFi.softAP(ssidFinal.c_str(), password);
  
  Serial.print("AP: "); Serial.println(ssidFinal);
  Serial.print("IP: "); Serial.println(WiFi.softAPIP());
  
  // Servidor Web
  server.on("/", HTTP_GET, []() { server.send(200, "text/html", html); });
  
  server.on("/control", HTTP_GET, []() {
    if(server.hasArg("cmd")) {
      String cmd = server.arg("cmd");
      procesarComandoConTiempo(cmd);
      Serial.println("CMD: " + cmd);
    }
    
    // Modo joystick
    if(server.hasArg("x") && server.hasArg("y")) {
      int x = server.arg("x").toInt();
      int y = server.arg("y").toInt();
      
      moverServo(x);
      
      if(y > 30) adelante();
      else if(y < -30) atras();
      else detener();
      
      Serial.printf("Joystick X:%d Y:%d\n", x, y);
    }
    server.send(204, "");
  });
  
  // Control individual de motores
  server.on("/motor", HTTP_GET, []() {
    if(server.hasArg("num") && server.hasArg("dir")) {
      int motorNum = server.arg("num").toInt();
      String direction = server.arg("dir");
      bool isTest = server.hasArg("test");
      
      controlMotorIndividual(motorNum, direction);
      
      Serial.printf("Motor %d: %s (test:%s)\n", motorNum, direction.c_str(), isTest?"si":"no");
    }
    server.send(204, "");
  });
  
  server.on("/status", HTTP_GET, []() {
    String json = "{\"vueltas\":" + String(contadorVueltas) + 
                  ",\"mensaje\":\"" + mensaje + "\"}";
    server.send(200, "application/json", json);
  });
  
  server.on("/reset", HTTP_POST, []() {
    contadorVueltas = 0;
    mensaje = "Reinicio";
    server.send(200, "text/plain", "ok");
  });
  
  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  server.handleClient();
  
  // Detector de linea
  bool lectura = digitalRead(pinSensorLinea);
  if(lectura != ultimoEstado) {
    ultimoEstado = lectura;
    if(lectura == LOW) {
      contadorVueltas++;
      Serial.printf("Linea detectada. Vueltas: %lu\n", contadorVueltas);
    }
  }
  
  // Mensaje de meta
  if(contadorVueltas == 5 && mensaje == "") {
    mensaje = "Meta alcanzada!";
    tiempoMensaje = millis();
    contadorVueltas = 0;
  }
  if(mensaje != "" && millis() - tiempoMensaje > 3000) {
    mensaje = "";
  }
  
  delay(9);
}