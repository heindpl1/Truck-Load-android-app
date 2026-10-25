'use strict';

const APP_VERSION='0.7.0';
const SERVICE='6e400001-b5a3-f393-e0a9-e50e24dcca9e',RX='6e400002-b5a3-f393-e0a9-e50e24dcca9e',TX='6e400003-b5a3-f393-e0a9-e50e24dcca9e';

const $=id=>document.getElementById(id);

const state={connected:false,connecting:false,sensor:'OFF',profile:'NONE',target:0,overload:0,points:0,weight:NaN,psi:NaN,percent:NaN,gpsFix:false,plant:false,gpsAuto:false,page:0,pattern:'BAR',work:false,profiles:[]};

const colorModes=[['LOW','Normal load','Below 75% of target','#00ff00'],['MID','Approaching target','75–89% of target','#ffff00'],['HIGH','Near target','90–99% of target','#ffa500'],['FULL','At / over target','100% of target or more','#ff0000'],['DRIVE','Driving mode','Used with automatic driving lights','#000028'],['WORK','Work lights','Choose white for normal work lighting','#ffffff']];

let device,rx,pending='',decoder=new TextDecoder(),writes=Promise.resolve(),toastTimer;

function notify(message,error=false){$('toast').textContent=message;$('toast').classList.toggle('error',error);$('toast').hidden=false;clearTimeout(toastTimer);toastTimer=setTimeout(()=>$('toast').hidden=true,4500);}

function showTab(id){

 document.querySelectorAll('[data-tab]').forEach(b=>{const active=b.dataset.tab===id;b.setAttribute('aria-selected',String(active));b.tabIndex=active?0:-1;});

 document.querySelectorAll('[role="tabpanel"]').forEach(p=>p.hidden=p.id!==id);

}

