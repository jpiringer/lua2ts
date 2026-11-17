-- Doom Fire Effect - Converted from JavaScript to Lua

if os.getenv("LOCAL_LUA_DEBUGGER_VSCODE") == "1" then
    require("lldebugger").start()
end

local showText = false
local maxFrames = fps*20

local fontSize = 400
local text = {
    "all", "cars",
    "are", "burning",
    "burn", "those", "cars",
}
local currentWordIndex = 1

local firePixelsArray = {}
local tileSize = 15
local fireWidth
local fireHeight

local fireIncrement = 3

local fireColorsPalette = {
    {r = 9, g = 2, b = 0},
    {r = 40, g = 2, b = 0},
    {r = 61, g = 6, b = 0},
    {r = 92, g = 6, b = 0},
    {r = 113, g = 9, b = 0},
    {r = 134, g = 12, b = 0},
    {r = 155, g = 12, b = 0},
    {r = 186, g = 15, b = 0},
    {r = 207, g = 18, b = 0},
    {r = 228, g = 25, b = 0},
    {r = 248, g = 28, b = 0},
    {r = 255, g = 28, b = 0},
    {r = 255, g = 31, b = 0},
    {r = 255, g = 35, b = 0},
    {r = 255, g = 35, b = 0},
    {r = 255, g = 38, b = 0},
    {r = 255, g = 38, b = 0},
    {r = 255, g = 41, b = 1},
    {r = 255, g = 44, b = 1},
    {r = 255, g = 47, b = 1},
    {r = 255, g = 50, b = 1},
    {r = 255, g = 54, b = 2},
    {r = 255, g = 54, b = 2},
    {r = 255, g = 58, b = 2},
    {r = 255, g = 63, b = 3},
    {r = 255, g = 63, b = 3},
    {r = 248, g = 70, b = 4},
    {r = 248, g = 70, b = 4},
    {r = 248, g = 75, b = 4},
    {r = 239, g = 75, b = 4},
    {r = 239, g = 75, b = 5},
    {r = 239, g = 75, b = 5},
    {r = 255, g = 84, b = 15},
    {r = 255, g = 111, b = 47},
    {r = 255, g = 159, b = 111},
    {r = 255, g = 199, b = 159},
    {r = 255, g = 255, b = 255}
}

--[[local fireColorsPalette = {
    {r = 7, g = 7, b = 7},
    {r = 31, g = 7, b = 7},
    {r = 47, g = 15, b = 7},
    {r = 71, g = 15, b = 7},
    {r = 87, g = 23, b = 7},
    {r = 103, g = 31, b = 7},
    {r = 119, g = 31, b = 7},
    {r = 143, g = 39, b = 7},
    {r = 159, g = 47, b = 7},
    {r = 175, g = 63, b = 7},
    {r = 191, g = 71, b = 7},
    {r = 199, g = 71, b = 7},
    {r = 223, g = 79, b = 7},
    {r = 223, g = 87, b = 7},
    {r = 223, g = 87, b = 7},
    {r = 215, g = 95, b = 7},
    {r = 215, g = 95, b = 7},
    {r = 215, g = 103, b = 15},
    {r = 207, g = 111, b = 15},
    {r = 207, g = 119, b = 15},
    {r = 207, g = 127, b = 15},
    {r = 207, g = 135, b = 23},
    {r = 199, g = 135, b = 23},
    {r = 199, g = 143, b = 23},
    {r = 199, g = 151, b = 31},
    {r = 191, g = 159, b = 31},
    {r = 191, g = 159, b = 31},
    {r = 191, g = 167, b = 39},
    {r = 191, g = 167, b = 39},
    {r = 191, g = 175, b = 47},
    {r = 183, g = 175, b = 47},
    {r = 183, g = 183, b = 47},
    {r = 183, g = 183, b = 55},
    {r = 207, g = 207, b = 111},
    {r = 223, g = 223, b = 159},
    {r = 239, g = 239, b = 199},
    {r = 255, g = 255, b = 255}
}]]

local function createFireDataStructure()
    local numberOfPixels = fireWidth * fireHeight
    for i = 1, numberOfPixels do
        firePixelsArray[i] = 0
    end
end

local function createFireSource()
    for column = 1, fireWidth do
        local overflowPixelIndex = fireWidth * fireHeight
        local pixelIndex = (overflowPixelIndex - fireWidth) + column
        firePixelsArray[pixelIndex] = 2 -- 36
    end
end

local function destroyFireSource()
    for column = 1, fireWidth do
        local overflowPixelIndex = fireWidth * fireHeight
        local pixelIndex = (overflowPixelIndex - fireWidth) + column
        firePixelsArray[pixelIndex] = 0
    end
end


