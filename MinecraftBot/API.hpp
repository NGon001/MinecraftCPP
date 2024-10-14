#include <string>
#include <iostream>
#include "nlohmann/json.hpp"

#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#define CURL_STATICLIB
#include <curl/curl.h>
#define M_PI   3.14159265358979323846264338327950288


class API {
private:  
    std::string base = "http://localhost:3960";
    std::string Move_url = base + "/move";
    std::string Look_url = base + "/look"; 
    std::string LookAt_url = base + "/lookAt";
    std::string player_url = base + "/player/";
    std::string bot_url = base + "/bot/";
    std::string getbots_url = base + "/bots";
    std::string createbot_url = base + "/createBot";
    std::string inventory_url = base + "/inventory";
    std::string dropItem_url = base + "/drop";
    std::string navigate_url = base + "/navigate";
    std::string gather_url = base + "/gather";
    std::string findblock_url = base + "/find-block";
    std::string blockId_url = base + "/block-id/";
    std::string itemId_url = base + "/item-id/";
    std::string mine_url = base + "/mine";
    std::string equipped_url = base + "/equipped";
    std::string equip_url = base + "/equip";
    std::string depositeItems_url = base + "/deposit";
    std::string withdrawItems_url = base + "/withdraw";
    std::string chestItemCount_url = base + "/chestItemCount";
    std::string smelt_url = base + "/smelt";
    std::string crafting_ingredients_url = base + "/crafting-ingredients";
    std::string crafting_place_url = base + "/crafting-place-items";
    std::string itemTypeToName_url = base + "/itemTypeToName";

    using json = nlohmann::json;

public:
    class Vector3 {
    public:
        double x, y, z;
    };
    class Chest {
    public:
    std::string ItemName;
    int itemCount;

