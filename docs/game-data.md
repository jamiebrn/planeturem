## Game Data
Game data in Planeturem is loaded in through json files at runtime. This includes:
 - Items
 - Objects
 - Tools/Armour
 - Entities
 - Item recipes
 - Planet generation/tilemaps/structures

Deserialisation of these files uses the [nlohmann json](https://github.com/nlohmann/json) library, each item data type having its own DataLoader class, e.g. `ItemData` has a static `ItemDataLoader` class which controls deserialisation of the `items.data` file into game data.

## Item Data
Items are just "basic items" with no unique behaviours in the game, e.g. materials used in recipes. They are loaded in order and assigned IDs based on that order, starting from 0.

Each of these items have unique data such as the name, description, texture UVs etc.

After these items have been loaded, items corresponding to objects and tools/armour are created. This allows the player to actually have these objects etc in their inventory.

For reference, here are all of the current fields in `ItemData`, as well as associated structs. Here you can see the links to other systems, e.g. `placesObjectType` links to the object system, where an item has been created for an object which can then be placed.

```cpp
struct ItemData
{
    std::string name;
    std::optional<std::string> displayName;
    std::string description;
    pl::Rect<int> textureRect;

    unsigned int maxStackSize = 99;

    ObjectType placesObjectType = -1;
    ToolType toolType = -1;
    ArmourType armourType = -1;
    ProjectileType projectileType = -1;
    bool placesLand = false;

    std::optional<BossSummonData> bossSummonData = std::nullopt;
    std::optional<ConsumableData> consumableData = std::nullopt;

    bool isMaterial = false;

    int currencyValue = 0;
    float sellValue = 0;

    pl::Color nameColor = pl::Color(255, 255, 255);

    std::string achievementUnlockOnObtain;
}

struct BossSummonData
{
    std::string bossName;
    bool useAtNight = false;
};

struct ConsumableData
{
    int cooldownTime;
    int healthIncrease = 0;
    int permanentHealthIncrease = 0;
};
```

## Object Data
Objects spawn naturally in the world and can be placed by players. They have many fields, such as name, health, texture UV, tile size, light emission and absorption, collision, crafting station name/level, item drops, chest capacity etc. They also all have IDs starting from 0, which are used when corresponding items are created.

Each of these objects have items created automatically, meaning the player can have any object in their inventory. NPCs are also just objects with unique interaction behaviour, i.e. dialogue, meaning the player can have an NPC in their inventory. However in a legitimate playthrough the player will never be able to obtain this, as they can only get items from item drops from other objects, crafting, and chest spawns in structures.

### Solving circular dependencies
In order for objects to be able to drop other objects and themselves, I create the item corresponding to each object before loading object data such as item drops. Without this, when an object would be loaded, the item for that object would not be created, so it would not be able to drop itself, and would also not be able to drop objects that haven't been loaded yet, i.e. objects further into the `objects.data` file. This obviously isn't ideal, as for example a wooden bench object should drop a wooden bench, and objects dropping other objects would have to be placed further into the object data file.

### Object class types
Unique object behaviours e.g. chests or rockets are implemented in derivative classes from the base `BuildableObject` class (all objects are of this type). These behaviours are implemented through virtual functions such as `interact()`, which allows all objects to be stored as `std::unique_ptr<BuildableObject>` and managed through the same interface. While this does mean storing of objects isn't in contiguous memory due to varying object sizes (therefore must be allocated on heap) so is therefore less performant, it allows each object to store as much data as required rather than each object containing redundant data, e.g. a tree allocating memory for its `chestID`.

The factory pattern is used in order to create the correct object type depending on object data. The code looks something like this:
```cpp
std::unique_ptr<BuildableObject> createObject(ObjectType objectType)
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
    if (objectData.chestCapacity > 0)
        return std::make_unique<ChestObject>();
    if (objectData.isRocket)
        return std::make_unique<RocketObject>();
}
```
I am not a massive fan of this pattern as it requires me to "register" each type to this function, but it allows for creating derived types based on criteria through a succinct interface, which is what I needed for implementing different behaviours.

### Component system
I did consider creating some sort of component system where each object could store a component for behaviour implementation, but decided basic inheritance would be simpler, and would allow simple downcasting via dynamic cast if I really required it. Whereas casting in a component system would require the base object to expose the component pointer, which could then be casted. But this then doesn't have access to the rest of the object unless the component stores a pointer to the object, which would be fragile due to potential moving/reallocation of the object. The inheritance system allows for derived classes to have access to the rest of the object as it is of the parent type, but just has extra functionality/data.

## Tool/Armour Data
Tool data consists of tools with different behaviours, such as a Wooden Pickaxe or a Titanium Fishing Rod. Each of these tools have a name, behaviour type (e.g. `MeleeWeapon`, `Pickaxe` etc), texture UVs, pivot offsets, damage values etc. As with the other systems, each of these tools have IDs starting from 0 and items created.

Tool data also contains projectiles, which also have their own IDs and items created. These are used in ranged weapons, and are controlled in the `ProjectileManager` system.

Armour data is very similar - each armour piece can be of type `Head`, `Chest`, or `Feet`. Each has a name, texture UVs for each player animation frame, defence values etc. Also have IDs and items created.

## Entity Data
Entity data contains entity names, textures for idle and walking animations, size, health, collision, item drops, behaviour type and behaviour parameters.

### Behaviour types
Behaviour types are classes written in the source code, which are selected at entity instantiation depending on the behaviour type string. I was debating on implementing some sort of scripting system for entity AI, such as using Lua. But I felt this was overkill and would have probably taken much more work than just writing different classes in C++. While this would have improved modding support, I would not have preferred to use Lua over C++, so just stuck with C++.

### Behaviour parameters
Behaviour customisation can be done however through the use of behaviour parameters. These are loaded into a hashmap at runtime, using a string for the key to represent parameter name and a float for the value. This allows the chosen entity behaviour to query parameters to customise the AI. Of course these parameters still have to be hardcoded into the executable in the behaviour classes, but I think it is a good enough tradeoff for customisation without implementing an external scripting system.

While most systems in the game convert string values into respective type IDs when data is loaded to increase performance, behaviour parameters remain as strings at runtime as otherwise each behaviour parameter would require an enum value/type ID to be assigned. This would reduce design iteration ability and ergonomics. This does therefore mean more compute is required when spawning entities as string hashes need to be calculated to look up values, however this will not be a noticeable performance hit, especially when this only needs to be done on entity initialisation. On client systems in multiplayer this is a larger hit as entities are reinitialised every 2 server ticks (22Hz), but is still completely inconsequential due to cache locality of strings.

## Recipe Data
Recipes loaded after all items, objects etc have been loaded, to ensure all items are present. Recipes specify an item to be created and amount, all required items and amounts, items required for the player to see the recipe (`keyItems`), and required crafting station name and level in order to see the recipe.

With no `keyItems`, the recipe will be visible to the player if they have one of the required items, given they are in range of the required crafting station with sufficient level.

This is the data layout of a loaded `RecipeData`:

```cpp
struct RecipeData
{
    ItemType product;
    unsigned int productAmount = 1;

    std::map<ItemType, unsigned int> itemRequirements;

    std::optional<std::vector<ItemType>> keyItems;
    
    std::string craftingStationRequired = "";
    int craftingStationLevelRequired;
}
```

The inventory GUI then iterates over each recipe when the player's inventory or the nearby crafting stations are modified, and stores the IDs of recipes that are craftable or visible. These are then displayed when the player is in the inventory GUI.

## Structures/Rooms
Rooms are locations that the player can travel to outside of planets, or in structures on planets. They have a name, a texture offset for the background/floor, size in tiles, collision bitmask texture offset, and an object map. This maps the blue channel values of the collision bitmask to objects that will be placed in the room. Rooms also have a "travel location" flag which allows the room to be travelled to via rocket (assuming the rocket has the room in its room destinations).

Structures each have a name, tile size, texture, collision bitmask texture offset, light bitmask texture offset, and a named room.

## Planet Generation
Planet generation data defines all procedural generation parameters for each planet.

For each planet, the following data can be specified for the entire planet:
 - Name
 - Water texture offset
 - Water colour
 - Cliff texture offset
 - River noise parameters
 - World size
 - Boss spawns

Biomes are then specified with the following data:
 - Name
 - Tiles with noise ranges
 - Object spawn chances per tile
 - Fish catches and probability
 - Water colour
 - Biome noise range
 - Resource regeneration time

### Tilemaps
Tilemaps are also specified in the `planet_generation.data` file. These are just keys representing the names of the tilemaps (which are referenced in the biome data for each planet), with data to represent the tilemap. This is the data structure for the grass tilemap for example:
```json
"Grass": [256, 192, 4, [30, 188, 115]]
```
With `256, 192` representing the texture offset, `4` representing the tilemap variation count (these are placed to the right of the tilemap texture offset in a row), and `[30, 188, 115]` representing the RGB colour value of this tilemap when shown on the game minimap.
