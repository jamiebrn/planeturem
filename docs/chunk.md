## ChunkManager

The ChunkManager system manages chunks in the game world, loading and unloading them as required. It provides an API to interact with the game world, such as placing and destroying objects, getting objects or entities at specified positions, testing collisions, etc.

This system isn't particularly interesting, as it essentially just calls respective functions on the correct chunk, and manages chunk lifetimes. I will mention a few systems that I feel are more interesting.

### Chunk loading
Chunk loading is carried out in the `updateChunks()` function. This uses the [ChunkViewRange](##ChunkViewRange) struct for each client in order to load any chunks in the player view area that have not been loaded.

There is a logical division between chunks - loaded chunks are currently being updated and in the player's view, while stored chunks are chunks that have been previously generated/loaded from a save file and are currently not active.

`loadedChunks` and `storedChunks` are both hashmaps using a `ChunkPosition` as the key, which is a struct containing an X and Y position corresponding to the chunk's world position. This struct contains a hashing implementation which enables it's use in hashmaps. The value corresponding to a key is a pointer to a chunk, or to be exact is of type `std::unique_ptr<Chunk>`.

The `updateChunks()` function essentially carries out these instructions:
```cpp

bool updateChunks()
{
    bool hasModifiedChunks = false;
    for chunkPosition in chunkViewRanges
    {
        if chunkPosition in loaded
            continue;

        hasModifiedChunks = true;

        if chunkPosition in stored
        {
            move stored chunk into loaded
            continue;
        }

        // Chunk needs to be loaded but is not in stored chunks
        generateChunk();
    }

    return hasModifiedChunks;
}

```

Unloading chunks is essentially the reverse of this.

### Finding spawn locations

The function ```findValidSpawnChunk()``` can be used to find a chunk valid for the player to spawn on. It works as follows:

```cpp
ChunkPosition findValidSpawnChunk(int waterlessAreaSize)
{
    // Calculate search area size to increment by each failed search
    int searchIncrement = 1 + waterlessAreaSize * 2;
    while searching
    {
        bool validSpawn = true;
        for chunkPosition in searchArea
        {
            Chunk chunk = generateChunkMinimal();

            // If chunk doesn't contain water, chunk is valid. Continue to next chunk
            if (!chunk.getContainsWater())
                continue;

            // Chunk contains water, search area is not valid. Go to next search area
            validSpawn = false;
            break;
        }

        if (validSpawn)
        {
            return searchArea centre chunk;
        }

        // Not valid spawn, increment
        searchArea += searchIncrement;
    }

    // Searching ended without finding chunk - recursively call with search with less strict parameters
    if (waterlessAreaSize > 0)
        return findValidSpawnChunk(waterlessAreaSize - 1);

    // Default fail case
    return ChunkPosition(0, 0);
}
```

### Chunk and Tile from Offset
I often found the need to find a Chunk/Tile pair in the world from an offset from another Chunk/Tile pair. This allowed for "relative" searching throughout the world. While it may seem simple I found myself spending a bit too long on this function, so thought I would include it here.

With `CHUNK_TILE_SIZE` being the width and height of a square chunk in tiles, the function works as follows:

```cpp
pair<ChunkPosition, Vector2i> getChunkTileFromOffset(ChunkPosition chunk, Vector2i tile, int xOffset, int yOffset, int worldSize)
{
    // Add offset to tile
    tile.x += xOffset;
    tile.y += yOffset;

    // Perform wrapping of tile and offset chunk value if not in chunk range
    if (tile.x < 0 || tile.x >= CHUNK_TILE_SIZE || tile.y < 0 || tile.y >= CHUNK_TILE_SIZE)
    {
        // Modify chunk values depending on tile value after being offset
        if (tile.x < 0)
            chunk.x -= ceil(abs(tile.x) / CHUNK_TILE_SIZE);
        else if (tile.x >= CHUNK_TILE_SIZE)
            chunk.x += ceil(1 + tile.x) / CHUNK_TILE_SIZE - 1;

        if (tile.y < 0)
            chunk.y -= ceil(abs(tile.y) / CHUNK_TILE_SIZE);
        else if (tile.y >= CHUNK_TILE_SIZE)
            chunk.y += ceil(1 + tile.y) / CHUNK_TILE_SIZE - 1;

        // Wrap tile to chunk range after shifting chunk position
        tile.x = (tile.x % CHUNK_TILE_SIZE + CHUNK_TILE_SIZE) % CHUNK_TILE_SIZE;
        tile.y = (tile.y % CHUNK_TILE_SIZE + CHUNK_TILE_SIZE) % CHUNK_TILE_SIZE;

        // Wrap chunk tile (may be out of world bounds)
        chunk.x = (chunk.x % worldSize + worldSize) % worldSize;
        chunk.y = (chunk.y % worldSize + worldSize) % worldSize;
    }

    return {chunk, tile};
}
```

### Nearby Crafting Stations
As this is a crafting game, and I wanted to limit player recipes/progression through crafting stations, I needed a function that would return the crafting stations that the player was in range of. Crafting stations types are represented as strings for the base type and an integer for the level. This allows higher levels to craft lower level recipes. For example, a "bench" level 1 can craft bench level 1 recipes, and a "bench" level 2 can craft bench level 1 and level 2 recipes. Crafting station types are represented as strings as they are loaded in at runtime; it is data-driven - meaning the game cannot predict what crafting stations may appear. This allows for easy addition of content without recompilation, which is great for iterative development and allows for player-created content.

To search which crafting stations the player is in range of, I wrote this function:
```cpp
unordered_map<string, int> getNearbyCraftingStationLevels(ChunkPosition playerChunk, Vector2i playerTile, int searchArea)
{
    unordered_map<string, int> craftingStationLevels;

    for offset in searchArea range
    {
        // Get chunk and tile from offset
        pair<ChunkPosition, Vector2i> chunkTile = getChunkTileFromOffset(playerChunk, playerTile, offset.x, offset.y, worldSize);

        // If chunk not loaded, do not get object
        if chunkTile.first not in loadedChunks
            continue;
        
        BuildableObject* object = getChunkObject(chunkTile.first, chunkTile.second);

        // If no object, do not get data
        if (!object)
            continue;

        // Get object type
        ObjectType objectType = object->getObjectType();
        if (objectType < 0)
            continue;

        // Get object data
        const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

        // If not a crafting station, do not add to hashmap
        if (objectData.craftingStation.empty())
            continue;

        // Add crafting station to map
        if (craftingStationLevels.count(objectData.craftingStation) > 0)
        {
            // Crafting station level exists, set level to highest level present
            craftingStationLevels[objectData.craftingStation] = max(objectData.craftingStationLevel, craftingStationLevels[objectData.craftingStation]);
        }
        else
        {
            craftingStationLevels[objectData.craftingStation] = objectData.craftingStationLevel;
        }
    }

    return craftingStationLevels;
}
```

## ChunkViewRange
The `ChunkViewRange` class represents a rectangular chunk area, used to represent an area of the world visible to a player. It allows creation of a hashset of key type `ChunkPosition` containing all chunks in the visible area, as well as creating a hashset from a vector of `ChunkViewRange`. This is particularly useful in multiplayer where all chunks visible to players need to be updated, but chunks in the intersection of multiple player's view areas should not be updated multiple times obviously. A hashset of all visible chunks ensures each `ChunkPosition` visible occurs only once.

The functionality of this class was previously built directly into the `ChunkManager` class functions. However, during the addition of multiplayer I felt the need to have a clean abstraction over a visible chunk area, as well as handling iteration over this range and handling of world wrapping at planet edges. This was because each player would have their own visible area in the world, which would be processed on the host system in multiplayer, so encapsulating this system only made sense.
