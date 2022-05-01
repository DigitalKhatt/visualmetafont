
local add_to_callback = luatexbase.add_to_callback 
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
local is_char           = direct.is_char
local tail              = direct.tail
local getboth           = direct.getboth
local setlink           = direct.setlink
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

local HLIST = node.id("hlist")
local RULE = node.id("rule")
local GLUE = node.id("glue")
local KERN = node.id("kern")

local SAJDAATTR = luatexbase.attributes["sajdaatt"] 
if not SAJDAATTR then
  SAJDAATTR = luatexbase.new_attribute("sajdaatt")
end 
local tajweedatt = luatexbase.attributes["tajweedatt"]
if not tajweedatt then
  tajweedatt = luatexbase.new_attribute("tajweedatt")
end

local stringformat            = string.format

local copynode                = node.copy
local insertnodeafter         = node.insert_after
local insertnodebefore        = node.insert_before
local newnode                 = node.new
local traversenodes           = node.traverse
local traversenodetype        = node.traverse_id

local nodecodes               = nodes.nodecodes

local glyph_t                 = nodecodes.glyph
local disc_t                  = nodecodes.disc
local hlist_t                 = nodecodes.hlist
local whatsit_t               = nodecodes.whatsit
local pdf_literal_t           = 16

local pdf_literal = newnode(whatsit_t, pdf_literal_t)

local justified_p     = "digitalkhatt_justified"


-- Set and get properties.
local function setprop(n, prop, value)
  local props = properties[n]
  if not props then
    props = {}
    properties[n] = props
  end
  props[prop] = value
end

local function getprop(n, prop)
  local props = properties[n]
  if props then
    return props[prop]
  end
end


local get_colortaj = function (color)  
  local push    = stringformat ("%.3g %.3g %.3g rg",
  ((color >> 24) & 0xff)/255,
  ((color >> 16) & 0xff)/255,
  ((color >> 8) & 0xff )/255)
  local pop     = "0 g"
  return push, pop
end


local function good_item(item)
  if item.id == GLUE and
    (item.subtype == node.subtype("leftskip")
     or item.subtype == node.subtype("rightskip")
     or item.subtype == node.subtype("parfillskip")) then
      return false
  else
    return true
  end
end

local function sajdaline(head, order, ratio, sign)
  
  if head.id == hlist_t then
    head = head.list
  end

  local item = head
  while item do

    if node.has_attribute(item, SAJDAATTR)  and good_item(item) then
        local item_line = node.new(RULE)        
        item_line.height = tex.sp("1.15em")
        --item_line.depth = tex.sp("-34.5pt")
        item_line.depth = -item_line.height + tex.sp("1pt")
        local end_node = item
        while end_node.next and
          good_item(end_node.next) and
          node.has_attribute(end_node.next, SAJDAATTR) do
            end_node = end_node.next
        end
        item_line.width = node.dimensions(ratio, sign, order, item, end_node.next)
        local item_kern = node.new(KERN, 1)
        item_kern.kern = -item_line.width
        node.insert_after(head, end_node, item_kern)
        node.insert_after(head, item_kern, item_line)
        item = item_line.next
    else
      item = item.next
    end
  end
end

function set_sajdabar(head)  
  for line in node.traverse_id(HLIST, head) do
    sajdaline(line.list, line.glue_order, line.glue_set, line.glue_sign)
  end 
  luatexbase.remove_from_callback("post_linebreak_filter","set_sajdabar")
  return head
end

local function tajweedcolorhpack(head)
  return tajweed(head)  
end

local function tajweedcolor(head)
  for line in node.traverse_id(HLIST, head) do
    tajweed(line.list)    
  end  
  return head
end

function tajweed(head)

  -- TODO verify list head change
  local headlist = head
  if head.id == hlist_t then
    headlist = headlist.list
  else
    head = nil
  end
  
  local push, pop = copynode(pdf_literal), copynode(pdf_literal)
  push.mode, push.data = 1, "0 g"
  pop.mode,  pop.data  = 1, "0 g"
  
  local item =  headlist
  while item do      
    local colatt = node.get_attribute(item, tajweedatt)      
    if  item.id == glyph_t and colatt then
      push.data = get_colortaj(colatt)
      local before, after = copynode(push), copynode(pop)
      headlist, before = insertnodebefore(headlist, item, before)
      headlist, after = insertnodeafter (headlist, item, after)
      item = after.next
    else
      item = item.next
    end      
  end

  return head or headlist
end




