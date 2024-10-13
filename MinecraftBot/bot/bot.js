const express = require('express');
const app = express();
const mineflayer = require('mineflayer');
const Vec3 = require('vec3'); // Import Vec3
const { pathfinder, Movements } = require('mineflayer-pathfinder');
const { GoalBlock } = require('mineflayer-pathfinder').goals; // Import the GoalBlock class





// Define the Bot class
class Bot {
    constructor(name) {
        this.name = name;
        this.position = { x: 0, y: 0, z: 0 };
    }

    move(x, y, z) {
        this.position = { x, y, z };
        console.log(`${this.name} moved to position:`, this.position);
    }

    lookAt(x, y, z) {
        console.log(`${this.name} is looking at position: { x: ${x}, y: ${y}, z: ${z} }`);
        // Logic to set the bot's look direction would go here
    }
}

const bots = {}; // Object to hold created bots
let defaultBotName = null; // Variable to hold the default bot name

function createBot(botName) {
    const bot = mineflayer.createBot({
        host: 'localhost', // Your Minecraft server address
        port: 53012,        // Your Minecraft server port
        username: botName,
    });

    // Handle bot events, e.g., when the bot logs in
    bot.on('spawn', () => {
        console.log(`${bot.username} has spawned.`);
    });


    // Add additional event listeners as needed

    bots[botName] = bot; // Store bot in the bots object

    // If this is the first bot created, set it as the default bot
    if (!defaultBotName) {
        defaultBotName = botName;
    }
}

app.use(express.json()); // Middleware to parse JSON

// Valid control states
const validControls = ['forward', 'back', 'left', 'right', 'jump', 'sneak', 'sprint'];

// Function to log the bot's current position
function logBotPosition() {
    const botPosition = bot.entity.position;
    //console.log(`Bot's Current Position: X: ${botPosition.x}, Y: ${botPosition.y}, Z: ${botPosition.z}`);
}

// Function to move to a specific position
function goToPosition(botname,x, y, z) {
    const targetPosition = new Vec3(x, y, z);
    const bot = bots[botname]; // Access the correct bot by name from the bots object
    bot.loadPlugin(pathfinder);

    const movements = new Movements(bot);
    bot.pathfinder.setMovements(movements);
    bot.pathfinder.goto(new mineflayer.goals.GoalBlock(targetPosition.x, targetPosition.y, targetPosition.z))
        .then(() => {
            console.log('Bot has arrived at the destination.');
        })
        .catch(err => {
            console.log('Error during navigation:', err);
        });
}

// Endpoint to navigate to a specified location
app.post('/navigate/:name', (req, res) => {
    const { name } = req.params; // Extract bot name from request parameters
    const { x, y, z } = req.body; // Extract coordinates from the request body

    if (typeof x === 'number' && typeof y === 'number' && typeof z === 'number') {
        goToPosition(name,x, y, z); // Call the movement function
        res.send({ message: `Bot is moving to (${x}, ${y}, ${z})` });
    } else {
        res.status(400).send({ error: 'Invalid coordinates. Please provide x, y, and z as numbers.' });
    }
});

// Endpoint to create a new bot
app.post('/createBot', (req, res) => {
    const { botName } = req.body;

    if (!botName) {
        return res.status(400).send({ error: 'Bot name is required.' });
    }

    if (bots[botName]) {
        return res.status(400).send({ error: 'Bot with this name already exists.' });
    }

    bots[botName] = new Bot(botName); // Create a new instance of the Bot class
    createBot(botName);
    res.send({ status: `Bot ${botName} created.` });
});


// Function to drop an item from the inventory
function dropItem(botName, itemName, count) {
    if (!bots[botName]) {
        return console.log("daCurrent inventory:", inventory);
    }

    const bot = bots[botName];
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
    bot.toss(itemInInventory.type, null,count, (err) => {
        if (err) {
            console.error(`Error dropping item: ${err}`);
        } else {
            console.log(`Dropped ${amountToDrop} of ${itemName}`);
        }
    });
}

// Endpoint to drop an item
app.post('/drop/:name', (req, res) => {
    const { name } = req.params; // Extract bot name from request parameters
    const { itemName, count } = req.body; // Extract itemName and count from request body
    console.log(`name: ${name} itemname: ${itemName} count: ${count}`);

    if (!itemName || typeof count !== 'number' || count <= 0) {
        return res.status(400).send({ error: 'Invalid request. Please provide a valid item name and count.' });
    }

    dropItem(name,itemName, count); // Call the dropItem function
    res.send({ status: `Dropping ${count} of ${itemName} from the inventory.` });
});

