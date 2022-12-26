digitalkhatt = digitalkhatt or { }

local charstringsCache = {}

local byte, lower, char, gsub = string.byte, string.lower, string.char, string.gsub

local streamreader       = utilities.files
local openfile           = streamreader.open
local closefile          = streamreader.close
----- skipbytes          = streamreader.skip
local setposition        = streamreader.setposition
local skipshort          = streamreader.skipshort
local readbytes          = streamreader.readbytes
local readstring         = streamreader.readstring
local readbyte           = streamreader.readcardinal1  --  8-bit unsigned integer
local readushort         = streamreader.readcardinal2  -- 16-bit unsigned integer
local readuint           = streamreader.readcardinal3  -- 24-bit unsigned integer
local readulong          = streamreader.readcardinal4  -- 32-bit unsigned integer
----- readchar           = streamreader.readinteger1   --  8-bit   signed integer
local readshort          = streamreader.readinteger2   -- 16-bit   signed integer
local readlong           = streamreader.readinteger4   -- 32-bit unsigned integer
local readfixed          = streamreader.readfixed4
local read2dot14         = streamreader.read2dot14     -- 16-bit signed fixed number with the low 14 bits of fraction (2.14) (F2DOT14)
local readfword          = readshort                   -- 16-bit   signed integer that describes a quantity in FUnits
local readufword         = readushort                  -- 16-bit unsigned integer that describes a quantity in FUnits
local readoffset         = readushort
local readcardinaltable  = streamreader.readcardinaltable
local readintegertable   = streamreader.readintegertable

local setmetatableindex  = table.setmetatableindex
local sortedkeys         = table.sortedkeys
local sortedhash         = table.sortedhash
local stripstring        = string.nospaces

local fonts              = fonts or { }
local handlers           = fonts.handlers or { }
local otf                = handlers.otf or { }
local readers            = otf.readers or { }
local helpers           = readers.helpers

local report             = logs.reporter("otf reader")

local function loadtables(f,specification,offset)
    if offset then
        setposition(f,offset)
    end
   
    local tables   = { }
    local basename = file.basename(specification.filename)
    local filesize = specification.filesize or 0
    local filetime = specification.filetime
    local fontdata = { -- some can/will go
        filename      = basename,
        filesize      = filesize,
        filetime      = filetime,
        version       = readstring(f,4),
        noftables     = readushort(f),
        searchrange   = readushort(f), -- not needed
        entryselector = readushort(f), -- not needed
        rangeshift    = readushort(f), -- not needed
        tables        = tables,
        foundtables   = false,
    }
    for i=1,fontdata.noftables do
        local tag      = lower(stripstring(readstring(f,4)))
     -- local checksum = readulong(f) -- not used
        local checksum = readushort(f) * 0x10000 + readushort(f)
        local offset   = readulong(f)
        local length   = readulong(f)
        if offset + length > filesize then
            report("bad %a table in file %a",tag,basename)
        end
        tables[tag] = {
            checksum = checksum,
            offset   = offset,
            length   = length,
        }
    end
  -- inspect(tables)
    fontdata.foundtables = sortedkeys(tables)
    if tables.cff or tables.cff2 then
        fontdata.format = "opentype"
    else
        fontdata.format = "truetype"
    end
    return fontdata, tables
  end


local function prepareglyps(fontdata)
    local glyphs = setmetatableindex(function(t,k)
        local v = {
            -- maybe more defaults
            index = k,
        }
        t[k] = v
        return v
    end)
    fontdata.glyphs  = glyphs
    fontdata.mapping = { }
end

local function readtable(tag,f,fontdata,specification,...)
    local reader = readers[tag]
    if reader then
        return reader(f,fontdata,specification,...)
    end
end

local function readdata(f,offset,specification)

    local fontdata, tables = loadtables(f,specification,offset)
    
    readtable("name",f,fontdata,specification)
    
    readtable("stat",f,fontdata,specification)
    readtable("avar",f,fontdata,specification)
    readtable("fvar",f,fontdata,specification)
    
    specification.digitalkhatt = true
    
    prepareglyps(fontdata)    

    local data = readtable("dkcff2",f,fontdata,specification)
    
    if data then    
      return {f = f, specification = specification, data = data, fontdata = fontdata}
    end
end

local parsecharstrings = digitalkhatt.parsecharstrings
  
digitalkhatt.readVarGlyph = function(spec, gid)
  local filename = spec.properties.filename
  local instance = spec.properties.instance
  local charstrings = charstringsCache[filename]  
  if not charstrings then
    local fontspec = {
        filename = filename,
        shapes   = true,
        streams  = nil,
        variable = true,
        subfont  = nil,
        instance = instance,
        glyphs = true,
    }
    local f = openfile(filename,true)
    local fileattr = lfs.attributes(filename)
    fontspec.filesize = fileattr and fileattr.size or 0
    charstrings = readdata(f,0,fontspec)
    charstringsCache[filename] = charstrings
  end
  if charstrings then
    local fontdata = charstrings.fontdata
    --local variabledata = fontdata.variabledata
    --local specification = charstrings.specification
    local data = charstrings.data
    if type(instance) == "string" then
        local factors = helpers.getfactors(fontdata,instance)
        if factors then
            --specification.factors = factors
            fontdata.factors  = factors
            fontdata.instance = instance
            data.digitalkhatt_gid = gid + 1
            local glyphs = {}
            data.factors = factors --specification.factors
            parsecharstrings(false,data,glyphs,true,"cff",nil)
            return glyphs[gid]
        end
    end
  end
  
  
end