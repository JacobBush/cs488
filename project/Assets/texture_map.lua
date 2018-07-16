--
--
--

ground = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
item1 = gr.material({0.5, 0.5, 0.5}, {0.3,0.3,0.3}, 20)

tm1 = gr.texture_map('Assets/textures/checker_large.png')
sand = gr.texture_map('Assets/textures/sand.png')
-- tm2 = gr.texture_map('nonhier.png')

-- ##############################################
-- the scene
-- ##############################################

scene = gr.node('scene')
scene:rotate('X', 23)

-- the floor

plane = gr.mesh( 'plane', 'Assets/plane.obj' )
scene:add_child(plane)
-- plane:set_material(ground)
plane:set_texture_map(sand)
plane:scale(30, 30, 30)

-- the object

s1 = gr.torus('s1', 0.25);
scene:add_child(s1)
s1:set_texture_map(tm1)
s1:scale(4.0, 4.0, 4.0)
s1:translate(-4.5, 3.0, 8.0)

s2 = gr.cube('s2', 0.25);
scene:add_child(s2)
s2:set_texture_map(tm1)
s2:translate(-0.5,-0.5,-0.5)
s2:rotate('y', 45)
s2:scale(4.5, 4.5, 4.5)
s2:translate(4, 2.5, 8.5)

gr.render(scene,
	  'texture_map.png', 512, 512,
	  {0, 2, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {gr.light({200, 202, 430}, {0.8, 0.8, 0.8}, {1, 0, 0})})