app.get('/inventory/:name', (req, res) => {
    const { name } = req.params; // Extract bot name from request parameters
    const bot = bots[name]; // Access the correct bot by name from the bots object

    if (!bot) {
        return res.status(404).send({ error: `Bot ${name} not found.` });
    }

    // Directly send the inventory items array as JSON
    res.send({ inventory: bot.inventory.items() });
});

// API endpoint to move the bot
app.post('/move/:name', (req, res) => {
    const { name } = req.params; // Extract bot name from request parameters
    const direction = req.body.direction; // Extract direction from request body
    const duration = req.body.duration; // Extract duration from request body

    // Validate direction and duration
    if (!direction || !validControls.includes(direction)) {
        return res.status(400).send({ error: 'Invalid direction. Valid options are: ' + validControls.join(', ') });
    }

    if (typeof duration !== 'number' || duration <= 0) {
        return res.status(400).send({ error: 'Invalid duration. Duration must be a positive number.' });
    }

    const bot = bots[name]; // Get the bot instance by name
    if (!bot) {
        return res.status(404).send({ error: `Bot ${name} not found.` });
    }

    // Move the bot
    bot.setControlState(direction, true); // Start moving
    setTimeout(() => {
        bot.setControlState(direction, false); // Stop moving after 'duration'
    }, duration);

    // Send a success response
    res.send({ status: `Bot ${name} moving ${direction}` });
});

// Example endpoint to get bot's position
app.get('/bot/:name/position', (req, res) => {
    const botName = req.params.name; // Get the bot name from the URL parameters
    const bot = bots[botName]; // Get the bot instance by name

    if (!bot) {
        return res.status(404).send({ error: `Bot ${botName} not found.` });
    }

    const botPosition = bot.entity.position; // Get the bot's position
    res.send({ position: { x: botPosition.x, y: botPosition.y, z: botPosition.z } });
});

app.post('/lookAt/:name', (req, res) => {
    const { name } = req.params; // Extract bot name from request parameters
    const { x, y, z, force } = req.body;

    // Validate input
    if (typeof x !== 'number' || typeof y !== 'number' || typeof z !== 'number') {
        return res.status(400).send({ error: 'Invalid coordinates.' });
    }

    const bot = bots[name]; // Get the bot instance by name
    if (!bot) {
        return res.status(404).send({ error: `Bot ${name} not found.` });
    }

    const point = new Vec3(x, y, z);
    bot.lookAt(point, force).then(() => {
        res.send({ status: `Bot ${name} is now looking at the specified point.` });
    }).catch(err => {
        console.error('Error setting look direction:', err);
        res.status(500).send({ error: 'Failed to set look direction.' });
    });
});

app.get('/bots', (req, res) => {
    const botNames = Object.keys(bots); // Get an array of bot names
    res.json({ bots: botNames }); // Respond with the list of bots in JSON format
});

// Endpoint to get player info by username using the default bot
app.get('/player/:name', (req, res) => {
    if (!defaultBotName) {
        return res.status(500).send({ error: 'No bots available.' });
    }

    const playerName = req.params.name; // Get the player name from the URL parameters
    const player = bots[defaultBotName].players[playerName]; // Access the player from the default bot's players object

    if (player) {
        // If player is found, send the entire player object as a response
        res.json(player); // Respond with player information in JSON format
    } else {
        // If player is not found, send a 404 error
        res.status(404).send({ error: `Player ${playerName} not found.` });
    }
});

app.post('/look/:name', (req, res) => {
    const { name } = req.params; // Extract bot name from request parameters
    const { yaw, pitch } = req.body;

    // Validate input
    if (typeof yaw !== 'number' || typeof pitch !== 'number') {
        console.error('Invalid yaw or pitch values.');
        return res.status(400).send({ error: 'Invalid yaw or pitch values.' });
    }

    const bot = bots[name]; // Get the bot instance by name
    if (!bot) {
        return res.status(404).send({ error: `Bot ${name} not found.` });
    }

    bot.look(yaw, pitch, true).then(() => {
        res.send({ status: `Bot ${name} is now looking at the specified direction.` });
    }).catch(err => {
        console.error('Error setting look direction:', err);
        res.status(500).send({ error: 'Failed to set look direction.' });
    });
});



// Start the server
const PORT = 3960; // Port to listen on
app.listen(PORT, () => {
    console.log(`Server is running on port ${PORT}`);
});
