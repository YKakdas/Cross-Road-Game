# Cross-Road-Game

Aim of the game is moving agent without crashing to vehicles and collect points. For each jump, player gets 1 point,
for coin collection, player gets 5 additional points. It is not allowed to jump reverse way of agent's direction. Moving left or right is free.
If agent crashes with vehicle or player tries to move reverse direction, game is over. To use sound effects, *.wav files should be in the
same directory of .sln file. Program should be run in win32 environment and needs glut and opengl libraries.

Design choices:

1.
For simplicity ( when testing, to see winning info panel which appears after you won the game ), if player resist 120 seconds without crash or reaches
200 points, player wins the game.

2.
In one step mode(when you click the right mouse), user may press as much as he/she desired. And then, when right click is pressed again,
last pressed keyboard key will be considered and agent will move according to this key.

3.
Initially I generate vehicles every 100 miliseconds, generate coins every 1000 miliseconds and move vehicles every 20 miliseconds.

4.
When every 6th coin is generated, I dissappear the oldest one from the game.



Extra features:

1.
When game is over, player may restart game with pressing r.

2.
After game is over or game is won, information panel appears on the center of the screen.

3.
When agent and vehicle crashes, agent collects coin or game is over, I play sound effect.(Sometimes sound comes with delay, I could not figure it out)

4.
There is hardness level of game which are determining by pressing 1,2 or 3 from keyboard. You may dynamically change hardness by pressing them during the game without pause.

1: easy : generate vehicles every 200ms and generate coins every 2000ms(if period of generating coin increases, older ones remain longer)
2: normal(by default) : generate vehicles every 100ms and generate coins every 1000ms
3: hard : generate vehicles every 20ms and generate coins every 500ms

Yaşar Can Kakdaş
	   