function render(){

 $('connectionBadge').classList.toggle('online',state.connected);$('connectionBadge').lastChild.textContent=state.connected?'Connected':state.connecting?'Connecting…':'Not connected';

 $('connect').textContent=state.connecting?'Connecting…':state.connected?'Disconnect':'Connect Heltec';$('connect').disabled=state.connecting;

 $('connectionHint').textContent=state.connected?'Live readings and settings from your Heltec.':'Connect your Heltec to see live readings and change settings.';

 document.querySelectorAll('[data-device]').forEach(b=>b.disabled=!state.connected);

 const selected=state.profile!=='NONE';$('activeProfileTop').textContent=selected?state.profile:'No trailer selected';

 const ready=state.sensor==='READY';const hasWeight=ready&&Number.isFinite(state.weight)&&state.weight>=0;

 $('weight').textContent='—';

 // Format the rounded reading separately, avoiding arithmetic/string coercion.

 if(hasWeight)$('weight').textContent=(Math.round(state.weight/10)*10).toLocaleString();

 $('psi').textContent=ready&&Number.isFinite(state.psi)?state.psi.toFixed(1):'—';

 const descriptions={OFF:'Sensor off · bench mode',SETTLING:'Sensor reading settling',UNSTABLE:'Sensor signal is unstable',NO_SIGNAL:'No valid sensor signal',READY:'Sensor reading ready'};

 const summary=descriptions[state.sensor]||'Waiting for sensor';$('sensorSummary').textContent=summary;$('sensorBadge').textContent=state.sensor==='READY'?'Ready':state.sensor==='OFF'?'Off':'Check signal';

 $('footerState').textContent=state.sensor==='OFF'?'Bench mode · Sensor off':summary;

 $('loadBadge').textContent=hasWeight?'Live reading':'Waiting for readings';

 $('loadHint').textContent=state.sensor==='OFF'?'Sensor input is off. Enable it after wiring the sensor.':!ready?'Waiting for a stable pressure reading.':!selected?'Select a trailer to calculate its load.':!hasWeight?'Add at least two calibration points for this trailer.':'Calibrated estimate for '+state.profile+'.';

 const pct=hasWeight&&Number.isFinite(state.percent)?state.percent:NaN;

 $('loadFill').style.width=Number.isFinite(pct)?Math.max(0,Math.min(100,pct))+'%':'0%';

 const slot=!Number.isFinite(pct)?'LOW':pct>=100?'FULL':pct>=90?'HIGH':pct>=75?'MID':'LOW';const pick=$('color-'+slot);$('loadFill').style.background=pick?pick.value:'#54d6b4';

 const meter=document.querySelector('.meter');if(Number.isFinite(pct))meter.setAttribute('aria-valuenow',String(Math.max(0,Math.min(100,pct))));else meter.removeAttribute('aria-valuenow');

 $('percentLabel').textContent=Number.isFinite(pct)?pct.toFixed(1)+'% of target':'— of target';$('targetSummary').textContent=state.target>0?'Target '+state.target.toLocaleString()+' lb':'Target not set';

 $('calibrationCount').textContent=state.points+' of 8 points saved';$('captureButton').disabled=!state.connected||!ready||!selected;

 $('captureHint').textContent=!selected?'Select a trailer first.':!ready?'Requires a stable, enabled sensor reading.':state.points>=8?'All 8 calibration slots are used.':'Ready to capture the current filtered pressure.';

 if(state.points>=8)$('captureButton').disabled=true;

 document.querySelector('[data-action="sensorOff"]').classList.toggle('selected',state.sensor==='OFF');document.querySelector('[data-action="sensorOn"]').classList.toggle('selected',state.sensor!=='OFF');

 document.querySelectorAll('[data-page]').forEach(b=>b.classList.toggle('selected',+b.dataset.page===state.page));

 document.querySelectorAll('[data-pattern]').forEach(b=>b.classList.toggle('selected',b.dataset.pattern===state.pattern));

 $('workState').textContent=state.work?'On':'Off';document.querySelector('[data-action="workOn"]').classList.toggle('selected',state.work);document.querySelector('[data-action="workOff"]').classList.toggle('selected',!state.work);

 $('gpsAutoState').textContent=state.gpsAuto?'On':'Off';document.querySelector('[data-action="gpsAutoOn"]').classList.toggle('selected',state.gpsAuto);document.querySelector('[data-action="gpsAutoOff"]').classList.toggle('selected',!state.gpsAuto);

 $('setPlantButton').disabled=!state.connected||!state.gpsFix;$('plantStatus').textContent=state.plant?'Plant location saved':'No plant location saved';

}

function markDisconnected(){geoUI.disconnected();clearTimeout(stripTimer);stripsReady=false;stripsSaving=false;updateStripEnabled(false);state.connected=false;state.fadePatterns=false;state.connecting=false;rx=null;pending='';decoder=new TextDecoder();state.psi=state.weight=state.percent=NaN;state.gpsFix=false;$('gpsSummary').textContent='Waiting';$('gpsDetail').textContent='Connect for GPS status';$('fixStatus').textContent='Not connected';$('sats').textContent='— satellites';$('speed').textContent='— mph';$('vehicleState').textContent='Unknown';$('plantDistance').textContent='—';render();}

async function connect(){

 if(state.connected){device.gatt.disconnect();return;}

 state.connecting=true;render();

 try{

  if(!navigator.bluetooth)throw new Error('Open this page in Chrome or Edge with Bluetooth support.');

  device=await navigator.bluetooth.requestDevice({filters:[{name:'TruckLoad-Heltec'}],optionalServices:[SERVICE]});

  device.addEventListener('gattserverdisconnected',markDisconnected);

  const server=await device.gatt.connect(),service=await server.getPrimaryService(SERVICE);rx=await service.getCharacteristic(RX);

  const tx=await service.getCharacteristic(TX);pending='';decoder=new TextDecoder();

  tx.addEventListener('characteristicvaluechanged',e=>feed(e.target.value));await tx.startNotifications();

  state.connected=true;state.connecting=false;render();notify('Heltec connected.');

  for(const cmd of ['GET_STATUS','PROFILE_LIST','GET_OLED_LAYOUT','GET_LED_COLORS','GET_LIGHT_STATE','GET_GPS_CONFIG','GET_FW_INFO','GET_STRIP_CONFIG','GET_DRIVE_CONFIG','GET_CAPS'])await send(cmd);

 }catch(error){if(device?.gatt?.connected)device.gatt.disconnect();markDisconnected();notify(error.message,true);}

}

