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
        // Initialize more materials as needed

        // Initialize mappings from ores to raw materials
        oreToRawMap["copper_ore"] = RAW_COPPER;
        oreToRawMap["iron_ore"] = RAW_IRON;
        oreToRawMap["gold_ore"] = RAW_GOLD;
        oreToRawMap["diamond_ore"] = diamond;
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

void Mine(int blockid) {
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
    std::vector<API::Vector3> positions;
    api.FindBlock(blockid, positions);
    for (auto position : positions)
    {
        if (api.Navigate(position))
            api.Mine(position);
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
                api.FindBlock(chestid, chestposition);
                api.Navigate(chestposition[0]);
                std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                std::vector<API::Inventory> inventoryda;
                api.GetInventory(inventoryda);

                for (auto inv : inventoryda)
                {
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
                api.FindBlock(chestid, chestposition);
                api.Navigate(chestposition[0]); 
                std::this_thread::sleep_for(std::chrono::milliseconds(3000));     
                std::vector<API::Inventory> inventoryda;
                api.GetInventory(inventoryda);

                for (auto inv : inventoryda)
                {
                   
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

   // api.Navigate(1840, 67, -3255);
    GetResource("diamond_ore", 50); //ancient_debris

    
    return 0;
}
