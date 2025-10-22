/* Diwali Fireworks — fireworks.js
   Creates a physics-driven fireworks display with launch, apex pause,
   burst patterns (peony, willow, crackle), trails, color variations, and
   click-to-launch interaction.

   Lightweight, dependency-free. Configure via the UI controls in index.html.
*/

(() => {
  const canvas = document.getElementById('canvas');
  const ctx = canvas.getContext('2d');
  let W = canvas.width = innerWidth;
  let H = canvas.height = innerHeight;
  let DPR = Math.max(1, devicePixelRatio || 1);
  canvas.width = W * DPR;
  canvas.height = H * DPR;
  canvas.style.width = W + 'px';
  canvas.style.height = H + 'px';
  ctx.scale(DPR, DPR);

  // UI
  const patternSelect = document.getElementById('pattern');
  const particleCountInput = document.getElementById('particleCount');
  const gravityInput = document.getElementById('gravity');
  const launchRandomBtn = document.getElementById('launchRandom');

  // Configuration (tunable)
  const config = {
    gravity: parseFloat(gravityInput.value),
    particleCount: parseInt(particleCountInput.value, 10),
    colors: [
      ['#ff2d55','#ff7a7a','#ffd1d1'], // ruby
      ['#28c76f','#7fffd4','#b9f6ca'], // emerald
      ['#4d7bff','#9fbaff','#d7e3ff'], // sapphire
      ['#ffd166','#ffd9a6','#fff0d9']  // gold
    ],
    launchVelocityRange: [9.5, 14],
    launchHorizontalSway: 0.5,
    rocketFlicker: 0.25 // flicker intensity
  };

  // State
  const rockets = [];
  const particles = [];
  const trails = [];

  // Utilities
  function rand(min, max) { return Math.random() * (max - min) + min; }
  function randInt(min, max) { return Math.floor(rand(min, max+1)); }
  function choose(arr){ return arr[Math.floor(Math.random()*arr.length)]; }

  // Simple sound manager (optional): generate short noises using WebAudio
  let audioCtx = null;
  function ensureAudio(){
    if (audioCtx) return audioCtx;
    try { audioCtx = new (window.AudioContext || window.webkitAudioContext)(); } catch(e){ audioCtx = null; }
    return audioCtx;
  }
  function playBeep(type='swoosh'){
    const ac = ensureAudio();
    if(!ac) return;
    const o = ac.createOscillator();
    const g = ac.createGain();
    o.connect(g); g.connect(ac.destination);
    if(type==='swoosh'){
      o.type='sawtooth'; o.frequency.setValueAtTime(300, ac.currentTime);
      o.frequency.exponentialRampToValueAtTime(800, ac.currentTime+0.2);
      g.gain.setValueAtTime(0.06, ac.currentTime); g.gain.exponentialRampToValueAtTime(0.001, ac.currentTime+0.25);
    } else if(type==='boom'){
      o.type='sine'; o.frequency.setValueAtTime(80, ac.currentTime);
      g.gain.setValueAtTime(0.2, ac.currentTime); g.gain.exponentialRampToValueAtTime(0.001, ac.currentTime+0.3);
    } else if(type==='crackle'){
      for(let i=0;i<6;i++){
        const so = ac.createOscillator(); const sg = ac.createGain(); so.connect(sg); sg.connect(ac.destination);
        so.type='triangle'; so.frequency.setValueAtTime(rand(400,1200), ac.currentTime + i*0.02);
        sg.gain.setValueAtTime(0.02, ac.currentTime + i*0.02); sg.gain.exponentialRampToValueAtTime(0.001, ac.currentTime + i*0.1);
        so.start(ac.currentTime + i*0.02); so.stop(ac.currentTime + i*0.08 + Math.random()*0.1);
      }
      return;
    }
    o.start(); o.stop(ac.currentTime + 0.35 + Math.random()*0.1);
  }

  // Rocket class — the ascending shell
  class Rocket {
    constructor(xTarget){
      this.x0 = rand(20, W-20);
      this.xTarget = (typeof xTarget === 'number') ? xTarget : this.x0 + rand(-40,40);
      this.y = H;
      this.x = this.x0;
      this.vx = (this.xTarget - this.x0) / rand(40,70); // small horizontal toward target
      this.vy = -rand(config.launchVelocityRange[0], config.launchVelocityRange[1]);
      this.size = rand(2,4);
      this.trail = [];
      this.color = ['#fff9e6','#fff3cc','#ffd1a6'][Math.floor(Math.random()*3)];
      this.age = 0;
      this.maxAge = 2000;
      this.apexY = rand(H*0.1, H*0.3); // will be adjusted
      this.hasBurst = false;
      // determine apex dynamically between 70% -90% of viewport height
      const r = rand(0.7,0.9);
      this.apexY = H * (1 - r);
      // slight sway params
      this.sway = rand(-config.launchHorizontalSway, config.launchHorizontalSway);
      this.swayOffset = Math.random()*2*Math.PI;
    }

    update(dt){
      this.age += dt;
      // physics
      this.vy += config.gravity * dt * 0.02; // gravity small during ascent
      // horizontal sway using a smooth curve
      this.x += this.vx + Math.sin(this.age*0.006 + this.swayOffset)*this.sway*0.6;
      this.y += this.vy;

      // trail
      this.trail.push({x:this.x, y:this.y, size:this.size, life: 0.4});
      if(this.trail.length>60) this.trail.shift();

      // check apex
      if(this.vy >= 0 || this.y <= this.apexY){
        this.vy = 0; // pause
        this.vx = 0;
        if(!this.hasBurst){
          // small pause then burst
          setTimeout(()=>{
            this.burst();
          }, rand(100,250));
          this.hasBurst = true;
        }
      }

      // decay rocket head brightness slightly
      this.size *= 0.999;
    }

    burst(){
      // rocket fades out
      // create flash
      const colorSet = choose(config.colors);
      flashes.push({x:this.x, y:this.y, r: 1, maxR: rand(30,60), a:1, color: colorSet[0], life:0});
      // explosion
      createBurst(this.x, this.y, config.particleCount, colorSet, patternSelect.value);
      playBeep('boom');
      // remove this rocket (mark by setting y below screen)
      this.y = H + 1000;
    }

    draw(ctx){
      // draw trail
      for(let i = this.trail.length-1; i>=0; i--){
        const t = this.trail[i];
        const idx = this.trail.length - i;
        const alpha = Math.max(0, 0.9 - idx/Math.max(10,this.trail.length));
        const size = Math.max(0.5, t.size * (idx/this.trail.length));
        ctx.beginPath();
        ctx.fillStyle = hexToRgba(this.color, alpha * 0.9);
        ctx.arc(t.x, t.y, size, 0, Math.PI*2);
        ctx.fill();
      }

      // rocket head (bright flicker)
      const flick = 0.85 + Math.random()*config.rocketFlicker;
      ctx.beginPath();
      ctx.fillStyle = hexToRgba('#fff', flick);
      ctx.rect(this.x-1.5*this.size, this.y-1.5*this.size, 3*this.size, 3*this.size);
      ctx.fill();
      // glow
      ctx.beginPath(); ctx.fillStyle = hexToRgba('#fff', 0.08); ctx.arc(this.x, this.y, 10*this.size,0,Math.PI*2); ctx.fill();
    }
  }

  // Particles from explosion
  class Particle {
    constructor(x,y,angle,speed,color,pattern){
      this.x = x; this.y = y;
      this.vx = Math.cos(angle) * speed;
      this.vy = Math.sin(angle) * speed;
      this.size = rand(1.6,3.2);
      this.color = perturbColor(color);
      this.life = 0;
      this.ttl = rand(900, 2500); // ms
      this.pattern = pattern;
      this.trail = [];
      this.alpha = 1;
      this.willCrackle = (pattern==='crackle' && Math.random()<0.18);
      this.crackled = false;
      this.gravity = config.gravity;
      // willow particles a bit heavier and longer trail
      if(pattern==='willow'){
        this.size *= 1.1; this.gravity *= 1.6;
      }
    }

    update(dt){
      // dt in ms
      this.life += dt;
      // apply gravity
      this.vy += this.gravity * dt * 0.002;
      // air drag
      this.vx *= 0.999;
      this.vy *= 0.999;

      // willow droop: add downward bias over time
      if(this.pattern==='willow'){
        this.vy += 0.002 * dt;
        // slight horizontal slow
        this.vx *= 0.997;
      }

      this.x += this.vx;
      this.y += this.vy;

      // trail for particle
      this.trail.push({x:this.x, y:this.y, life:0});
      if(this.trail.length>25) this.trail.shift();

      // flicker / glitter
      if(Math.random()<0.02){ this.alpha = 0.1 + Math.random()*0.9; }

      // size fade
      this.size *= 0.9995;

      // crackle secondary burst
      if(this.willCrackle && !this.crackled && this.life > this.ttl*0.25){
        this.crackled = true;
        // create 3-6 small particles
        const cnt = randInt(3,6);
        for(let i=0;i<cnt;i++){
          const a = rand(0, Math.PI*2);
          const s = rand(1.6,3.6);
          particles.push(new Particle(this.x, this.y, a, s, this.color, 'crackleSmall'));
        }
      }
    }

    draw(ctx){
      // draw trail
      for(let i = this.trail.length-1; i>=0; i--){
        const t = this.trail[i];
        const alpha = (i / this.trail.length) * 0.6 * (1 - (this.life/this.ttl));
        ctx.beginPath(); ctx.fillStyle = hexToRgba(this.color, alpha); ctx.arc(t.x, t.y, Math.max(0.3, this.size*(i/this.trail.length)), 0, Math.PI*2); ctx.fill();
      }

      // main particle
      const lifeRatio = Math.max(0, 1 - this.life/this.ttl);
      ctx.beginPath(); ctx.fillStyle = hexToRgba(this.color, lifeRatio * this.alpha); ctx.arc(this.x, this.y, Math.max(0.2, this.size*lifeRatio), 0, Math.PI*2); ctx.fill();
    }

    isDead(){ return this.life >= this.ttl || this.y > H+50; }
  }

  // flashes
  const flashes = [];

  // helpers
  function hexToRgba(hex, a=1){
    if(hex.startsWith('#')) hex = hex.slice(1);
    let r = parseInt(hex.substr(0,2),16);
    let g = parseInt(hex.substr(2,2),16);
    let b = parseInt(hex.substr(4,2),16);
    return `rgba(${r},${g},${b},${a})`;
  }
  function perturbColor(hex){
    // slightly vary hue by mixing with white or black
    const v = Math.random()*0.14 - 0.07; // -0.07..0.07
    // convert to rgb and shift
    if(hex.startsWith('#')) hex = hex.slice(1);
    let r = parseInt(hex.substr(0,2),16);
    let g = parseInt(hex.substr(2,2),16);
    let b = parseInt(hex.substr(4,2),16);
    r = Math.max(0,Math.min(255, r + (v*255))|0);
    g = Math.max(0,Math.min(255, g + (v*255))|0);
    b = Math.max(0,Math.min(255, b + (v*255))|0);
    return `#${(r<<16 | g<<8 | b).toString(16).padStart(6,'0')}`;
  }

  // create burst patterns
  function createBurst(x,y,count, colorSet, pattern){
    const baseColor = colorSet[0];
    if(pattern === 'peony'){
      for(let i=0;i<count;i++){
        const angle = rand(0, Math.PI*2);
        const speed = rand(1.8, 6.5);
        particles.push(new Particle(x,y,angle,speed, choose(colorSet), 'peony'));
      }
    } else if(pattern === 'willow'){
      for(let i=0;i<count;i++){
        const angle = rand(0, Math.PI*2);
        const speed = rand(2.2, 5.0);
        const p = new Particle(x,y,angle,speed, choose(colorSet), 'willow');
        // make willow have a downward bias
        p.vy += rand(0.4,1.2);
        // longer life
        p.ttl = rand(1400, 2800);
        particles.push(p);
      }
    } else if(pattern === 'crackle'){
      for(let i=0;i<count;i++){
        const angle = rand(0, Math.PI*2);
        const speed = rand(2.5,7.5);
        const p = new Particle(x,y,angle,speed, choose(colorSet), 'crackle');
        particles.push(p);
      }
    } else {
      // fallback peony
      createBurst(x,y,count,colorSet,'peony');
    }
  }

  // launch function
  function launchAt(x){
    const r = new Rocket(x);
    // override target to aim at x
    r.xTarget = Math.max(20, Math.min(W-20, x));
    rockets.push(r);
    playBeep('swoosh');
  }

  // random launch
  function launchRandom(){
    const x = rand(40, W-40);
    launchAt(x);
  }

  // wire up events
  canvas.addEventListener('click', (e)=>{
    const rect = canvas.getBoundingClientRect();
    const x = e.clientX - rect.left;
    launchAt(x);
  });
  launchRandomBtn.addEventListener('click', ()=>launchRandom());

  window.addEventListener('resize', ()=>{
    W = canvas.width = innerWidth;
    H = canvas.height = innerHeight;
    canvas.width = W * DPR; canvas.height = H * DPR; canvas.style.width = W + 'px'; canvas.style.height = H + 'px'; ctx.scale(DPR, DPR);
  });

  // UI binding
  particleCountInput.addEventListener('input', ()=>{ config.particleCount = parseInt(particleCountInput.value,10); });
  gravityInput.addEventListener('input', ()=>{ config.gravity = parseFloat(gravityInput.value); });

  // animation loop
  let last = performance.now();
  function loop(now){
    const dt = Math.min(40, now - last); // clamp
    last = now;

    // clear with mild alpha for trails
    ctx.fillStyle = 'rgba(0,0,8,0.25)';
    ctx.fillRect(0,0,W,H);

    // update rockets
    for(let i=rockets.length-1;i>=0;i--){
      const r = rockets[i];
      r.update(dt);
      r.draw(ctx);
      if(r.y > H+50) rockets.splice(i,1);
    }

    // update particles
    for(let i=particles.length-1;i>=0;i--){
      const p = particles[i];
      p.update(dt);
      p.draw(ctx);
      if(p.isDead()) particles.splice(i,1);
    }

    // flashes
    for(let i=flashes.length-1;i>=0;i--){
      const f = flashes[i];
      f.life += dt;
      f.r += dt*0.04;
      f.a *= 0.92;
      ctx.beginPath(); ctx.fillStyle = hexToRgba(f.color, Math.max(0, f.a*0.9)); ctx.arc(f.x, f.y, f.r,0,Math.PI*2); ctx.fill();
      if(f.a < 0.02) flashes.splice(i,1);
    }

    requestAnimationFrame(loop);
  }

  // auto-launch lovely sequence
  let seqTimer = 0;
  setInterval(()=>{ launchRandom(); }, 900);

  requestAnimationFrame(loop);

})();
