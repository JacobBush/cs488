checker = gr.texture_map('Assets/textures/checker_large.png')

mat1 = gr.material({0.4, 0.2, 0.4}, {0.0, 0.0, 0.0}, 0)

scene = gr.node('scene')
scene:rotate('X', 23)

-- the floor

plane = gr.mesh( 'plane', 'Assets/plane.obj' )
scene:add_child(plane)
plane:set_texture_map(checker)
plane:scale(30, 30, 30)

-- the objects

for i = 1, 12 do
	s = gr.cube('s');
	scene:add_child(s)
	s:set_material(mat1)
	s:translate(-0.5, -0.5, -0.5)
	s:scale(10, 1, 1)
	s:translate(0.0, 0.5, 20.0 - 4.0*i)
end

gr.render(scene,
	  'depth_of_field.png', 256, 256,
	  {0, 2, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {gr.light({-200, 202, 430}, {0.8, 0.8, 0.8}, {1, 0, 0})}, 5.0)
