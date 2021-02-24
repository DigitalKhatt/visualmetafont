require('lpdf-ini')

local oldharf = fonts.readers.harf

fonts.readers.harf = function(spec)
    return  digitalkhatt.variableshapes(oldharf(spec))
end

digitalkhatt = digitalkhatt or { }
digitalkhatt.usetype3 = true
digitalkhatt.type3manualgen = false

local type3fontbyfontid = {}

local pdfdictionary       = lpdf.dictionary
local pdfarray            = lpdf.array
local pdfboolean          = lpdf.boolean
local pdfconstant         = lpdf.constant
local pdfreference        = lpdf.reference
local pdfunicode          = lpdf.unicode
local pdfstring           = lpdf.string
local pdfflushobject      = lpdf.flushobject
local pdfreserveobject    = lpdf.reserveobject
local pdfpagereference    = lpdf.pagereference
local pdfverbose          = lpdf.verbose

local formatters = string.formatters

local f_c = formatters["%.6N %.6N %.6N %.6N %.6N %.6N c"]
local f_l = formatters["%.6N %.6N l"]
local f_m = formatters["%.6N %.6N m"]

local concat = table.concat

local function segmentstopdf(segments,factor,bt,et)
    local t = { }
    local m = 0
    local n = #segments
    local d = false
    for i=1,n do
        local s = segments[i]
        local w = s[#s]
        if w == "c" then
            m = m + 1
            t[m] = f_c(s[1]*factor,s[2]*factor,s[3]*factor,s[4]*factor,s[5]*factor,s[6]*factor)
        elseif w == "l" then
            m = m + 1
            t[m] = f_l(s[1]*factor,s[2]*factor)
        elseif w == "m" then
            m = m + 1
            t[m] = f_m(s[1]*factor,s[2]*factor)
        elseif w == "q" then
            local p = segments[i-1]
            local n = #p
            local l_x = factor*p[n-2]
            local l_y = factor*p[n-1]
            local m_x = factor*s[1]
            local m_y = factor*s[2]
            local r_x = factor*s[3]
            local r_y = factor*s[4]
            m = m + 1
            t[m] = f_c (
                l_x + 2/3 * (m_x-l_x), l_y + 2/3 * (m_y-l_y),
                r_x + 2/3 * (m_x-r_x), r_y + 2/3 * (m_y-r_y),
                r_x, r_y
            )
        end
    end
    m = m + 1
    t[m] = "h f" -- B*
    if bt and et then
        t[0]   = bt
        t[m+1] = et
        return concat(t,"\n",0,m+1)
    else
        return concat(t,"\n")
    end
end

digitalkhatt.getNewKey = function(table,gid) 
  local lastkey = 1
  for key in pairs(table) do
    if key > lastkey then
      lastkey = key
    end
  end  
  if lastkey > 0x203000 then
    return lastkey + 1
  else
    return 0x203000 + 1
  end
end

testvariation = function()
    --[[
    local variations = {}
    local ltat = hb.Variation.new()
    ltat.tag = hb.Tag.new("LTAT")
    ltat.value = 5 --*2/3 --/ (7200/7227)
    
    local rtat = hb.Variation.new()
    rtat.tag = hb.Tag.new("RTAT")
    rtat.value = 5 --0.42 --(7200/7227)

    
    variations[#variations + 1] = ltat
    variations[#variations + 1] = rtat
    
    local tt = variations[1].value
    local kk = variations[1].tag
    
    local hhh = variations[2].value
    local nnn = variations[2].tag
    
    
    --local test = hbfont:set_variations(variations)
    
    local coords = hbfont:get_var_coords_normalized()
  --]]
end

digitalkhatt.generatechar = function(tfmdata,instance,char)  
  
    tfmdata.varchar = tfmdata.varchar or {}
    
    local varchar = tfmdata.varchar[char] or {}
    tfmdata.varchar[char] = varchar
    
    local hashedchar = varchar[instance]
    
    if hashedchar then
      return hashedchar
    end
  
    local spec = {
        properties = {
            filename = tfmdata.specification.filename,
            instance =  instance
        }
    }  
    local shapes = fonts.handlers.otf.loadoutlinedata(spec,nil,true)
    
    local characters = tfmdata.characters
    local parameters = tfmdata.parameters
    local hfactor    = parameters.hfactor * (7200/7227)
    local factor     = hfactor / 65536
    local getactualtext = fonts.handlers.otf.getactualtext
    local character = characters[char]
    if character then
        local shape = shapes.glyphs[character.gid]
        if shape then
            local segments = shape.segments
            if segments then
            -- we need inline in order to support color
                --local bt, et = getactualtext(character.tounicode or character.unicode or unicode)
                
                
                local lastkey = digitalkhatt.getNewKey(characters,character.gid)           
                characters[lastkey] = {
                  --index  = character.index,
                  width  = character.width,
                  height = character.height,
                  depth  = character.depth,
                  italic = character.italic,
                  --commands = commands
                }
                if digitalkhatt.usetype3 then
                  local code = segmentstopdf(segments,1,nil,nil)
                  digitalkhatt.converttotype3char(tfmdata,lastkey,code)
                else                  
                  characters[lastkey].commands = {
                      { "pdf", "origin", segmentstopdf(segments,factor,bt,et) }
                  }
                end
                varchar[instance] = lastkey
                return lastkey
                
            end
        end
    end
    
    return char
  
end

digitalkhatt.variableshapes = function(tfmdata,instance)  
    local spec = {
        properties = {
            filename = tfmdata.specification.filename,
            instance =  instance
        }
    }  
    local shapes = fonts.handlers.otf.loadoutlinedata(spec)    
    if not shapes or not shapes.glyphs then
        return tfmdata
    end
    local glyphs = shapes.glyphs  
    local characters = tfmdata.characters
    local parameters = tfmdata.parameters

    parameters.hfactor = tfmdata.size / tfmdata.units_per_em
    local hfactor    = parameters.hfactor * (7200/7227)
    local factor     = hfactor / 65536
    local getactualtext = fonts.handlers.otf.getactualtext
    for unicode, char in next, characters do
        if char.commands then
            -- can't happen as we're doing this before other messing around
        else
            if not char.gid then
                char.gid = char.index
            end
            char.index = nil
            local shape = glyphs[char.gid]
            if shape then
                local segments = shape.segments
                if segments then
                -- we need inline in order to support color
                  if digitalkhatt.usetype3 then
                    local code = segmentstopdf(segments,1,nil,nil) -- segmentstopdftype3(segments)
                    digitalkhatt.converttotype3char(tfmdata,unicode,code)
                  else
                    local bt, et = getactualtext(char.tounicode or char.unicode or unicode)
                    char.commands = {
                        { "pdf", "origin", segmentstopdf(segments,factor,bt,et) }
                    }
                  end
                end
            end
        end
    end    
    return tfmdata
end

function digitalkhatt.converttotype3char(tfmdata,slot, code)
  
    if not digitalkhatt.usetype3 then
      return
    end

    local character = tfmdata.characters[slot]
    
    local llx = -500
    local lly =-500
    local urx = 1500 
    local ury = 1200

    if not tfmdata.currenttype3charactercode  or tfmdata.currenttype3charactercode == 255 then
        tfmdata.currenttype3charactercode = 1          
        tfmdata.currenttype3font = (tfmdata.currenttype3font or 0) + 1;
        tfmdata.type3font = tfmdata.type3font or {}
        tfmdata.type3font[tfmdata.currenttype3font] = {characters = {},info = {}, bbox = {llx = llx, lly = lly, urx = urx, ury = ury } };
    else
        tfmdata.type3font[tfmdata.currenttype3font].bbox.llx = math.min(tfmdata.type3font[tfmdata.currenttype3font].bbox.llx ,llx)
        tfmdata.type3font[tfmdata.currenttype3font].bbox.lly = math.min(tfmdata.type3font[tfmdata.currenttype3font].bbox.lly ,lly)
        tfmdata.type3font[tfmdata.currenttype3font].bbox.urx = math.max(tfmdata.type3font[tfmdata.currenttype3font].bbox.urx ,urx)
        tfmdata.type3font[tfmdata.currenttype3font].bbox.ury = math.max(tfmdata.type3font[tfmdata.currenttype3font].bbox.ury ,ury)
        tfmdata.currenttype3charactercode = tfmdata.currenttype3charactercode + 1;
    end

    local type3font = tfmdata.type3font
    local currenttype3font = tfmdata.currenttype3font
    local currenttype3charactercode = tfmdata.currenttype3charactercode
    
    type3font[currenttype3font].characters[currenttype3charactercode] = {
        tounicode = string.format("%04x", slot),
        commands = {},          
        name = "glyph" .. slot,
        width = character.width; 
    }     
    
    local width = character.width / tfmdata.parameters.hfactor
      
    --code   = width .. ' 0 ' .. llx .. ' ' .. lly .. ' ' .. urx .. ' ' .. ury .. ' d1 ' .. code
    
    code   = width .. ' 0 '  .. ' d0 ' .. code
    
    type3font[currenttype3font].info[currenttype3charactercode] = {
        pdfcode = code,
        unicode = slot,
        name = "glyph" .. slot,
        width = width,
    }
    
    if type3font[currenttype3font].id then       
      local fontid = type3font[currenttype3font].id
      local newCharacter = {
            [currenttype3charactercode] = type3font[currenttype3font].characters[currenttype3charactercode]
          }        
      font.addcharacters(fontid,{characters = newCharacter})
    else
         
          
      type3font[currenttype3font].name = string.format("digitalkhatttype3font%d",currenttype3font)      
      local type3data = {
        type = "real",
        characters     = type3font[currenttype3font].characters,
        descriptions   = {},
        parameters     = {},            
        resources      = {},
        name          = type3font[currenttype3font].name,
        auto_expand  = true,
        stretch = 30,
        shrink = 20,
        step = 10,
        size = tfmdata.size,
        --psname = "none",
      }
      
      if not digitalkhatt.type3manualgen then
        type3data.psname = "none"
      end
      
      local id = font.define(type3data)      
      type3font[currenttype3font].id  = id
      type3fontbyfontid[id] = type3font[currenttype3font]
    end
    
    tfmdata.fonts = tfmdata.fonts or {{id = 0}}
    
    local fontindex = 0
    
    for index = 1,#tfmdata.fonts do
      if tfmdata.fonts[index].id == type3font[currenttype3font].id then
        fontindex = index
        break
      end
    end
    
    if fontindex == 0 then
      fontindex = #tfmdata.fonts + 1
    end
    
    tfmdata.fonts[fontindex] = { id = type3font[currenttype3font].id }

    tfmdata.characters[slot].commands = {      
      { 'font', fontindex}, 
      { 'char', currenttype3charactercode }, 
    }
      
    
end

function digitalkhatt.addtype3()   
  
    if not digitalkhatt.type3manualgen then
      return
    end
    
    if next(type3fontbyfontid) == nil then
        return
    end    

    for fontid, type3font in pairs(type3fontbyfontid) do
        
        
        local charprocs = pdfdictionary {}
        local diffcharname = pdfarray {1}    
        local widtharray = pdfarray()
        for index,value in ipairs(type3font.info) do
            local objnum = pdf.immediateobj("stream", value.pdfcode)
            charprocs[value.name] = pdfreference(objnum);
            diffcharname[#diffcharname + 1] = pdfconstant(value.name);
            widtharray[#widtharray + 1] = value.width
        end
        local charprocs_objnum = pdfflushobject(charprocs);
        
        local encoding = pdfdictionary {
            Differences = diffcharname,
            Type = pdfconstant("Encoding")      
        }
        
        local encoding_objnum = pdfflushobject(encoding)
        
        local font = pdfdictionary {
            Type          = pdfconstant("Font"),
            Subtype       = pdfconstant("Type3"),
            Name          = pdfconstant(type3font.name),
            FontBBox      = pdfarray {type3font.bbox.llx,type3font.bbox.lly,type3font.bbox.urx,type3font.bbox.ury},
            FontMatrix    = pdfverbose('[0.001 0 0 0.001 0 0]'), --pdfarray {.001,0,0,.001,0,0},  
            CharProcs     = pdfreference(charprocs_objnum),
            Encoding      = pdfreference(encoding_objnum),
            FirstChar     = 1,
            LastChar      = #type3font.info,
            Widths        = widtharray,
        }
        
        --[[
        font = '<<\n'
        font = font .. ' /Type /Font\n' 
        font = font .. ' /Subtype/Type3\n' 
        font = font .. ' /Name/' .. type3font[ii].name .. '\n' 
        font = font .. ' /FontBBox ' .. tostring(pdfarray {type3font[ii].bbox.llx,type3font[ii].bbox.lly,type3font[ii].bbox.urx,type3font[ii].bbox.ury}) .. '\n'
        font = font .. ' /FontMatrix ' .. tostring(pdfarray {0.001,0,0,0.001,0,0}) .. '\n'
        font = font .. ' /CharProcs ' .. tostring(pdfreference(charprocs_objnum)) .. '\n'
        font = font .. ' /Encoding ' .. tostring(pdfreference(encoding_objnum)) .. '\n'
        font = font .. ' /FirstChar 1' .. '\n'
        font = font .. ' /LastChar ' .. #type3font[ii].info + 1 .. '\n'
        font = font .. ' /Widths ' .. tostring(widtharray) .. '\n'
        font = font .. '>>'
        --]]
        
        local font_objnum = pdf.fontobjnum(type3font.id)
        
        --pdf.refobj(font_objnum)

        
        --pdf.print('direct',string.format("%d 0 obj\010",font_objnum))
        --pdf.print('direct',tostring(font) ..'\010')    
        --pdf.print('direct',"endobj\010")
        
        pdf.immediateobj(font_objnum, tostring(font))   
        
        
    end
    
end

local add_to_callback = luatexbase.add_to_callback

function digitalkhatt.read_pk_file(name)
  return true,"",0
end

function digitalkhatt.find_pk_file(name)
  return ""
end

function digitalkhatt.provide_charproc_data(param1,param2,param3)
  if param1 == 2 then        
    local type3 = type3fontbyfontid[param2]
    local info = type3.info[param3]    
    local width = info.width
    local objnum = pdf.immediateobj("stream", info.pdfcode)    
    return objnum, width  
  elseif param1 == 3 then
    return 0.001
  else
    -- param1 == 1 : prerollt3user
    local test = 5
  end
end

luatexbase.callbacktypes['provide_charproc_data'] = 3


add_to_callback('finish_pdffile', digitalkhatt.addtype3, 'digitalkhatt.addtype3')
--add_to_callback('read_pk_file', digitalkhatt.read_pk_file, 'digitalkhatt.read_pk_file')
add_to_callback('find_pk_file', digitalkhatt.find_pk_file, 'digitalkhatt.find_pk_file')
add_to_callback('provide_charproc_data', digitalkhatt.provide_charproc_data, 'digitalkhatt.provide_charproc_data')
