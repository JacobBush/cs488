mat1 = gr.material({0.4, 0.4, 0.4}, {0.0, 0.0, 0.0}, 0)

scene = gr.node('scene')

side_length =  50 -- side_length ^3 spheres

for i = 1, side_length do
	for j = 1, side_length do
		for k = 1, side_length do
		   s = gr.nh_box(
		   	'obj-' .. tostring(i) .. ', ' .. tostring(j) .. ', ' .. tostring(k),
		   	{2.0*i - side_length, 2.0*j - side_length, 2.0*k - side_length}, 1.0)
		   -- s = gr.mesh( 'obj-' .. tostring(i) .. ', ' .. tostring(j) .. ', ' .. tostring(k),
		   	-- 'Assets/smstdodeca.obj' )
		   s:translate(2.0*i - side_length, 2.0*j - side_length, 2.0*k - side_length)
		   scene:add_child(s)
		   s:set_material(mat1)
		end
	end
end


gr.render(scene,
	  'space_partitioning_small.png', 16, 16,
	  {0, 2, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {gr.light({-200, 202, 430}, {0.8, 0.8, 0.8}, {1, 0, 0})}, 0.0)
