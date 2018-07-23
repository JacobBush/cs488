--
--
--

ground = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
dial = gr.dialectric({0.0, 0.0, 0.0}, 1.52, 20)
grid = gr.texture_map('Assets/textures/grid.png')


-- ##############################################
-- the scene
-- ##############################################

scene = gr.node('scene')
scene:rotate('X', 23)

-- the floor

plane = gr.mesh( 'plane', 'Assets/plane.obj' )
scene:add_child(plane)
plane:set_texture_map(grid)
plane:scale(30, 30, 30)

-- the object

s1 = gr.sphere('s1');
scene:add_child(s1)
s1:set_material(dial)
s1:scale(5.0, 5.0, 5.0)
s1:translate(2.5, 5.0, 8.0)


gr.render(scene,
	  'dialectric.png', 1024, 1024,
	  {0, 2, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {gr.light({200, 202, 430}, {0.8, 0.8, 0.8}, {1, 0, 0})}, 0.0)