function feed(bytes){pending+=decoder.decode(bytes,{stream:true});let end;while((end=pending.indexOf('\n'))>=0){parse(pending.slice(0,end).trim());pending=pending.slice(end+1);}if(pending.length>4096){pending='';notify('A Bluetooth message was interrupted. Reconnect if readings stop.',true);}}

function send(command){

 const job=writes.catch(()=>{}).then(async()=>{if(!rx||!state.connected)throw new Error('Connect the Heltec first.');await rx.writeValueWithResponse(new TextEncoder().encode(command+'\n'));});

 writes=job;job.catch(error=>notify(error.message,true));return job;

}

function updateInput(id,value){const input=$(id);if(input.dataset.dirty!=='true')input.value=value;}

function parse(line){
 geoUI.record(line);

 const a=line.split('|');

 if(a[0]==='CAPS')state.fadePatterns=(a[2]||'').split(',').includes('FADE_PATTERNS');
 if(a[0]==='DRIVE_CONFIG'){updateInput('drivePatternSelect',a[2]);updateInput('driveBri',Math.max(1,Math.round(+a[3]*100/255)));$('driveSummary').textContent='Mode: '+a[1]+' · '+a[2]+' · '+Math.round(+a[3]*100/255)+'% brightness';}
 if(a[0]==='FW_INFO'){$('stripStatus').textContent='App v'+APP_VERSION+' · Firmware v'+a[1]+(a[1]===APP_VERSION?'':' · Update firmware to match this app.');}
 if(a[0]==='STRIP_CONFIG')readStrips(a);
 if(a[0]==='OK'&&a[1]==='SET_STRIP_CONFIG'){clearTimeout(stripTimer);stripsSaving=false;stripDirty=false;updateStripEnabled(state.connected);for(const id of stripIds)$(id).dataset.dirty='false';$('stripStatus').textContent='Saved on Heltec. Settings survive power loss.';}
 if(a[0]==='ERR'&&['BAD_STRIP_CONFIG','STRIP_SAVE_FAILED'].includes(a[1])){clearTimeout(stripTimer);stripsSaving=false;updateStripEnabled(state.connected);$('stripStatus').textContent=a[1]==='STRIP_SAVE_FAILED'?'Could not save on Heltec. Your edits are still here.':'Invalid strip settings. Check the counts and density.';}
 if(a[0]==='T'){state.psi=+a[1];state.weight=+a[2];state.percent=+a[3];state.target=+a[4];state.profile=a[5]||'NONE';}

 if(a[0]==='SENSOR')state.sensor=a[1]==='OFF'?'OFF':a[2];

 if(a[0]==='STATUS'){

  state.profile=a[1]||'NONE';state.target=+a[2];state.overload=+a[3];state.points=+a[4];state.gpsAuto=a[7]==='GPS_AUTO_ON';state.plant=a[8]==='PLANT_SET';

  updateInput('target',state.target>0?state.target:'');updateInput('overload',state.overload>0?state.overload:'');updateInput('wb',Math.max(1,Math.round(+a[6]/255*100)));$('brightnessValue').textContent=$('wb').value+'%';

 }

 if(a[0]==='GPS'){

  state.gpsFix=a[1]==='1';state.gpsAuto=a[9]==='1';state.plant=a[10]==='1';

  $('gpsSummary').textContent=state.gpsFix?'GPS ready':'Searching';$('gpsDetail').textContent=(+a[6]||0)+' satellites · '+(+a[4]||0).toFixed(1)+' mph';

  $('fixStatus').textContent=state.gpsFix?'GPS is ready':'Waiting for a satellite fix';$('sats').textContent=(+a[6]||0)+' satellites';$('speed').textContent=state.gpsFix?(+a[4]).toFixed(1)+' mph':'— mph';

  const names={NO_FIX:'Waiting for GPS',AT_PLANT:'At plant',DRIVING:'Driving',STOPPED_AWAY:'Stopped away'};$('vehicleState').textContent=names[a[8]]||'Unknown';$('plantDistance').textContent=Number.isFinite(+a[7])?Math.round(+a[7]).toLocaleString()+' m':'—';

 }

 if(a[0]==='GPS_CONFIG'){updateInput('radius',+a[1]);state.plant=a[2]==='1';state.gpsAuto=a[3]==='1';}

 if(a[0]==='OLED_LAYOUT')state.page=+a[1];

 if(a[0]==='LIGHT_STATE'){updateInput('loadPatternSelect',a[2]);state.work=a[1]==='WORK';state.pattern=a[2];updateInput('wb',Math.max(1,Math.round(+a[4]/255*100)));$('brightnessValue').textContent=$('wb').value+'%';}

 if(a[0]==='PLIST_BEGIN')state.profiles=[];

 if(a[0]==='PROFILE')state.profiles.push({name:a[2],active:a[3]==='1',points:+a[4]});

 if(a[0]==='PLIST_END')renderProfiles();

 if(a[0]==='LED_COLOR'&&/^[0-9A-Fa-f]{6}$/.test(a[2]||'')){

  const picker=$('color-'+a[1]);if(picker&&picker.dataset.dirty!=='true'){picker.value='#'+a[2].toLowerCase();colorPreview(a[1]);}

 }

 if(a[0]==='OK'){if(a[1]==='SET_DRIVE_PATTERN'){$('drivePatternSelect').dataset.dirty='false';notify('Driving pattern saved.');}if(a[1]==='SET_DRIVE_BRIGHTNESS'){$('driveBri').dataset.dirty='false';notify('Driving brightness saved.');}if(a[1]==='SET_LED_PATTERN'){$('loadPatternSelect').dataset.dirty='false';}

  const messages={PROFILE_ADD:'Trailer added.',PROFILE_SELECT:'Trailer selected.',CAL_CAPTURE:'Calibration point saved.',SET_TARGET:'Target weight saved.',SET_OVERLOAD:'Overload weight saved.',WORK_LIGHT:'Lighting mode updated.',SET_WORK_LIGHT_BRIGHTNESS:'Brightness saved.',SET_PLANT_HERE:'Plant location saved.',CLEAR_PLANT:'Plant location cleared.',SET_PLANT_RADIUS:'Plant radius saved.',GPS_AUTO:'Driving automation updated.',SENSOR_ENABLE:'Sensor setting updated.',SET_OLED_LAYOUT:'OLED page selected.',SET_LED_PATTERN:'Load pattern selected.',SET_LED_COLOR:'Color saved to the Heltec.'};

  if(a[1]==='SET_LED_COLOR'){const p=$('color-'+a[2]);if(p){p.dataset.dirty='false';colorPreview(a[2]);}}

  if(a[1]==='SET_OLED_LAYOUT')state.page=+a[2];

  if(a[1]==='SET_WORK_LIGHT_BRIGHTNESS')$('wb').dataset.dirty='false';

  if(a[1]==='SET_TARGET')$('target').dataset.dirty='false';if(a[1]==='SET_OVERLOAD')$('overload').dataset.dirty='false';

  if(['PROFILE_SELECT','PROFILE_ADD'].includes(a[1])){$('target').dataset.dirty=$('overload').dataset.dirty='false';send('PROFILE_LIST').catch(()=>{});}

  if(['CAL_CAPTURE','SET_TARGET','SET_OVERLOAD','SENSOR_ENABLE','GPS_AUTO','SET_PLANT_HERE','CLEAR_PLANT'].includes(a[1]))send('GET_STATUS').catch(()=>{});

  if(a[1]==='SET_PLANT_RADIUS'){$('radius').dataset.dirty='false';send('GET_GPS_CONFIG').catch(()=>{});}

  if(messages[a[1]])notify(messages[a[1]]);

 }

 if(a[0]==='ERR'){

  const errors={PRESSURE_INVALID:'No stable sensor reading. Keep bench mode on until the sensor is wired.',CAL_CAPTURE:'Could not capture. Check the trailer, weight and available calibration slots.',PROFILE_ADD:'Could not add the trailer. Check its name and available profile slots.',PROFILE_NOT_FOUND:'That trailer was not found. Refresh the profile list.',NO_GPS_FIX:'Wait for a GPS fix before saving the plant location.',BAD_RADIUS:'Choose a plant radius between 25 and 5,000 meters.',BAD_COLOR:'Choose a valid LED color.',OLED_MANUAL_ONLY:'Choose an OLED page using its button.'};notify(errors[a[1]]||'The Heltec could not apply that setting. Try again.',true);

 }

 render();geoUI.update();

}

