# R-Type — Player's manual

A cooperative, horizontally scrolling shoot'em up for **1 to 5 players**,
played over the network. Fly through the level together, shoot down the
waves of enemies, pick up weapons and beat the boss at the end.

This document explains how to start the game and how to use every page,
from the first connection to the leaderboard. To build the project, see the
[main README](../../README.md). A French version of this manual is available:
[README.fr.md](README.fr.md).

---

## Table of contents

- [Starting the game](#starting-the-game)
- [Connecting to a server](#connecting-to-a-server)
- [Main menu](#main-menu)
- [Account: create, log in, log out](#account-create-log-in-log-out)
- [Lobbies](#lobbies)
- [Playing](#playing)
- [End of a game](#end-of-a-game)
- [Scores and leaderboard](#scores-and-leaderboard)
- [Settings](#settings)
- [Network statistics](#network-statistics)
- [Files written by the game](#files-written-by-the-game)
- [Troubleshooting](#troubleshooting)

---

## Starting the game

The game is made of two programs, both in the `Games/` folder:

| Program | Role |
|---|---|
| `r-type_server` | The server. One server runs every lobby and keeps the accounts. |
| `r-type_client` | The game itself, one per player. |

The assets are loaded relative to `Games/`, so **always start both from
that folder**:

```sh
cd Games
./r-type_server     # once, on the machine that hosts the games
./r-type_client     # on each player's machine
```

The server listens on **UDP port 4242**. Each lobby then gets its own port,
between **4243 and 6767**. To play with people on other machines, these
ports must be reachable on the server's machine (firewall, router).

> [!NOTE]
> Several clients can run on the same machine as the server: use the
> address `127.0.0.1`.

---

## Connecting to a server

The first page asks for the **server's IP address**.

1. Click the field and type the address of the machine running
   `r-type_server` (`127.0.0.1` if it's your own machine).
2. Click **Connect to Server**.

A "Connecting..." text is shown while the client reaches the server. Once
connected, the main menu appears.

**Leave Game** closes the game.

Once you have logged in to an account, the address is saved in
`credentials.txt` and filled in for you on the next start.

If the server stops answering for 10 seconds, the game comes back to this
page.

---

## Main menu

| Button | What it does |
|---|---|
| **Start Game** | Opens the list of lobbies. |
| **Login** | Opens the login window. Greyed out when you are already logged in. |
| **Disconnect** | Logs you out of your account (you stay connected to the server). Greyed out when you are not logged in. |
| **Settings** | Window mode, volume and controls. |
| **Change Server** | Goes back to the server address page. |
| **Leaderboard** (bottom left) | The best players of the server, and your own rank. |

You can browse the lobbies without an account, but **you must be logged in
to create or join one**.

---

## Account: create, log in, log out

Click **Login** in the main menu. The window has two tabs at the top:

- **Login**: log in to an existing account.
- **Create account**: create a new account. You are logged in right away.

Fill in **Username** and **Password**, then click **Confirm** (or press
**Enter**). **Tab** goes from one field to the other, **Escape** or
**Back** closes the window.

| Field | Rules |
|---|---|
| Username | 3 to 16 characters: letters, digits and `_` |
| Password | 4 to 32 characters, no space (shown as `*`) |

Possible messages:

| Message | Meaning |
|---|---|
| Wrong username or password | The account doesn't exist, or the password is wrong. |
| This username is already taken | Someone already created this account. |
| Invalid username / Invalid password | The rules above are not respected. |

After a successful login, the server address, your username and your
password are saved in `credentials.txt`. On the next start, if you connect
to the same server, the game logs you in by itself. It also logs you in
again every time you come back from a lobby.

> [!WARNING]
> The password is stored in clear text in `credentials.txt`, and it is
> sent unencrypted over the network. Don't reuse a password you use
> anywhere else.

**Disconnect** logs you out and deletes `credentials.txt`, which stops the
automatic login.

---

## Lobbies

A lobby is a game room for **up to 5 players**. Every lobby is a separate
game, running on its own port of the server.

### Lobby list

**Start Game** shows every lobby of the server, with its name, its number
of players (`n/5`) and its state (**Waiting** or **In game**). The list
updates by itself.

- **Click a lobby** to select it, then click **Join Lobby**. A full lobby
  (5 players) can't be joined.
- **Create Lobby** opens a window to name your lobby (16 characters at
  most), then **Create** (or **Enter**). **Back** (or **Escape**) cancels.
- **Back** returns to the main menu.

You can join a lobby that is already **In game**: you get a ship right
away and join the fight.

A lobby that stays empty is closed by the server: right after the last
player leaves, or 30 seconds after its creation if nobody joined it.

### Inside a lobby

The page lists the players of the lobby. Your own line is highlighted. The
**host** is the player who has been in the lobby the longest; if they
leave, the next one becomes the host.

| Button | What it does |
|---|---|
| **Ready / Not Ready** | Tells the others you are ready. |
| **Start Game** | Host only, once every player is ready. Starts the game for everyone. |
| **Leave Game** | Leaves the lobby and goes back to the main menu. |

The text under the list says what the lobby is waiting for ("Players
ready 1/3", "Everyone is ready, start the game!"...).

---

## Playing

### Controls

| Action | Default key |
|---|---|
| Move up | **Z** |
| Move left | **Q** |
| Move down | **S** |
| Move right | **D** |
| Shoot | **Space** |
| Show the network statistics | **F5** |

Every action can be moved to another key or to a mouse button, see
[Settings](#settings).

### Shooting

- **Tap Shoot** to fire a shot. You can fire at most 5 shots per second.
- **Hold Shoot** to charge a powerful shot: a glow appears in front of the
  ship after 0.35 seconds. Every 0.35 seconds the charge gains a level, up
  to level 5 (after 1.75 seconds). **Release** to fire: the higher the
  level, the bigger the shot and the more damage it does (2 damage at
  level 1, up to 12 at level 5).

### The Force and the lasers

Destroyed enemies sometimes drop a **power-up** (5% chance). It stays on
screen for 12 seconds. Fly into it to pick it up:

- the first power-up gives your ship a **Force**: a pod attached to its
  nose. It **blocks enemy shots** and **damages the enemies it touches**;
- a second power-up makes the Force bigger (level 2, the maximum).

While you have a Force, **Shoot fires its laser** instead of the normal
shot. The colour of the power-up tells which laser you get:

| Colour | Laser | Behaviour |
|---|---|---|
| Blue | Reflect | Beams that bounce off the walls of the level |
| Red | Helix | A strong twisting beam straight ahead |
| Green | Ring | Rings; two spread-out rings at level 2 |

Picking up a power-up of another colour changes the laser of your Force.
The Force is lost when your ship is destroyed.

### Lives

- Each player has **3 lives**, shown as small ships next to your score.
- An enemy, an enemy shot, or **the walls and floor of the level** destroy
  your ship. You lose a life and come back at the left of the screen.
- After coming back, your ship blinks: it is **invincible for 3 seconds**.
- With no life left, you stay out until the end of the game, and can watch
  the others.

The game is lost when **every player has lost all their lives**.

### Enemies

| Enemy | Hits to destroy | Points | Behaviour |
|---|---|---|---|
| Scout | 1 | 100 | Flies in waves, in a wavy line |
| Robot gunner | 3 | 300 | Zigzags and shoots at you |
| Walker | 2 | 400 | Walks and jumps on the floor, and shoots |
| Alien | 20 | 1,000 | Tough, attacks with a beam |
| Boss | 100 per player | 5,000 | At the end of the level |

"Hits" are counted with the normal shot (1 damage). Charged shots and
lasers do more.

### The boss

At the end of the level, the scrolling stops and the **boss** comes in.
It is protected most of the time: **it can only be hurt while it is
open**. Its health grows with the number of players. Destroy it to win.

### Score and HUD

- Each enemy destroyed gives its points **to the player who destroyed
  it**. Your score is shown at the bottom of the screen, in your ship's
  colour, with your remaining lives.
- The **team score**, the total of all players, is shown as well.
- Your name is written above your ship.

---

## End of a game

When the game ends, an end screen shows **GAME OVER** (every player lost
all their lives) or **VICTORY** (the boss is destroyed), with the team
score and each player's score.

| Button | What it does |
|---|---|
| **Play Again** | Votes to play again. The game restarts when **every player** has voted, with full lives and scores back to 0. The button shows the votes. |
| **Back to Lobby** | Host only: brings the whole lobby back to the lobby page. |
| **Main Menu** | Leaves the lobby. |

At the end of every game, each player's score is sent to their account
(see below).

---

## Scores and leaderboard

Your account keeps your **best score**. At the end of each game (game
over or victory), if your score beats your best score, it replaces it.
Scores are only saved when you are logged in.

**Leaderboard**, at the bottom left of the main menu, shows:

- the **top 5** players of the server, as `PLACE - USERNAME - SCORE`;
- under **YOUR SCORE**, your own line, with your place among every
  account. Your lines are highlighted.

Players with the same score share the same place (1, 2, 2, 4...). When you
are not logged in, your line says "Log in to see your score".

---

## Settings

**Settings** in the main menu. Changes are applied at once and saved in
`settings.txt`.

### Window mode

The button switches between **Windowed** and **Fullscreen** (borderless,
at your screen's resolution). The game keeps its proportions: when the
window isn't in 16:9, black bands fill the sides.

### Volume

The arrows lower and raise the volume of every sound and music, in 10
steps. The bar shows the current level.

### Controls

The right column shows the input of each action: **Up**, **Left**,
**Down**, **Right** and **Shoot**.

1. Click the button of an action. It shows "Press a key", and the other
   buttons are greyed out.
2. Press a key, **or click a mouse button**: the action is bound to it.
   You can, for example, shoot with the left mouse button.
3. **Escape** cancels.

- If the key is already used by another action, the two actions swap.
- **Enter**, **Tab** and **Backspace** can't be bound: the menus need
  them.
- **Reset** brings back Z, Q, S, D and Space.

The new controls only apply while you play. In the menus, the keys and
the mouse keep their usual role.

> [!TIP]
> If you bind an action to **F5**, F5 no longer shows the network
> statistics.

---

## Network statistics

Press **F5** at any time to show or hide, in a corner of the screen:

- the **FPS** (frames per second);
- the **ping** to the server (round trip, refreshed every 2 seconds);
- the **lost messages**, in each direction.

Useful to know whether a lag comes from the network or from your machine.

---

## Files written by the game

| File | Side | Content |
|---|---|---|
| `credentials.txt` | client | Server address, username, password (clear text) |
| `settings.txt` | client | Volume and controls |
| `userDB.txt` | server | Accounts: username, hashed password, best score |

They are created in the folder the program is started from (`Games/`).
Delete `credentials.txt` (or use **Disconnect**) to forget the saved
account, and `settings.txt` to come back to the default settings.

---

## Troubleshooting

| Problem | Fix |
|---|---|
| The sprites or sounds are missing | The client was not started from `Games/`: `cd Games`, then `./r-type_client`. |
| "Connecting..." stays on screen | Check the address, that `r-type_server` is running, and that UDP port 4242 is open on the server's machine. |
| The lobby list works, but joining a lobby fails | The lobby ports (4243 to 6767) are blocked on the server's machine. |
| **Create Lobby** or **Join Lobby** does nothing | You are not logged in (the server refuses the request): use **Login** first. |
| **Start Game** stays greyed out in the lobby | Only the host can start, and only when every player is **Ready**. |
| My best score didn't change | You weren't logged in, or the score didn't beat your best score. |
| A key does nothing in game | It may have been bound to another action: check **Settings**, or press **Reset**. |
| Black bands on the sides | The window isn't in 16:9: resize it, or use **Fullscreen**. |
