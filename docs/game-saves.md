## Game Saves
Planeturem saves are split into 3 parts - planet saves, room saves, and the player save.

### Planet Saves
Planet saves store all data for a specific planet, with the file name being formatted as `PlanetName.dat` in the `Planets` subfolder. This data includes:
 - Chunk data (tiles, objects, entities, structures)
 - Chest data
 - Room data (structure rooms)
 - Map data
 - Version data

This data is serialised in a binary format using [cereal](https://uscilab.github.io/cereal/) then compressed using [lzav](https://github.com/avaneev/lzav).

#### Version data
Everything in Planeturem that is loaded in at runtime is assigned IDs (items, objects, etc). This is dependent on the order of data loaded in, which can change depending on game versions or player modification.

Binary data stored in planet and room saves store these IDs directly for efficient storage. This means the data may become meaningless if the game data loaded changes, as IDs corresponding to items/objects may change.

In order to counteract this, alongside the binary data storage, I store a map of each item/object/etc's name alongside their ID, essentially taking a snapshot of the loaded game data when that save file was generated.

When this is loaded back in, we can use the current game data to create a map used to convert old IDs to new IDs. Before loading the save file into the game, we can then just run all ID through this map to ensure consistency across game versions.

### Room Saves
Room saves refer to save files for "room destinations", which are non-planet locations, such as the space station. They are formatted as `RoomName.dat` in the `Rooms` subfolder. This data includes:
 - Object data (used for metadata e.g. chests, as object layout is determined by room bitmask data)
 - Chest data
 - Version data

This data is serialised in a binary format using cereal.

### Player Save
The player save named `Player.dat` stores all player data and core information about the save file, e.g. current game time, seed, etc.

This stores data for all players in the game, each with the following data:
 - Name
 - Position
 - Inventory
 - Health
 - Location (planet, room, etc)
 - Colour
 - Seen recipes
 - Previously used rocket positions (to ensure travelling back to a planet returns the player to the same location)
 - Last used rocket type (ensures players always get rocket of same type placed when travelling to new planet, e.g. when travelled to a room destination from a planet, when travelling to another planet after, must use same rocket type)
 - Spawn point for each planet

This data is serialised in a readable json format, deserialised using the [nlohmann json](https://github.com/nlohmann/json) library. This allows the player to modify it if they wish, alongside the fact that this is data is small in comparison to the planet data, so binary serialisation would not provide much storage benefit.
