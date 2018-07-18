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

c1 = gr.cylinder('c1', 0.25);
scene:add_child(c1)
c1:set_material(item1)
c1:scale(5.0, 5.0, 5.0)
c1:translate(2.5, 5.0, 8.0)

c2 = gr.cylinder('c2', 0.25);
scene:add_child(c2)
c2:set_material(item2)
c2:scale(1.0, 23.0, 1.0)
c2:translate(0.0,-9.0,0.0)
c2:rotate('X', 90)
c2:rotate('Y', 45)
c2:rotate('Z', 30)
c2:translate(-2.0, 8.8, 7.0)

gr.render(scene,
	  'cylinder.png', 1024, 1024,
	  {0, 2, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {gr.light({200, 202, 430}, {0.8, 0.8, 0.8}, {1, 0, 0})})