function renderProfiles(){

 $('profiles').replaceChildren();$('profileCount').textContent=state.profiles.length+' saved trailer'+(state.profiles.length===1?'':'s');

 if(!state.profiles.length){const p=document.createElement('p');p.className='helper';p.textContent='Add your first trailer below.';$('profiles').append(p);}

 for(const profile of state.profiles){const b=document.createElement('button');b.className='profile-option'+(profile.active?' selected':'');b.type='button';b.dataset.device='';const label=document.createElement('span');label.textContent=profile.name;const sub=document.createElement('small');sub.textContent=profile.points+' points'+(profile.active?' · selected':'');b.append(label,sub);b.onclick=()=>send('PROFILE_SELECT|'+profile.name).catch(()=>{});$('profiles').append(b);}render();

}

function colorPreview(mode){const picker=$('color-'+mode);$('hex-'+mode).textContent=picker.value.toUpperCase();$('color-status-'+mode).textContent=picker.dataset.dirty==='true'?'Not saved yet':'Saved color';$('card-'+mode).classList.toggle('dirty',picker.dataset.dirty==='true');const palette=$('palette-'+mode);if(palette)palette.style.background=picker.value;}

function buildColors(){

 for(const [mode,name,description,hex]of colorModes){

  const card=document.createElement('article');card.className='card color-card';card.id='card-'+mode;

  card.innerHTML='<span class="eyebrow">'+(mode==='DRIVE'||mode==='WORK'?'LIGHTING MODE':'LOAD STAGE')+'</span><h3>'+name+'</h3><p class="helper">'+description+'</p><div class="color-picker-row"><input type="color" id="color-'+mode+'" value="'+hex+'" aria-label="'+name+' color"><div><code id="hex-'+mode+'">'+hex.toUpperCase()+'</code><small id="color-status-'+mode+'">Connect to read saved color</small></div></div><button class="button" data-device id="save-'+mode+'">Save '+(mode==='DRIVE'?'driving':mode==='WORK'?'work-light':'load')+' color</button>';

  $('colorCards').append(card);$('color-'+mode).oninput=()=>{$('color-'+mode).dataset.dirty='true';colorPreview(mode);render();};

  $('save-'+mode).onclick=()=>send('SET_LED_COLOR|'+mode+'|'+$('color-'+mode).value.slice(1)).catch(()=>{});

  if(!['DRIVE','WORK'].includes(mode)){const swatch=document.createElement('span');swatch.id='palette-'+mode;swatch.style.background=hex;swatch.title=name;$('palette').append(swatch);}

 }

}

