
checker = gr.texture_map('Assets/textures/checker_large.png')
sand = gr.texture_map('Assets/textures/sand.png')
brick_bm = gr.bump_map('Assets/textures/brick.png')

mat1 = gr.material({0.4, 0.4, 0.4}, {0.3, 0.3, 0.3}, 10)

scene = gr.node('scene')
scene:rotate('X', 23)

-- the floor

plane = gr.mesh( 'plane', 'Assets/plane.obj' )
scene:add_child(plane)
plane:set_texture_map(sand)
plane:scale(30, 30, 30)

-- the objects

s = gr.sphere('s');
scene:add_child(s)
s:set_material(mat1)
s:set_bump_map(brick_bm)
s:scale(3.0, 3.0, 3.0)
s:translate(4.0, 3.0, 8.0)


s2 = gr.sphere('s2');
scene:add_child(s2)
s2:set_material(mat1)
-- s4:set_bump_map(brick_bm)
s2:scale(3.0, 3.0, 3.0)
s2:translate(-4.0, 3.0, 8.0)

gr.render(scene,
	  'bump_map2.png', 1024, 1024,
	  {0, 2, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {gr.light({-200, 202, 430}, {0.8, 0.8, 0.8}, {1, 0, 0})}, 0.0)
