
addEventListener("load", onLoad);

function getTextWidth(text, font) {
  // re-use canvas object for better performance
  const canvas = getTextWidth.canvas || (getTextWidth.canvas = document.createElement("canvas"));
  const context = canvas.getContext("2d");
  context.font = font;
  const metrics = context.measureText(text);
  return metrics.width;
}

var calculatewidth;
var spaceWidth;

function getTextWidthDOM(text, font, fontFeatureSettings = null) {


  calculatewidth.style.font = font;
  calculatewidth.style.fontFeatureSettings = fontFeatureSettings;
  calculatewidth.innerHTML = text;

  return calculatewidth.clientWidth


}

function getCssStyle(element, prop) {
  return window.getComputedStyle(element, null).getPropertyValue(prop);
}

function getCanvasFont(el = document.body) {
  const fontWeight = getCssStyle(el, 'font-weight') || 'normal';
  const fontSize = getCssStyle(el, 'font-size') || '16px';
  const fontFamily = getCssStyle(el, 'font-family') || 'Times New Roman';

  return `${fontWeight} ${fontSize} ${fontFamily}`;
}

function initElem(rootElem) {



  if (!rootElem) return;

  let div = document.createElement('div');

  div.style.width = 'fit-content';

  let nbSpaces = 0;
  let nbAyaSpaces = 0;
  let ayaSpaceElements = [];
  let spaceElements = [];
  let wordElements = [];

  if (rootElem.hasChildNodes()) {
    let children = Array.from(rootElem.childNodes);
    let lastWordElem = null;
    let lastSpaceElem = null;
    for (const node of children) {
      if (node.nodeType == Node.TEXT_NODE) {
        let currenText = "";
        const text = node.textContent
        for (let i = 0; i < text.length; i++) {
          const char = text.charAt(i);
          if (char === " ") {
            nbSpaces++;
            let className = "space"
            let ayaSpace = false
            if ((text.charCodeAt(i - 1) >= 0x0660 && text.charCodeAt(i - 1) <= 0x0669) || (text.charCodeAt(i + 1) === 0x06DD)) {
              nbAyaSpaces++;
              className = "ayaspace"
              ayaSpace = true
            }
            if (currenText) {
              if (!lastWordElem) {
                lastWordElem = document.createElement('span');
                lastWordElem.classList.add('word');
                div.appendChild(lastWordElem);
                wordElements.push(lastWordElem);
              }
              const textNode = document.createTextNode(currenText);
              lastWordElem.appendChild(textNode);
              currenText = "";
            }
            lastWordElem = null;

            lastSpaceElem = document.createElement('span');
            lastSpaceElem.classList.add(className);
            const textNode = document.createTextNode(char);
            lastSpaceElem.appendChild(textNode);
            div.appendChild(lastSpaceElem);
            spaceElements.push(lastSpaceElem)
            if (ayaSpace) {
              ayaSpaceElements.push(lastSpaceElem)
            }
          } else {
            currenText += char;
          }
        }
        if (currenText) {
          if (!lastWordElem) {
            lastWordElem = document.createElement('span');
            lastWordElem.classList.add('word');
            div.appendChild(lastWordElem);
            wordElements.push(lastWordElem);
          }
          const textNode = document.createTextNode(currenText);
          lastWordElem.appendChild(textNode);
          currenText = "";
        }
      } else {
        if (lastWordElem) {
          lastWordElem.appendChild(node);
        } else {
          lastWordElem = document.createElement('span');
          lastWordElem.classList.add('word');
          lastWordElem.appendChild(node);
          div.appendChild(lastWordElem);
          wordElements.push(lastWordElem);
        }
      }
    }
  }

  while (rootElem.firstChild) {
    rootElem.removeChild(rootElem.lastChild);
  }

  rootElem.appendChild(div);

  rootElem.nbSpaces = nbSpaces;
  rootElem.nbAyaSpaces = nbAyaSpaces;
  rootElem.ayaSpaceElements = ayaSpaceElements;
  rootElem.spaceElements = spaceElements;
  rootElem.wordElements = wordElements;


}

