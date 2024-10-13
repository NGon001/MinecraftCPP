const express = require('express');
const mineflayer = require('mineflayer');
const Vec3 = require('vec3'); // Import Vec3
const { pathfinder } = require('mineflayer-pathfinder');
const { GoalBlock } = require('mineflayer-pathfinder').goals; // Import the GoalBlock class
const mcData = require('minecraft-data');

const app = express();
app.use(express.json()); // Middleware to parse JSON request bodies

// Create the bot
const bot = mineflayer.createBot({
    host: 'localhost', // Your Minecraft server address
    port: 65477,       // Your Minecraft server port
    username: 'Bot3'   // Your bot's username
});


// Load the pathfinder plugin
bot.loadPlugin(pathfinder);



// Handle the spawn event
bot.on('spawn', () => {
    console.log('Bot has spawned');

    // Adjusting pathfinding settings globally
    bot.pathfinder.defaultOptions = {
        maxDistance: 64,
        maxRetries: 5
    };
});

// Function to check if the bot is close to the destination
const isCloseToDestination = (bot, destination, threshold = 2) => {
    return bot.entity.position.distanceTo(destination) < threshold;
};

app.post('/navigate', (req, res) => {
    const { x, y, z } = req.body; // Extract coordinates from the request body

    const targetPosition = new Vec3(x, y, z); // Create Vec3 object for target coordinates
    const goal = new GoalBlock(targetPosition.x, targetPosition.y, targetPosition.z, 1); // 1 block radius

    const isCloseToDestination = (bot, destination, threshold = 2) => {
        return bot.entity.position.distanceTo(destination) < threshold;
    };

    // Set the bot's pathfinding goal
    bot.pathfinder.setGoal(goal);

    const checkNavigationCompletion = () => {
        if (isCloseToDestination(bot, targetPosition)) {
            bot.pathfinder.setGoal(null); // Clear the goal after reaching the destination
            res.json({ status: 'success' }); // Send "success" when destination is reached
        } else if (!bot.pathfinder.isMoving()) {
            res.json({ status: 'error' }); // Send "error" if the bot is not moving or navigation fails
        } else {
            setTimeout(checkNavigationCompletion, 1000); // Check every second if the bot has stopped
        }
    };

    checkNavigationCompletion(); // Start checking if the bot reached the destination
});

app.post('/equip', (req, res) => {
    const { slot, type } = req.body; // Get the slot number and type (e.g., hand, head, torso, etc.)

    // Validate the slot number
    if (typeof slot !== 'number' || slot < 0 || slot > 44) { // Minecraft inventory has 45 slots (0-44)
        return res.json({
            status: 'error',
            message: 'Invalid slot number. Must be between 0 and 44.'
        });
    }

    const item = bot.inventory.slots[slot]; // Get the item in the specified slot

    if (!item) {
        return res.json({
            status: 'error',
            message: `No item found in slot ${slot}`
        });
    }

    // Check if type is valid (hand, head, torso, legs, feet)
    const validTypes = ['hand', 'head', 'torso', 'legs', 'feet'];
    if (!validTypes.includes(type)) {
        return res.json({
            status: 'error',
            message: `Invalid equip type. Must be one of ${validTypes.join(', ')}.`
        });
    }

    bot.equip(item, type) // Equip the item in the specified armor slot or hand
        .then(() => {
            res.json({
                status: 'success',
                message: `Item equipped in ${type} from slot ${slot}`,
                item: {
                    name: item.name,
                    displayName: item.displayName,
                    count: item.count
                }
            });
        })
        .catch(err => {
            res.json({
                status: 'error',
                message: `Failed to equip item: ${err.message}`
            });
        });
});

app.get('/equipped', (req, res) => {
    const { type } = req.query; // Get the type of equipment to check (hand, head, torso, legs, feet)
    console.log(type);
    // Check if type is valid (hand, head, torso, legs, feet)
    const validTypes = ['hand', 'head', 'torso', 'legs', 'feet'];
    if (!validTypes.includes(type)) {
        return res.json({
            status: 'error',
            message: `Invalid equip type. Must be one of ${validTypes.join(', ')}.`
        });
    }

    let equippedItem;

    // Check for the currently equipped item based on the specified type
    switch (type) {
        case 'hand':
            equippedItem = bot.heldItem; // Held item in hand
            break;
        case 'head':
            equippedItem = bot.inventory.slots[5]; // Slot for helmet (armor head)
            break;
        case 'torso':
            equippedItem = bot.inventory.slots[6]; // Slot for chestplate (armor torso)
            break;
        case 'legs':
            equippedItem = bot.inventory.slots[7]; // Slot for leggings (armor legs)
            break;
        case 'feet':
            equippedItem = bot.inventory.slots[8]; // Slot for boots (armor feet)
            break;
        default:
            return res.json({
                status: 'error',
                message: 'Invalid equip type.'
            });
    }

    // If there is no item equipped in the selected slot
    if (!equippedItem) {
        return res.json({
            status: 'success',
            message: `No item equipped in ${type}.`
        });
    }

    // If an item is equipped, return the item details
    res.json({
        status: 'success',
        message: `Item equipped in ${type}.`,
        item: {
            name: equippedItem.name,
            displayName: equippedItem.displayName,
            count: equippedItem.count
        }
    });
});

