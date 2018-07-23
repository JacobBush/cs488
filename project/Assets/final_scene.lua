--
--
--

obj = gr.material({0.7, 0.1, 0.1}, {0.0, 0.0, 0.0}, 0)
glass = gr.dialectric({0.05, 0.07, 0.05}, 1.52, 20)
background = gr.texture_map('Assets/textures/field.png')
wood_grain = gr.texture_map('Assets/textures/wood_grain.png')

olive_mat = gr.material({0.5, 0.6, 0.25}, {0.4, 0.4, 0.4}, 20)
toothpick_mat = gr.material({0.7, 0.57, 0.47}, {0.0, 0.0, 0.0}, 0)
sugar_mat = gr.material({0.9,0.9,0.9}, {0.0,0.0,0.0}, 0)
sugar_bm = gr.bump_map('Assets/textures/sponge_cake.png')

-- ##############################################
-- the scene
-- ##############################################

scene = gr.node('scene')
scene:rotate('X', 23)

-- the floor

plane = gr.mesh( 'plane', 'Assets/plane.obj' )
scene:add_child(plane)
plane:set_texture_map(wood_grain)
-- plane:set_material(ground)
plane:scale(15, 15, 15)
plane:translate(0,0,5)

-- the object

sugar1 = gr.cube('sugar1')
scene:add_child(sugar1)
sugar1:set_material(sugar_mat)
sugar1:set_bump_map(sugar_bm)
sugar1:translate(-9.0, 0.0, 4.0)

sugar2 = gr.cube('sugar1')
scene:add_child(sugar2)
sugar2:set_material(sugar_mat)
sugar2:set_bump_map(sugar_bm)
sugar2:translate(-10.1, 0.0, 4.0)

sugar3 = gr.cube('sugar1')
scene:add_child(sugar3)
sugar3:set_material(sugar_mat)
sugar3:set_bump_map(sugar_bm)
sugar3:translate(-9.0, 0.0, 5.1)

sugar4 = gr.cube('sugar1')
scene:add_child(sugar4)
sugar4:set_material(sugar_mat)
sugar4:set_bump_map(sugar_bm)
sugar4:translate(-10.1, 0.0, 5.1)

sugar5 = gr.cube('sugar1')
scene:add_child(sugar5)
sugar5:set_material(sugar_mat)
sugar5:set_bump_map(sugar_bm)
sugar5:translate(-9.55, 1.0, 4.55)

olive2 = gr.sphere('olive2')
scene:add_child(olive2)
olive2:set_material(olive_mat)
olive2:scale(0.8, 0.6, 0.6)
olive2:translate(-7.0, 0.6, 2.0)

olive3 = gr.sphere('olive2')
scene:add_child(olive3)
olive3:set_material(olive_mat)
olive3:scale(0.8, 0.6, 0.6)
olive3:translate(-5.0, 0.6, 5)

olive4 = gr.sphere('olive2')
scene:add_child(olive4)
olive4:set_material(olive_mat)
olive4:scale(0.8, 0.6, 0.6)
olive4:translate(-10.5, 0.6, 0.5)

olive = gr.sphere('olive')
scene:add_child(olive)
olive:set_material(olive_mat)
olive:scale(0.6, 0.8, 0.6)
olive:rotate('x', -20)
olive:rotate('z', 40)
olive:translate(3.5, 10.5, 8.0)

toothpick = gr.cylinder('toothpick')
scene:add_child(toothpick)
toothpick:set_material(toothpick_mat)
toothpick:scale(0.1, 3.0, 0.1)
toothpick:rotate('x', -20)
toothpick:rotate('z', 40)
toothpick:translate(3.4, 10.7, 7.9)

s1 = gr.mesh( 'wineglass', 'Assets/glass.obj' )
scene:add_child(s1)
s1:set_material(glass)
s1:scale(3.0, 3.0, 3.0)
s1:translate(-3.0, 0.0, 8.0)

-- right = gr.mesh( 'right', 'Assets/plane.obj' )
-- scene:add_child(right)
-- -- plane:set_texture_map(grid)
-- right:set_texture_map(background)
-- right:scale(10, 10, 30)
-- right:rotate('z', 90)
-- right:translate(14.0, 10, 0.0)

-- rightbound = gr.cube('rightbound')
-- scene:add_child(rightbound)
-- rightbound:set_texture_map(background)
-- rightbound:translate(-0.5,-0.5,-0.5)
-- rightbound:scale(1.0, 100.0, 100.0)
-- rightbound:rotate('y', -10)
-- rightbound:translate(15.0, 0.1, 0.0)

-- leftbound = gr.cube('leftbound')
-- scene:add_child(leftbound)
-- leftbound:set_texture_map(background)
-- leftbound:translate(-0.5,-0.5,-0.5)
-- leftbound:scale(1.0, 100.0, 100.0)
-- leftbound:rotate('y', 10)
-- leftbound:translate(-15.0, 0.1, 0.0)

-- backbound = gr.cube('backbound')
-- scene:add_child(backbound)
-- backbound:set_texture_map(background)
-- backbound:translate(-0.5,-0.5,-0.5)
-- backbound:scale(100.0, 100.0, 1.0)
-- backbound:translate(0.0, 0.1, -20.0)


gr.render(scene,
	  'final_scene.png', 2048, 2048,
	  {0, 2, 30}, {0, 2, 10}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {gr.light({100, 202, 130}, {0.8, 0.8, 0.8}, {1, 0, 0})}, 0.5)
