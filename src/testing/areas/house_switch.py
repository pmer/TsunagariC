# Unlock the door!
if unlocked_the_door == False:
    unlocked_the_door = True

    tile = Area.tiles(4, 0, 0.0) # closed exit on north wall, property layer
    tile.exit = newExit("areas/secret_room.tmx", 4, 5, 0.0)
    tile.flags.nowalk = False

    tile = Area.tiles(4, 0, -0.2) # closed exit on north wall, graphics layer
    tile.type = Area.get_tile_type(66) # change to open exit

    Area.request_redraw()
    Sound.play("sounds/door.oga") # unlocking sound

