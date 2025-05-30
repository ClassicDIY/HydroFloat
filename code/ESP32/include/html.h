#pragma once

#include "Arduino.h"

const char head_html[] PROGMEM = R"rawliteral(
	<!DOCTYPE html><html lang=\"en\"><head><meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
	<title>{n}</title>

	<style>

        body {
            font-family: apercu-pro, -apple-system, system-ui, BlinkMacSystemFont, "Helvetica Neue", sans-serif;
            line-height: 1em;
            font-weight: 100;
        }
        .container {
            display: flex;
            justify-content: center; 
            align-items: center; 
            height: 100vh; 
        }
        .form-group {
            margin-bottom: 10px;
            margin-top: 10px;
            display: flex;
            flex-direction: column;
            height: 100%; 
			width:100%;
        }
        .fs {
            display: inline-block;
            border-radius:0.3rem;
            margin: 0px;
            width:92%; 
        }
        .fld {
            color: #000080;
            clear: both;
            display: flex;
            text-align: left; 
        }
        .fld label {
            width: 50%;
            text-align: right;
            margin-right: 10px;
        }
        .fld input[type="text"],
        .fld input[type="number"] {
            flex: 1;
            width: 50%;
        }
        button{
            border:0;
            border-radius:0.3rem;
            background-color:#16A1E7;
            color:#fff;
            line-height:2.4rem;
            font-size:1.2rem;
            width:100%;

            margin-top: 10px;
		} 
        .hide{display: none;}
        .ver {
            font-size: .6em;
        }
        #config {
			width: 400px;
			margin: 0 auto;
            align: center;
		}
    </style>
	</head><body>
		<div id="config">
		<div>
			<h2>{n}</h2>
			<div class="ver">Firmware config version '{v}'</div>
			<hr>
		</div>
		<div class="container">
		<div class="form-group">
	)rawliteral";

	const char settings_html[] PROGMEM = R"rawliteral(
			<div class="center-flex">
				<fieldset id="network"><legend>Network</legend>
					<p><div class="fld">SSID: {ssid}</div></p>
					<p><div class="fld">AP Password: {appw}</div></p>
				</fieldset>
				<fieldset id="levels"><legend>Levels</legend>
					<p><div class="fld">Overflow: {of}</div></p>
					<p><div class="fld">Start Lag: {slag}</div></p>
					<p><div class="fld">Start Lead: {slead}</div></p>
					<p><div class="fld">Stop: {stop}</div></p>
				</fieldset>
				<p><a href='/log' target='_blank'>Web Log</a></p>
				<p><a href="/config">Configuration</a></p>
				<p><a href='/'>Return to home page.</a></p>

			</div>
				
		</div></div></div>
		</body></html>
	)rawliteral";

const char config_html[] PROGMEM = R"rawliteral(
			<div class="center-flex">
				<form action='/submit' method='post'>
				<fieldset id="network"><legend>Network</legend>
				<p><div class="fld"><label for="ssid">SSID</label><input type="text" id="ssid" name="ssid" value={ssid} required maxlength="32"></div></p>
				<p><div class="fld"><label for="appw">AP Password</label><input type="text" id="appw" name="appw" value={appw} minlength="8" required maxlength="32"></div></p>
				</fieldset>
				<fieldset id="levels"><legend>Level</legend>
				<p><div class="fld"><label for="overflow">Overflow</label><input type="number" id="overflow" name="overflow" placeholder="1..100" value={of} min="1" max="100" required step="1"></div></p>
				<p><div class="fld"><label for="slag">Start Lag</label><input type="number" id="slag" name="slag" placeholder="1..100" value={slag} min="1" max="100" required step="1"></div></p>
				<p><div class="fld"><label for="slead">Start Lead</label><input type="number" id="slead" name="slead" placeholder="1..100" value={slead} min="1" max="100" required step="1"></div></p>
				<p><div class="fld"><label for="stop">Stop</label><input type="number" id="stop" name="stop" placeholder="1..100" value={stop} min="1" max="100" required step="1"></div></p>
				</fieldset>
				<button type="submit" >Apply</button>
				</form>
				<p><a href='update'>Firmware update</a></p>
				<p><a href='/'>Return to home page.</a></p>
				<p><a href='settings'>Return to Settings</a></p>
				<p><a href='reboot'>Reboot ESP32</a></p>
			</div>
		</div></div></div>
		</body></html>
	)rawliteral";

