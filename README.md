# TsunagariC Tile Engine

| BUILD   | STATUS                                                                                                                                      |
| ------- | ------------------------------------------------------------------------------------------------------------------------------------------- |
| Windows | [![Appveyor](https://ci.appveyor.com/api/projects/status/github/pmer/TsunagariC?svg=true)](https://ci.appveyor.com/project/pmer/TsunagariC) |
| macOS   | [![TravisCI](https://api.travis-ci.com/pmer/TsunagariC.svg)](https://travis-ci.com/pmer/TsunagariC)                                         |
| Linux   | [![CircleCI](https://circleci.com/gh/pmer/TsunagariC.svg?style=shield)](https://circleci.com/gh/pmer/TsunagariC)                            |
| FreeBSD | [![builds.sr.ht](https://builds.sr.ht/~pdm/tsunagaric/freebsd.yml.svg)](https://builds.sr.ht/~pdm/tsunagaric/freebsd.yml)                   |
| NetBSD  | [![builds.sr.ht](https://builds.sr.ht/~pdm/tsunagaric/netbsd.yml.svg)](https://builds.sr.ht/~pdm/tsunagaric/netbsd.yml)                     |

[![LoC](https://tokei.rs/b1/github/pmer/TsunagariC)](https://github.com/XAMPPRocky/tokei)

TsunagariC is a tiling game engine inspired by the cult classic game Yume
Nikki. It is intended as an open source alternative to the popular proprietary
RPGMaker game development suite that the original and most fangames were/are
written on. TsunagariC is written in C++, using the Gosu framework.

Features:
TsunagariC will be a comprehensive game design suite for singleplayer 2D games.
The engine will support several styles, such as roguelikes, and old
console-style RPGs. Multiplayer support is planned for the second stable
release. TsunagariC allows C++ scripting for the event system, and for
additional custom functionality.

Current features include:
* Yume Nikki-like and roguelike movement modes.
* Music and sound effects handling.
* Tile and sprite animations.
* Intelligent viewport tracking.
* Support for infinite graphical and walkable layers.
* Resource caching for nonexistent load times.
* Command line options for fine-tuning.
* Subpixel rendering.
* Looping areas support.
* NPCs.
* Event scripting interface in C++.

Requirements:

| NAME        | LICENSE     | LINK                   |
| ----------- | ----------- | ---------------------- |
| SDL2        | zlib        | http://www.libsdl.org  |

or

| NAME        | LICENSE     | LINK                          |
| ----------- | ----------- | ----------------------------- |
| libgosu     | MIT         | http://github.com/gosu/gosu/  |