    // Parsing function for Item from JSON
    static bool ParseFromJson(const nlohmann::json& jsonData, Chest& chest) {
        try {
            chest.ItemName = jsonData["itemName"].get<std::string>();
            chest.itemCount = jsonData["itemCount"].get<int>();
        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing Item JSON: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
    };

    class Player {
    public:
        std::string uuid;
        std::string username;
        std::string displayName;
        int gamemode;
        int ping;

        // Inline Equipment structure inside the Player class
        

        class Entity {
        public:
            double yaw;
            double pitch;
            bool onGround;
            double height;
            double width;
            Vector3 position;
            Vector3 velocity;

            Entity() : yaw(0), pitch(0), onGround(false), height(0), width(0) {
                position = { 0, 0, 0 };
                velocity = { 0, 0, 0 };
            }

            static bool ParseFromJson(const nlohmann::json& jsonData, Entity& entity) {
                try {
                    // Initialize entity fields from JSON
                    entity.position.x = jsonData["position"]["x"].get<double>();
                    entity.position.y = jsonData["position"]["y"].get<double>();
                    entity.position.z = jsonData["position"]["z"].get<double>();

                    entity.velocity.x = jsonData["velocity"]["x"].get<double>();
                    entity.velocity.y = jsonData["velocity"]["y"].get<double>();
                    entity.velocity.z = jsonData["velocity"]["z"].get<double>();

                    entity.yaw = jsonData["yaw"].get<double>();
                    entity.pitch = jsonData["pitch"].get<double>();
                    entity.onGround = jsonData["onGround"].get<bool>();
                    entity.height = jsonData["height"].get<double>();
                    entity.width = jsonData["width"].get<double>();
                }
                catch (const std::exception& e) {
                    std::cerr << "Error parsing Entity JSON: " << e.what() << std::endl;
                    return false;
                }
                return true;
            }
        } entity;

        static bool ParseFromJson(const std::string& jsonResponse, Player& player) {
            try {
                // Parse the JSON response
                nlohmann::json jsonResponseParsed = nlohmann::json::parse(jsonResponse);

                // Extract player information from JSON
                player.uuid = jsonResponseParsed["uuid"].get<std::string>();
                player.username = jsonResponseParsed["username"].get<std::string>();
                player.displayName = jsonResponseParsed["displayName"]["json"]["extra"][0]["text"].get<std::string>(); // Assuming the structure is always the same
                player.gamemode = jsonResponseParsed["gamemode"].get<int>();
                player.ping = jsonResponseParsed["ping"].get<int>();

                // Initialize the entity object from the JSON
                if (!Entity::ParseFromJson(jsonResponseParsed["entity"], player.entity)) {
                    return false; // Return false if parsing the entity fails
                }

            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing Player JSON: " << e.what() << std::endl;
                return false;
            }
            return true;
        }
    };

    class Inventory {
    public:
        class Item {
        public:
            int type;                  // Represents the item type ID
            int count;                 // Number of items in this stack
            int metadata;              // Metadata of the item (like damage value)
            std::string name;          // Internal name of the item
            std::string displayName;   // The display name of the item
            int stackSize;             // Max stack size for the item
            int maxDurability;         // Maximum durability (if applicable)
            int slot;                  // The inventory slot the item occupies

            // Parsing function for Item from JSON
            static bool ParseFromJson(const nlohmann::json& jsonData, Item& item) {
                try {
                    item.type = jsonData["type"].get<int>();
                    item.count = jsonData["count"].get<int>();
                    item.metadata = jsonData["metadata"].get<int>();
                    item.name = jsonData["name"].get<std::string>();
                    item.displayName = jsonData["displayName"].get<std::string>();
                    item.stackSize = jsonData["stackSize"].get<int>();
                    if (jsonData.contains("maxDurability")) {
                        item.maxDurability = jsonData["maxDurability"].get<int>();
                    }
                    else {
                        item.maxDurability = -1; // Set a default value, e.g., -1 for items without durability
                    }
                    item.slot = jsonData["slot"].get<int>();
                }
                catch (const std::exception& e) {
                    std::cerr << "Error parsing Item JSON: " << e.what() << std::endl;
                    return false;
                }
                return true;
            }
        };
        Item item;

        static bool ParseFromJson(std::string jsonData, std::vector<Inventory>& inventory) {
            nlohmann::json jsonResponseParsed = nlohmann::json::parse(jsonData);
            try {
                if (jsonResponseParsed["inventory"].is_array()) {
                    for (const auto& item : jsonResponseParsed["inventory"]) {
                        if (item != nullptr) { // Check for null before creating Equipment
                            Item itemda;           
                            Item::ParseFromJson(item,itemda);
                            Inventory da;
                            da.item = itemda;
                            inventory.push_back(da);
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing Entity JSON: " << e.what() << std::endl;
                return false;
            }
            return true;
        }
    };

    class ItemRecipe {
    public:
        // Nested Item class
        class Item {
        public:
            int id;
            int count;
            std::nullptr_t metadata;

            Item(int id, int count, std::nullptr_t metadata = nullptr)
                : id(id), count(count), metadata(metadata) {}
        };

        int recipeId;
        Item result;
        std::string ingredients;
        std::vector<std::vector<Item>> inShape;        // 2D array for the recipe shape
        std::vector<std::vector<int>> inShapeIds;      // 2D array for item IDs
        std::vector<Item> delta;
        bool requiresTable;

        // Default constructor
        ItemRecipe()
            : recipeId(0), result(Item(0, 0)), ingredients(""),
            inShape(), inShapeIds(), delta(), requiresTable(false) {}

        // Parameterized constructor
        ItemRecipe(int recipeId, const Item& result, const std::string& ingredients,
            const std::vector<std::vector<Item>>& inShape,
            const std::vector<Item>& delta, bool requiresTable)
            : recipeId(recipeId), result(result), ingredients(ingredients),
            inShape(inShape), delta(delta), requiresTable(requiresTable) {

            // Populate inShapeIds based on inShape
            for (const auto& row : inShape) {
                std::vector<int> idRow;
                for (const auto& item : row) {
                    idRow.push_back(item.id);
                }
                inShapeIds.push_back(idRow);
            }
        }

        // Deserialize from JSON (for a single recipe)
        static ItemRecipe from_json(const json& j) {
            int recipeId = j.at("recipeId").get<int>();
            Item result(j.at("result").at("id").get<int>(), j.at("result").at("count").get<int>());
            std::string ingredients = j.at("ingredients").get<std::string>();

            // Parse inShape
            std::vector<std::vector<Item>> inShape;
            for (const auto& row : j.at("inShape")) {
                std::vector<Item> rowItems;
                for (const auto& item : row) {
                    rowItems.emplace_back(item.at("id").get<int>(), item.at("count").get<int>());
                }
                inShape.push_back(rowItems);
            }

            // Populate inShapeIds based on inShape
            std::vector<std::vector<int>> inShapeIds;
            for (const auto& row : inShape) {
                std::vector<int> idRow;
                for (const auto& item : row) {
                    idRow.push_back(item.id);
                }
                inShapeIds.push_back(idRow);
            }

            // Parse delta
            std::vector<Item> delta;
            for (const auto& item : j.at("delta")) {
                delta.emplace_back(item.at("id").get<int>(), item.at("count").get<int>());
            }

            bool requiresTable = j.at("requiresTable").get<bool>();

            ItemRecipe recipe(recipeId, result, ingredients, inShape, delta, requiresTable);
            recipe.inShapeIds = inShapeIds; // Set inShapeIds after populating

            return recipe;
        }

        // Deserialize from JSON (for an array of recipes)
        static std::vector<ItemRecipe> from_json_array(const json& j) {
            std::vector<ItemRecipe> recipes;
            for (const auto& item : j) {
                recipes.push_back(from_json(item));
            }
            return recipes;
        }
    };


    // Implementations are included within the class definition
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t total_size = size * nmemb;
        output->append(reinterpret_cast<char*>(contents), total_size);
        return total_size;
    }

    static size_t HeaderCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        std::string header((char*)contents, size * nmemb);
        std::string* headerData = static_cast<std::string*>(userp);

        // Check for X-SECURITY-TOKEN
        if (header.find("X-SECURITY-TOKEN:") == 0) {
            size_t pos = header.find(":");
            if (pos != std::string::npos) {
                std::string token = header.substr(pos + 1);
                token.erase(std::remove(token.begin(), token.end(), ' '), token.end()); // Trim whitespace
                *headerData += "X-SECURITY-TOKEN:" + token + "\n";
                return size * nmemb;
            }
        }

        // Check for CST
        if (header.find("CST:") == 0) {
            size_t pos = header.find(":");
            if (pos != std::string::npos) {
                std::string token = header.substr(pos + 1);
                token.erase(std::remove(token.begin(), token.end(), ' '), token.end()); // Trim whitespace
                *headerData += "CST:" + token + "\n";
                return size * nmemb;
            }
        }

        return size * nmemb;
    }

    bool CurlIn(CURL*& curl)
    {
        CURLcode res;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (!curl)
            return 0;
        return 1;
    }

    bool CurlClean(CURL*& curl, struct curl_slist*& headers)
    {
        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 1;
    }

    bool CurlReq(const std::string& url, const std::string& postFields, curl_slist* headers, std::string& response, std::string& headerData, const std::string& requestType) {

        CURL* curl;
        if (!CurlIn(curl)) return 0;

        if (curl == nullptr) {
            std::cerr << "Invalid CURL handle" << std::endl;
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        if (headers) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);

        // Set timeouts
      //  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L); // Timeout for the entire request
       // curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 120L); // Timeout for connection

        if (requestType == "POST") {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        }
        else if (requestType == "GET") {
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        }
        else if (requestType == "DELETE") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        }
        else if (requestType == "PUT") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        }
        else {
            std::cerr << "Unsupported request type: " << requestType << std::endl;
            CurlClean(curl, headers);
            return false;
        }

        // Enable verbose output for debugging
       // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            CurlClean(curl, headers);
            return false;
        }

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        //  std::cout << "HTTP Response Code: " << response_code << std::endl;

