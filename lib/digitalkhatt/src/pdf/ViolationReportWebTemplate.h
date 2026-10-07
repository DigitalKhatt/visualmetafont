#pragma once

namespace digitalkhatt::pdf::web {
inline constexpr char beforeData[] = R"DKWEB(<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>XPBD · Placement review</title>
<style>
:root{font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;color:#202c2b;background:#f6f7f5;--muted:#66736f;--line:#dce3de;--accent:#126653;--red:#c23635;--amber:#ac690a;--blue:#3972c4;--side-margin:clamp(16px,3vw,72px);--tile-width:160px;--tile-height:100px;--image-scale:2.2}
*{box-sizing:border-box}body{margin:0}button,input,select{font:inherit}button,a,input,select{outline-offset:3px}button{cursor:pointer}button:disabled{opacity:.4;cursor:default}a{color:var(--accent)}
header{padding:12px var(--side-margin);background:#fff;border-bottom:1px solid var(--line);display:flex;align-items:center;justify-content:space-between;gap:16px}h1{font-size:20px;font-weight:650;letter-spacing:-.6px;margin:3px 0}h2{font-size:19px;margin:0}.eyebrow{font-size:9px;font-weight:700;letter-spacing:1.8px;color:var(--accent)}.muted{color:var(--muted)}.subtitle{font-size:11px;margin:4px 0 0}.actions{display:flex;gap:6px;align-items:center;flex-wrap:wrap}.button,button{background:#fff;border:1px solid var(--line);border-radius:7px;padding:7px 10px;font-size:12px;color:inherit;text-decoration:none}.primary{background:var(--accent);border-color:var(--accent);color:#fff}button:hover,.button:hover{filter:brightness(.96)}
main{width:100%;padding:10px var(--side-margin)}details{font-size:11px;color:var(--muted);margin:0 0 8px}details summary{cursor:pointer}details ul{line-height:1.8;padding-left:20px}.toolbar{background:#fff;border:1px solid var(--line);border-radius:7px;padding:8px;display:flex;gap:8px;align-items:end;flex-wrap:wrap}.toolbar label{display:grid;gap:3px;font-size:9px;font-weight:650;letter-spacing:.3px;color:var(--muted)}input,select{background:#fff;color:#202c2b;border:1px solid #cad5ce;border-radius:5px;padding:6px 7px;font-size:11px;min-width:100px}input[type=number]{width:68px;min-width:0}.search{flex:1;min-width:170px}.search input{width:100%}.toolbar .check{display:flex;gap:5px;align-items:center;height:29px;letter-spacing:0;font-size:11px}input[type=checkbox]{width:auto;min-width:0;accent-color:var(--accent)}.second{display:flex;align-items:center;justify-content:space-between;gap:10px;margin:10px 0 8px;flex-wrap:wrap}.count{font-size:11px;font-weight:650}.legend{display:flex;gap:12px;font-size:10px;color:var(--muted)}.dot{display:inline-block;width:7px;height:7px;border-radius:50%;margin-right:4px}.red{background:var(--red)}.amber{background:var(--amber)}.blue{background:var(--blue)}
.grid{display:grid;width:60%;margin-inline:auto;grid-template-columns:repeat(auto-fill,minmax(min(100%,calc(var(--tile-width)*var(--image-scale))),1fr));gap:6px}.card{background:#fff;border:1px solid var(--line);border-radius:5px;overflow:hidden}.preview{display:block;border:0;border-radius:0;padding:4px;width:100%;background:#fff;text-align:left}.preview:hover{background:#f1f6f0;filter:none}.preview:focus-visible{outline:2px solid var(--accent);outline-offset:-2px}.card svg{display:block;width:100%;height:calc(var(--tile-height)*var(--image-scale))}.empty{padding:60px 20px;text-align:center;grid-column:1/-1}.pager{display:flex;justify-content:center;align-items:center;gap:14px;margin:16px 0;font-size:12px}.foot{font-size:11px;color:var(--muted);line-height:1.7}.notice{color:#98611b}
dialog{border:1px solid var(--line);border-radius:12px;padding:0;width:min(1080px,94vw);max-height:94vh;box-shadow:0 15px 90px #163d2833}dialog::backdrop{background:#142f2877}.modalhead{display:flex;align-items:center;justify-content:space-between;gap:12px;padding:18px 22px;border-bottom:1px solid var(--line)}.modalcontent{display:grid;grid-template-columns:minmax(0,1fr) 270px}.stage{padding:20px;background:#fafbf8;min-height:390px;display:flex;flex-direction:column}.viewport{flex:1;display:flex;align-items:center;overflow:auto;min-height:340px}.viewport svg{width:100%;height:340px;flex:none}.zoom{display:flex;align-items:center;justify-content:center;gap:10px;font-size:11px}.zoom input{width:150px}.info{padding:20px;border-left:1px solid var(--line);font-size:12px;line-height:1.6;overflow-wrap:anywhere}.info dl{display:grid;grid-template-columns:1fr 1fr;gap:7px;margin:14px 0}.info dt{color:var(--muted)}.info dd{margin:0;font-variant-numeric:tabular-nums}.info p{margin:14px 0}.info .review{width:100%;margin-top:6px}.modalnav{display:flex;gap:8px}.glyphs{font-size:11px}.wordtext{font-size:24px;line-height:1.8}.notes{width:100%;min-height:70px;border:1px solid var(--line);border-radius:5px;padding:8px;font:inherit;resize:vertical}
@media(max-width:720px){:root{--side-margin:12px;--tile-width:120px;--tile-height:90px}header{padding:10px var(--side-margin);display:block}.actions{margin-top:8px}main{padding:8px var(--side-margin)}.modalcontent{grid-template-columns:1fr}.info{border-left:0;border-top:1px solid var(--line)}.stage{min-height:230px}.viewport,.viewport svg{min-height:210px;height:210px}.toolbar{gap:6px}.grid{gap:4px}}
.sizeControl{display:flex;align-items:center;gap:5px;height:29px}.sizeControl button{padding:4px 8px;font-size:14px;line-height:18px}.sizeControl input{width:110px;min-width:0;padding:0;accent-color:var(--accent)}.sizeControl output{min-width:34px;font-size:11px;font-variant-numeric:tabular-nums}
</style></head><body>
<header><div><div class="eyebrow">DIGITALKHATT / XPBD</div><h1>Placement review</h1><p class="subtitle muted" id="overview"></p></div><div class="actions"><a class="button" id="detailLink">Detailed PDF ↗</a><a class="button" id="csvLink">CSV ↗</a><button id="import">Import reviews</button><input type="file" id="importFile" accept="application/json,.json" hidden><button class="primary" id="export">Export reviews</button></div></header>
<main><details><summary>Report settings and reading guide</summary><ul id="notes"></ul><p>Word numbers count source tokens within a line, including standalone ayah tokens. Contexts show the same geometry as the PDFs. Save Collision reporting uses decomposed outlines for all glyphs; full solver reporting uses mark hulls and decomposed bases. Outlines approximate the font curves. Filters apply only to findings retained by the report settings.</p></details>
<section class="toolbar" aria-label="Finding filters">
<label class="search">SEARCH<input id="search" type="search" placeholder="Word, glyph name or diagnostic…"></label>
<label>CONSTRAINT<select id="type"><option value="">All constraints</option></select></label>
<label>KIND<select id="kind"><option value="">All kinds</option><option value="hard">Hard</option><option value="soft">Review / soft</option></select></label>
<label>CHANGE<select id="change"><option value="">All findings</option><option value="changed">New or worse</option><option value="new">New</option><option value="worse">Worse</option><option value="unchanged">Unchanged</option></select></label>
<label>REVIEW<select id="reviewFilter"><option value="">All reviews</option><option value="unreviewed">Unreviewed</option><option value="acceptable">Acceptable</option><option value="fix">Needs fix</option></select></label>
<label>MIN. EXCESS<input id="minimum" type="number" min="0" step="1" value="0"></label>
<label>PAGE<input id="page" type="number" min="1" placeholder="Any"></label><label>LINE<input id="line" type="number" min="1" placeholder="Any"></label><label>WORD<input id="word" type="number" min="1" placeholder="Any"></label>
<label>SORT<select id="sort"><option value="rank">Report order</option><option value="severity">Severity</option><option value="priority">Review priority</option><option value="location">Page / line / word</option></select></label>
<label>CASES PER PAGE<select id="perPage"><option value="24">24</option><option value="48">48</option><option value="96">96</option><option value="192">192</option><option value="384">384</option><option value="all" selected>All</option></select></label>
<div><label for="imageSize">IMAGE SIZE</label><div class="sizeControl"><button id="smallerImages" aria-label="Decrease image size">−</button><input id="imageSize" type="range" min="60" max="380" step="10" value="220"><button id="largerImages" aria-label="Increase image size">+</button><output id="imageSizeLabel" for="imageSize">220%</output></div></div>
<div><label for="gridWidth">GRID WIDTH</label><div class="sizeControl"><button id="narrowerGrid" aria-label="Decrease grid width">−</button><input id="gridWidth" type="range" min="20" max="100" step="5" value="60"><button id="widerGrid" aria-label="Increase grid width">+</button><output id="gridWidthLabel" for="gridWidth">60%</output></div></div>
<label class="check"><input id="original" type="checkbox" checked>Shaped positions</label><button id="reset">Reset filters</button></section>
<div class="second"><span class="count" id="count" role="status" aria-live="polite"></span><div class="legend"><span><i class="dot red"></i>Hard finding</span><span><i class="dot amber"></i>Review / soft</span><span><i class="dot blue"></i>Shaped position</span></div></div>
<section class="grid" id="grid" aria-label="Violation contexts"></section><nav class="pager" aria-label="Results pagination"><button id="previous">← Previous</button><span id="pagination"></span><button id="next">Next →</button></nav>
<p class="foot">Hover for a reference; click an image for details and review controls. Use ← / → in the detail view to move through filtered findings.<br><span id="persistence">Review labels and notes are saved in this browser for this report. Export them to keep a portable copy.</span></p></main>
<dialog id="detail" aria-labelledby="detailTitle"><div class="modalhead"><div><div class="eyebrow" id="detailRank"></div><h2 id="detailTitle"></h2></div><div class="modalnav"><button id="detailPrev" aria-label="Previous finding">←</button><button id="detailNext" aria-label="Next finding">→</button><button id="close">Close ✕</button></div></div><div class="modalcontent"><div class="stage"><div class="viewport" id="scene"></div><label class="zoom">Zoom<input id="zoom" type="range" min="100" max="300" step="25" value="100"><span id="zoomLabel">100%</span></label></div><aside class="info" id="info"></aside></div></dialog>
<script id="reportData" type="application/json">
)DKWEB";

inline constexpr char afterData[] = R"DKWEB(</script>
<script>
'use strict';
function startReport(reportId){
const data=JSON.parse(document.getElementById('reportData').textContent), entries=data.entries;
const $=id=>document.getElementById(id), ns='http://www.w3.org/2000/svg';
const searchable=text=>text.normalize('NFKD').replace(/\p{M}/gu,'').toLocaleLowerCase();
for(const e of entries)e.searchText=searchable([e.type,e.detail,...e.glyphs.flatMap(g=>[g.name,g.text])].join(' '));
const storeKey='digitalkhatt-xpbd:'+reportId;let reviews={},filtered=[],batch=0,active=null,searchTimer;
try{reviews=JSON.parse(localStorage.getItem(storeKey)||'{}');if(!reviews||typeof reviews!=='object'||Array.isArray(reviews))reviews={};}catch{storageNotice();}
function storageNotice(){$('persistence').textContent='Browser storage is unavailable. Export reviews before closing this page.';$('persistence').classList.add('notice');}
function state(e){const s=reviews[e.rank]?.state;return ['acceptable','fix'].includes(s)?s:'unreviewed';}
function save(e,value,note){reviews[e.rank]={state:value,note:note??reviews[e.rank]?.note??''};try{localStorage.setItem(storeKey,JSON.stringify(reviews));}catch{storageNotice();}}
function element(tag,cls,text){const el=document.createElement(tag);if(cls)el.className=cls;if(text!==undefined)el.textContent=text;return el;}
function svgElement(tag,attrs){const el=document.createElementNS(ns,tag);for(const [k,v]of Object.entries(attrs))el.setAttribute(k,v);return el;}
function location(e){let text=`Page ${e.page} · Line ${e.line} · Word ${e.word}`;if(e.otherLine>0&&(e.otherLine!==e.line||e.otherWord!==e.word))text+=` / L${e.otherLine} W${e.otherWord}`;return text;}
function preview(e){
 const svg=svgElement('svg',{role:'img','aria-label':location(e)+' '+e.type});let box=[Infinity,Infinity,-Infinity,-Infinity];
 for(const g of e.glyphs){const b=g.box;box=[Math.min(box[0],b[0]),Math.min(box[1],b[1]),Math.max(box[2],b[2]),Math.max(box[3],b[3])];
  if($('original').checked&&(g.index===e.a||g.index===e.b)){box=[Math.min(box[0],b[0]-g.dx),Math.min(box[1],b[1]-g.dy),Math.max(box[2],b[2]-g.dx),Math.max(box[3],b[3]-g.dy)];}}
 if(!Number.isFinite(box[0]))return svg;
 const pad=Math.max(box[2]-box[0],box[3]-box[1],1)*.055;svg.setAttribute('viewBox',`${box[0]-pad} ${-box[3]-pad} ${Math.max(1,box[2]-box[0])+2*pad} ${Math.max(1,box[3]-box[1])+2*pad}`);
 const group=svgElement('g',{transform:'scale(1,-1)',fill:'none','stroke-linejoin':'round'});svg.append(group);
 const path=(g,stroke,width,extra={})=>group.append(svgElement('path',{d:g.path,stroke,'stroke-width':width,'vector-effect':'non-scaling-stroke',...extra}));
 for(const g of e.glyphs)path(g,'#a3afa6',.65);
 for(const g of e.glyphs){if(g.index!==e.a&&g.index!==e.b)continue;
  if($('original').checked&&Math.hypot(g.dx,g.dy)>1e-9){path(g,'#3972c4',.8,{transform:`translate(${-g.dx},${-g.dy})`,'stroke-dasharray':'3 2'});
   const cx=(g.box[0]+g.box[2])/2,cy=(g.box[1]+g.box[3])/2;group.append(svgElement('line',{x1:cx-g.dx,y1:cy-g.dy,x2:cx,y2:cy,stroke:'#3972c4','stroke-width':.65,'vector-effect':'non-scaling-stroke'}));}
  path(g,e.hard?'#c23635':'#ac690a',1.1);}
 if(e.markers.length===2)group.append(svgElement('line',{x1:e.markers[0][0],y1:e.markers[0][1],x2:e.markers[1][0],y2:e.markers[1][1],stroke:'#bc4ab3','stroke-width':1.1,'vector-effect':'non-scaling-stroke'}));
 else if(e.markers.length===1){const m=e.markers[0],r=pad*.1;group.append(svgElement('path',{d:`M${m[0]-r} ${m[1]}L${m[0]+r} ${m[1]}M${m[0]} ${m[1]-r}L${m[0]} ${m[1]+r}`,stroke:'#bc4ab3','stroke-width':1,'vector-effect':'non-scaling-stroke'}));}
 return svg;
}
function reviewSelect(e){const select=element('select');select.setAttribute('aria-label','Review finding '+e.rank);for(const [value,label]of [['unreviewed','Unreviewed'],['acceptable','Acceptable'],['fix','Needs fix']]){const option=element('option',null,label);option.value=value;select.append(option);}select.value=state(e);select.addEventListener('change',()=>{save(e,select.value);apply(false);});return select;}
function render(){
 const perPage=$('perPage').value==='all'?Math.max(1,filtered.length):Number($('perPage').value);
 batch=Math.max(0,Math.min(batch,Math.ceil(filtered.length/perPage)-1));$('grid').replaceChildren();
 for(const e of filtered.slice(batch*perPage,(batch+1)*perPage)){
  const card=element('article','card'),button=element('button','preview');
  const reference=`#${e.rank} · ${location(e)}`;
  button.setAttribute('aria-label','Inspect '+reference+' · '+e.type);
  button.title=`${reference}\n${e.type} · ${e.hard?'Hard':'Review / soft'} · ${e.structural?'Structural':'Excess '+e.severity.toFixed(2)}\nChange: ${e.status} · Review: ${state(e)}\n${e.detail}\nClick for details`;
  button.append(preview(e));button.addEventListener('click',()=>open(e.rank));card.append(button);$('grid').append(card);
 }
 if(!filtered.length)$('grid').append(element('div','empty muted','No findings match these filters.'));
 $('count').textContent=`${filtered.length.toLocaleString()} of ${entries.length.toLocaleString()} retained findings · ${entries.filter(e=>state(e)!=='unreviewed').length} reviewed`;
 $('pagination').textContent=filtered.length?`${batch*perPage+1}–${Math.min((batch+1)*perPage,filtered.length)} of ${filtered.length}`:'0 results';$('previous').disabled=batch===0;$('next').disabled=(batch+1)*perPage>=filtered.length;
}
function apply(reset=true){
 if(reset)batch=0;const query=searchable($('search').value.trim()),type=$('type').value,kind=$('kind').value,change=$('change').value,review=$('reviewFilter').value,min=Number($('minimum').value)||0;
 filtered=entries.filter(e=>{
  if(type&&e.type!==type||kind&&(e.hard?'hard':'soft')!==kind||change==='changed'&&e.status==='unchanged'||change&&change!=='changed'&&e.status!==change||review&&state(e)!==review||!e.structural&&e.severity<min)return false;
  if($('page').value&&e.page!==Number($('page').value))return false;
  // Line and word refer to the same participant, rather than separate ends of a pair.
  const match=(line,word)=>(!$('line').value||line===Number($('line').value))&&(!$('word').value||word===Number($('word').value));
  if(!match(e.line,e.word)&&!(e.otherLine>0&&match(e.otherLine,e.otherWord)))return false;
  return !query||e.searchText.includes(query);
 });
 const sort=$('sort').value;
 filtered.sort((a,b)=>sort==='location'?a.page-b.page||a.line-b.line||a.word-b.word||a.rank-b.rank:sort==='rank'?a.rank-b.rank:a.group-b.group||((a.structural||sort==='priority')?b.priority-a.priority:0)||b.severity-a.severity||a.rank-b.rank);
 render();if($('detail').open&&active!==null)renderDetail();
}
function renderDetail(){
 const e=entries.find(e=>e.rank===active);if(!e)return;
 $('detailRank').textContent=`FINDING #${e.rank} / ${e.type}`;$('detailTitle').textContent=location(e);$('scene').replaceChildren(preview(e));$('zoom').value=100;zoom();
 const info=$('info');info.replaceChildren(element('strong',null,e.hard?'Hard constraint':'Review / soft'));
 const word=Array.from(new Set(e.glyphs.map(g=>g.text).filter(Boolean))).join(' · ');const source=element('div','wordtext',word);source.dir='rtl';info.append(source);
 const metrics=element('dl');for(const [label,value]of [['Excess severity',e.structural?'Structural':e.severity.toFixed(4)],['Raw severity',e.rawSeverity.toFixed(4)],['Allowed slack',e.allowed.toFixed(4)],['Signed residual',e.residual.toFixed(4)],['Initial severity',e.initial<0?'No matching finding':e.initial.toFixed(4)],['Change',e.status]])metrics.append(element('dt',null,label),element('dd',null,value));info.append(metrics,element('p',null,e.detail));
 if(e.waqf){const w=e.waqf,values=element('dl'),format=v=>v==null?'No previous line':v.toFixed(2);info.append(element('strong',null,'Waqf association (font units)'));
  for(const [label,value]of [['Horizontal offset',`${Math.abs(w.horizontalOffset).toFixed(2)} ${w.horizontalOffset<0?'left':'right'}`],['Allowed left / right',`${w.allowedLeft.toFixed(2)} / ${w.allowedRight.toFixed(2)}`],['Top above own baseline',format(w.heightAboveBaseline)],['Below previous baseline',format(w.previousBaselineDistance)],['Required baseline margin',format(w.previousLineMargin)],['Previous ink-box clearance',format(w.previousInkBoxClearance)]])values.append(element('dt',null,label),element('dd',null,value));info.append(values,element('p','muted','Baseline intrusion indicates association risk. Ink-box clearance is a bounding-box distance, not an exact collision measurement.'));}
 for(const index of [...new Set([e.a,e.b])]){const g=e.glyphs.find(g=>g.index===index);if(g)info.append(element('p','glyphs',`#${g.index} ${g.name} · base ${g.base<0?'none':'#'+g.base} · shift (${g.dx.toFixed(3)}, ${g.dy.toFixed(3)})`));}
 info.append(element('label','muted','Review decision'));const select=reviewSelect(e);select.className='review';info.append(select);
 const note=element('textarea','notes');note.placeholder='Review note…';note.setAttribute('aria-label','Review note');note.value=reviews[e.rank]?.note||'';note.addEventListener('input',()=>save(e,state(e),note.value));info.append(element('p','muted','Notes'),note);
 const idx=filtered.findIndex(x=>x.rank===active);$('detailPrev').disabled=idx<=0;$('detailNext').disabled=idx<0||idx>=filtered.length-1;
}
function open(rank){active=rank;renderDetail();$('detail').showModal();}
function step(delta){const idx=filtered.findIndex(e=>e.rank===active),e=filtered[idx+delta];if(idx>=0&&e){active=e.rank;renderDetail();}}
function zoom(){const v=Number($('zoom').value);const svg=$('scene').querySelector('svg');if(svg){svg.style.width=v+'%';svg.style.height=(window.innerWidth<=720?210:340)*v/100+'px';}$('zoomLabel').textContent=v+'%';}
function resizeImages(){const input=$('imageSize'),value=Number(input.value);document.documentElement.style.setProperty('--image-scale',value/100);$('imageSizeLabel').textContent=value+'%';$('smallerImages').disabled=value<=Number(input.min);$('largerImages').disabled=value>=Number(input.max);}
function changeImageSize(delta){const input=$('imageSize');input.value=Math.max(Number(input.min),Math.min(Number(input.max),Number(input.value)+delta));resizeImages();}
function resizeGrid(){const input=$('gridWidth'),value=Number(input.value);$('grid').style.width=value+'%';$('gridWidthLabel').textContent=value+'%';$('narrowerGrid').disabled=value<=Number(input.min);$('widerGrid').disabled=value>=Number(input.max);}
function changeGridWidth(delta){const input=$('gridWidth');input.value=Math.max(Number(input.min),Math.min(Number(input.max),Number(input.value)+delta));resizeGrid();}
$('overview').textContent=`${entries.length.toLocaleString()} retained findings across ${new Set(entries.map(e=>e.page)).size} Mushaf pages · ${data.eligible.toLocaleString()} eligible before the report limit`;
for(const note of data.notes)$('notes').append(element('li',null,note));
for(const type of [...new Set(entries.map(e=>e.type))].sort()){const option=element('option',null,`${type} (${entries.filter(e=>e.type===type).length})`);option.value=type;$('type').append(option);}
const filename=decodeURIComponent(locationPath());function locationPath(){return window.location.pathname.split('/').pop()||'violations.html';}
const stem=filename.replace(/\.html$/i,'');$('detailLink').href='./'+encodeURIComponent(stem+'_summary.pdf');$('csvLink').href='./'+encodeURIComponent(stem+'.csv');
for(const id of ['type','kind','change','reviewFilter','sort','perPage','original'])$(id).addEventListener('change',()=>apply());
for(const id of ['minimum','page','line','word'])$(id).addEventListener('input',()=>apply());
$('imageSize').addEventListener('input',resizeImages);$('smallerImages').addEventListener('click',()=>changeImageSize(-10));$('largerImages').addEventListener('click',()=>changeImageSize(10));
$('gridWidth').addEventListener('input',resizeGrid);$('narrowerGrid').addEventListener('click',()=>changeGridWidth(-5));$('widerGrid').addEventListener('click',()=>changeGridWidth(5));
$('search').addEventListener('input',()=>{clearTimeout(searchTimer);searchTimer=setTimeout(()=>apply(),150);});
$('reset').addEventListener('click',()=>{for(const id of ['search','type','kind','change','reviewFilter','page','line','word'])$(id).value='';$('minimum').value=0;$('sort').value='rank';$('original').checked=true;apply();});
$('previous').addEventListener('click',()=>{--batch;render();});$('next').addEventListener('click',()=>{++batch;render();});
$('close').addEventListener('click',()=>$('detail').close());$('detailPrev').addEventListener('click',()=>step(-1));$('detailNext').addEventListener('click',()=>step(1));$('zoom').addEventListener('input',zoom);
$('detail').addEventListener('click',e=>{if(e.target===$('detail')){const b=$('detail').getBoundingClientRect();if(e.clientX<b.left||e.clientX>b.right||e.clientY<b.top||e.clientY>b.bottom)$('detail').close();}});
document.addEventListener('keydown',e=>{if(!$('detail').open||['INPUT','TEXTAREA','SELECT'].includes(e.target.tagName))return;if(e.key==='ArrowLeft'||e.key==='ArrowRight'){e.preventDefault();step(e.key==='ArrowLeft'?-1:1);}});
$('export').addEventListener('click',()=>{const blob=new Blob([JSON.stringify({reportId,exportedAt:new Date().toISOString(),reviews},null,2)],{type:'application/json'}),url=URL.createObjectURL(blob),a=element('a');a.href=url;a.download=stem+'_reviews.json';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);});
$('import').addEventListener('click',()=>$('importFile').click());$('importFile').addEventListener('change',async()=>{const file=$('importFile').files[0];if(!file)return;try{const imported=JSON.parse(await file.text());if(imported.reportId!==reportId||!imported.reviews||typeof imported.reviews!=='object')throw Error('This review file belongs to a different report.');for(const e of entries){const r=imported.reviews[e.rank];if(r&&['unreviewed','acceptable','fix'].includes(r.state)&&typeof r.note==='string')reviews[e.rank]={state:r.state,note:r.note};}try{localStorage.setItem(storeKey,JSON.stringify(reviews));}catch{storageNotice();}apply();}catch(e){alert('Cannot import reviews: '+e.message);}finally{$('importFile').value='';}});
resizeImages();resizeGrid();apply();
}
</script>
)DKWEB";
}  // namespace digitalkhatt::pdf::web
