-- puppet.lua
-- A simplified puppet without posable joints, but that
-- looks roughly humanoid.

rootnode = gr.node('root')
rootnode:rotate('y', -20.0)
rootnode:scale( 0.25, 0.25, 0.25 )
rootnode:translate(0.0, 0.0, -1.0)

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

torso = gr.mesh('cube', 'torso')
rootnode:add_child(torso)
torso:set_material(white)
torso:scale(0.5,1.0,0.5);

torso_neck = gr.joint('torso_neck', {-0.6,0.0,0.6}, {0.0,0.0,0.0})
torso:add_child(torso_neck)
torso_neck:scale(1.0/0.5, 1.0, 1.0/0.5)
torso_neck:translate(0.0, 0.5, 0.0)

neck = gr.mesh('sphere', 'neck')
torso_neck:add_child(neck)
neck:scale(0.15, 0.3, 0.15)
neck:translate(0.0, 0.1, 0.0)
neck:set_material(blue)

neck_head = gr.joint('neck_head', {-0.6,0.0,0.6}, {0.0,0.0,0.0})
neck:add_child(neck_head)
neck_head:scale(1.0/0.15, 1.0/0.3, 1.0/0.15)
neck_head:translate(0.0, 0.15, 0.0)

head = gr.mesh('cube', 'head')
neck_head:add_child(head)
head:scale(0.4, 0.4, 0.4)
head:translate(0.0, 0.25, 0.0)
head:set_material(red)

ears = gr.mesh('sphere', 'ears')
head:add_child(ears)
ears:scale(1.2, 0.08, 0.08)
ears:set_material(red)
ears:set_material(blue)

leftEye = gr.mesh('cube', 'leftEye')
head:add_child(leftEye)
leftEye:scale(0.2, 0.1, 0.1)
leftEye:translate(-0.2, 0.2, 0.5)
leftEye:set_material(blue)

rightEye = gr.mesh('cube', 'rightEye')
head:add_child(rightEye)
rightEye:scale(0.2, 0.1, 0.1)
rightEye:translate(0.2, 0.2, 0.5)
rightEye:set_material(blue)

leftShoulder = gr.mesh('sphere', 'leftShoulder')
torso:add_child(leftShoulder)
leftShoulder:scale(1/0.5,1.0,1/0.5);
leftShoulder:scale(0.2, 0.2, 0.2)
leftShoulder:translate(-0.4, 0.35, 0.0)
leftShoulder:set_material(blue)

left_shoulder_arm = gr.joint('left_shoulder_arm', {-0.6,0,0.6}, {0,0,0})
leftShoulder:add_child(left_shoulder_arm)
left_shoulder_arm:scale(1.0/0.2, 1.0/0.2, 1.0/0.2)
left_shoulder_arm:translate(-0.1, 0.0, 0.0)

leftArm = gr.mesh('cube', 'leftArm')
left_shoulder_arm:add_child(leftArm)
leftArm:scale(0.4, 0.1, 0.1)
leftArm:rotate('z', 50);
leftArm:translate(-0.2, -0.2, 0.0)
leftArm:set_material(red)

leftElbow = gr.mesh('sphere', 'leftElbow')
leftArm:add_child(leftElbow)
leftElbow:rotate('z', -50);
leftElbow:scale(1.0/0.4,1.0/0.1,1.0/0.1)
leftElbow:scale(0.1,0.1,0.1)
leftElbow:translate(-0.4, 0.0, 0.0)
leftElbow:set_material(blue)

left_elbow_forearm = gr.joint('left_elbow_forearm', {-0.9,0,1.35}, {0,0,0})
leftElbow:add_child(left_elbow_forearm)
left_elbow_forearm:scale(1.0/0.1, 1.0/0.1, 1.0/0.1)

leftForearm = gr.mesh('cube', 'leftForearm')
left_elbow_forearm:add_child(leftForearm)
leftForearm:scale(0.4, 0.1, 0.1)
leftForearm:translate(-0.2, 0.0, 0.0)
leftForearm:rotate('y', 80)
leftForearm:set_material(red)

leftWrist = gr.mesh('sphere', 'leftWrist')
leftForearm:add_child(leftWrist)
leftWrist:rotate('y', -80);
leftWrist:scale(1.0/0.4,1.0/0.1,1.0/0.1)
leftWrist:scale(0.08,0.08,0.08)
leftWrist:translate(-0.4, 0.0, 0.0)
leftWrist:set_material(blue)

left_wrist_hand = gr.joint('left_wrist_hand', {0,0,0}, {-1.0,0,1.0})
leftWrist:add_child(left_wrist_hand)
left_wrist_hand:scale(1.0/0.08, 1.0/0.08, 1.0/0.08)

leftHand = gr.mesh('cube', 'leftHand')
left_wrist_hand:add_child(leftHand)
leftHand:scale(0.1,0.2,0.15)
leftHand:translate(0.0,0.0,0.075)
leftHand:set_material(blue)