        CurlClean(curl, headers);
        return true;
    }


    bool Navigate(Vector3 position)
    {
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string postFields = "{\"x\": " + std::to_string(position.x) + ", \"y\": " + std::to_string(position.y) + ", \"z\": " + std::to_string(position.z) + "}";

        if (CurlReq(navigate_url, postFields, headers, response, headerData, "POST")) {
            nlohmann::json jsonResponseParsed = nlohmann::json::parse(response);
            if (jsonResponseParsed["status"].get<std::string>() == "success")
                return true;
            if (jsonResponseParsed.contains("error"))
                return false;
        }
        return false;
    }

    bool GetInventory(std::vector<Inventory>& inventory)
    {
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        if (CurlReq(inventory_url, "", headers, response, headerData, "GET")) {
            if (Inventory::ParseFromJson(response, inventory)) {
                return true;
            }
            else {
                std::cerr << "Failed to parse player JSON." << std::endl;
                return false;
            }

        }
        return false;
    }

    bool GetBlockId(std::string name,int& blockID)
    {
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        if (CurlReq(blockId_url + name, "", headers, response, headerData, "GET")) {

            nlohmann::json jsonResponseParsed = nlohmann::json::parse(response);
            if(jsonResponseParsed.contains("blockId"))
            {
                blockID = jsonResponseParsed["blockId"].get<int>();
                return true;
            }
        }
        return false;
    } 
    bool GetItemId(std::string name,int& blockID)
    {
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        if (CurlReq(itemId_url + name, "", headers, response, headerData, "GET")) {

            nlohmann::json jsonResponseParsed = nlohmann::json::parse(response);
            if(jsonResponseParsed.contains("item"))
            {
                blockID = jsonResponseParsed["item"].get<int>();
                return true;
            }
        }
        return false;
    }
    bool FindBlock(int resourceID, std::vector<Vector3>& positions, int maxdistance = 64)
    {
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        std::string url = findblock_url + "?id="+std::to_string(resourceID)+"&maxDistance=" + std::to_string(maxdistance);

        if (CurlReq(url, "", headers, response, headerData, "GET")) {
            nlohmann::json jsonResponseParsed = nlohmann::json::parse(response);
            if(jsonResponseParsed.contains("error"))
                return false;
            if (jsonResponseParsed.contains("blocks") && jsonResponseParsed["blocks"].is_array())
            {
                for (const auto& item : jsonResponseParsed["blocks"]) {
                    if (item != nullptr) { // Check for null before creating Equipment
                        Vector3 pos;
                        pos.x = item["x"];
                        pos.y = item["y"];
                        pos.z = item["z"];
                        positions.push_back(pos);
                    }
                }
                return true;
            }
        }
        return false;
    }