const char update_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang=\"en\"><head><meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<title>Firmware Update</title>
</head><body>
<h1>HydroFloat Firmware Update</h1>
<h1>ESP32 OTA Update</h1>
<form method='POST' action='/doupdate' enctype='multipart/form-data'>
<input type='file' name='firmware'>
</form>
</body></html>
)rawliteral";

const char home_html[] PROGMEM = R"rawliteral(
    <!DOCTYPE html><html lang=\"en\"><head><meta name="viewport" content="width=device-width, initial-scale=0.8, user-scalable=no">
	<title>{n}</title>

	<style>
		.c{text-align: center;} 
		body {
            font-family: apercu-pro, -apple-system, system-ui, BlinkMacSystemFont, "Helvetica Neue", sans-serif;
            padding: 1em;
            line-height: 1em;
            font-weight: 100;
        } 
		.hide{display: none;}

		.box {
			width: 90px;
			height: 25px;
			margin: 5px;
			// display: inline-block;
			background-color:grey;
			display: inline-flex; /* Changed to inline-flex to allow the use of Flexbox */
			align-items: center;  /* Vertically center the content */
			justify-content: center; /* Horizontally center the content */
			border: 1px solid #000; /* Added a border for better visibility */
		}
		#config {
			width: 500px;
			margin: 0 auto;
		}
        </style>

		<script>
		<!--- Start of gauge code  --->
		eval(function(p,a,c,k,e,r){e=function(c){return(c<a?'':e(parseInt(c/a)))+((c=c%a)>35?String.fromCharCode(c+29):c.toString(36))};if(!''.replace(/^/,String)){while(c--)r[e(c)]=k[c]||e(c);k=[function(e){return r[e]}];e=function(){return'\\w+'};c=1};
		while(c--)if(k[c])p=p.replace(new RegExp('\\b'+e(c)+'\\b','g'),k[c]);return p}('t W=v(f){W.2t.3T(A);A.B={Z:2u,19:1q,1h:1q,1J:U,1b:D,K:0,V:[\'0\',\'20\',\'40\',\'2A\',\'2B\',\'D\'],2r:10,2C:M,1E:U,2q:{2D:3,2E:2},2H:M,1c:{2j:10,2m:3y,2O:\'3v\'},J:{2P:\'#4h\',V:\'#31\',2r:\'#3k\',1J:\'#37\',1E:\'#37\',3e:\'#31\',1t:{2s:\'1e(3Y, 3d, 3d, 1)\',3c:\'1e(1Y, 5I, 5E, .9)\'}},1o:[{1n:20,1D:2A,1F:\'#3j\'},{1n:2A,1D:2B,1F:\'#36\'},{1n:2B,1D:D,1F:\'#5n\'}]};t g=0,1p=A,N=0,1S=0,1G=U;A.5d=v(a){N=f.1c?g:a;t b=(f.1b-f.K)/D;1S=a>f.1b?1S=f.1b+b:a<f.K?f.K-b:a;g=a;f.1c?3l():A.1g();C A};A.3m=v(a){N=g=a;A.1g();C A};A.4T=v(){g=N=1S=A.B.K;A.1g();C A};A.4R=v(){C g};A.13=v(){};v 2k(a,b){Q(t i 4P b){z(1H b[i]=="1W"&&!(4O.4y.2V.4p(b[i])===\'[1W 4n]\')&&i!=\'Z\'){z(1H a[i]!="1W"){a[i]={}}2k(a[i],b[i])}O{a[i]=b[i]}}};2k(A.B,f);A.B.K=1R(A.B.K);A.B.1b=1R(A.B.1b);f=A.B;N=g=f.K;z(!f.Z){4m 4j("4g 4d 4b 46 44 41 3Z 3W W 1W!");}t j=f.Z.5K?f.Z:2R.5v(f.Z),q=j.3u(\'2d\'),1i,1y,1A,14,17,u,1d;v 2M(){j.19=f.19;j.1h=f.1h;1i=j.4s(M);1d=1i.3u(\'2d\');1y=j.19;1A=j.1h;14=1y/2;17=1A/2;u=14<17?14:17;1i.2J=U;1d.3P(14,17);1d.G();q.3P(14,17);q.G()};2M();A.4Z=v(a){2k(A.B,a);2M();A.1g();C A};t k={4q:v(p){C p},4e:v(p){C E.1L(p,2)},4c:v(p){C E.1L(p,5)},3v:v(p){C 1-E.1O(E.5C(p))},5k:v(p){C 1-(v(p){Q(t a=0,b=1;1;a+=b,b/=2){z(p>=(7-4*a)/11){C-E.1L((11-6*a-11*p)/4,2)+E.1L(b,2)}}})(1-p)},4S:v(p){C 1-(v(p){t x=1.5;C E.1L(2,10*(p-1))*E.1T(20*E.1a*x/3*p)})(1-p)}};t l=2u;v 3S(d){t e=2v 3R;l=2x(v(){t a=2v 3R-e,1M=a/d.2m;z(1M>1){1M=1}t b=1H d.2g=="v"?d.2g:k[d.2g];t c=b(1M);d.3Q(c);z(1M==1){2b(l)}},d.2j||10)};v 3l(){l&&2b(l);t b=(1S-N),1n=N,29=f.1c;3S({2j:29.2j,2m:29.2m,2g:29.2O,3Q:v(a){N=1R(1n)+b*a;1p.1g()}})};q.5l="3O";A.1g=v(){z(!1i.2J){1d.3M(-14,-17,1y,1A);1d.G();t a=q;q=1d;3L();3K();3I();3H();3F();3D();3z();1i.2J=M;q=a;5G a}q.3M(-14,-17,1y,1A);q.G();q.4a(1i,-14,-17,1y,1A);z(!W.28){t b=2x(v(){z(!W.28){C}2b(b);2K();2L();z(!1G){1p.13&&1p.13();1G=M}},10)}O{2K();2L();z(!1G){1p.13&&1p.13();1G=M}}C A};v S(a){C a*E.1a/4J};v 1l(a,b,c){t d=q.4Y(0,0,0,c);d.1V(0,a);d.1V(1,b);C d};v 3L(){t a=u/D*5g,3x=u-a,2a=u/D*5q,5u=u-2a,1f=u/D*5z,5A=u-1f;3t=u/D*5F;q.G();z(f.2H){q.2o=3x;q.2n=\'1e(0, 0, 0, 0.5)\'}q.P();q.16(0,0,a,0,E.1a*2,M);q.L=1l(\'#42\',\'#43\',a);q.T();q.R();q.P();q.16(0,0,2a,0,E.1a*2,M);q.L=1l(\'#49\',\'#36\',2a);q.T();q.P();q.16(0,0,1f,0,E.1a*2,M);q.L=1l(\'#3j\',\'#3s\',1f);q.T();q.P();q.16(0,0,3t,0,E.1a*2,M);q.L=f.J.2P;q.T();q.G()};v 3H(){t r=u/D*2T;q.2e=2;q.2U=f.J.V;q.G();Q(t i=0;i<f.V.H;++i){t a=45+i*(1U/(f.V.H-1));q.1z(S(a));q.P();q.1K(0,r);q.F(0,r-u/D*15);q.1X();q.R();q.G()}z(f.2C){q.1z(S(2X));q.P();q.16(0,0,r,S(45),S(4N),U);q.1X();q.R();q.G()}};v 3I(){t r=u/D*2T;q.2e=1;q.2U=f.J.2r;q.G();t b=f.2r*(f.V.H-1);Q(t i=0;i<b;++i){t a=45+i*(1U/b);q.1z(S(a));q.P();q.1K(0,r);q.F(0,r-u/D*7.5);q.1X();q.R();q.G()}};v 3F(){t r=u/D*55;Q(t i=0;i<f.V.H;++i){t a=45+i*(1U/(f.V.H-1)),p=1w(r,S(a));q.1x=20*(u/1q)+"2i 2Y";q.L=f.J.3e;q.2e=0;q.2h="2f";q.27(f.V[i],p.x,p.y+3)}};v 3D(){z(!f.1J){C}q.G();q.1x=24*(u/1q)+"2i 2Y";q.L=f.J.1J;q.2h="2f";q.27(f.1J,0,-u/4.25);q.R()};v 3z(){z(!f.1E){C}q.G();q.1x=22*(u/1q)+"2i 2Y";q.L=f.J.1E;q.2h="2f";q.27(f.1E,0,u/3.25);q.R()};v 32(a){t b=f.2q.2E,34=f.2q.2D;a=1R(a);t n=(a<0);a=E.35(a);z(b>0){a=a.5t(b).2V().1j(\'.\');Q(t i=0,s=34-a[0].H;i<s;++i){a[0]=\'0\'+a[0]}a=(n?\'-\':\'\')+a[0]+\'.\'+a[1]}O{a=E.3O(a).2V();Q(t i=0,s=34-a.H;i<s;++i){a=\'0\'+a}a=(n?\'-\':\'\')+a}C a};v 1w(r,a){t x=0,y=r,1O=E.1O(a),1T=E.1T(a),X=x*1T-y*1O,Y=x*1O+y*1T;C{x:X,y:Y}};v 3K(){q.G();t a=u/D*2T;t b=a-u/D*15;Q(t i=0,s=f.1o.H;i<s;i++){t c=f.1o[i],39=(f.1b-f.K)/1U,1P=S(45+(c.1n-f.K)/39),1N=S(45+(c.1D-f.K)/39);q.P();q.1z(S(2X));q.16(0,0,a,1P,1N,U);q.R();q.G();t d=1w(b,1P),3a=1w(a,1P);q.1K(d.x,d.y);q.F(3a.x,3a.y);t e=1w(a,1N),3b=1w(b,1N);q.F(e.x,e.y);q.F(3b.x,3b.y);q.F(d.x,d.y);q.1C();q.L=c.1F;q.T();q.P();q.1z(S(2X));q.16(0,0,b,1P-0.2,1N+0.2,U);q.R();q.1C();q.L=f.J.2P;q.T();q.G()}};v 2L(){t a=u/D*12,1f=u/D*8,1u=u/D*3X,1r=u/D*20,2l=u/D*4,1B=u/D*2,38=v(){q.3f=2;q.3g=2;q.2o=10;q.2n=\'1e(5L, 3h, 3h, 0.45)\'};38();q.G();z(N<0){N=E.35(f.K-N)}O z(f.K>0){N-=f.K}O{N=E.35(f.K)+N}q.1z(S(45+N/((f.1b-f.K)/1U)));q.P();q.1K(-1B,-1r);q.F(-2l,0);q.F(-1,1u);q.F(1,1u);q.F(2l,0);q.F(1B,-1r);q.1C();q.L=1l(f.J.1t.2s,f.J.1t.3c,1u-1r);q.T();q.P();q.F(-0.5,1u);q.F(-1,1u);q.F(-2l,0);q.F(-1B,-1r);q.F(1B/2-2,-1r);q.1C();q.L=\'1e(1Y, 1Y, 1Y, 0.2)\';q.T();q.R();38();q.P();q.16(0,0,a,0,E.1a*2,M);q.L=1l(\'#3s\',\'#36\',a);q.T();q.R();q.P();q.16(0,0,1f,0,E.1a*2,M);q.L=1l("#47","#48",1f);q.T()};v 3i(x,y,w,h,r){q.P();q.1K(x+r,y);q.F(x+w-r,y);q.23(x+w,y,x+w,y+r);q.F(x+w,y+h-r);q.23(x+w,y+h,x+w-r,y+h);q.F(x+r,y+h);q.23(x,y+h,x,y+h-r);q.F(x,y+r);q.23(x,y,x+r,y);q.1C()};v 2K(){q.G();q.1x=40*(u/1q)+"2i 30";t a=32(g),2Z=q.4f(\'-\'+32(0)).19,y=u-u/D*33,x=0,2W=0.12*u;q.G();3i(-2Z/2-0.21*u,y-2W-0.4i*u,2Z+0.3n*u,2W+0.4k*u,0.21*u);t b=q.4l(x,y-0.12*u-0.21*u+(0.12*u+0.3o*u)/2,u/10,x,y-0.12*u-0.21*u+(0.12*u+0.3o*u)/2,u/5);b.1V(0,"#37");b.1V(1,"#3k");q.2U=b;q.2e=0.3n*u;q.1X();q.2o=0.3p*u;q.2n=\'1e(0, 0, 0, 1)\';q.L="#4o";q.T();q.R();q.3f=0.3q*u;q.3g=0.3q*u;q.2o=0.3p*u;q.2n=\'1e(0, 0, 0, 0.3)\';q.L="#31";q.2h="2f";q.27(a,-x,y);q.R()}};W.28=U;(v(){t d=2R,h=d.3r(\'4r\')[0],2S=4t.4u.4v().4w(\'4x\')!=-1,2Q=\'4z://4A-4B.4C/4D/4E/4F-7-4G.\'+(2S?\'4H\':\'4I\'),1I="@1x-4K {"+"1x-4L: \'30\';"+"4M: 2Q(\'"+2Q+"\');"+"}",1s,r=d.3w(\'1v\');r.2N=\'1I/4Q\';z(2S){h.2p(r);1s=r.2I;1s.3A=1I}O{4U{r.2p(d.4V(1I))}4W(e){r.3A=1I}h.2p(r);1s=r.2I?r.2I:(r.4X||d.3B[d.3B.H-1])}t b=2x(v(){z(!d.3C){C}2b(b);t a=d.3w(\'50\');a.1v.51=\'30\';a.1v.52=\'53\';a.1v.1h=a.1v.19=0;a.1v.54=\'56\';a.57=\'.\';d.3C.2p(a);58(v(){W.28=M;a.59.5a(a)},3y)},1)})();W.2t=[];W.2t.5b=v(a){z(1H(a)==\'5c\'){Q(t i=0,s=A.H;i<s;i++){z(A[i].B.Z.18(\'5e\')==a){C A[i]}}}O z(1H(a)==\'5f\'){C A[a]}O{C 2u}};v 3E(a){z(2G.3G){2G.3G(\'5h\',a,U)}O{2G.5i(\'5j\',a)}}3E(v(){v 2F(a){t b=a[0];Q(t i=1,s=a.H;i<s;i++){b+=a[i].1Z(0,1).5m()+a[i].1Z(1,a[i].H-1)}C b};v 3J(a){C a.5o(/^\\s+|\\s+$/g,\'\')};t c=2R.3r(\'5p\');Q(t i=0,s=c.H;i<s;i++){z(c[i].18(\'1k-2N\')==\'5r-5s\'){t d=c[i],B={},1m,w=2c(d.18(\'19\')),h=2c(d.18(\'1h\'));B.Z=d;z(w){B.19=w}z(h){B.1h=h}Q(t e=0,1s=d.3N.H;e<1s;e++){1m=d.3N.5w(e).5x;z(1m!=\'1k-2N\'&&1m.1Z(0,5)==\'1k-\'){t f=1m.1Z(5,1m.H-5).5y().1j(\'-\'),I=d.18(1m);z(!I){2z}5B(f[0]){2y\'J\':{z(f[1]){z(!B.J){B.J={}}z(f[1]==\'1t\'){t k=I.1j(/\\s+/);z(k[0]&&k[1]){B.J.1t={2s:k[0],3c:k[1]}}O{B.J.1t=I}}O{f.5D();B.J[2F(f)]=I}}26}2y\'1o\':{z(!B.1o){B.1o=[]}2w=I.1j(\',\');Q(t j=0,l=2w.H;j<l;j++){t m=3J(2w[j]).1j(/\\s+/),1Q={};z(m[0]&&m[0]!=\'\'){1Q.1n=m[0]}z(m[1]&&m[1]!=\'\'){1Q.1D=m[1]}z(m[2]&&m[2]!=\'\'){1Q.1F=m[2]}B.1o.3T(1Q)}26}2y\'1c\':{z(f[1]){z(!B.1c){B.1c={}}z(f[1]==\'2O\'&&/^\\s*v\\s*\\(/.5H(I)){I=3U(\'(\'+I+\')\')}B.1c[f[1]]=I}26}5J:{t n=2F(f);z(n==\'13\'){2z}z(n==\'V\'){I=I.1j(/\\s+/)}O z(n==\'2C\'||n==\'2H\'){I=I==\'M\'?M:U}O z(n==\'2q\'){t o=I.1j(\'.\');z(o.H==2){I={2D:2c(o[0]),2E:2c(o[1])}}O{2z}}B[n]=I;26}}}}t g=2v W(B);z(d.18(\'1k-3V\')){g.3m(1R(d.18(\'1k-3V\')))}z(d.18(\'1k-13\')){g.13=v(){3U(A.B.Z.18(\'1k-13\'))}}g.1g()}}});',62,358,'||||||||||||||||||||||||||ctx|||var|max|function||||if|this|config|return|100|Math|lineTo|save|length|attrValue|colors|minValue|fillStyle|true|fromValue|else|beginPath|for|restore|radians|fill|false|majorTicks|Gauge|||renderTo||||onready|CX||arc|CY|getAttribute|width|PI|maxValue|animation|cctx|rgba|r2|draw|height|cache|split|data|lgrad|prop|from|highlights|self|200|rOut|ss|needle|rIn|style|rpoint|font|CW|rotate|CH|pad2|closePath|to|units|color|imready|typeof|text|title|moveTo|pow|progress|ea|sin|sa|hlCfg|parseFloat|toValue|cos|270|addColorStop|object|stroke|255|substr||025||quadraticCurveTo|||break|fillText|initialized|cfg|r1|clearInterval|parseInt||lineWidth|center|delta|textAlign|px|delay|applyRecursive|pad1|duration|shadowColor|shadowBlur|appendChild|valueFormat|minorTicks|start|Collection|null|new|hls|setInterval|case|continue|60|80|strokeTicks|int|dec|toCamelCase|window|glow|styleSheet|i8d|drawValueBox|drawNeedle|baseInit|type|fn|plate|url|document|ie|81|strokeStyle|toString|th|90|Arial|tw|Led|444|padValue||cint|abs|ccc|888|shad|vd|pe|pe1|end|128|numbers|shadowOffsetX|shadowOffsetY|143|roundRect|eee|666|animate|setRawValue|05|045|012|004|getElementsByTagName|f0f0f0|r3|getContext|cycle|createElement|d0|250|drawUnits|cssText|styleSheets|body|drawTitle|domReady|drawNumbers|addEventListener|drawMajorTicks|drawMinorTicks|trim|drawHighlights|drawPlate|clearRect|attributes|round|translate|step|Date|_animate|push|eval|value|the|77|240|creating||when|ddd|aaa|specified||not|e8e8e8|f5f5f5|fafafa|drawImage|was|quint|element|quad|measureText|Canvas|fff|04|Error|07|createRadialGradient|throw|Array|babab2|call|linear|head|cloneNode|navigator|userAgent|toLocaleLowerCase|indexOf|msie|prototype|http|smart|ip|net|styles|fonts|digital|mono|eot|ttf|180|face|family|src|315|Object|in|css|getValue|elastic|clear|try|createTextNode|catch|sheet|createLinearGradient|updateConfig|div|fontFamily|position|absolute|overflow||hidden|innerHTML|setTimeout|parentNode|removeChild|get|string|setValue|id|number|93|DOMContentLoaded|attachEvent|onload|bounce|lineCap|toUpperCase|999|replace|canvas|91|canv|gauge|toFixed|d1|getElementById|item|nodeName|toLowerCase|88|d2|switch|acos|shift|122|85|delete|test|160|default|tagName|188'.split('|'),0,{}))
		<!--- End of gauge code --->


		function initWebSocket() {
			const socket = new WebSocket('ws://' + window.location.hostname + '/ws_home');
			socket.onmessage = function(event) {
				const gpioValues = JSON.parse(event.data);
				Gauge.Collection.get('CW_gauge').setValue(gpioValues.level)
				document.getElementById(`relay4`).style.backgroundColor = (gpioValues.relay4 === 'on') ? 'green' : 'red';
				document.getElementById(`relay3`).style.backgroundColor = (gpioValues.relay3 === 'on') ? 'green' : 'red';
				document.getElementById(`relay2`).style.backgroundColor = (gpioValues.relay2 === 'on') ? 'green' : 'red';
				document.getElementById(`relay1`).style.backgroundColor = (gpioValues.relay1 === 'on') ? 'green' : 'red';
				document.getElementById(`relay1`).innerHTML = (gpioValues.relay1 === 'on') ? 'Run' : 'Stop';

			};
			socket.onerror = function(error) {
				console.error('WebSocket error:', error);
			};

			window.addEventListener('beforeunload', function() {
				if (socket) {
					socket.close();
				}
			});
		}

		window.onload = function() {
		  initWebSocket();
		}
      	</script>
    </head>
  <body>
  <div id="config">
    <div>
		<h2>{n}</h2>
		<div style='font-size: .6em;'>Firmware config version '{v}'</div>
		<hr>
	</div>
	<div>
    <canvas id="CW_gauge" width="420" height="420"
               data-type="canv-gauge"
               data-title="Water Level"
               data-major-ticks="0 10 20 30 40 50 60 70 80 90 100"
               data-minor-ticks="5"            
               data-units="%"
               data-value-format="2.1"
               data-max-value="100"
               data-min-value="0"
               data-color-plate="#0FF"
               data-highlights="0 2000 #FFe">
    </canvas>
	</div>
	
	<div  id="boxes-container"> 
		<div class="box" id="relay1"> Stop </div>
		<div class="box" id="relay2"> Start Lead </div>
		<div class="box" id="relay3"> Start Lag </div> 
		<div class="box" id="relay4"> Overflow </div> 
	</div>
	<div>
		<hr>
		<p><a href='settings'>View Current Settings</a></p>
	</div>
	</div>
  </body>
  </html>
)rawliteral";

const char redirect_html[] PROGMEM = R"rawliteral(
	<!DOCTYPE html>
	<html lang="en">
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>{n}</title>
		<script>
			window.onload = function() {
				window.location.href = "http://{ip}/";
			};
		</script>
	</head>
	<body>
		<h1>Redirecting to Home Page...</h1>
	</body>
	</html>
	)rawliteral";

const char reboot_html[] PROGMEM = R"rawliteral(
	<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>
	<title>ESP32 Reboot</title>
	</head><body>
	<h1>Rebooting ESP32</h1>
	<p><a href='settings'>Return to  Settings after reboot has completed.</a></p>
	</body></html>
	)rawliteral";