// Endpoint to get player info by username using the default bot
app.get('/player/:name', (req, res) => {
    const playerName = req.params.name; // Get the player name from the URL parameters
    console.log(playerName);
    const player = bot.players[playerName]; // Access the player from the default bot's players object

    if (player) {
        // If player is found, send the entire player object as a response
        res.json(player); // Respond with player information in JSON format
    } else {
        // If player is not found, send a 404 error
        res.status(404).send({ error: `Player ${playerName} not found.` });
    }
});

app.post('/mine', async (req, res) => {
    const { x, y, z } = req.body; // Extract coordinates from the request body

    // Create a Vec3 instance with the given coordinates
    const targetBlock = new Vec3(x, y, z);

    // Attempt to find the target block
    const blockToMine = bot.blockAt(targetBlock);

    // Check if the block exists and is mineable
    if (!blockToMine || blockToMine.type === 0) {
        console.log(`No block found at (${x}, ${y}, ${z}) or it is not mineable.`);
        return res.status(404).send({ error: `No mineable block found at (${x}, ${y}, ${z}).` });
    }

   

    // Mine the block
    try {
        console.log(`Mining block at (${x}, ${y}, ${z})...`);
        await bot.dig(blockToMine);
        console.log(`Successfully mined block at (${x}, ${y}, ${z}).`);

        // Wait for a short moment to ensure the item drops
        await new Promise(resolve => setTimeout(resolve, 500));

        // Find the dropped items in the world
        const droppedItems = Object.values(bot.entities).filter(entity =>
            entity.position.distanceTo(blockToMine.position.offset(0, -1, 0)) < 2 && // Check distance from the block
            entity.kind === 'Drops' // Check if the entity is an item
        );

        // Collect each dropped item
        for (const item of droppedItems) {
            await bot.collectBlock.collect(item);
            console.log(`Collected item: ${item.displayName}`);
        }

        res.send({ status: `Successfully mined block at (${x}, ${y}, ${z}).` });
    } catch (err) {
        console.log(`Error mining block at (${x}, ${y}, ${z}): ${err.message}`);
        res.status(500).send({ error: `Failed to mine block at (${x}, ${y}, ${z}): ${err.message}` });
    }
});


app.get('/find-block', async (req, res) => {
    const resourceId = parseInt(req.query.id); // Get the ID from the query string
    const maxDistance = parseInt(req.query.maxDistance) || 64; // Set max distance

    console.log(`Received request to find block with ID: ${resourceId} and max distance: ${maxDistance}`);

    // Log the bot's current position for reference
    const botPosition = bot.entity.position;
    console.log(`Bot position: (${botPosition.x}, ${botPosition.y}, ${botPosition.z})`);

    if (isNaN(resourceId)) {
        console.log(`Invalid resource ID: ${resourceId}`);
        return res.status(400).send({ error: 'Invalid resource ID' });
    }

    try {
        // Find nearby blocks
        const nearbyBlocks = bot.findBlocks({
            matching: resourceId,
            maxDistance: maxDistance
        });

        console.log(`Nearby blocks count: ${nearbyBlocks.length}`);

        if (nearbyBlocks.length > 0) {
            nearbyBlocks.forEach(pos => {
                const block = bot.blockAt(pos);
                console.log(`Found block at (${pos.x}, ${pos.y}, ${pos.z}) of type ${block.name}`);
            });
            res.send({ blocks: nearbyBlocks });
        } else {
            console.log('No blocks found with the given resource ID within the specified distance.');
            res.status(404).send({ error: 'Block not found' });
        }
    } catch (error) {
        console.error(`Error occurred while finding block: ${error.message}`);
        res.status(500).send({ error: `Server error: ${error.message}` });
    }
});

function dropItem(itemName, count) {
    const inventory = bot.inventory.items(); // Get the current items in the bot's inventory

    console.log("Current inventory:", inventory);

    // Find the item in inventory
    const itemInInventory = inventory.find(item => item.name === itemName);

    if (!itemInInventory) {
        return console.error(`1Error dropping item: ${err}`);
    }

    // Check if the bot has enough of the item to drop
    if (itemInInventory.count < count) {
        return console.error(`2Error dropping item: ${err}`);
    }

    // Drop the item
    bot.toss(itemInInventory.type, null, count, (err) => {
        if (err) {
            console.error(`Error dropping item: ${err}`);
        } else {
            console.log(`Dropped ${amountToDrop} of ${itemName}`);
        }
    });
}

// Endpoint to drop an item
app.post('/drop', (req, res) => {
    const { itemName, count } = req.body; // Extract itemName and count from request body


    dropItem(itemName, count); // Call the dropItem function
    res.send({ status: `Dropping ${count} of ${itemName} from the inventory.` });
});