    bool Mine(Vector3 position)
    {
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string postFields = "{\"x\": " + std::to_string(position.x) + ", \"y\": " + std::to_string(position.y) + ", \"z\": " + std::to_string(position.z) + "}";

        if (CurlReq(mine_url, postFields, headers, response, headerData, "POST")) {
            return true;
        }
        return false;
    }

    bool equip(int slot,std::string type)
    {
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string postFields = "{\"slot\": " + std::to_string(slot) + ", \"type\": \"" + type + "\"}";

        if (CurlReq(equip_url, postFields, headers, response, headerData, "POST")) {
            return true;
        }
        return false;
    }
    bool equipped(std::string type,std::string& message,std::string& itemname) //    const validTypes = ['hand', 'head', 'torso', 'legs', 'feet'];
    {
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (CurlReq(equipped_url + "?type=" + type, "", headers, response, headerData, "GET")) {
            nlohmann::json jsonResponseParsed = nlohmann::json::parse(response);
            message = jsonResponseParsed["message"].get<std::string>();
            if (message == "Item equipped in hand.")
            {
                if (jsonResponseParsed.contains("item"))
                {
                     itemname = jsonResponseParsed["item"]["name"];
                }
                
            }
            return true;
        }
        return false;
    }
    bool DropItem(std::string itemName, int count)
    {
        if (count > 63)
            count = 63;
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string postFields = "{\"itemName\": \"" + itemName + "\", \"count\": " + std::to_string(count) + "}";

        if (CurlReq(dropItem_url, postFields, headers, response, headerData, "POST")) {
            return true;
        }
        return false;
    }
    bool GetPlayer(const std::string& playerName, Player& player)
    {
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;
        std::string url = player_url + playerName; // Construct the URL
        headers = curl_slist_append(headers, "Content-Type: application/json");

        if (CurlReq(url, "", headers, response, headerData, "GET")) {
            if (Player::ParseFromJson(response, player)) {
            }
            else {
                std::cerr << "Failed to parse player JSON." << std::endl;
            }
            //std::cout << "Player Position Response: " << response << std::endl;
            return true;
        }
        return false;
    }
    
