function gen_map(iii)
	for j = 0,7,1 do
		for i=0,127,1 do
			if i < iii then
				io.write(string.format("0x%02x,", 0x00))
			else
				io.write(string.format("0x%02x,", 0xff))
			end
		end
		io.write("\n")
	end
end
function gen_array(a)
	t = {}
	for i = a,128,a do
		table.insert(t, i)
		-- gray_array:insert(i)
	end
	return t
end
for k,v in ipairs(gen_array(16)) do
	gen_map(v)
end
