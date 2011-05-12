-- Example of inheritance extension
function TileLayer:fill(tile)
   for x=0,self.width-1 do
      for y=0,self.height-1 do
         self[x][y] = tile
      end
   end
end

function TileLayer:clear()
   self:fill(nil)
end

local desertSet = Map:getTileset("Desert");
local sand = desertSet:tileAt(5,3) -- Sand tile
Map.Ground:fill(sand)

function drawExtendedTileBox(arg)
   local destX = arg.x+arg.width-1
   local destY = arg.y+arg.height-1
   -- Fill
   for y = arg.y+1,destY-1 do
      for x = arg.x+1,destX-1 do
         arg.layer[x][y] = arg.tileBox:tileAt(arg.tileBox.width/2,arg.tileBox.height/2)
      end
   end
   -- Edges
   for x = arg.x+1,destX-1 do
         arg.layer[x][arg.y] = arg.tileBox:tileAt(arg.tileBox.width/2,0)
         arg.layer[x][destY] = arg.tileBox:tileAt(arg.tileBox.width/2,arg.tileBox.height-1)
   end
   for y = arg.y+1,destY-1 do
         arg.layer[arg.x][y] = arg.tileBox:tileAt(0,arg.tileBox.height/2)
         arg.layer[destX][y] = arg.tileBox:tileAt(arg.tileBox.width-1,arg.tileBox.height/2)
   end                        
   -- Corners
   arg.layer[arg.x][arg.y] = arg.tileBox:tileAt(0,0)
   arg.layer[destX][arg.y] = arg.tileBox:tileAt(arg.tileBox.width-1,0)
   arg.layer[destX][destY] = arg.tileBox:tileAt(arg.tileBox.width-1,arg.tileBox.height-1)
   arg.layer[arg.x][destY] = arg.tileBox:tileAt(0,arg.tileBox.height-1)
end

local TilesetBox = {}
function TilesetBox:new(arg)
   local inst = {}
   self.__index = self;
   setmetatable(inst, self)
   inst.x = arg.x
   inst.y = arg.y
   inst.width = arg.width
   inst.height = arg.height
   inst.tileset = arg.tileset
   return inst;
end

function TilesetBox:tileAt(x, y)
   return self.tileset:tileAt(self.x+x, self.y+y)
end

local concreteTilesetBox = TilesetBox:new {
   x = 0, y = 3,
   width = 3, height = 3,
   tileset = desertSet
}

local dirtTilesetBox = TilesetBox:new {
   x = 5, y = 0,
   width = 3, height = 3,
   tileset = desertSet
}

local obstacles = { 
   desertSet:tileAt(6,3),
   desertSet:tileAt(7,3),
   desertSet:tileAt(5,4),
   desertSet:tileAt(6,4),
   desertSet:tileAt(7,4),
   desertSet:tileAt(5,5),
   desertSet:tileAt(6,5),
   desertSet:tileAt(7,5)
}

local Box = {}
function Box:new(x,y,w,h)
   local inst = {}
   self.__index = self;
   setmetatable(inst, self)
   inst.x = x
   inst.y = y
   inst.width = w
   inst.height = h
   return inst;
end

function Box:random(arg)
   return self:new(
      math.random(arg.min_x,arg.max_x),
      math.random(arg.min_y,arg.max_y),
      math.random(arg.min_w,arg.max_w),
      math.random(arg.min_h,arg.max_h))
end

function Box:collides(box)
   if box == nil then return false end
   
   local absX = self.x + self.width
   local absY = self.y + self.height
   local boxAbsX = box.x+box.width
   local boxAbsY = box.y+box.height
   
   if (box.x > absX) then return false end
   if (boxAbsX < self.x) then return false end
   
   if (box.y > absY) then return false end
   if (boxAbsY < self.y) then return false end
   
   return true
end

function Box:collidesWith(boxlist)
   for i=1,#boxlist do
      if self:collides(boxlist[i]) then
         return true
      end
   end
   return false
end


-- Draw random obstacles
for i=1,100 do
   local x = math.random(Map.Ground.width)
   local y = math.random(Map.Ground.height)
   Map.Ground[x][y] = obstacles[math.random(#obstacles)]
end

local randomBoxes = {}
function drawRandomTilesetBoxes(tilesetBox)   
   for i=1,10 do
       local randomBox = Box:random {
         min_x = 1, max_x = Map.Ground.width-10,
         min_y = 1, max_y = Map.Ground.height-10,
         min_w = 5, max_w = 10,
         min_h = 5, max_h =10
      }
      if not randomBox:collidesWith(randomBoxes) then
         randomBoxes[#randomBoxes+1] = randomBox
         drawExtendedTileBox{
            x=randomBox.x,
            y=randomBox.y,
            width=randomBox.width,
            height=randomBox.height,
            layer=Map.Ground,
            tileBox=tilesetBox
         }
      end
   end
end

drawRandomTilesetBoxes(concreteTilesetBox)
drawRandomTilesetBoxes(dirtTilesetBox)