    bool Deposite(Vector3 position,int itemtype,int count,int itemSlotIndex)
    {
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string postFields = "{\"x\": " + std::to_string(position.x) + ", \"y\": " + std::to_string(position.y) + ", \"z\": " + std::to_string(position.z) + ",\"itemType\": " + std::to_string(itemtype) + ",\"itemCount\": " + std::to_string(count) + ",\"itemSlotIndex\": " + std::to_string(itemSlotIndex) +"}";
        if (CurlReq(depositeItems_url, postFields, headers, response, headerData, "POST")) {
            
            return true;
        }
        return false;
    }
    bool Withdraw(Vector3 position, std::string ItemName, int count)
    {
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string postFields = "{\"x\": " + std::to_string(position.x) +
            ", \"y\": " + std::to_string(position.y) +
            ", \"z\": " + std::to_string(position.z) +
            ", \"itemName\": \"" + ItemName + "\"" +  // Add quotes around string fields
            ", \"quantity\": " + std::to_string(count) +
            "}";
        if (CurlReq(withdrawItems_url, postFields, headers, response, headerData, "POST")) {

            return true;
        }
        return false;
    }
    bool chestItemCount(Vector3 position, std::string ItemName,Chest& chest)
    {
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string postFields = "{\"x\": " + std::to_string(position.x) +
            ", \"y\": " + std::to_string(position.y) +
            ", \"z\": " + std::to_string(position.z) +
            ", \"itemName\": \"" + ItemName + "\"" +  // Add quotes around string fields 
            "}";
        if (CurlReq(chestItemCount_url, postFields, headers, response, headerData, "POST")) {
            nlohmann::json jsonResponseParsed = nlohmann::json::parse(response);
            Chest::ParseFromJson(jsonResponseParsed,chest);
            return true;
        }
        return false;
    }
    bool Smelt(Vector3 position, std::string smeltItemName, std::string fuelItem,int smeltCount)
    {
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string postFields = "{\"x\": " + std::to_string(position.x) +
            ", \"y\": " + std::to_string(position.y) +
            ", \"z\": " + std::to_string(position.z) +
            ", \"itemName\": \"" + smeltItemName + "\"" +  // Add quotes around string fields 
            ", \"fuelName\": \"" + fuelItem + "\"" +  // Add quotes around string fields 
            ", \"count\": \"" + std::to_string(smeltCount) + "\"" +  // Add quotes around string fields 
            "}";
        if (CurlReq(smelt_url, postFields, headers, response, headerData, "POST")) {
            return true;
        }
        return false;
    }
    bool LookAtPoint(Vector3 position, bool force) {
        std::string response;        // Variable to hold the response body
        std::string headerData;      // Variable to hold the response headers
        struct curl_slist* headers = NULL; // Pointer for request headers

        // Construct the JSON string dynamically
        std::string postFields = "{\"x\": " + std::to_string(position.x) +
            ", \"y\": " + std::to_string(position.y) +
            ", \"z\": " + std::to_string(position.z) +
            ", \"force\": " + (force ? "true" : "false") + "}";

        // Log the constructed JSON for debugging
       // std::cout << "Constructed JSON for looking at point: " << postFields << std::endl;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // Call CurlReq to make the HTTP request
        if (CurlReq(LookAt_url, postFields, headers, response, headerData, "POST")) {
            return true; // Indicate success
        }
        else {
            std::cerr << "Request failed." << std::endl;
            return false; // Indicate failure
        }
    }
    bool GetReciepeIng(int ItemType,Vector3 craftingTableLocation,bool useCraftingTable, std::vector<ItemRecipe>& itemRecipe)
    {
        std::string response;        // Variable to hold the response body
        std::string headerData;      // Variable to hold the response headers
        struct curl_slist* headers = NULL; // Pointer for request headers

        std::string postFields = "{\"itemType\": " + std::to_string(ItemType) +
            ", \"craftingTableLocation\": {\"x\": " + std::to_string(craftingTableLocation.x) +
            ", \"y\": " + std::to_string(craftingTableLocation.y) +
            ", \"z\": " + std::to_string(craftingTableLocation.z) +
            "}, \"useCraftingTable\": " + (useCraftingTable ? "true" : "false") + "}";

        // Log the constructed JSON for debugging
       // std::cout << "Constructed JSON for looking at point: " << postFields << std::endl;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // Call CurlReq to make the HTTP request
        if (CurlReq(crafting_ingredients_url, postFields, headers, response, headerData, "POST")) {
            nlohmann::json jsonResponseParsed = nlohmann::json::parse(response);
            
            if (jsonResponseParsed.contains("recipes"))
            {        
                itemRecipe = ItemRecipe::from_json_array(jsonResponseParsed["recipes"]);
            }
            return true; // Indicate success
        }
        else {
            std::cerr << "Request failed." << std::endl;
            return false; // Indicate failure
        }
    }

