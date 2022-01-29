-----------------------------------------------------------------------
--         FILE:  luaotfload-harf-plug.lua
--  DESCRIPTION:  part of luaotfload / HarfBuzz / fontloader plugin
-----------------------------------------------------------------------
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

local hb                = luaotfload.harfbuzz
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

local line_t            = 1
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

local function itemize(head, fontid, direction,linewidth)
  local fontdata = font.getfont(fontid)
  local hbdata   = fontdata and fontdata.hb
  local spec     = fontdata and fontdata.specification
  local options  = spec and spec.features.raw

  local runs, codes = {}, {}
  local dirstack = {}
  local currdir = direction or 0
  local lastskip, lastdir = true
  local lastrun = {}
  local lastdisc
  local in_disc

  for n, id, subtype in traverse(head) do
    
    local typeid = node.types()[id]
    
    if in_disc == n then in_disc = nil end
    local disc
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
      if uses_font(n, fontid) then
        local _, _, rep, _, _, rep_tail = getdisc(n, true)
        setfield(n, 'replace', nil)
        local prev, next = getboth(n)
        in_disc = next
        disc = {
          disc = n,
          anchor_cluster = #codes - (lastrun.start or 0) + 1,
          after_cluster = #codes - (lastrun.start or 0) + 1 + length(rep),
        }
        if rep then
          setlink(rep_tail, next)
          setnext(n, rep) -- This one is just to keep the loop going
          n = rep
        else
          n = next
        end
        setlink(prev, n)
        code = nil
        skip = false
        if not prev then
          head = n
        end
      else
        skip = true
      end
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

    if (disc or not in_disc) and (lastdir ~= currdir or lastskip ~= skip) then
      if disc then
        disc.after_cluster = disc.after_cluster - disc.anchor_cluster
        disc.anchor_cluster = 0
      end
      lastrun.after = n
      lastrun = {
        start = ncodes + 1,
        len = code and 1 or 0,
        font = fontid,
        dir = currdir == 1 and dir_rtl or dir_ltr,
        skip = skip,
        codes = codes,
        discs = disc,
        linewidth = linewidth
      }
      runs[#runs + 1] = lastrun
      lastdir, lastskip, lastdisc = currdir, skip, disc
    elseif code then
      lastrun.len = lastrun.len + 1
    elseif disc then
      if lastdisc then
        lastdisc.next = disc
        lastdisc = disc
      else
        lastrun.discs, lastdisc = disc, disc
      end
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

-- Make s a sub run, used by discretionary nodes.
local function makesub(run, codes, nodelist)
  local subrun = {
    start = 1,
    len = #codes,
    font = run.font,
    dir = run.dir,
    fordisc = true,
    codes = codes,
  }
  local glyphs
  if nodelist == 0 then -- FIXME: This shouldn't happen
    nodelist = nil
  end
  nodelist, nodelist, glyphs = shape(nodelist, nodelist, subrun)
  assert(glyphs, [[Shaping discretionary list failed. This shouldn't happen.]])
  return { glyphs = glyphs, run = subrun, head = nodelist }
end

local function printnodes(label, head, after)
  after = tonode(after)
  for n in node.traverse(tonode(head)) do
    if n == after then return end
    print(label, n, n.font or '', n.char or '')
    local pre, post, rep = getdisc(todirect(n))
    if pre then
      printnodes(label .. '<', pre)
    end
    if post then
      printnodes(label .. '>', post)
    end
    if rep then
      printnodes(label .. '=', rep)
    end
  end
end
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
  local discs = (not run.fordisc) and run.discs

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
      n = getnext(n)
    end
    for _, feat in next, current_features do
      features[#features + 1] = feat
    end
  end
  
  if run.linewidth and digitalkhatt.justify then
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

    local break_glyph, break_cluster, break_node = 1, offset, node
    local disc_glyph, disc_cluster, disc_node
    local disc_cluster
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

        -- Is this a safe breakpoint?
      if discs and ((not glyph) or codes[cluster] == 0x20 or codes[cluster+1] == 0x20 or codes[cluster+1] == 0xFFFC
          or not unsafetobreak(glyph)) then
        -- Should we change the discretionary state?
        local anchor_cluster, after_cluster = offset + discs.anchor_cluster, offset + discs.after_cluster
        local saved_anchor, saved_after = (discs.next and discs.next.anchor_cluster or math.huge) + offset
        while disc_cluster and after_cluster <= cluster
           or not disc_cluster and anchor_cluster <= cluster do
          if disc_cluster then
            if not saved_after and saved_anchor < cluster then
              saved_after = discs.next.after_cluster + offset
              if saved_after > cluster then
                saved_after, after_cluster = after_cluster, saved_after
                break
              end
            elseif saved_after then
              saved_after, after_cluster = after_cluster, saved_after
            end
            local rep_glyphs = table.move(glyphs, disc_glyph, i - 1, 1, {})
            for j = 1, #rep_glyphs do
              local glyph = rep_glyphs[j]
              glyph.cluster = glyph.cluster - disc_cluster
              glyph.nextcluster = glyph.nextcluster - disc_cluster
            end
            do
              local cluster_offset = disc_cluster - cluster + (saved_after and 2 or 1) -- The offset the glyph indices will move
              for j = i, #glyphs do
                local glyph = glyphs[j]
                glyph.cluster = glyph.cluster + cluster_offset
              end
              len = len + cluster_offset
              table.move(glyphs, i, #glyphs + i - disc_glyph, disc_glyph + (saved_after and 2 or 1))

              local discs = discs.next
              while discs do
                discs.anchor_cluster = discs.anchor_cluster + cluster_offset
                discs.after_cluster = discs.after_cluster + cluster_offset
                discs = discs.next
              end
            end

            local pre, post, _, _, lastpost, _ = getdisc(discs.disc, true)
            local precodes, postcodes, repcodes = {}, {}
            table.move(codes, disc_cluster + 1, anchor_cluster, 1, precodes)
            for n in traverse(pre) do
              precodes[#precodes + 1] = is_char(n, fontid) or 0xFFFC
            end
            for n in traverse(post) do
              postcodes[#postcodes + 1] = is_char(n, fontid) or 0xFFFC
            end
            table.move(codes, after_cluster + 1, cluster, #postcodes + 1, postcodes)

            if saved_after then
              repcodes = table.move(codes, disc_cluster + 1, cluster, 1, {})
              table.move(codes, cluster + 1, #codes + cluster - disc_cluster, disc_cluster + 3)
              codes[disc_cluster + 1], codes[disc_cluster + 2] = 0xFFFC, 0xFFFC
            else
              table.move(codes, cluster + 1, #codes + cluster - disc_cluster, disc_cluster + 2)
              codes[disc_cluster + 1] = 0xFFFC
            end

            do
              local iter = disc_node
              for _ = disc_cluster, anchor_cluster-1 do iter = getnext(iter) end
              if iter ~= disc_node then
                local newpre = copynodelist(disc_node, iter)
                setlink(tail(newpre), pre)
                pre = newpre
              end
              for _ = anchor_cluster, after_cluster-1 do iter = getnext(iter) end
              if post then
                setlink(lastpost, copynodelist(iter, node))
              else
                post = copynodelist(iter, node)
              end
            end
            local prev = getprev(disc_node)
            if disc_cluster ~= cluster then
              setprev(disc_node, nil)
              setnext(getprev(node), nil)
            end
            setlink(prev, discs.disc, node)
            if disc_node == firstnode then
              firstnode = discs.disc
              if head == disc_node then
                head = firstnode
              end
            end
            glyphs[disc_glyph] = {
              replace = {
                glyphs = rep_glyphs,
                head = disc_node ~= node and disc_node or nil,
                run = {
                  start = 1,
                  font = run.font,
                  dir = run.dir,
                },
              },
              pre = makesub(run, precodes, pre),
              post = makesub(run, postcodes, post),
              cluster = disc_cluster,
              nextcluster = disc_cluster + 1,
              codepoint = 0xFFFC,
            }
            if saved_after then
              local next_disc = discs.next
              setlink(discs.disc, next_disc.disc, node)
              local next_pre, next_post, _, _, next_lastpost, _ = getdisc(next_disc.disc, true)
              local next_rep = copynodelist(next_pre)
              -- Let's play the game again. We need three parts:
              -- The pre and post branches in the outer post branch
              local next_precodes, next_postcodes, next_repcodes = {}, {}, {}
              do
                local saved_offset = length(post) - cluster
                local saved_anchor, saved_after = saved_anchor + saved_offset, saved_after + saved_offset
                table.move(postcodes, 1, saved_anchor, 1, next_precodes)
                for n in traverse(next_pre) do
                  next_precodes[#next_precodes + 1] = is_char(n, fontid) or 0xFFFC
                end
                for n in traverse(next_post) do
                  next_postcodes[#next_postcodes + 1] = is_char(n, fontid) or 0xFFFC
                end
                table.move(postcodes, saved_after + 1, #postcodes, #next_postcodes + 1, next_postcodes)

                local iter = post
                for _ = 1, saved_anchor do iter = getnext(iter) end
                if iter ~= post then
                  local newpre = copynodelist(post, iter)
                  setlink(tail(newpre), next_pre)
                  next_pre = newpre
                end
                for _ = saved_anchor, saved_after - 1 do iter = getnext(iter) end
                if next_post then
                  setlink(next_lastpost, copynodelist(iter))
                else
                  next_post = copynodelist(iter)
                end
              end
              -- The pre branch in the outer replace branch. The post branch is implicitly the same as the previous one
              do
                local saved_anchor = saved_anchor - disc_cluster
                table.move(repcodes, 1, saved_anchor, 1, next_repcodes)
                for n in traverse(next_rep) do
                  next_repcodes[#next_repcodes + 1] = is_char(n, fontid) or 0xFFFC
                end

                local rep = glyphs[disc_glyph].replace.head
                local iter = rep
                for _ = 1, saved_anchor do iter = getnext(iter) end
                if iter ~= rep then
                  local newpre = copynodelist(rep, iter)
                  setlink(tail(newpre), next_rep)
                  next_rep = newpre
                end
              end
              setsubtype(discs.disc, firstdisc_t)
              setsubtype(next_disc.disc, seconddisc_t)
              discs = discs.next
              disc_glyph = disc_glyph + 1
              disc_cluster = disc_cluster + 1
              glyphs[disc_glyph] = {
                replace = makesub(run, next_repcodes, next_rep),
                pre = makesub(run, next_precodes, next_pre),
                post = makesub(run, next_postcodes, next_post),
                cluster = disc_cluster,
                nextcluster = disc_cluster + 1,
                codepoint = 0xFFFC,
              }
            end
            i = disc_glyph + 1
            assert(node == getnext(discs.disc))
            cluster = disc_cluster + 1

            disc_cluster = nil
            discs = discs.next
            while discs and discs.anchor_cluster + offset < cluster do
              freenode(discs.disc)
              discs = discs.next
            end
            if not discs then break end
            anchor_cluster, after_cluster = offset + discs.anchor_cluster, offset + discs.after_cluster
            saved_anchor, saved_after = (discs.next and discs.next.anchor_cluster or math.huge) + offset
          elseif anchor_cluster == cluster then
            disc_glyph, disc_cluster, disc_node = i, cluster, node
          else
            disc_glyph, disc_cluster, disc_node = break_glyph, break_cluster, break_node
          end
        end
        break_glyph, break_cluster, break_node = i, cluster, node
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

      local disc, discindex
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
          elseif not discindex and id == disc_t then
            local props = properties[disc]
            if not (props and props.zwnj) then
              disc, discindex = node, j
            end
          end
          node = getnext(node)
        end
        glyph.tounicode = hex
        glyph.string = str
      end
      if not fordisc and discindex then
      end
    end
    return head, firstnode, glyphs, run.len - len
  else
    if not fontdata.shaper_warning then
      local shaper = shapers[1]
      if shaper then
        tex.error("luaotfload | harf : Shaper failed", {
          string.format("You asked me to use shaper %q to shape", shaper),
          string.format("the font %q", fontdata.name),
          "but the shaper failed. This probably means that either the shaper is not",
          "available or the font is not compatible.",
          "Maybe you should try the default shaper instead?"
        })
      else
        tex.error(string.format("luaotfload | harf : All shapers failed for font %q.", fontdata.name))
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
      
      setprop(node,startnode_p,{run.index,nodeindex})
      setprop(node,endnode_p,{run.index,glyph.cluster + 1})
     
      
      lastprops = getproperty(node)
      nodeindex = glyph.cluster + 1
    elseif nextcluster + 1 == nodeindex then -- Oops, we went too far
      nodeindex = nodeindex - 1
      local new = inherit(glyph_t, getprev(node), lastprops)
      setfont(new, fontid)
      head, node = insertbefore(head, node, new)
      setprop(node,startnode_p,{run.index,nodeindex})
      setprop(node,endnode_p,{run.index,nodeindex})
    else
      setprop(node,startnode_p,{run.index,nodeindex})
      setprop(node,endnode_p,{run.index,nodeindex})
    end
    local gid = glyph.codepoint
    local char = nominals[gid] or gid_offset + gid
    local orig_char, id = is_char(node, fontid)
    
    local glyphcolor = glyph.color
    
    if glyphcolor > 1000 then
     direct.set_attribute(node,tajweedatt,glyphcolor)     
    --  print("glyphcolor=" .. glyphcolor)
    end

    if glyph.replace then
      -- For discretionary the glyph itself is skipped and a discretionary node
      -- is output in place of it.
      local rep, pre, post = glyph.replace, glyph.pre, glyph.post

      setdisc(node, tonodes(pre.head, pre.head, pre.run, pre.glyphs),
                    tonodes(post.head, post.head, post.run, post.glyphs),
                   (tonodes(rep.head, rep.head, rep.run, rep.glyphs)))
      node = getnext(node)
      nodeindex = nodeindex + 1
    else
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
              setprop(node, startactual_p, tounicode)
              glyphs[i + glyph.nglyphs - 1].endactual = true
            end
          end
          if glyph.endactual then
            setprop(node, endactual_p, true)
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
  end
  while node ~= run.after do
    local oldnode = node
    head, node = removenode(head, node)    
    freenode(oldnode)    
  end

  return head, node
end

local cache = {}

local function shape_run(head, current, run, justify)
  if not run.skip then
    -- Font loaded with our loader and an HarfBuzz face is present, do our
    -- shaping.
    local glyphs, offset
    if not run.discs and not justify then      
      run.copylist = copynodelist(current, run.after)      
    end

    head, current, glyphs, offset = shape(head, current, run)
    if glyphs then
      return offset, tonodes(head, current, run, glyphs)
    end
  end
  return 0, head, run.after
end

local currentfontid
local currentdirection



function process(head, fontid, _attr, direction,nofused, groupcode,linewidth,packtype)  
  -- TODO flush nodes
  cache = {}  
  local line
  if linewidth ~= 0 then      
      line = hpack(head,linewidth,packtype,direction)
      local glue_order = getfield(line, "glue_order") 
      local glue_set = getfield(line, "glue_set") 
      local glue_sign = getfield(line, "glue_sign") 
      --local widthdim = dimensions(glue_set,glue_sign,glue_order,getlist(line))
      local width = getwidth(line)
      
      if glue_order ~= 0 then
        linewidth = nil
      elseif packtype == 'exactly' and width > 0 then 
        -- TODO here for test : we suppose one run which take all the width
        linewidth = width
      else
        -- TODO
        -- if additional (i.e \hbox spread) we can justify based on natural width * spread (i.e  width value). 
        -- However this function is called during hpack_filter callback. So, after this callback, luatex will call hpack with new natural width 
        -- of the already justified line and spread again
        -- we have to call this function after the hpack but there is no callback (contact LuaTex for the reason. infinite recursivity?)
        -- do the pack ourselve and put the packed list in the outer list and adjust the width to the original natural width
        -- For now we do not justify
        linewidth = nil
      end
      --setlist(line,nil)
      --freenode(line)
  end
  
  local newhead, runs = itemize(head, fontid, direction,linewidth)
  cache.runs = runs
  local current = newhead
  currentfontid = fontid
  currentdirection = direction
  local offset = 0
  for i = 1,#runs do
    local run = runs[i]
    run.index = i
    run.start = run.start - offset
    local new_offset
    new_offset, newhead, current = shape_run(newhead, current, run)
    offset = offset + new_offset
  end  

  return newhead or head
end

local debugmemory = 0

local function justifyline(line)
  
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
  
 
  
  
  
  local head = getlist(line)
  
  
  local order = getfield(line, "glue_order") 
  
  if order ~= 0 then
    return head
  end
  
  local linewidth = getwidth(line)  
  
  
  local currentrun
  local start
  local endl
  for n, id, subtype in traverse(head) do
    local props = properties[n]
    local startpro = props and props[startnode_p]
    local endprop = props and props[endnode_p]
    if not start and startpro then
      start = startpro
      endl = endprop    
    elseif start and endprop and endprop[1] == start[1] then
      endl = endprop
    elseif start and endprop and endprop[1] ~= start[1] then
      break         
    end    
  end
  if start and endl then
    
    local oldrun = cache.runs[start[1]] 
    
    local newrun = {
      start = start[2],
      len = endl[2] - start[2] + 1,
      font = oldrun.font,
      dir = oldrun.dir,
      skip = false,
      codes = oldrun.codes,      
      linewidth = linewidth
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
    
    
    new_offset, current, current = shape_run(current, current, newrun, true)
    -- or use rehpack context\tex\texmf-context\tex\context\base\mkiv\node-aux.lua in the calling function
    flush_list(head)
    head = hpack(current,linewidth,'exactly',newrun.dir)
    
    
    
    if (debugmemory % 130)  == 0 then
      print("debugmemory = ".. debugmemory)
      print("memory=" .. collectgarbage("count"))
      --collectgarbage("collect") 
      --print("memory=" .. collectgarbage("count"))
      --print("status=".. status.node_mem_usage)
    end
    
    debugmemory = debugmemory + 1
    
    --collectgarbage("collect") 
    --print("memory=" .. collectgarbage("count"))
       
      
  end
  return head
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
  for n, id, subtype, list in traverse_list(head) do
    if id == hlist_t and subtype == line_t then      
      setlist(n, justifyline(n))      
    end
  end  
end

local function post_process_vlist_nodes(head)
  if digitalkhatt.justify and cache.runs then     
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
  return tonode(post_process_vlist(todirect(head)))
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

fonts.handlers.otf.registerplugin('harf', process)

local add_to_callback = luatexbase.add_to_callback

add_to_callback('post_linebreak_filter', post_process_vlist_nodes, 'luaotfload.harf.finalize_vlist')
add_to_callback('hpack_filter', post_process_nodes, 'luaotfload.harf.finalize_hlist')
add_to_callback('wrapup_run', run_cleanup, 'luaotfload.cleanup_files')
add_to_callback('finish_pdffile', set_tounicode, 'luaotfload.harf.finalize_unicode')
add_to_callback('glyph_info', get_glyph_info, 'luaotfload.glyphinfo')
