--
--
--

ground = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
item1 = gr.material({0.2, 0.5, 0.7}, {0.3, 0.3, 0.3}, 20)
item2 = gr.material({0.7, 0.5, 0.2}, {0.3, 0.3, 0.3}, 20)


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

t1 = gr.torus('t1', 0.25);
scene:add_child(t1)
t1:set_material(item1)
t1:scale(5.0, 5.0, 5.0)
t1:translate(2.5, 6.25, 8.0)

t2 = gr.torus('t2', 0.25);
scene:add_child(t2)
t2:set_material(item2)
t2:rotate('X', 90)
t2:scale(5.0, 5.0, 5.0)
t2:translate(-2.5, 6.25, 8.0)



gr.render(scene,
	  'torus-shiny.png', 256, 256,
	  {0, 2, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {gr.light({200, 202, 430}, {0.8, 0.8, 0.8}, {1, 0, 0})}, 0.0)