let stripsReady=false,stripDirty=false,stripsSaving=false,stripTimer;
const stripIds=['groups1','groups2','density1','density2','perGroup1','perGroup2'];
function updateStripEnabled(connected){for(const id of [...stripIds,'length1','length2','calculate1','calculate2','saveStrips'])$(id).disabled=!connected||!stripsReady||stripsSaving;}
function stripSummary(z){const count=+$('groups'+z).value,density=+$('density'+z).value,group=+$('perGroup'+z).value;if(count>0&&density>0&&group>0){$('stripSummary'+z).textContent=count+' groups · '+count*group+' physical LEDs · '+(count*group/density).toFixed(3)+' m';}}
function readStrips(a){if(a.length!==9||!a.slice(1).every(v=>/^\d+$/.test(v)&&+v>0))return;stripsReady=true;updateStripEnabled(state.connected);$('groups1').max=a[7];$('groups2').max=a[8];if(!stripDirty){stripIds.forEach((id,i)=>$(id).value=a[i+1]);for(const z of [1,2]){$('length'+z).value=(+$('groups'+z).value*+$('perGroup'+z).value/+$('density'+z).value).toFixed(3);stripSummary(z);}}}
function bindStrips(){
 for(const id of stripIds)$(id).addEventListener('input',()=>{stripDirty=true;$(id).dataset.dirty='true';stripSummary(id.endsWith('1')?1:2);$('stripStatus').textContent='Changes have not been saved.';});
 for(const z of [1,2])$('calculate'+z).onclick=()=>{const length=+$('length'+z).value,density=+$('density'+z).value,group=+$('perGroup'+z).value,raw=length*density/group,count=Math.round(raw);if(!Number.isFinite(raw)||length<=0||density<1||group<1||count<1||count>+$('groups'+z).max){notify('Length must fit the supported group count. Check LED density and LEDs per group.',true);return;}$('groups'+z).value=count;stripDirty=true;stripSummary(z);$('stripStatus').textContent='Calculated '+count+' whole groups'+(Math.abs(raw-count)>0.0001?' (rounded to the nearest complete group)':'')+'. Review the size above, then save.';};
 $('stripForm').onsubmit=async e=>{e.preventDefault();if(!state.connected||!stripsReady||stripsSaving)return;const values=stripIds.map(id=>$(id).value);if(!stripIds.every(id=>$(id).checkValidity()&&/^\d+$/.test($(id).value))){notify('Enter whole numbers within the displayed limits.',true);return;}stripsSaving=true;updateStripEnabled(state.connected);stripTimer=setTimeout(()=>{stripsSaving=false;updateStripEnabled(state.connected);$('stripStatus').textContent='Save confirmation unavailable. Reconnect to read saved sizes before retrying.';},6000);$('stripStatus').textContent='Saving… waiting for Heltec confirmation.';try{await send('SET_STRIP_CONFIG|'+values.join('|'));}catch{clearTimeout(stripTimer);stripsSaving=false;updateStripEnabled(state.connected);$('stripStatus').textContent='Save interrupted. Reconnect and read the Heltec settings.';}};
}
function bind(){
 $('loadPatternForm').onsubmit=e=>{e.preventDefault();send('SET_LED_PATTERN|'+$('loadPatternSelect').value).catch(()=>{});};
 $('drivePatternForm').onsubmit=e=>{e.preventDefault();const value=$('drivePatternSelect').value;if(value==='GROUND'&&!geoUI.ready){notify('Connect firmware 0.7.0 for ground-speed lighting.',true);return;}if(['COMET','SCANNER','WAVE','AURORA','THEATER'].includes(value)&&!state.fadePatterns){notify('Update firmware to v0.6.4 for the new patterns.',true);return;}send('SET_DRIVE_PATTERN|'+value).catch(()=>{});};
 $('driveBrightnessForm').onsubmit=e=>{e.preventDefault();if($('driveBri').checkValidity())send('SET_DRIVE_BRIGHTNESS|'+Math.max(1,Math.round(+$('driveBri').value*255/100))).catch(()=>{});};


 document.querySelectorAll('[data-tab]').forEach((b,i)=>{b.onclick=()=>showTab(b.dataset.tab);b.onkeydown=e=>{const tabs=[...document.querySelectorAll('[data-tab]')];let n;if(e.key==='ArrowRight')n=(i+1)%tabs.length;if(e.key==='ArrowLeft')n=(i+tabs.length-1)%tabs.length;if(e.key==='Home')n=0;if(e.key==='End')n=tabs.length-1;if(n!==undefined){e.preventDefault();showTab(tabs[n].dataset.tab);tabs[n].focus();}};});

 document.querySelectorAll('[data-go]').forEach(b=>b.onclick=()=>showTab(b.dataset.go));document.querySelector('.brand').onclick=e=>{e.preventDefault();showTab('dashboard');};$('connect').onclick=connect;

 const actions={sensorOff:'SENSOR_ENABLE|OFF',sensorOn:'SENSOR_ENABLE|ON',refreshProfiles:'PROFILE_LIST',workOff:'WORK_LIGHT|OFF',workOn:'WORK_LIGHT|ON',setPlant:'SET_PLANT_HERE',clearPlant:'CLEAR_PLANT',gpsAutoOff:'GPS_AUTO|OFF',gpsAutoOn:'GPS_AUTO|ON',driveOn:'DRIVE_CONTROL|ON',driveLoad:'DRIVE_CONTROL|LOAD',driveAuto:'DRIVE_CONTROL|AUTO'};

 document.querySelectorAll('[data-action]').forEach(b=>b.onclick=()=>{const cmd=b.dataset.action==='saveBrightness'?'SET_WORK_LIGHT_BRIGHTNESS|'+Math.max(1,Math.round(+$('wb').value/100*255)):actions[b.dataset.action];send(cmd).catch(()=>{});});

 document.querySelectorAll('[data-pattern]').forEach(b=>b.onclick=()=>send('SET_LED_PATTERN|'+b.dataset.pattern).catch(()=>{}));document.querySelectorAll('[data-page]').forEach(b=>b.onclick=()=>send('SET_OLED_LAYOUT|'+b.dataset.page).catch(()=>{}));

 for(const id of ['target','overload','radius','wb','loadPatternSelect','drivePatternSelect','driveBri'])$(id).addEventListener('input',()=>{$(id).dataset.dirty='true';if(id==='wb')$('brightnessValue').textContent=$('wb').value+'%';});

 $('addProfile').onsubmit=e=>{e.preventDefault();const name=$('pname').value.trim().replaceAll('|','-');if(!name||new TextEncoder().encode(name).length>27){notify('Choose a shorter trailer name.',true);return;}send('PROFILE_ADD|'+name).catch(()=>{});};

 $('loadSettings').onsubmit=async e=>{e.preventDefault();if(state.profile==='NONE'){notify('Select a trailer first.',true);return;}try{await send('SET_TARGET|'+$('target').value);await send('SET_OVERLOAD|'+$('overload').value);}catch{}};

 $('capture').onsubmit=e=>{e.preventDefault();if(state.sensor!=='READY'||state.profile==='NONE'){notify('Select a trailer and wait for a stable sensor reading.',true);return;}send('CAL_CAPTURE|'+$('known').value).catch(()=>{});};$('radiusForm').onsubmit=e=>{e.preventDefault();send('SET_PLANT_RADIUS|'+$('radius').value).catch(()=>{});};

}

const geoUI=new GeoUI();buildColors();bind();bindStrips();render();geoUI.update();
