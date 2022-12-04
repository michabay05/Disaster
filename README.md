# Disaster

This is a UCI bitboard chess engine written in C++. This engine has an estimated rating of 1800 elo.

## Features
- Bitboard representation
- Magic number for sliding piece attack precalculation
- Move generation via the COPY/MAKE approach
- Performance tester
- Negamax algorithm with alpha-beta pruning
- PV Lines
- Iterative deepening
- Aspiration Windows
- UCI protocol commands
- Drawn endgame evaluation

## Resources used
I've used a lot of resources to make this chess engine.

- [Chess Programming](https://www.youtube.com/@chessprogramming591/featured) YouTube Channel ([Maksim Korzh](https://github.com/maksimKorzh))
    - [Bitboard chess engine in C](https://www.youtube.com/playlist?list=PLmN0neTso3Jxh8ZIylk74JpwfiWNI76Cs) YouTube Tutorial Playlist
        - [GitHub Repo of the tutorial series](https://github.com/maksimKorzh/chess_programming/tree/master/src/bbc)
- [*How to Write a Bitboard Chess Engine: How Chess Programs Work*](https://www.google.com/search?rlz=1C1RXQR_enUS990US990&sxsrf=ALiCzsYRNKjpASZ87Xrc8r98kgwJBYRzAQ:1670121754670&q=How+to+Write+a+Bitboard+Chess+Engine:+How+Chess+Programs+Work&si=AC1wQDApU_L7mEr08NL2tyPAiwbkKXiOQMzKp1cMwbGxhXrbjBzlf1SKuiDCttZFogAsVkuzg0qwSK8LzDL1lqgQhuqOUGwOntdh1qCSaHG7Bb8fzT7AldTne2zDDVZhAzuIbJPOTgjxYa7RRZGFC4PWkyXT99ME6kDUpq_9gAWf5591PV-_WliphFvyE4O5nEMjdL2jXofDdlMteytyzdIbTd9pj9Y1eqyu6rmzCj_aSjMeywyQzTk%3D&sa=X&ved=2ahUKEwibqayv-N77AhWwFVkFHd1mBhMQ6RN6BAg8EAE&biw=1920&bih=1007&dpr=1) by FM Bill Jordan
- [Blunder Chess Engine](https://github.com/algerbrex/blunder)
- [Chess Programming Website](https://www.chessprogramming.org/)