local function printnode(head, nested, notrecursive)
  if not nested then
    nested = ""
  end
  local subtypes = node.subtypes(getid(head)) 
  io.write(nested .. "node=" .. head .. ',')  
  io.write("type=".. node.type(getid(head)).. ',')  
  io.write("width = " .. (getwidth(head) or 'nil').. ',')
  io.write("glue_order = " .. (getfield(head, "glue_order") or 'nil').. ',')
  io.write("glue_set = " .. (getfield(head, "glue_set") or 'nil').. ',')
  io.write("glue_sign = " .. (getfield(head, "glue_sign") or 'nil').. ',')
  print("subtype = " .. (subtypes and subtypes[getfield(head, "subtype")] or 'nil'))
  if not notrecursive then
    local list = getfield(head, "list")
    if list then --and not getprop(head,justified_p) then
      nested = nested .. "."
      --for n in traverse(list) do
        printnode(list, nested)
      --end
      --setprop(head,justified_p ,true)  
    end
    local next = getnext(head)
    if next then
      printnode(next, nested)
    end
  end
end

local function post_process_print(head, groupcode, size, packtype, direction, attributelist)  
  print("groupcode=" .. (groupcode or "") .. ",size=" .. (size or "") .. ", packtype=" .. (packtype or "") .. ",direction=" .. (direction or ""))     
  for n in traverse(head) do
    printnode(n)
    if node.type(getid(head)) == "hlist" then
      local list = getfield(n,'head')
      if list then
        print("nested list")
        printnode(list)
      else
        print("list is nil")
      end
    end
    
  end
  return head
end

local function post_process(head,  width)  
    
  --local newhead = digitalkhatt.harfprocess(head,24,width,1,nil,true)
  
  return newhead or head
end

local function post_process_vlist(head,groupcode)
  for n, id, subtype, list in traverse_list(head) do
    if id == hlist_t and subtype == line_t then     
    local natwidth = dimensions(n)
    local width = getwidth(n)
  
    local glue_order = getfield(n, "glue_order")
    local glue_set = getfield(n, "glue_set")
    local glue_sign = getfield(n, "glue_sign")
  
    local natwidthlist = dimensions(glue_set,glue_sign,glue_order,list)
    local natwidthlist2 = dimensions(list)
    --printnode(n,"",true)
    --print(".natural width hlist=" .. (natwidth or 'nil') .. ",natural width list=" .. (natwidthlist or 'nil') .. ",natural width list without glue=" .. (natwidthlist2  or 'nil'))      
    
    setlist(n, post_process(list, width))
    end
  end
  return true
end

local function post_process_vlist_nodes(head,groupcode)  
  return tonode(post_process_vlist(todirect(head),groupcode))
end

local function append_to_vlist(head, locationcode, prevdepth)  
  print("")
  print("start append_to_vlist,locationcode=" .. (locationcode or 'nil') .. ",prevdepth=" .. (prevdepth or 'nil')  )
  printnode(head)  
  --for node, id, subtype, list in traverse_list(head) do
  --  if id == hlist_t then
  --    local natwidth = dimensions(node)
  --    print("natural width=" .. (natwidth or 'nil')) 
  --    printnode(list)  
  --  end
  --end  
end

local function append_to_vlist_filter(head, locationcode, prevdepth)  
  tonode(post_process_vlist(append_to_vlist(todirect(head),locationcode,prevdepth),groupcode))
  return head, prevdepth
end

local function post_process_hlist(head,groupcode, size, packtype, direction, attributelist)  
  return tonode(post_process(todirect(head), groupcode, size, packtype, direction, attributelist))
end

local texnest       = tex.nest

function contribute_filter(groupcode)
  if groupcode == "box" then -- "pre_box"
      local whatever = texnest[texnest.ptr]
      if whatever then
          local line = whatever.tail
          if line then
              line = todirect(line)
              printnode(line)
          end
      end
  end
end

function addsajdacallback()
  local cal, desc = luatexbase.remove_from_callback("post_linebreak_filter",'luaotfload.digitalkhatt.finalize_vlist')
  add_to_callback("post_linebreak_filter",set_sajdabar, "set_sajdabar")
  add_to_callback("post_linebreak_filter",cal, desc)
end

local cal, desc = luatexbase.remove_from_callback("post_linebreak_filter",'luaotfload.digitalkhatt.finalize_vlist')

add_to_callback("post_linebreak_filter",tajweedcolor, "quran.tajweedcolor")
add_to_callback("post_linebreak_filter",cal, desc)


add_to_callback('hpack_filter', tajweedcolorhpack, 'quran.tajweedcolorlist')
--add_to_callback('contribute_filter', contribute_filter, 'q-uran.contribute_filter')

add_to_callback('post_linebreak_filter', post_process_vlist_nodes, 'quran.finalize_vlist')
--add_to_callback('hpack_filter', post_process_hlist, 'quran.finalize_hlist')
--add_to_callback('append_to_vlist_filter', append_to_vlist_filter, 'quran.append_to_vlist_filter')