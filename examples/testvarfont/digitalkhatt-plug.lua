-----------------------------------------------------------------------
--         FILE:  luaotfload-harf-plug.lua
--  DESCRIPTION:  part of luaotfload / HarfBuzz / fontloader plugin
-----------------------------------------------------------------------
--[[
do -- block to avoid to many local variables error
 assert(luaotfload_module, "This is a part of luaotfload and should not be loaded independently") { 
     name          = "luaotfload-harf-plug",
     version       = "3.17",       --TAGVERSION
     date          = "2021-01-08", --TAGDATE
     description   = "luaotfload submodule / HarfBuzz shaping",
     license       = "GPL v2.0",
     author        = "Khaled Hosny, Marcel Krüger",
     copyright     = "Luaotfload Development Team",     
 }
end
--]]

local hb                = require('luadigitalkhatt')
local logreport         = luaotfload.log.report

local assert            = assert
local next              = next
local tonumber          = tonumber
local type              = type
local format            = string.format
local open              = io.open
local tableinsert       = table.insert
local tableremove       = table.remove
local ostmpname         = os.tmpname
local osremove          = os.remove
local osexit            = os.exit

local direct            = node.direct
local tonode            = direct.tonode
local todirect          = direct.todirect
local traverse          = direct.traverse
local traverse_list     = direct.traverse_list
local insertbefore      = direct.insert_before
local insertafter       = direct.insert_after
local protectglyph      = direct.protect_glyph
local newnode           = direct.new
local freenode          = direct.free
local copynode          = direct.copy
local removenode        = direct.remove
local copynodelist      = direct.copy_list
local ischar            = direct.is_char
local uses_font         = direct.uses_font
local length            = direct.length
local getglue           = direct.getglue
local setglue           = direct.setglue
local gettail           = direct.tail

local getattrs          = direct.getattributelist
local setattrs          = direct.setattributelist
local getchar           = direct.getchar
local setchar           = direct.setchar
local getdirection      = direct.getdirection
local getdisc           = direct.getdisc
local setdisc           = direct.setdisc
local getfont           = direct.getfont
local getdata           = direct.getdata
local setdata           = direct.setdata
local getfont           = direct.getfont
local setfont           = direct.setfont
local getwhatsitfield   = direct.getwhatsitfield or direct.getfield
local setwhatsitfield   = direct.setwhatsitfield or direct.setfield
local getfield          = direct.getfield
local setfield          = direct.setfield
local getid             = direct.getid
local getkern           = direct.getkern
local setkern           = direct.setkern
local getnext           = direct.getnext
local setnext           = direct.setnext
local getoffsets        = direct.getoffsets
local setoffsets        = direct.setoffsets
local getproperty       = direct.getproperty
local setproperty       = direct.setproperty
local getprev           = direct.getprev
local setprev           = direct.setprev
local getsubtype        = direct.getsubtype
local setsubtype        = direct.setsubtype
local getwidth          = direct.getwidth
local setwidth          = direct.setwidth
local setlist           = direct.setlist
local getlist           = direct.getlist
local is_char           = direct.is_char
local tail              = direct.tail
local getboth           = direct.getboth
local setlink           = direct.setlink
local hpack             = direct.hpack
local flush_list        = direct.flush_list
local flush_node        = direct.flush_node
local dimensions        = direct.dimensions

local properties        = direct.get_properties_table()

local hlist_t           = node.id("hlist")
local disc_t            = node.id("disc")
local glue_t            = node.id("glue")
local glyph_t           = node.id("glyph")
local dir_t             = node.id("dir")
local kern_t            = node.id("kern")
local localpar_t        = node.id("local_par")
local whatsit_t         = node.id("whatsit")
local pdfliteral_t      = node.subtype("pdf_literal")
local leftskip_t        = 8
local rightskip_t       = 9


local line_t            = 1
local box_t             = 2
local explicitdisc_t    = 1
local firstdisc_t       = 4
local seconddisc_t      = 5
local fontkern_t        = 0
local italiccorr_t      = 3
local regulardisc_t     = 3
local spaceskip_t       = 13

local dir_ltr           = hb.Direction.new("ltr")
local dir_rtl           = hb.Direction.new("rtl")
local fl_unsafe         = hb.Buffer.GLYPH_FLAG_UNSAFE_TO_BREAK

local startactual_p     = "luaotfload_startactualtext"
local endactual_p       = "luaotfload_endactualtext"
local startnode_p       = "digitalkhatt_startnode"
local endnode_p         = "digitalkhatt_endnode"

local empty_table       = {}

local iftrue   = token.create("iftrue")

-- "Copy" properties as done by LuaTeX: Make old properties metatable 
local function copytable(old)
  local new = {}
  for k, v in next, old do
    if type(v) == "table" then v = copytable(v) end
    new[k] = v
  end
  setmetatable(new, getmetatable(old))
  return new
end

-- Set and get properties.
local function setprop(n, prop, value)
  if not n then
    return
  end
  local props = properties[n]
  if not props then
    props = {}
    properties[n] = props
  end
  props[prop] = value
end