function justifyElem(rootElem) {

  const desiredWidth = rootElem.clientWidth;

  const nbSpaces = rootElem.nbSpaces
  const nbAyaSpaces = rootElem.nbAyaSpaces
  const ayaSpaceElements = rootElem.ayaSpaceElements
  const spaceElements = rootElem.spaceElements
  const wordElements = rootElem.wordElements

  const div = rootElem.firstChild

  let text = rootElem.textContent;

  const rootStyle = getComputedStyle(rootElem)

  const fontSize = parseInt(rootStyle.fontSize);

  spaceWidth = getTextWidthDOM(" ", getCanvasFont(rootElem));

  let stretchBySpace = 0;
  let stretchByByAyaSpace = 0;
  const nbSimpleSpaces = (nbSpaces - nbAyaSpaces);

  if (nbSpaces && desiredWidth > div.clientWidth) {
    let maxStretchBySpace = spaceWidth * 0.5;
    let maxStretchByAyaSpace = spaceWidth * 2;

    let maxStretch = maxStretchBySpace * nbSimpleSpaces + maxStretchByAyaSpace * nbAyaSpaces;

    let stretch = Math.min(desiredWidth - div.clientWidth, maxStretch);
    let ratio = maxStretch != 0 ? stretch / maxStretch : 0;
    stretchBySpace = ratio * maxStretchBySpace;
    stretchByByAyaSpace = ratio * maxStretchByAyaSpace;
    rootElem.style.wordSpacing = (stretchBySpace / fontSize) + "em";
    for (const elem of ayaSpaceElements) {
      elem.style.wordSpacing = (stretchByByAyaSpace / fontSize) + "em";
    }
  }
  let stop = false;

  for (let i = 1; i <= 5 && !stop; i++) {
    stop = applyExpa(div, wordElements, desiredWidth, i, i, 'jt')
    if (!stop) {
      stop = applyExpa(div, wordElements, desiredWidth, i, i, 'kt')
    }   
  }


  if (desiredWidth > div.clientWidth) {
    const underfull = desiredWidth - div.clientWidth;
    const addedStretch = nbSimpleSpaces !== 0 ? underfull / nbSimpleSpaces : 0;
    //rootElem.style.wordSpacing = ((addedStretch + stretchBySpace) / fontSize) + "em";
    rootElem.style.wordSpacing = (addedStretch + stretchBySpace) + "px";
  } else {
    //shrink by changing font size
  }
}

function applyExpa(div, wordElements, desiredWidth, min, max, type, reverse) {
  for (let i = min; i <= max; i++) {
    for (const elem of wordElements) {

      var fontFeatureSettings = elem.style.fontFeatureSettings;
      if (!elem.style.fontFeatureSettings) {
        elem.style.fontFeatureSettings = `'${type}01'`;
      } else {
        elem.style.fontFeatureSettings = `${fontFeatureSettings},'${type}0${i}'`;
      }
      if (div.clientWidth > desiredWidth) {
        elem.style.fontFeatureSettings = fontFeatureSettings;
        return true
      }
    }
    return false
  }
}

function onLoad(event) {

  calculatewidth = document.getElementById("calculatewidth")  

  const justifyButton = document.getElementById("justification")

  justifyButton.addEventListener("click", (event) => applyJustification(justifyButton.checked));
  

  const lines = document.getElementsByClassName("line")

  for (const line of lines) {
    initElem(line)
  }

  applyJustification(false)


};

function applyJustification(apply) {
  const lines = document.getElementsByClassName("line")
  var date1 = new Date();

  for (const line of lines) {
    line.style.wordSpacing
    for (const elem of line.wordElements) {
      elem.style.fontFeatureSettings = null
    }
    for (const elem of line.ayaSpaceElements) {
      line.style.wordSpacing = null
    }

    if (apply) {
      justifyElem(line)
    }
    
  }

  var date2 = new Date();
  var diff = date2 - date1; //milliseconds interval
  console.info(`elapsed time=${diff}`)
}
