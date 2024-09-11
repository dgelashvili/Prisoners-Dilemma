# Prisoners-Dilemma

This project is a server-side of a simple, yet very interesting game called 'Prisoners Dilemma'

### Rules
Rules are simple: You are paired with (or against) another player and for each round, you have to choose either to split, or to steal money from the other player. The number of rounds is unknown but is somewhere between 3 and 5.

For each round, once both players type their decisions, following occurs: If both players chose to split, they get 3-3 coins. If both of them chose to steal, they only get 1-1 coins. But if one of them chose to split and the other one stole the money, the first one gets 0 coins but the other one gets 5.

### Leaderboard
At the end of the game, for each player, total number of coins is counted and divided by the number of rounds. This is how the score is calculated. Leaderboard lets you see top 5 players on the server with the highest average score.