local function inherit(t, base, properties)
  local n = newnode(t)
  setattrs(n, getattrs(base))
  if properties then
    setproperty(n, setmetatable({}, {__index = properties}))
  end
  return n
end
-- New kern node of amount `v`, inheriting the properties/attributes of `n`.
local function newkern(v, n)
  local kern = inherit(kern_t, n, getproperty(n))
  setkern(kern, v)
  return kern
end

local function insertkern(head, current, kern, rtl)
  if rtl then
    head = insertbefore(head, current, kern)
  else
    head, current = insertafter(head, current, kern)
  end
  return head, current
end

-- Convert list of integers to UTF-16 hex string used in PDF.
local function to_utf16_hex(uni)
  if uni < 0x10000 then
    return format("%04X", uni)
  else
    uni = uni - 0x10000
    local hi = 0xD800 + (uni // 0x400)
    local lo = 0xDC00 + (uni % 0x400)
    return format("%04X%04X", hi, lo)
  end
end

local process

local function itemize(head, fontid, direction)
  local fontdata = font.getfont(fontid)
  local hbdata   = fontdata and fontdata.hb
  local spec     = fontdata and fontdata.specification
  local options  = spec and spec.features.raw

  local runs, codes = {}, {}
  local dirstack = {}
  local currdir = direction or 0
  local lastskip, lastdir = true
  local lastrun = {}

  for n, id, subtype in traverse(head) do
    
    local typeid = node.types()[id]        
    
    local code = 0xFFFC -- OBJECT REPLACEMENT CHARACTER
    local skip = lastskip
    local props = properties[n]

    if props and props.zwnj then
      code = 0x200C
      -- skip = false -- Not sure about this, but lastskip should be a bit faster
    elseif id == glyph_t then
      local char = is_char(n, fontid)
      if char then
        code = char
        skip = false
      else
        skip = true
      end
    elseif id == glue_t and subtype == spaceskip_t then
      code = 0x0020 -- SPACE
    elseif id == disc_t then
      skip = true
      --print "disc not supported"
      --osexit();
    elseif id == dir_t then
      local dir, cancel = getdirection(n)
      local direction, kind = getdirection(n)
      if cancel then
        assert(currdir == dir)
        -- Pop the last direction from the stack.
        currdir = tableremove(dirstack)
      else
        -- Push the current direction to the stack.
        tableinsert(dirstack, currdir)
        currdir = dir
      end
    elseif id == localpar_t then
      currdir = getdirection(n)
    end

    local ncodes = #codes -- Necessary to count discs correctly
    codes[ncodes + 1] = code

    if (lastdir ~= currdir or lastskip ~= skip) then      
      lastrun.after = n
      lastrun = {
        start = ncodes + 1,
        len = code and 1 or 0,
        font = fontid,
        dir = currdir == 1 and dir_rtl or dir_ltr,
        skip = skip,
        codes = codes      
      }
      runs[#runs + 1] = lastrun
      lastdir, lastskip = currdir, skip
    elseif code then
      lastrun.len = lastrun.len + 1    
    end
  end

  return head, runs
end


-- Check if it is not safe to break before this glyph.
local function unsafetobreak(glyph)
  return glyph
     and glyph.flags
     and glyph.flags & fl_unsafe
end

local shape

-- Main shaping function that calls HarfBuzz, and does some post-processing of
-- the output.
function shape(head, firstnode, run)
  local node = firstnode
  local codes = run.codes
  local offset = run.start - 1 -- -1 because we want the cluster
  local len = run.len
  local fontid = run.font
  local dir = run.dir
  local cluster


  local fontdata = font.getfont(fontid)
  local hbdata = fontdata.hb
  local spec = fontdata.specification
  local features = spec.hb_features
  local options = spec.features.raw
  local hbshared = hbdata.shared
  local hbfont = hbshared.font

  local lang = spec.language
  local script = spec.script
  local shapers = options.shaper and { options.shaper } or {}

  local buf = hb.Buffer.new()
  if buf.set_flags then
    buf:set_flags(hbdata.buf_flags)
  end
  buf:set_direction(dir)
  buf:set_script(script)
  buf:set_language(lang)
  buf:set_cluster_level(buf.CLUSTER_LEVEL_MONOTONE_CHARACTERS)
  buf:add_codepoints(codes, offset, len)

  local hscale = hbdata.hscale
  local vscale = hbdata.vscale
  hbfont:set_scale(hscale, vscale)

  do
    features = table.merged(features) -- We don't want to modify the global features
   
    local current_features = {}
    local n = node
    for i = offset, offset+len-1 do
      local props = properties[n] or empty_table      
      if props then
        local local_feat = props.glyph_features or empty_table
        if local_feat then
          for tag, value in next, current_features do
            local loc = local_feat[tag]
            loc = loc ~= nil and (tonumber(loc) or (loc and 1 or 0)) or nil
            if value.value ~= loc then -- This includes loc == nil
              value._end = i
              features[#features + 1] = value
              current_features[tag] = nil
            end
          end
          for tag, value in next, local_feat do
            if not current_features[tag] then
              local feat = hb.Feature.new(tag)
              feat.value = tonumber(value) or (value and 1 or 0)
              feat.start = i
              current_features[tag] = feat
            end
          end
        end
      end
      if run.digitalKhattOptions and run.digitalKhattOptions.tajweedColor then
        local tag = "tjwd"
        if not current_features[tag] then
          local feat = hb.Feature.new(tag)
          feat.value = 1
          feat.start = i
          current_features[tag] = feat
        end        
      end
      n = getnext(n)
    end
    for _, feat in next, current_features do
      features[#features + 1] = feat
    end
  end
  
  if run.linewidth then
    local width = run.linewidth / fontdata.hb.scale
    buf:set_justify(width)
    local feat = hb.Feature.new("shr1")
    feat.value = 1
    feat.start = 1
    features[#features + 1] = feat
  end  

  if hb.shape_full(hbfont, buf, features, shapers) then
    -- The engine wants the glyphs in logical order, but HarfBuzz outputs them
    -- in visual order, so we reverse RTL buffers.

    if dir:is_backward() then buf:reverse() end

    local glyphs = buf:get_glyphs()   
   
    local i = 0
    local glyph
    -- The following is a repeat {...} while glyph {...} loop.
    while true do
      repeat
        i = i+1
        glyph = glyphs[i]
      until not glyph or glyph.cluster ~= cluster
      do
        local oldcluster = cluster
        cluster = glyph and glyph.cluster or offset + len
        if oldcluster then
          for _ = oldcluster+1, cluster do
            node = getnext(node)
          end
        end
      end      

      if not glyph then break end

      local nextcluster
      for j = i+1, #glyphs do
        nextcluster = glyphs[j].cluster
        if cluster ~= nextcluster then
          glyph.nglyphs = j - i
          goto NEXTCLUSTERFOUND -- break
        end
      end -- else -- only executed if the loop reached the end without
                  -- finding another cluster
        nextcluster = offset + len
        glyph.nglyphs = #glyphs + 1 - i
      ::NEXTCLUSTERFOUND:: -- end
      glyph.nextcluster = nextcluster      
      -- Calculate the Unicode code points of this glyph. If cluster did not
      -- change then this is a glyph inside a complex cluster and will be
      -- handled with the start of its cluster.
      do
        local hex = ""
        local str = ""
        local node = node
        for j = cluster+1,nextcluster do
          local char, id = is_char(node, fontid)
          if char then
            -- assert(char == codes[j])
            hex = hex .. to_utf16_hex(char)
            str = str .. utf8.char(char)          
          end
          node = getnext(node)
        end
        glyph.tounicode = hex
        glyph.string = str
      end    
    end
    return head, firstnode, glyphs, run.len - len
  else
    if not fontdata.shaper_warning then
      local shaper = shapers[1]
      if shaper then
        tex.error("luaotfload | digitalkhatt : Shaper failed", {
          string.format("You asked me to use shaper %q to shape", shaper),
          string.format("the font %q", fontdata.name),
          "but the shaper failed. This probably means that either the shaper is not",
          "available or the font is not compatible.",
          "Maybe you should try the default shaper instead?"
        })
      else
        tex.error(string.format("luaotfload | digitalkhatt : All shapers failed for font %q.", fontdata.name))
      end
      fontdata.shaper_warning = true -- Only warn once for every font
    end
  end
end

local function color_to_rgba(color)
  local r = color.red   / 255
  local g = color.green / 255
  local b = color.blue  / 255
  local a = color.alpha / 255
  if a ~= 1 then
    -- XXX: alpha
    return format('%s %s %s rg', r, g, b)
  else
    return format('%s %s %s rg', r, g, b)
  end
end

-- Cache of color glyph PNG data for bookkeeping, only because I couldn't
-- figure how to make the engine load the image from the binary data directly.
local pngcache = {}
local pngcachefiles = {}
local function cachedpng(data)
  local hash = md5.sumhexa(data)
  local i = pngcache[hash]
  if not i then
    local path = ostmpname()
    pngcachefiles[#pngcachefiles + 1] = path
    open(path, "wb"):write(data):close()
    -- local file = open(path, "wb"):write():close()
    -- file:write(data)
    -- file:close()
    i = img.scan{filename = path}
    pngcache[hash] = i
  end
  return i
end

local function get_png_glyph(gid, fontid, characters, haspng)
  return gid
end

local push_cmd = { "push" }
local pop_cmd = { "pop" }
local nop_cmd = { "nop" }
--[[
  In the following, "text" actually refers to "font" mode and not to "text"
  mode. "font" mode is called "text" inside of virtual font commands (don't
  ask me why, but the LuaTeX source does make it clear that this is intentional)
  and behaves mostly like "page" (especially it does not enter a "BT" "ET"
  block) except that it always resets the current position to the origin.
  This is necessary to ensure that the q/Q pair does not interfere with TeX's
  position tracking.
  ]]
local save_cmd = { "pdf", "text", "q" }
local restore_cmd = { "pdf", "text", "Q" }

local function round(num, numDecimalPlaces)
  local mult = 10^(numDecimalPlaces or 0)
  return math.floor(num * mult + 0.5) / mult
end

-- Convert glyphs to nodes and collect font characters.
local function tonodes(head, node, run, glyphs)
  local nodeindex = run.start
  local dir = run.dir
  local fontid = run.font
  local fontdata = font.getfont(fontid)
  local space = fontdata.parameters.space
  local characters = fontdata.characters
  local hbdata = fontdata.hb
  local hfactor = (fontdata.extend or 1000) / 1000
  local palette = hbdata.palette
  local hbshared = hbdata.shared
  local hbface = hbshared.face
  local nominals = hbshared.nominals
  local hbfont = hbshared.font
  local fontglyphs = hbshared.glyphs

  local gid_offset = hbshared.gid_offset
  local rtl = dir:is_backward()
  local lastprops

  local scale = hbdata.scale

  local haspng = hbshared.haspng
  local fonttype = hbshared.fonttype

  local nextcluster
  
  local tajweedatt = luatexbase.attributes["tajweedatt"]

  for i, glyph in ipairs(glyphs) do
    if glyph.cluster + 1 >= nodeindex then -- Reached a new cluster
      nextcluster = glyph.nextcluster
      assert(nextcluster)
      for j = nodeindex, glyph.cluster do
        local oldnode = node
        head, node = removenode(head, node)
        freenode(oldnode)
      end
      
      setprop(node,startnode_p,{run.cacheIndex,nodeindex})
      setprop(node,endnode_p,{run.cacheIndex,nextcluster})
     
      
      lastprops = getproperty(node)
      nodeindex = glyph.cluster + 1
    elseif nextcluster + 1 == nodeindex then -- Oops, we went too far
      nodeindex = nodeindex - 1
      local new = inherit(glyph_t, getprev(node), lastprops)
      setfont(new, fontid)
      head, node = insertbefore(head, node, new)
      setprop(node,startnode_p,{run.cacheIndex,nodeindex})
      setprop(node,endnode_p,{run.cacheIndex,nodeindex})
    else
      setprop(node,startnode_p,{run.cacheIndex,nodeindex})
      setprop(node,endnode_p,{run.cacheIndex,nodeindex})
    end
    local gid = glyph.codepoint
    local char = nominals[gid] or gid_offset + gid
    local orig_char, id = is_char(node, fontid)
    
    local glyphcolor = glyph.color
    
    if glyphcolor and glyphcolor > 1000 then
     direct.set_attribute(node,tajweedatt,glyphcolor)     
    --  print("glyphcolor=" .. glyphcolor)
    end    
    if lastprops and lastprops.zwnj and nodeindex == glyph.cluster + 1 then
    elseif orig_char then
      local done
      local fontglyph = fontglyphs[gid]
      local character = characters[char]
      if fontglyph.layers then
        local test = 5
      end

      --if not character.commands then
      if not character.addedtocolor then
        if palette then
          local layers = fontglyph.layers
          if layers == nil then
            layers = hbface:ot_color_glyph_get_layers(gid) or false
            fontglyph.layers = layers
          end
          if layers then
            local cmds = {} -- Every layer will add 5 cmds
            local prev_color = nil
            local k = 1 -- k == j except that k does only get increased if the layer isn't dropped
            for j = 1, #layers do
              local layer = layers[j]
              local layerchar = characters[gid_offset + layer.glyph]
              if layerchar.height > character.height then
                character.height = layerchar.height
              end
              if layerchar.depth > character.depth then
                character.depth = layerchar.depth
              end
              -- color_index has a special value, 0x10000, that mean use text
              -- color, we don't check for it here explicitly since we will
              -- get nil anyway.
              local color = palette[layer.color_index]
              if not color or color.alpha ~= 0 then
                cmds[5*k - 4] = (color and not prev_color) and save_cmd or nop_cmd
                cmds[5*k - 3] = prev_color == color and nop_cmd or (color and {"pdf", "page", color_to_rgba(color)} or restore_cmd)
                cmds[5*k - 2] = push_cmd
                cmds[5*k - 1] = {"char", layer.glyph + gid_offset}
                cmds[5*k] = pop_cmd
                fontglyphs[layer.glyph].used = true
                prev_color = color
                k = k+1
              end
            end
            cmds[#cmds + 1] = prev_color and restore_cmd
            if not character.colored then
              local coloredcharacter = {}
              for k,v in next, character do
                coloredcharacter[k] = v
              end
              coloredcharacter.commands = cmds
              coloredcharacter.addedtocolor = true
              local newcharacters = {[gid + 0x130000] = coloredcharacter}                
              characters[gid + 0x130000] = coloredcharacter
              if char ~= gid + gid_offset then
                newcharacters[char] = coloredcharacter
                characters[char] = coloredcharacter
                character.colored = char                  
              else
                character.colored = gid + 0x130000
              end
              
              --digitalkhatt.converttotype3char(fontdata,gid + 0x130000,cmds)
              
              font.addcharacters(fontid, {characters = newcharacters})                
            end
            char = character.colored
            character = characters[char]
          end
        end

        if haspng then
          local pngglyph = character.pngglyph
          if pngglyph == nil then
            local pngblob = hbfont:ot_color_glyph_get_png(gid)
            if pngblob then
              local glyphimg = cachedpng(pngblob:get_data())
              local pngchar = { }
              for k,v in next, character do
                pngchar[k] = v
              end
              local i = img.copy(glyphimg)
              i.width = character.width
              i.depth = 0
              i.height = character.height + character.depth
              pngchar.commands = fonttype and {
                {"push"}, {"char", gid_offset + gid}, {"pop"},
                {"down", character.depth}, {"image", i}
              } or { {"down", character.depth}, {"image", i} }
              if not nominals[gid] then
                char = 0x130000 + gid
              end
              pngchar.addedtocolor = true
              characters[char] = pngchar
              pngglyph = char
              font.addcharacters(fontid, {characters = {[char] = pngchar}})
            end
            character.pngglyph = pngglyph
          end
          if pngglyph then
            char = pngglyph
          elseif not fonttype then
            -- Color bitmap font with no glyph outlines (like Noto
            -- Color Emoji) but has no bitmap for current glyph (most likely
            -- `.notdef` glyph). The engine does not know how to embed such
            -- fonts, so we don't want them to reach the backend as it will cause
            -- a fatal error. We use `nullfont` instead.  That is a hack, but I
            -- think it is good enough for now. We could make the glyph virtual
            -- with empty commands suh that LuaTeX ignores it, but we still want
            -- a missing glyph warning.
            -- We insert the glyph node and move on, no further work is needed.
            setfont(node, 0)
            done = true
          end
        end
      end
      if not done then
        local oldcharacter = characters[orig_char]
        -- If the glyph index of current font character is the same as shaped
        -- glyph, keep the node char unchanged. Helps with primitives that
        -- take characters as input but actually work on glyphs, like
        -- `\rpcode`.
        if not oldcharacter then
          if gid == 0 then
            local new = copynode(node)
            head, node = insertafter(head, node, new)
          end
          setchar(node, char)
        elseif character.commands or character.index ~= oldcharacter.index then
          if glyph.lefttatweel ~= 0 or glyph.righttatweel ~= 0 then
            local rounddecimal = 10
            local ltat,rtat
            -- TODO DigitalKhatt get max value from fvar
            if glyph.lefttatweel <0 then                
              ltat = round((glyph.lefttatweel - (2/16384.0) )* 20 ,rounddecimal)
            else
              ltat = round((glyph.lefttatweel  + (2/16384.0)) * 20,rounddecimal)
            end
            if glyph.righttatweel <0 then
              rtat = round((glyph.righttatweel  - (2/16384.0)) * 20,rounddecimal)
            else
              rtat = round((glyph.righttatweel + (2/16384.0)) * 20,rounddecimal)
            end              
            
            local instance = "ltat=" .. ltat .. ",rtat=".. rtat
            
            local oldchar = char
            char = digitalkhatt.generatechar(fontdata,instance,oldchar)    
            if char then
              local newcharacters = {[char] = characters[char]} 
              
              font.addcharacters(fontid, {characters = newcharacters, fonts = fontdata.fonts})
              
            end                            
          end
          setchar(node, char)
        end
        local xoffset = (rtl and -glyph.x_offset or glyph.x_offset) * scale
        local yoffset = glyph.y_offset * scale
        setoffsets(node, xoffset, yoffset)

        fontglyph.used = fonttype and true

        -- The engine will use this string when printing a glyph node e.g. in
        -- overfull messages, otherwise it will be trying to print our
        -- invalid pseudo Unicode code points.
        -- If the string is empty it means this glyph is part of a larger
        -- cluster and we don't to print anything for it as the first glyph
        -- in the cluster will have the string of the whole cluster.
        local props = properties[node]
        if not props then
          props = {}
          properties[node] = props
        end
        props.glyph_info = glyph.string or ""

        -- Handle PDF text extraction:
        -- * Find how many characters in this cluster and how many glyphs,
        -- * If there is more than 0 characters
        --   * One glyph: one to one or one to many mapping, can be
        --     represented by font's /ToUnicode
        --   * More than one: many to one or many to many mapping, can be
        --     represented by /ActualText spans.
        -- * If there are zero characters, then this glyph is part of complex
        --   cluster that will be covered by an /ActualText span.
        local tounicode = glyph.tounicode
        if tounicode then
          if glyph.nglyphs == 1
              and not character.commands
              and not fontglyph.tounicode then
            fontglyph.tounicode = tounicode
          elseif character.commands or tounicode ~= fontglyph.tounicode then
            --setprop(node, startactual_p, tounicode)
            glyphs[i + glyph.nglyphs - 1].endactual = true
          end
        end
        if glyph.endactual then
          --setprop(node, endactual_p, true)
        end
        local x_advance = glyph.x_advance
        local width = fontglyph.width * hfactor
        if width ~= x_advance then
          -- The engine always uses the glyph width from the font, so we need
          -- to insert a kern node if the x advance is different.
          local kern = newkern((x_advance - width) * scale, node)
          head, node = insertkern(head, node, kern, rtl)
        end
      end
    elseif id == glue_t and getsubtype(node) == spaceskip_t then
      -- If the glyph advance is different from the font space, then a
      -- substitution or positioning was applied to the space glyph changing
      -- it from the default. We try to maintain as much as possible from the
      -- original value because we assume that we want to keep spacefactors and
      -- assume that we got mostly positioning applied. TODO: Handle the case that
      -- we became a glyph in the process.
      -- We are intentionally not comparing with the existing glue width as
      -- spacing after the period is larger by default in TeX.
      local width = glyph.x_advance * scale
      -- if space > width + 2 or width > space + 2 then
      if space ~= width then
        setwidth(node, getwidth(node) - space + width)
        if width == 0 then
          setfield(node, "stretch", 0)
          setfield(node, "shrink", 0)
        else
          --setfield(node, "stretch", width / 2)
          --setfield(node, "shrink", width / 3)
        end
        
      end
    elseif id == kern_t and getsubtype(node) == italiccorr_t then
      -- If this is an italic correction node and the previous node is a
      -- glyph, update its kern value with the glyph's italic correction.
      local prevchar, prevfontid = ischar(getprev(node))
      if prevfontid == fontid and prevchar and prevchar > 0 then
        local italic = characters[prevchar].italic
        if italic then
          setkern(node, italic)
        end
      end
    end
    node = getnext(node)
    nodeindex = nodeindex + 1
    
  end
  while node ~= run.after do
    local oldnode = node
    head, node = removenode(head, node)    
    freenode(oldnode)    
  end

  return head, node
end

local cache = {}

local nameid = 1

local function shape_run(head, current, run)
  if not run.skip then
    -- Font loaded with our loader and an HarfBuzz face is present, do our
    -- shaping.
    local glyphs, offset   

    head, current, glyphs, offset = shape(head, current, run)
    if glyphs then
      local newhead,newcurrent = tonodes(head, current, run, glyphs)            
      -- change font size
      local minExpFactor = nil
      local maxExpFactor = nil
      if run.linewidth and run.digitalKhattOptions and  run.digitalKhattOptions.fontExpansion then
        minExpFactor = run.digitalKhattOptions.minExpFactor
        maxExpFactor = run.digitalKhattOptions.maxExpFactor
      end
      if minExpFactor and  maxExpFactor then
        local fontid = run.font
        local fontdata = font.getfont(fontid)
        local space = fontdata.parameters.space
        local characters = fontdata.characters
        local hbdata = fontdata.hb        
        local currentlineWidth = 0
        for i = 1, #glyphs do
            local glyph = glyphs[i]
            currentlineWidth = currentlineWidth + glyph.x_advance
        end
        currentlineWidth = currentlineWidth * hbdata.scale
        local diff = currentlineWidth - run.linewidth
        -- at least 1 point of underfull or overfull
        -- Add as parameter
        expand = math.abs(diff) > 1 * 65536
       
        if expand then           

          local ratio = run.linewidth / currentlineWidth 
          if ratio > maxExpFactor then
            ratio = maxExpFactor
          end

          if ratio < minExpFactor then
            ratio = minExpFactor
          end

          
          if ratio ~= 1 and ratio > 0 then
            for cnode, id, subtype in traverse(current) do

              if cnode == newcurrent then
                break
              end
      
              local typeid = node.types()[id]            
             
              local props = properties[cnode]
  
              if id == glyph_t then
                local char = is_char(cnode, fontid)
                if char then
                  local newfontid, newcharid = digitalkhatt.getnewfontid(fontid,char,ratio)
                  if newfontid and newcharid then
                    direct.setfont(cnode,newfontid)
                    direct.setchar(cnode,newcharid)
                    local xoffset, yoffset = getoffsets(cnode)
                    setoffsets(cnode, xoffset * ratio, yoffset * ratio)
                  end                
                end
              elseif id == glue_t and subtype == spaceskip_t then
                setwidth(cnode, getwidth(cnode) * ratio)          
                setfield(cnode, "stretch", getfield(cnode, "stretch") * ratio)
                setfield(cnode, "shrink", getfield(cnode, "shrink") * ratio)
              elseif id == kern_t then
                local kernValue = getkern(cnode)
                setkern(cnode, kernValue * ratio)  
              end
            end
          end
        end               
      end       
      return offset, newhead, newcurrent              
      -- return offset, tonodes(head, current, run, glyphs)
    end
  end
  return 0, head, run.after
end

local currentfontid
local currentdirection



function process(head, fontid, _attr, direction,nofused, groupcode,_,_)  
  
  local newhead, runs = itemize(head, fontid, direction)
  local cacheRuns = cache.runs or {}
  cache.runs = cacheRuns
  local lastCacheIndex = #cacheRuns;
  local current = newhead
  currentfontid = fontid
  currentdirection = direction
  local offset = 0
  local needJustification = token.create("ifdigitalkhatt@justify").mode == iftrue.mode
  local digitalKhattOptions = {
    needJustification = needJustification,
    tajweedColor = token.create("ifdigitalkhatt@tajweedColor").mode == iftrue.mode
  }    
  if needJustification then
    digitalKhattOptions.fontExpansion = token.create("ifdigitalkhatt@fontExpansion").mode == iftrue.mode    
    digitalKhattOptions.minExpFactor = tonumber(token.get_macro("digitalkhatt@minExpFactor")) or 1
    digitalKhattOptions.maxExpFactor = tonumber(token.get_macro("digitalkhatt@maxExpFactor")) or 1
  end
  
  for i = 1,#runs do
    local run = runs[i]
    run.digitalKhattOptions = digitalKhattOptions
    if needJustification then
      table.insert(cacheRuns,run)
      run.cacheIndex = lastCacheIndex + i
      run.copylist = copynodelist(current)       
    end
    
    run.start = run.start - offset    
    local new_offset
    new_offset, newhead, current = shape_run(newhead, current, run)
    offset = offset + new_offset
  end  

  return newhead or head
end

local function replacechain(head,from,to,newlist)
  local prev = getprev(from)
  if prev then
    setnext(prev,newlist)
  else
    head = newlist
  end
  local tail = gettail(newlist)
  setnext(tail,getnext(to))
  return head
end

local function justifyline(list)
  
  -- TODO just for test : suppose one run by line 
  -- TODO we have to concatenate multiple runs within same line
  -- TODO Implement cluster many to many : validate startnode_p and endnode_p in tonodes()
  -- TODO use  dimensions(glue_set,glue_sign,glue_order,list) to get the width of the run if it does not span whole line
  -- check badness as context\tex\texmf-context\tex\context\base\mkiv\font-sol.lua?  
  -- use rehpack context\tex\texmf-context\tex\context\base\mkiv\node-aux.lua in the calling function otherwise nest list for tajweed and sajda  
  -- Copy last harfbuzz (just algo)
  -- check scripts/luatex.txt for script
  -- test if glue_order != 0 (infinity) dont justify
  -- flush copylist in cache.runs in harf-plug
  -- verify out of memory (harfbuzz just proc  ? ) : Come from digitalkhatt.generatechar  a lot of new generated characters -> round left and right tatweel
  -- verify round axis now 2 decimals ? used to avoid out of memory -> profile memory usage in digitalkhatt.generatechar
  -- manage multiple runs in the same list
  
  
  local head = getlist(list)
  local linewidth = getwidth(list)  
  local gluesetratio, listorder, sign = getglue(list)  
  
  
  if listorder ~= 0  or gluesetratio == 0 then
    return head
  end

  local natwidth = dimensions(head)
  
  local currentrun
  local start
  local endl
  local oristart
  local oriend
  local runsfound = {}
  local cuurrentfound
  local parLine = false
  for n, id, subtype in traverse(head) do
    local props = properties[n]
    local startpro = props and props[startnode_p]
    -- justification disabled
    if startpro and startpro[1] == nil  then
      return head
    end
    local endprop = props and props[endnode_p]
    if startpro and cuurrentfound == nil then
      local nodeId = getid(n)
      local subType = getsubtype(n)
      if not (nodeId == glue_t and subType == rightskip_t) then
        cuurrentfound = {
          start = startpro,
          endl = endprop,
          oristart = n,
          oriend = n,
        }      
        runsfound[#runsfound + 1] = cuurrentfound
      else
        parLine = true
      end      
    elseif cuurrentfound and endprop and endprop[1] == cuurrentfound.start[1] then
      cuurrentfound.endl = endprop
      cuurrentfound.oriend = n
    else
      cuurrentfound = nil          
    end    
  end
  if #runsfound ~= 1 then
      local stop = 1
  end
  if #runsfound == 1 then

    local start = runsfound[1].start
    local endl = runsfound[1].endl
    local oristart = runsfound[1].oristart
    local oriend = runsfound[1].oriend
    
    local prevprev = getprev(oristart)
    
    --[[
    local prevprev = getprev(oristart)
    local endend = getnext(oriend)
    if prevprev ~= nil then
      texio.write_nl("prevprev:") 
      digitalkhatt.printnode(prevprev,"",false)
    end
    if endend ~= nil then
      texio.write_nl("endend:") 
      digitalkhatt.printnode(endend,"",false)
    end
    --TODO supports multiple runs in the same hbox
    --if getprev(oristart) ~= nil or  getnext(oriend) ~= nil then
    --  return head
    --end
    --]]

    local natwidthport = dimensions(oristart,getnext(oriend))

    local portionwidth = linewidth - (natwidth - natwidthport)
    
    local oldrun = cache.runs[start[1]] 

    if not oldrun then
      print("DigitalKhatt problem!")
      return
    end
    
    local newrun = {
      start = start[2],
      len = endl[2] - start[2] + 1,
      font = oldrun.font,
      dir = oldrun.dir,
      skip = false,
      codes = oldrun.codes,      
      linewidth = portionwidth,
      digitalKhattOptions = oldrun.digitalKhattOptions
    }
    
    local current, new_offset,lastnode
    
    local locallist = oldrun.copylist --copynodelist(oldrun.copylist)
    
    for l = oldrun.start,endl[2] do
      if l == newrun.start then
        current = locallist      
      end
      if l == endl[2] then
        lastnode = locallist
      end  
      locallist = getnext(locallist)          
    end
    
    current = copynodelist(current,locallist)   
    
    
    new_offset, current, current = shape_run(current, current, newrun) 
    
    head = replacechain(head,oristart,oriend,current)
    
    local newlist = hpack(head,linewidth,'exactly',newrun.dir)  
    
    
    --flush_list(head)
    local set, order, sign = getglue(newlist)
    setglue(list,set,order,sign)
    setlist(list,getlist(newlist))    
    setlist(newlist)
    flush_node(newlist) 
    
    
  end
end



local function pageliteral(data)
  local n = newnode(whatsit_t, pdfliteral_t)
  setwhatsitfield(n, "mode", 1) -- page
  setwhatsitfield(n, "data", data) -- page
  return n
end

local function post_process(head)
  for n in traverse(head) do
    local props = properties[n]

    local startactual, endactual
    if props then
      startactual = rawget(props, startactual_p)
      endactual = rawget(props, endactual_p)
    end

    if startactual then
      local actualtext = "/Span<</ActualText<FEFF"..startactual..">>>BDC"
      head = insertbefore(head, n, pageliteral(actualtext))
      props[startactual_p] = nil
    end

    if endactual then
      head = insertafter(head, n, pageliteral("EMC"))
      props[endactual_p] = nil
    end

    local replace = getfield(n, "replace")
    if replace then
      setfield(n, "replace", post_process(replace))
    end
  end
  return head
end

local function post_process_vlist(head)
  for n, id, subtype, list in traverse_list(head) do
    if id == hlist_t and subtype == line_t then
      setlist(n, post_process(list))
    end
  end
  return true
end

local function post_process_nodes(head)
  return tonode(post_process(todirect(head)))
end

local function justify_lines(head)
  for n, id, subtype, lhead in traverse_list(head) do
    if id == hlist_t and (subtype == line_t or subtype == box_t) then
      --local lhead = getlist(n)
      justify_lines(lhead)               
      justifyline(n)
    end
  end  
end

local function post_linebreak_filter_callback(head)
  if cache.runs then     
    local nc = #cache.runs
    if nc ~= 0 then
        justify_lines(todirect(head))
    end   
  end
  return head  
end

local function vpack_filter_callback(head)
  --TODO add flag to not repeat justification already done in post_linebreak_filter_callback
  if cache.runs then     
    local nc = #cache.runs
    if nc ~= 0 then
        justify_lines(todirect(head))
    end
    local runs = cache.runs
    for i=1,nc do
      local run = runs[i]
      flush_list(run.copylist)      
    end
    cache = { }
  end
  return head  
end




local function run_cleanup()
  -- Remove temporary PNG files that we created, if any.
  -- FIXME: It would be nice if we wouldn't need this
  for _, path in next, pngcachefiles do
    osremove(path)
  end
end

local function set_tounicode()
  for fontid, fontdata in font.each() do
    local hbdata = fontdata.hb
    if hbdata and fontid == pdf.getfontname(fontid) then
      local characters = fontdata.characters
      local newcharacters = {}
      local hbshared = hbdata.shared
      local glyphs = hbshared.glyphs
      local nominals = hbshared.nominals
      local gid_offset = hbshared.gid_offset
      for gid = 0, #glyphs do
        local glyph = glyphs[gid]
        if glyph.used then
          local character = characters[gid + gid_offset]
          newcharacters[gid + gid_offset] = character
          local unicode = nominals[gid]
          if unicode then
            newcharacters[unicode] = character
          end
          character.tounicode = glyph.tounicode or "FFFD"
          character.used = true
        end
      end
      font.addcharacters(fontid, { characters = newcharacters })
    end
  end
end

-- FIXME: Move this into generic parts of luaotfload
local utfchar = utf8.char
local function get_glyph_info(n)
  n = todirect(n)
  local props = properties[n]
  local info = props and props.glyph_info
  if info then return info end
  local c = getchar(n)
  if c == 0 then
    return '^^@'
  elseif c < 0x110000 then
    return utfchar(c)
  else
    return string.format("^^^^^^%06X", c)
  end
end

fonts.handlers.otf.registerplugin('digitalkhatt', process)

local add_to_callback = luatexbase.add_to_callback
-- for paragraph
add_to_callback('post_linebreak_filter', post_linebreak_filter_callback, 'luaotfload.digitalkhatt.post_linebreak_filter')
-- for hbox
add_to_callback('vpack_filter', vpack_filter_callback, 'luaotfload.digitalkhatt.vpack_filter_callback')
add_to_callback('wrapup_run', run_cleanup, 'luaotfload.cleanup_files')
add_to_callback('finish_pdffile', set_tounicode, 'luaotfload.digitalkhatt.finalize_unicode')
--add_to_callback('glyph_info', get_glyph_info, 'luaotfload.glyphinfo')
--luatexbase.remove_from_callback("glyph_info",'luaotfload.glyphinfo')
--add_to_callback('glyph_info', get_glyph_info, 'luaotfload.digitalkhatt.glyphinfo')
