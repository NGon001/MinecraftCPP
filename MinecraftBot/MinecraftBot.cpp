#include "API.hpp"
#include "thread"
API api;

class RawMaterials {
public:
    // Enum to define different raw materials
    enum MaterialType {
        RAW_COPPER,
        RAW_IRON,
        RAW_GOLD,
        coal,
        diamond,
        // Add more raw materials as needed
        MATERIAL_COUNT // This keeps track of the number of materials
    };

private:
    // Static map to hold names of raw materials
    static std::unordered_map<MaterialType, std::string> materialNames;
    // Static map to hold ore to raw material mappings
    static std::unordered_map<std::string, MaterialType> oreToRawMap;

public:
    // Static method to initialize material names and mappings
    static void initialize() {
        materialNames[RAW_COPPER] = "raw_copper";
        materialNames[RAW_IRON] = "raw_iron";
        materialNames[RAW_GOLD] = "raw_gold";
        materialNames[diamond] = "diamond";
        materialNames[coal] = "coal";
        // Initialize more materials as needed

        // Initialize mappings from ores to raw materials
        oreToRawMap["copper_ore"] = RAW_COPPER;
        oreToRawMap["iron_ore"] = RAW_IRON;
        oreToRawMap["gold_ore"] = RAW_GOLD;
        oreToRawMap["diamond_ore"] = diamond;
        oreToRawMap["coal_ore"] = coal;
        // Add more mappings as needed
    }

    // Static method to get the name of a material
    static std::string getName(MaterialType type) {
        if (materialNames.find(type) != materialNames.end()) {
            return materialNames[type];
        }
        return "unknown_material";
    }

    // Static method to check if the ore corresponds to a raw material
    static bool matchesRawMaterial(const std::string& oreName, std::string& rawMaterial) {
        auto it = oreToRawMap.find(oreName);
        if (it != oreToRawMap.end()) {
            rawMaterial = getName(it->second);
            return true;
        }
        return false;
    }
};
std::unordered_map<RawMaterials::MaterialType, std::string> RawMaterials::materialNames;
std::unordered_map<std::string, RawMaterials::MaterialType> RawMaterials::oreToRawMap;

// Function to check if an item is a tool
bool isTool(const std::string& itemName) {
    std::vector<std::string> toolIDs = {
        "wooden_pickaxe", "stone_pickaxe", "iron_pickaxe", "golden_pickaxe", "diamond_pickaxe", "netherite_pickaxe",
        "wooden_axe", "stone_axe", "iron_axe", "golden_axe", "diamond_axe", "netherite_axe",
        "wooden_shovel", "stone_shovel", "iron_shovel", "golden_shovel", "diamond_shovel", "netherite_shovel",
        "wooden_sword", "stone_sword", "iron_sword", "golden_sword", "diamond_sword", "netherite_sword"
    };

    // Check if the item name is in the toolIDs vector
    for (const auto& toolID : toolIDs) {
        if (toolID == itemName) {
            return true; // Item is a tool
        }
    }
    return false; // Item is not a tool
}