// Function to check if an item is a tool
const isTool = (item) => {
    const toolIDs = [
        'wooden_pickaxe', 'stone_pickaxe', 'iron_pickaxe', 'golden_pickaxe', 'diamond_pickaxe', 'netherite_pickaxe',
        'wooden_axe', 'stone_axe', 'iron_axe', 'golden_axe', 'diamond_axe', 'netherite_axe',
        'wooden_shovel', 'stone_shovel', 'iron_shovel', 'golden_shovel', 'diamond_shovel', 'netherite_shovel',
        'wooden_sword', 'stone_sword', 'iron_sword', 'golden_sword', 'diamond_sword', 'netherite_sword'
    ];

    return toolIDs.includes(item.name);
};

// Function to face the chest (example)
async function faceChest(chestBlock) {
    const direction = new Vec3(0, 1, 0); // Default facing direction (up)
    bot.lookAt(chestBlock.position.offset(0.5, 0, 0.5), true); // Look towards the chest
}

function wait(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

const depositItemsIntoChest = async (chestPosition, itemType, itemCount) => {
    const chestVec3 = new Vec3(chestPosition.x, chestPosition.y, chestPosition.z);
    const chestBlock = bot.blockAt(chestVec3);
    console.log(`Deposit request received for chest at: ${JSON.stringify(chestBlock)}, itemtype: ${itemType}, itemcount: ${itemCount}`);

    // Ensure the bot is facing the chest
    // Ensure the bot is facing the chest
    await faceChest(chestBlock);

    let chest;
    try {
        // Open the chest and get the window reference
        chest = await bot.openChest(chestBlock);
        console.log(`Successfully opened the chest at ${JSON.stringify(chestBlock)}`);
    } catch (error) {
        console.error(`Error opening chest: ${error.message}`);
        return;
    }

    // Check if the chest window is valid
    if (!chest) {
        console.error('Failed to open chest or chest window is invalid.');
        return;
    }

    // Attempt to access the chest's inventory
    const chestInventory = chest.slots; // Accessing the slots directly
    console.log(`Chest inventory: ${JSON.stringify(chestInventory)}`);

    // Check if the bot has the item to deposit
    const itemInBotInventory = bot.inventory.items().find(item => item.type === itemType);
    if (!itemInBotInventory) {
        console.error(`Cannot find item type ${itemType} in bot inventory.`);
        return;
    }

    // Determine how many items to deposit
    const depositCount = Math.min(itemInBotInventory.count, itemCount);
    console.log(`Preparing to deposit ${depositCount} of item type ${itemType}.`);

    // Check for available slots in the chest
    const availableSlots = chestInventory.filter(slot => slot === null).length;
    console.log(`Available slots in chest: ${availableSlots}`);

    // Proceed with deposit if there are available slots
    if (availableSlots > 0) {
        // Set metadata to null to match anything
        const metadata = null; // or set this to a specific value if needed

        // Attempt to deposit the item
        try {
            await chest.deposit(itemType, metadata, depositCount, null); // Use null for NBT matching
            console.log(`Successfully deposited ${depositCount} of item type ${itemType} into the chest.`);
        } catch (error) {
            console.error(`Error depositing item: ${error.message}`);
        }
    } else {
        console.error('Cannot deposit item: destination full');
    }

    // Close the chest window after the operation
    await chest.close();
};

// Endpoint to deposit items into a chest
app.post('/deposit', (req, res) => {
    const { x, y, z, itemType, itemCount, itemSlotIndex } = req.body; // Get the chest coordinates from the request body
    const chestPosition = { x, y, z };
    console.log(`Deposit request received for chest at: ${JSON.stringify(chestPosition)} , itemtype: ${itemType},itemcount: ${itemCount}`); // Debug log

    
    depositItemsIntoChest(chestPosition, itemType, itemCount, itemSlotIndex)
        .then((results) => {
            res.status(200).json({
                status: 'success',
                message: 'Items deposited successfully.',
                results: results
            });
        })
        .catch((error) => {
            console.error('Deposit error:', error); // Log the error in detail
            res.status(500).json({
                status: 'error',
                message: error // Return the error message to the client
            });
        });
});

app.post('/lookAt', (req, res) => {
    const { x, y, z, force } = req.body;

    // Validate input
    if (typeof x !== 'number' || typeof y !== 'number' || typeof z !== 'number') {
        return res.status(400).send({ error: 'Invalid coordinates.' });
    }

    const point = new Vec3(x, y, z);
    bot.lookAt(point, force).then(() => {
        res.send({ status: `Bot is now looking at the specified point.` });
    }).catch(err => {
        console.error('Error setting look direction:', err);
        res.status(500).send({ error: 'Failed to set look direction.' });
    });
});

app.get('/block-id/:name', (req, res) => {
    const blockName = req.params.name;
    const blockId = bot.registry.blocksByName[blockName]?.id;

    if (blockId !== undefined) {
        res.send({ blockId });
    } else {
        res.status(404).send({ error: 'Block not found' });
    }
});

app.get('/inventory', (req, res) => {
    // Directly send the inventory items array as JSON
    res.send({ inventory: bot.inventory.items() });
});

// Start your server
app.listen(3960, () => {
    console.log('Server is running on port 3000');
});
