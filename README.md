# Lunar_Lander

Created for CS3113

This is my Project for a Lunar Lander: Tie Fighter Landing <br>
Here, I have coded a Lunar Lander, with my Theme being Star Wars, with a Tie Fighter landing on a planet (I based it on Mustafar). The animation I included is for the rocket fire when it is coming out of the bottom of the Tie Fighter. I made some changes to Entity.h and Entity.cpp as well, as I needed to create some variables and functions for checking collisions with my platforms and the lava floor. I also added horizontal drag to either side to reduce horizontal acceleration over time (the sliding effect that was required.) I needed to modify the animation conditions as well for only when the entity uses an animation atlas as I needed the rocket exhaust fire to work independently of the movement of the Tie Fighter. 

Controls: <br>
W-> Move Up: Thrusts the Tie Fighter Upwards and uses Fuel. <br>
A-> Accelerate Left: Accelerates the Tie Fighter Left and uses Fuel. Affected by Drag. <br>
D-> Accelerate Right: Accelerates the Tie Fighter Right and uses Fuel. Affected by Drag. <br>
R-> Restart Game: At the end of the game (win or lose), allows you to restart the game. <br>
Q-> Quit Game: At any time, quits the game <br>