rightShoulder = gr.mesh('sphere', 'rightShoulder')
torso:add_child(rightShoulder)
rightShoulder:scale(1/0.5,1.0,1/0.5);
rightShoulder:scale(0.2, 0.2, 0.2)
rightShoulder:translate(0.4, 0.35, 0.0)
rightShoulder:set_material(blue)

right_shoulder_arm = gr.joint('right_shoulder_arm', {-0.6,0,0.6}, {0,0,0})
rightShoulder:add_child(right_shoulder_arm)
right_shoulder_arm:scale(1.0/0.2, 1.0/0.2, 1.0/0.2)
right_shoulder_arm:translate(0.1, 0.0, 0.0)

rightArm = gr.mesh('cube', 'rightArm')
right_shoulder_arm:add_child(rightArm)
rightArm:scale(0.4, 0.1, 0.1)
rightArm:rotate('z', -50);
rightArm:translate(0.2, -0.2, 0.0)
rightArm:set_material(red)

rightElbow = gr.mesh('sphere', 'rightElbow')
rightArm:add_child(rightElbow)
rightElbow:rotate('z', 50);
rightElbow:scale(1.0/0.4,1.0/0.1,1.0/0.1)
rightElbow:scale(0.1,0.1,0.1)
rightElbow:translate(0.4, 0.0, 0.0)
rightElbow:set_material(blue)

right_elbow_forearm = gr.joint('right_elbow_forearm', {-0.9,0,1.35}, {0,0,0})
rightElbow:add_child(right_elbow_forearm)
right_elbow_forearm:scale(1.0/0.1, 1.0/0.1, 1.0/0.1)

rightForearm = gr.mesh('cube', 'rightForearm')
right_elbow_forearm:add_child(rightForearm)
rightForearm:scale(0.4, 0.1, 0.1)
rightForearm:translate(0.2, 0.0, 0.0)
rightForearm:rotate('y', -80)
rightForearm:set_material(red)

rightWrist = gr.mesh('sphere', 'rightWrist')
rightForearm:add_child(rightWrist)
rightWrist:rotate('y', 80);
rightWrist:scale(1.0/0.4,1.0/0.1,1.0/0.1)
rightWrist:scale(0.08,0.08,0.08)
rightWrist:translate(0.4, 0.0, 0.0)
rightWrist:set_material(blue)

right_wrist_hand = gr.joint('right_wrist_hand', {0,0,0}, {-1.0,0,1.0})
rightWrist:add_child(right_wrist_hand)
right_wrist_hand:scale(1.0/0.08, 1.0/0.08, 1.0/0.08)

rightHand = gr.mesh('cube', 'rightHand')
right_wrist_hand:add_child(rightHand)
rightHand:scale(0.1,0.2,0.15)
rightHand:translate(0.0,0.0, 0.075)
rightHand:set_material(blue)

leftHip = gr.mesh('sphere', 'leftHip')
torso:add_child(leftHip)
leftHip:scale(1/0.5,1.0,1/0.5);
leftHip:scale(0.21, 0.21, 0.21)
leftHip:translate(-0.38, -0.5, 0.0)
leftHip:set_material(blue)

left_hip_leg = gr.joint('left_hip_leg', {-0.8,0,0.8}, {0,0,0})
leftHip:add_child(left_hip_leg)
left_hip_leg:scale(1.0/0.21, 1.0/0.21, 1.0/0.21)
left_hip_leg:translate(-0.105, 0.0, 0.0)

leftLeg = gr.mesh('cube', 'leftLeg')
left_hip_leg:add_child(leftLeg)
leftLeg:scale(0.1,0.55,0.1)
leftLeg:translate(0,-0.4,0)
leftLeg:set_material(red)

leftKnee = gr.mesh('sphere', 'leftKnee')
leftLeg:add_child(leftKnee)
leftKnee:scale(1.0/0.1,1.0/0.55,1.0/0.1);
leftKnee:scale(0.1, 0.1, 0.1)
leftKnee:translate(0, -0.4, 0.0)
leftKnee:set_material(blue)

rightHip = gr.mesh('sphere', 'rightHip')
torso:add_child(rightHip)
rightHip:scale(1/0.5,1.0,1/0.5);
rightHip:scale(0.21, 0.21, 0.21)
rightHip:translate(0.38, -0.5, 0.0)
rightHip:set_material(blue)

right_hip_leg = gr.joint('right_hip_leg', {-0.8,0,0.8}, {0,0,0})
rightHip:add_child(right_hip_leg)
right_hip_leg:scale(1.0/0.21, 1.0/0.21, 1.0/0.21)
right_hip_leg:translate(0.105, 0.0, 0.0)

rightLeg = gr.mesh('cube', 'rightLeg')
right_hip_leg:add_child(rightLeg)
rightLeg:scale(0.1,0.55,0.1)
rightLeg:translate(0,-0.4,0)
rightLeg:set_material(red)

rightKnee = gr.mesh('sphere', 'rightKnee')
rightLeg:add_child(rightKnee)
rightKnee:scale(1.0/0.1,1.0/0.55,1.0/0.1);
rightKnee:scale(0.1, 0.1, 0.1)
rightKnee:translate(0, -0.4, 0.0)
rightKnee:set_material(blue)

return rootnode
