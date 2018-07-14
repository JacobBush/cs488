--
--
--

ground = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
item1 = gr.material({0.5, 0.5, 0.5}, {0.0, 0.0, 0.0}, 0)

tm = gr.texture_map('Assets/textures/Checkerboard_pattern.png')

-- ##############################################
-- the scene
-- ##############################################

scene = gr.node('scene')
scene:rotate('X', 23)

-- the floor

plane = gr.mesh( 'plane', 'Assets/plane.obj' )
scene:add_child(plane)
plane:set_material(ground)
plane:scale(30, 30, 30)

-- the object

s1 = gr.sphere('s1', 0.25);
scene:add_child(s1)
s1:set_material(item1)
s1:set_texture_map(tm)
s1:scale(5.0, 5.0, 5.0)
s1:translate(2.5, 5.0, 8.0)

gr.render(scene,
	  'texture_map.png', 512, 512,
	  {0, 2, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {gr.light({200, 202, 430}, {0.8, 0.8, 0.8}, {1, 0, 0})})