#ifndef WEBAPPHTML_H
#define WEBAPPHTML_H

#define QUOTE(...) #__VA_ARGS__
const char * webAppHTML = QUOTE(

<!DOCTYPE html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>SPA</title><style>*{margin:0;padding:0;font-family:arial;box-sizing:border-box}body,html{width:100%;height:100%}body{display:flex;justify-content:center;background-color:#EEE}h2{margin-top:4px;font-size:1.2em}ul{list-style-type:none}select{border:solid #CCC 1px;border-bottom:solid #CCC 2px;background:0 0}#content{max-width:400px;padding:10px;margin:0 5px;width:100%;height:100%;background-color:#fff}#colors .color_group{display:flex}#colors .color{border:solid #fff 2px;height:40px;flex:1 1 auto;cursor:pointer}#colors .color.selected{border:solid #000 2px}#animation_select{width:100%;padding:4px;font-size:1.2em}</style></head><body><div id="content"><h1>PhoxDevice</h1><div><h2>Colors</h2><ul id="colors"></ul></div><br><div id="patterns"><h2>Patterns</h2><select id="animation_select"></select></div></div><script>(function(){
/* jshint esnext: true */

/* UTILS */
function delegate(event, el, selector, fn){
    el.addEventListener(event, e => {
        if(e.target.matches(selector)){
            fn.call(e.target, e);
        }
    });
}

function get(url, cb){
    var xhr = new XMLHttpRequest();
    let handleErr = function(msg){
        return function(){
            cb(`${xhr.status}: ${msg}`, null);
        };
    };
    xhr.open("get", url);
    xhr.onload = function() {
        if (xhr.status === 200) {
            cb(null, xhr.response);
        } else {
            handleErr();
        }
    };
    xhr.onerror = handleErr("error");
    xhr.onabort = handleErr("abort");
    xhr.ontimeout = handleErr("timeout");
    xhr.send();
}


/* ESP API */
class EspAPI {
    constructor(host){
        this.host = host;
        this.api = `${host}/api/v1`;
    }

    setColor(rgb){
        let url = `${this.api}/color?r=${rgb[0]}&g=${rgb[1]}&b=${rgb[2]}`;
        console.log(url);
        get(url, (err, response) => {
            if(err){
                console.warn(err);
                return;
            }
            console.log(response);
        });
    }

    setAnimation(id){
        let url = `${this.api}/animation?id=${id}`;
        console.log(url);
        get(url, (err, response) => {
            if(err){
                console.warn(err);
                return;
            }
            console.log(response);
        });
    }

    getReady(){
        let url = `${this.api}/ready`;
        console.log(url);
        get(url, (err, response) => {
            if(err){
                console.warn(err);
                return;
            }
            console.log(response);
        });
    }
}
//const API = new EspAPI("192.168.4.1");
const API = new EspAPI(window.location.origin);


/* UI CRAP */
function generateColors(palette){
    let colorsEl = document.getElementById("colors");

    let lightColorsHTML = palette.map(color => {
        let rgb = color.map(d => d === 0 ? 150 : d).join(","),
            rgbString = `rgb(${rgb})`;
        return `<div class="color" data-rgb="${rgb}" style="background-color: ${rgbString};"></div>`;
    }).join("");

    let baseColorsHTML = palette.map(color => {
        let rgb = color.join(","),
            rgbString = `rgb(${rgb})`;
        return `<div class="color" data-rgb="${rgb}" style="background-color: ${rgbString};"></div>`;
    }).join("");

    let darkColorsHTML = palette.map(color => {
        let rgb = color.map(d => Math.floor(d * 0.6)).join(","),
            rgbString = `rgb(${rgb})`;
        return `<div class="color" data-rgb="${rgb}" style="background-color: ${rgbString};"></div>`;
    }).join("");

    colorsEl.innerHTML = `
        <div class="color_group">${lightColorsHTML}</div>
        <div class="color_group">${baseColorsHTML}</div>
        <div class="color_group">${darkColorsHTML}</div>
    `;

    delegate("click", colorsEl, ".color", function(e){
        let colorEls = colorsEl.querySelectorAll(".color");
        Array.prototype.forEach.call(colorEls, el => el.classList.remove("selected"));
        this.classList.add("selected");
        API.setColor(this.dataset.rgb.split(",").map(d => +d));
    });
}

function generateAnimations(animations){
    let animationsEl = document.getElementById("animation_select");
    let animationOptionsHTML = animations.map(p => `<option value="${p.id}">${p.label}</option>`).join("");
    animationsEl.innerHTML = animationOptionsHTML;
    animationsEl.addEventListener("change", e => {
        API.setAnimation(animationsEl.value);
    });

}

function init(){
    let palette = [
        [255,0,0],
        [255,200,0],
        [255,255,0],
        [200,255,0],
        [0,255,0],
        [0,255,255],
        [0,200,255],
        [0,0,255],
        [255,0,100]
    ];
    generateColors(palette);

    let animations = [
        { label: "spinny1", id: 10 },
        { label: "wackadoo", id: 20 }
    ];
    generateAnimations(animations);

    API.getReady();
}

init();
        })();</script></body></html>
);

#endif