local function init()
    local x,y,w,h = getPageRect()

    fireWidth = math.floor(w / tileSize)  
    fireHeight = math.floor(h / tileSize)
    createFireDataStructure()
    createFireSource()
    -- setInterval(calculateFirePropagation, 50)
    -- Lua note: Use a game loop or timer instead of setInterval
end

local function updateFireIntensityPerPixel(row, column)
    local pixelIndex = column + (fireWidth * row)
    local belowPixelIndex = pixelIndex + fireWidth
    local windDirection = math.random()

    local column_To_Apply = column
    if windDirection < 0.2 then
        column_To_Apply = (column - 1)
    elseif windDirection > 0.8 then
        column_To_Apply = (column + 1)
    end
    local newPixelIndex = column_To_Apply + (fireWidth * row)

    -- below pixel index overflows canvas
    if belowPixelIndex >= fireWidth * fireHeight then
        return
    end

    local decay = math.floor(math.random() * 3)
    local belowPixelFireIntensity = firePixelsArray[belowPixelIndex] or 0
    local newFireIntensity = belowPixelFireIntensity - decay
    if newFireIntensity < 0 then
        newFireIntensity = 0
    end

    firePixelsArray[newPixelIndex] = newFireIntensity
end

local function update()
    for column = 1, fireWidth do
        for row = 1, fireHeight do
            updateFireIntensityPerPixel(row, column)
        end
    end
end

local function renderFire()
    -- Returns 2D grid of RGB values (0-255)
    -- Original JS built HTML; this Lua version returns data for rendering
    local grid = {}
    for row = 1, fireHeight do
        grid[row] = {}
        for column = 1, fireWidth do
            local pixelIndex = column + (fireWidth * row)
            local fireIntensity = firePixelsArray[pixelIndex] or 0
            -- Clamp to valid palette range [0, 35]
            local idx = fireIntensity + 1  -- Lua tables are 1-based
            if idx < 1 then 
                idx = 1 
            end
            if idx > #fireColorsPalette then 
                idx = #fireColorsPalette 
            end
            local color = fireColorsPalette[idx]
            grid[row][column] = {r = color.r, g = color.g, b = color.b}
        end
    end
    return grid
end

local function increaseFireSource()
    for column = 1, fireWidth do
        local overflowPixelIndex = fireWidth * fireHeight
        local pixelIndex = (overflowPixelIndex - fireWidth) + column
        local currentFireIntensity = firePixelsArray[pixelIndex] or 0
        if currentFireIntensity < 36 then
            local increase = math.floor(math.random() * fireIncrement)
            local newFireIntensity = currentFireIntensity + increase
            if newFireIntensity >= 36 then
                newFireIntensity = 36
            end
            firePixelsArray[pixelIndex] = newFireIntensity
        end
    end
end

local function decreaseFireSource()
    for column = 1, fireWidth do
        local overflowPixelIndex = fireWidth * fireHeight
        local pixelIndex = (overflowPixelIndex - fireWidth) + column
        local currentFireIntensity = firePixelsArray[pixelIndex] or 0
        if currentFireIntensity > 0 then
            local decay = math.floor(math.random() * fireIncrement)
            local newFireIntensity = currentFireIntensity - decay
            if newFireIntensity < 0 then
                newFireIntensity = 0
            end
            firePixelsArray[pixelIndex] = newFireIntensity
        end
    end
end

local function draw()
    local x,y,w,h = getPageRect()
    local grid = renderFire()

    setColor(0,0,0,1)
	clearPage()

    if showText then
        setColor(1, 0, 0, 1)
        drawCenteredText(w/2, h/2+fontSize/3, text[currentWordIndex])
    end

    -- draw the grid using your rendering method
    for row = 1, fireHeight do
        for column = 1, fireWidth do
            local color = grid[row][column]
            if color.r > 10 then
                setColor(color.r / 255, color.g / 255, color.b / 255, 1)
                local xp = (column - 1) * (w / fireWidth)
                local yp = h - (row + 1) * (h / fireHeight)
                local wp = w / fireWidth
                local hp = h / fireHeight
                fillRectangle(xp, yp, wp, hp)
            end
        end
    end
end

--- @param frameNr number
--- @return nil
local function ff(frameNr)
	local x,y,w,h = getPageRect()

	if frameNr == 1 then
		init()
		setColorSpace("RGB")
		setFont("DejaVuSerif")
		setFontSize(fontSize)
	end

	draw()
	update()

    if frameNr % 30 == 0 then
        increaseFireSource()
        currentWordIndex = currentWordIndex + 1
        if currentWordIndex > #text then
            currentWordIndex = 1
        end
    end
	--fireStrength = fireStrength + fireInc
end

return {frameFunction = ff, maxFrames = maxFrames, exportsAlpha = false}