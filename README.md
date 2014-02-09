# Flapper

Flapper is a screen-watching bot for Flappy Bird, particularly
[this flash version](http://www.kongregate.com/games/maxblive/flappy-bird-flash).
It finds the game on the screen, starts it, then plays it by watching the game and clicking.

## Features

- The application requires almost no dependencies: only a C++11-compliant compiler, X11, and Qt are needed.

- The X11 code is abstracted into an interface and could easily be adapted to other graphics systems.

- The application uses native code and concurrency to reach extremely high speeds (60-100 FPS).
  New frames are retrieved from the screen while the previous ones are processed.

- Without the use of computer vision libraries, the application does a good job of tracking
  in-game objects, including the bird, the ground, and the pipe obstacles.

## Known Issues / Delusional ravings of an exhausted developer

- The AI is a crapshoot.
  Several hours were spent tweaking its parameters,
  but ultimately its performance depends largely on whatever course is generated.
  Given additional time, the bot could perform much better using either a machine learning
  approach or more careful force/response analysis,
  but the author is not good at parabolic motion calculations after an all-nighter.
  Hopefully this isn't just sour grapes, but the game iself seems to have
  overzealous hit detection at times, which aggravates the AI issues.

- The display exhibits an odd skew when the bot is started at the start page of the game.
  Since the underlying X11 code is not doing anything differently, this remains a mystery.

## License

See `License.md`
