function str_split(str, del)
    res = {}
    cur = ""
    for i = 1, #str do
        local c = str:sub(i,i)
        if c == del then
            if cur ~= "" then
                table.insert(res, cur)
                cur = ""
            end
        else
            cur = cur .. c
        end
    end

    if cur ~= "" then
        table.insert(res, cur)
    end

    return res
end

function enum(e, str_vals, offset)
    e = {}
    vals = str_split(str_vals, " ")
    for i,v in ipairs(vals) do
        e[v] = i + (offset or 0)
    end
    return e
end

function error(str)
    local w = debug.getinfo(2, "S")
    local error_loc_info = w.short_src..":"..w.linedefined
    print("ERROR (" .. error_loc_info .. "): " .. str)
end
