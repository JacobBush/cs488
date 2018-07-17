
checker = gr.texture_map('Assets/textures/checker_large.png')
sand = gr.texture_map('Assets/textures/sand.png')
barber_pole = gr.texture_map('Assets/textures/barber_pole.png')
cake = gr.texture_map('Assets/textures/sponge_cake.png')
bowling_ball = gr.texture_map('Assets/textures/bowling_ball.png')
dirt = gr.texture_map('Assets/textures/dirt.png')


scene = gr.node('scene')
scene:rotate('X', 23)

-- the floor

plane = gr.mesh( 'plane', 'Assets/plane.obj' )
scene:add_child(plane)
plane:set_texture_map(checker)
plane:scale(30, 30, 30)

-- the objects

s1 = gr.torus('s1', 0.25);
scene:add_child(s1)
s1:set_texture_map(cake)
s1:scale(4.0, 4.0, 4.0)
s1:translate(-4.5, 2.0, 8.0)

s2 = gr.cube('s2', 0.25);
scene:add_child(s2)
s2:set_texture_map(sand)
s2:translate(-0.5,-0.5,-0.5)
s2:rotate('y', 45)
s2:scale(4.5, 4.5, 4.5)
s2:translate(4, 2.5, 8.5)

s3 = gr.cylinder('s3', 0.25);
scene:add_child(s3)
s3:set_texture_map(barber_pole)
s3:scale(2.0, 10.0, 2.0)
s3:translate(-4.5, 5.0, 8.0)

s4 = gr.sphere('s4');
scene:add_child(s4)
s4:set_texture_map(bowling_ball)
s4:scale(3.0, 3.0, 3.0)
s4:translate(0.0, 3.0, 0.0)

gr.render(scene,
	  'texture_map.png', 1024, 1024,
	  {0, 2, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {gr.light({200, 202, 430}, {0.8, 0.8, 0.8}, {1, 0, 0})})
