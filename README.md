# TsunagariC Tile Engine

![MIT Licensed](https://img.shields.io/github/license/pmer/TsunagariC.svg)
[![Code Climate](https://codeclimate.com/github/pmer/TsunagariC/badges/gpa.svg)](https://codeclimate.com/github/pmer/TsunagariC)
[![LoC](https://tokei.rs/b1/github/pmer/TsunagariC)](https://github.com/Aaronepower/tokei)

Based on Tsunagari Alpha Preview Release 4 "Akatsuki" Revision 4

-----

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

| NAME        | LICENSE     | LINK                          |
| ----------- | ----------- | ----------------------------- |
| libgosu     | MIT         | http://github.com/gosu/gosu/  |
