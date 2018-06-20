-- 

hide = gr.material({0.84, 0.6, 0.53}, {0.3, 0.3, 0.3}, 20)
water = gr.material({0.0, 0.45, 0.75}, {0.2, 0.2, 0.4}, 10)
moon_sand = gr.material({0.63, 0.65, 0.68}, {0.0, 0.0, 0.0}, 0)

scene_root = gr.node('root')

cow_poly = gr.mesh('cow', 'Assets/cow.obj')

cow_poly:set_material(hide)

cow_poly:translate(0.0, 2.5, 0.0)
cow_poly:scale(0.5, 0.5, 0.5)
cow_poly:rotate('y', 180)
cow_poly:rotate('z', -50)
scene_root:add_child(cow_poly)

earth = gr.nh_sphere( "Earth", {0,0,0}, 0.5 )
earth:translate(0.5, -0.5, 0.0)
earth:scale(40.0,40.0,40.0)
earth:set_material(water)
scene_root:add_child(earth)

moon = gr.nh_sphere( "moon", {0,0,0}, 0.5 )
moon:translate(-1.0, 0.0, 0.0)
moon:scale(15.0,15.0,15.0)
moon:set_material(moon_sand)
scene_root:add_child(moon)


gr.render(scene_root,
	  'sample.png', 50, 50,
	  {0, 2, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {
	  gr.light({200, 200, 0}, {0.1, 0.1, 0.8}, {1, 0, 0}),
	  gr.light({-200, 200, 0}, {0.8, 0.8, 0.8}, {1, 0, 0})
	  })