void Mine(int blockid) {
    
    std::vector<API::Vector3> positions;
    api.FindBlock(blockid, positions, 300);
    for (auto position : positions)
    {
        if (api.Navigate(position))
        {
            std::string message;
            std::string itemeuipname;
            std::vector<API::Inventory> inventory;
            api.GetInventory(inventory);
            api.equipped("hand", message, itemeuipname);
            if (message == "No item equipped in hand." || !itemeuipname.empty())
            {
                for (auto singleinv : inventory)
                {
                    if (itemeuipname.find("pickaxe") != std::string::npos)
                        break;
                    if (singleinv.item.name.find("pickaxe") != std::string::npos)
                    {
                        api.equip(singleinv.item.slot, "hand");
                        break;
                    }


                }
            }
            api.Mine(position);
        }          
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        }

    }
}
bool GetResource(std::string name, int count) //"copper_ore",10
{
    API::Player bot;
  
    api.GetPlayer("Bot3",bot);
    API::Vector3 botstartposition = bot.entity.position;
    int blockid;
    api.GetBlockId(name, blockid);
    std::string rawMaterial;
    // Check if the ore corresponds to a raw material
    if (RawMaterials::matchesRawMaterial(name, rawMaterial)) {
        std::cout << "The raw material for " << name << " is " << rawMaterial << std::endl;
    }
    std::string message;
    std::string itemeuipname;
    bool haveitem = false;
    while (!haveitem)
    {
        std::vector<API::Inventory> inventory;
        api.GetInventory(inventory);
        api.equipped("hand", message, itemeuipname);
        if (message == "No item equipped in hand." || !itemeuipname.empty())
        {
            for (auto singleinv : inventory)
            {
                if (itemeuipname.find("pickaxe") != std::string::npos)
                    break;
                if (singleinv.item.name.find("pickaxe") != std::string::npos)
                {
                    api.equip(singleinv.item.slot, "hand");
                    break;
                }


            }
        }
        for (auto singleinvitem : inventory)
        {
            if (rawMaterial.empty())
            {
                if (singleinvitem.item.name == name)
                {
                    if (singleinvitem.item.count >= count) { haveitem = true; break; }
                }
            }
            else
            {
                if (singleinvitem.item.name == rawMaterial)
                {
                    if (singleinvitem.item.count >= count) {
                        haveitem = true; 
                        break; 
                    }
                }
            }
        }
        if(!haveitem)
        Mine(blockid);
    }
    if (haveitem)
    {
        api.Navigate(bot.entity.position);
        while (true)
        {
            api.GetPlayer("Bot3", bot);
            if (bot.entity.position.x == botstartposition.x &&
                bot.entity.position.y == botstartposition.y &&
                bot.entity.position.z == botstartposition.z) {

                std::vector<API::Vector3> chestposition;
                int chestid;
                api.GetBlockId("chest", chestid);
                api.FindBlock(chestid, chestposition, 300);
                api.Navigate(chestposition[0]);
                std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                std::vector<API::Inventory> inventoryda;
                api.GetInventory(inventoryda);

                for (auto inv : inventoryda)
                {
                    if (!isTool(inv.item.name))
                    api.Deposite(chestposition[0], inv.item.type, inv.item.count,inv.item.slot);
                }
                return true;
            }

            // Check if the bot's position is within -1 or -2 blocks in X and Z
            if ((std::abs(bot.entity.position.x - botstartposition.x) <= 2 &&
                bot.entity.position.y == botstartposition.y &&
                std::abs(bot.entity.position.z - botstartposition.z) <= 2)) {

                std::vector<API::Vector3> chestposition;
                int chestid;
                api.GetBlockId("chest", chestid);
                api.FindBlock(chestid, chestposition,300);
                api.Navigate(chestposition[0]); 
                std::this_thread::sleep_for(std::chrono::milliseconds(3000));     
                std::vector<API::Inventory> inventoryda;
                api.GetInventory(inventoryda);

                for (auto inv : inventoryda)
                {
                   if(!isTool(inv.item.name))
                    api.Deposite(chestposition[0], inv.item.type,inv.item.count, inv.item.slot);
                }
               
                return true;
            }
        } 
    }
    return false;
}

int main() {
    RawMaterials::initialize();
    std::vector<API::Vector3> chestposition;
    int chestid;
    api.GetBlockId("chest", chestid);
    api.FindBlock(chestid, chestposition, 300);
    API::Chest chest;
    api.Navigate(chestposition[0]);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    //api.chestItemCount(chestposition[0], "raw_iron", chest);
    api.Withdraw(chestposition[0], "iron_ingot", 2);
    api.Withdraw(chestposition[0], "stick", 1);
    //api.itemTypeToName(23);
    std::vector<API::Vector3> trash;
    int craftingtable_blockid;
    api.GetBlockId("crafting_table", craftingtable_blockid);
    api.FindBlock(craftingtable_blockid, trash, 300);
    api.Navigate(trash[0]);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    int chestcrafting_id;
    api.GetItemId("iron_sword", chestcrafting_id);
    std::vector<API::ItemRecipe> recepe;
    api.GetReciepeIng(chestcrafting_id, trash[0], true, recepe);
    api.Craft(chestcrafting_id, trash[0],0, true);
   // api.GetReciepeIng("diamond_sword");

  /*  GetResource("coal_ore", 7); //ancient_debris
   // api.Navigate(1840, 67, -3255);
    GetResource("iron_ore", 50); //ancient_debris

    std::vector<API::Vector3> chestposition;
    std::vector<API::Vector3> furnanceposition;
    int chestid;
    api.GetBlockId("chest", chestid);
    api.FindBlock(chestid, chestposition, 300);
    API::Chest chest;
    api.Navigate(chestposition[0]);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    //api.chestItemCount(chestposition[0], "raw_iron", chest);
    api.Withdraw(chestposition[0], "raw_iron", 30);
    api.Withdraw(chestposition[0], "coal", 7);

    int furnaceID;
    api.GetBlockId("furnace", furnaceID);
    api.FindBlock(furnaceID, furnanceposition, 300);
    api.Navigate(furnanceposition[0]);
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    api.Smelt(furnanceposition[0],"raw_iron","coal",3);
    std::vector<API::Inventory> inventoryda;
    api.GetInventory(inventoryda);

    for (auto inv : inventoryda)
    {
        if (!isTool(inv.item.name))
            api.Deposite(chestposition[0], inv.item.type, inv.item.count, inv.item.slot);
    }
    
    return 0;*/
}
