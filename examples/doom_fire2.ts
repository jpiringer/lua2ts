if ((os.getenv("LOCAL_LUA_DEBUGGER_VSCODE") === "1")) {
  require("lldebugger").start();
}
let showText: any = false;
let maxFrames: any = (fps * 20);
let fontSize: any = 400;
let text: any = ["all", "cars", "are", "burning", "burn", "those", "cars"];
let currentWordIndex: any = 1;
let firePixelsArray: any = {};
let tileSize: any = 15;
let fireWidth: any;
let fireHeight: any;
let fireIncrement: any = 3;
let fireColorsPalette: any = [{ r: 9, g: 2, b: 0 }, { r: 40, g: 2, b: 0 }, { r: 61, g: 6, b: 0 }, { r: 92, g: 6, b: 0 }, { r: 113, g: 9, b: 0 }, { r: 134, g: 12, b: 0 }, { r: 155, g: 12, b: 0 }, { r: 186, g: 15, b: 0 }, { r: 207, g: 18, b: 0 }, { r: 228, g: 25, b: 0 }, { r: 248, g: 28, b: 0 }, { r: 255, g: 28, b: 0 }, { r: 255, g: 31, b: 0 }, { r: 255, g: 35, b: 0 }, { r: 255, g: 35, b: 0 }, { r: 255, g: 38, b: 0 }, { r: 255, g: 38, b: 0 }, { r: 255, g: 41, b: 1 }, { r: 255, g: 44, b: 1 }, { r: 255, g: 47, b: 1 }, { r: 255, g: 50, b: 1 }, { r: 255, g: 54, b: 2 }, { r: 255, g: 54, b: 2 }, { r: 255, g: 58, b: 2 }, { r: 255, g: 63, b: 3 }, { r: 255, g: 63, b: 3 }, { r: 248, g: 70, b: 4 }, { r: 248, g: 70, b: 4 }, { r: 248, g: 75, b: 4 }, { r: 239, g: 75, b: 4 }, { r: 239, g: 75, b: 5 }, { r: 239, g: 75, b: 5 }, { r: 255, g: 84, b: 15 }, { r: 255, g: 111, b: 47 }, { r: 255, g: 159, b: 111 }, { r: 255, g: 199, b: 159 }, { r: 255, g: 255, b: 255 }];
function createFireDataStructure(): any {
  let numberOfPixels: any = (fireWidth * fireHeight);
  for (let i = 1; i <= numberOfPixels; i++) {
    firePixelsArray[i] = 0;
  }
}
function createFireSource(): any {
  for (let column = 1; column <= fireWidth; column++) {
    let overflowPixelIndex: any = (fireWidth * fireHeight);
    let pixelIndex: any = ((overflowPixelIndex - fireWidth) + column);
    firePixelsArray[pixelIndex] = 2;
  }
}
function destroyFireSource(): any {
  for (let column = 1; column <= fireWidth; column++) {
    let overflowPixelIndex: any = (fireWidth * fireHeight);
    let pixelIndex: any = ((overflowPixelIndex - fireWidth) + column);
    firePixelsArray[pixelIndex] = 0;
  }
}
function init(): any {
  let x: any = getPageRect(), y: any, w: any, h: any;
  fireWidth = Math.floor((w / tileSize));
  fireHeight = Math.floor((h / tileSize));
  createFireDataStructure();
  createFireSource();
}
function updateFireIntensityPerPixel(row: any, column: any): any {
  let pixelIndex: any = (column + (fireWidth * row));
  let belowPixelIndex: any = (pixelIndex + fireWidth);
  let windDirection: any = Math.random();
  let column_To_Apply: any = column;
  if ((windDirection < 0.2)) {
    column_To_Apply = (column - 1);
  } else if ((windDirection > 0.8)) {
    column_To_Apply = (column + 1);
  }
  let newPixelIndex: any = (column_To_Apply + (fireWidth * row));
  if ((belowPixelIndex >= (fireWidth * fireHeight))) {
    return;
  }
  let decay: any = Math.floor((Math.random() * 3));
  let belowPixelFireIntensity: any = (firePixelsArray[belowPixelIndex] || 0);
  let newFireIntensity: any = (belowPixelFireIntensity - decay);
  if ((newFireIntensity < 0)) {
    newFireIntensity = 0;
  }
  firePixelsArray[newPixelIndex] = newFireIntensity;
}
function update(): any {
  for (let column = 1; column <= fireWidth; column++) {
    for (let row = 1; row <= fireHeight; row++) {
      updateFireIntensityPerPixel(row, column);
    }
  }
}
function renderFire(): any {
  let grid: any = {};
  for (let row = 1; row <= fireHeight; row++) {
    grid[row] = {};
    for (let column = 1; column <= fireWidth; column++) {
      let pixelIndex: any = (column + (fireWidth * row));
      let fireIntensity: any = (firePixelsArray[pixelIndex] || 0);
      let idx: any = (fireIntensity + 1);
      if ((idx < 1)) {
        idx = 1;
      }
      if ((idx > (fireColorsPalette).length)) {
        idx = (fireColorsPalette).length;
      }
      let color: any = fireColorsPalette[idx];
      grid[row][column] = { r: color.r, g: color.g, b: color.b };
    }
  }
  return grid;
}
function increaseFireSource(): any {
  for (let column = 1; column <= fireWidth; column++) {
    let overflowPixelIndex: any = (fireWidth * fireHeight);
    let pixelIndex: any = ((overflowPixelIndex - fireWidth) + column);
    let currentFireIntensity: any = (firePixelsArray[pixelIndex] || 0);
    if ((currentFireIntensity < 36)) {
      let increase: any = Math.floor((Math.random() * fireIncrement));
      let newFireIntensity: any = (currentFireIntensity + increase);
      if ((newFireIntensity >= 36)) {
        newFireIntensity = 36;
      }
      firePixelsArray[pixelIndex] = newFireIntensity;
    }
  }
}
function decreaseFireSource(): any {
  for (let column = 1; column <= fireWidth; column++) {
    let overflowPixelIndex: any = (fireWidth * fireHeight);
    let pixelIndex: any = ((overflowPixelIndex - fireWidth) + column);
    let currentFireIntensity: any = (firePixelsArray[pixelIndex] || 0);
    if ((currentFireIntensity > 0)) {
      let decay: any = Math.floor((Math.random() * fireIncrement));
      let newFireIntensity: any = (currentFireIntensity - decay);
      if ((newFireIntensity < 0)) {
        newFireIntensity = 0;
      }
      firePixelsArray[pixelIndex] = newFireIntensity;
    }
  }
}
function draw(): any {
  let x: any = getPageRect(), y: any, w: any, h: any;
  let grid: any = renderFire();
  setColor(0, 0, 0, 1);
  clearPage();
  if (showText) {
    setColor(1, 0, 0, 1);
    drawCenteredText((w / 2), ((h / 2) + (fontSize / 3)), text[currentWordIndex]);
  }
  for (let row = 1; row <= fireHeight; row++) {
    for (let column = 1; column <= fireWidth; column++) {
      let color: any = grid[row][column];
      if ((color.r > 10)) {
        setColor((color.r / 255), (color.g / 255), (color.b / 255), 1);
        let xp: any = ((column - 1) * (w / fireWidth));
        let yp: any = (h - ((row + 1) * (h / fireHeight)));
        let wp: any = (w / fireWidth);
        let hp: any = (h / fireHeight);
        fillRectangle(xp, yp, wp, hp);
      }
    }
  }
}
function ff(frameNr: any): any {
  let x: any = getPageRect(), y: any, w: any, h: any;
  if ((frameNr === 1)) {
    init();
    setColorSpace("RGB");
    setFont("DejaVuSerif");
    setFontSize(fontSize);
  }
  draw();
  update();
  if (((frameNr % 30) === 0)) {
    increaseFireSource();
    currentWordIndex = (currentWordIndex + 1);
    if ((currentWordIndex > (text).length)) {
      currentWordIndex = 1;
    }
  }
}
return { frameFunction: ff, maxFrames: maxFrames, exportsAlpha: false };
