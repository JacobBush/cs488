--
--
--

ground = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
item = gr.material({0.2, 0.5, 0.7}, {0.3, 0.3, 0.3}, 20)


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

torus = gr.torus('torus', 0.25);
scene:add_child(torus)
torus:set_material(item)
torus:scale(5.0, 5.0, 5.0)
torus:translate(0.0, 6.25, 5.0)


gr.render(scene,
	  'torus.png', 512, 512,
	  {0, 2, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {gr.light({200, 202, 430}, {0.8, 0.8, 0.8}, {1, 0, 0})})