    bool Craft(int itemType,Vector3 craftingTableLocation, int recipeIndex,bool useCraftingTable)
    {
        std::string postFields = "{\"itemType\": " + std::to_string(itemType) +
            ", \"craftingTableLocation\": {\"x\": " + std::to_string(craftingTableLocation.x) +
            ", \"y\": " + std::to_string(craftingTableLocation.y) +
            ", \"z\": " + std::to_string(craftingTableLocation.z) + "}," +
            "\"recipeIndex\": " + std::to_string(recipeIndex) +
            ", \"useCraftingTable\": " + (useCraftingTable ? "true" : "false") + "}";

            std::string response;        // Variable to hold the response body
            std::string headerData;      // Variable to hold the response headers
            struct curl_slist* headers = NULL; // Pointer for request headers
            headers = curl_slist_append(headers, "Content-Type: application/json");
            if (CurlReq(crafting_place_url, postFields, headers, response, headerData, "POST")) {
                std::cout << response << std::endl;
                return true;
            }

            return false;
    }

    bool itemTypeToName(int itemType)
    {
        std::string response;        // Variable to hold the response body
        std::string headerData;      // Variable to hold the response headers
        struct curl_slist* headers = NULL; // Pointer for request headers
        headers = curl_slist_append(headers, "Content-Type: application/json");

        std::string postFields = "{\"itemType\": " + std::to_string(itemType)  + "}";

        if (CurlReq(itemTypeToName_url, postFields, headers, response, headerData, "POST")) {
            return true;
        }

        return false;
    }
};