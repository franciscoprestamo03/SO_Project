# TODO

- [ ] redo alien and bullet allocation and such with arrays

- [ ] make enemies throw down missiles
- [ ] add lives counter
- [x] make projectile and enemy movement calculation on two different threads
- [ ] store users and top 10 user high scores
- [ ] implement save/load game
- [x] add a memory vector to store data
- [ ] make projectiles move 2x as fast as enemies (collisions might pose an issue)

## Alien Generation

aliens are generated from the top left corner 0,0

1. alienRespawnTimes runs in background
  1.1 All timers are decreased or kept in 0
  1.2 if NewAlien Object is empty: check all timers, when one is 0, an alien is writen in NewAlien Object
2. We need to generate an alien only when the latest generated alien has left enough space + 1 for a new alien to be generated:
  2.1 when an alien is generated, a timer, starting at alienSize/alienSpeed + 1 is started, decreased by 1 on every frame
  2.2 if the timer is 0 and the NewAlien Object is full:
    2.2.1 add the alien
    2.2.2 start the timer
    2.2.3 delete the object


## You need to store

- player location
- alien location
  - array of alien pointers
- bullet location
  - similar array
- current score
