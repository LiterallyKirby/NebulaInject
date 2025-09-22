#include "Mapping.h"
#include <unordered_map>
#include <string>
#include <string_view>

static std::unordered_map<std::string_view, std::string_view> g_Mappings;

std::map<std::string, std::shared_ptr<CM>> lookup;
std::mutex lookup_mutex;

CM* Mapping::getClass(const char* key) {
    if (!key) {
        std::cerr << "[Mapping ERROR] Null key provided to getClass" << std::endl;
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(lookup_mutex);
    std::string k(key);
    
    // Use safe find instead of at()
    auto it = lookup.find(k);
    if (it == lookup.end()) {
        std::cerr << "[Mapping ERROR] Class not found in lookup: " << k << std::endl;
        return nullptr;
    }
    return it->second.get(); // Return raw pointer from shared_ptr
}

const char* Mapping::getClassName(const char* key) {
    CM* cm = getClass(key);
    return cm ? cm->name : nullptr;
}

// Helper function to create CM objects with fields and methods
std::shared_ptr<CM> createCM(const char* className, 
                           const std::map<std::string, std::pair<std::string, std::string>>& fields,
                           const std::map<std::string, std::pair<std::string, std::string>>& methods) {
    auto cm = std::make_shared<CM>(className);
    
    // Add fields
    for (const auto& field : fields) {
        cm->fields[field.first] = Mem(field.second.first.c_str(), field.second.second.c_str(), false);
    }
    
    // Add methods
    for (const auto& method : methods) {
        cm->methods[method.first] = Mem(method.second.first.c_str(), method.second.second.c_str(), false);
    }
    
    return cm;
}

void Mapping::Initialize(const GameVersions version)
{
    // Clear any previous mappings
    g_Mappings.clear();
    {
        std::lock_guard<std::mutex> lock(lookup_mutex);
        lookup.clear();
    }

    switch (version)
    {
    case CASUAL_1_7_10: {
        // Initialize g_Mappings for backward compatibility
        g_Mappings = {
            {"net/minecraft/client/Minecraft", "bao"},
            {"net/minecraft/client/entity/EntityClientPlayerMP", "bjk"},
            {"net/minecraft/client/multiplayer/WorldClient", "bjf"},
            {"net/minecraft/client/gui/GuiScreen", "bdw"},
            {"net/minecraft/client/renderer/entity/RenderManager", "bnn"},
            { "net/minecraft/entity/Entity", "sa" },
            {"net/minecraft/client/renderer/entity/Render", "bno"},
            {"net/minecraft/client/model/ModelBase", "bhr"},
            {"net/minecraft/client/renderer/ActiveRenderInfo", "baj"},
            { "net/minecraft/util/IChatComponent", "fj" },
            {"net/minecraft/client/model/ModelRenderer", "bix"},
            {"net/minecraft/util/Timer", "bbr"},
            {"net/minecraft/client/gui/Gui", "bbw"},
            {"net/minecraft/client/gui/FontRenderer", "bbu"},
            { "net/minecraft/util/AxisAlignedBB", "azt" },
            {"net/minecraft/client/gui/GuiChat", "bct"},
            {"net/minecraft/client/renderer/entity/RendererLivingEntity", "boh"},
            { "net/minecraft/item/ItemStack", "add" },
            { "net/minecraft/item/ItemSword", "aeh" },
            { "net/minecraft/item/ItemAxe", "abf" },
            { "net/minecraft/item/Item", "adb" },
            { "net/minecraft/item/ItemBlock", "abh" },
            {"net/minecraft/util/MovingObjectPosition$MovingObjectType", "azv"},
            {"net/minecraft/util/MovingObjectPosition", "azu"},
            {"net/minecraft/client/gui/inventory/GuiInventory", "bfu"},
            {"net/minecraft/entity/player/InventoryPlayer", "yx"},
            {"net/minecraft/item/ItemEnderPearl", "aco"},
            { "net/minecraft/block/Block", "aji" },
            {"net/minecraft/client/settings/GameSettings", "bbj"},
            { "net/minecraft/client/settings/KeyBinding", "bal" },
            {"net/minecraft/block/BlockAir", "aja"}
        };

        std::lock_guard<std::mutex> lock(lookup_mutex);
        
        // Minecraft class - Complete mapping
        lookup["net/minecraft/client/Minecraft"] = createCM("bao", {
            {"thePlayer", {"h", "Lbjk;"}},
            {"theWorld", {"f", "Lbjf;"}},
            {"gameSettings", {"u", "Lbbj;"}},
            {"currentScreen", {"n", "Lbdw;"}},
            {"objectMouseOver", {"t", "Lazu;"}},
            {"timer", {"Q", "Lbbr;"}},
            {"displayWidth", {"d", "I"}},
            {"displayHeight", {"e", "I"}},
            {"fontRendererObj", {"l", "Lbbu;"}}
        }, {
            {"theMinecraft", {"M", "()Lbao;"}}
        });

        // EntityClientPlayerMP class - Complete mapping
        lookup["net/minecraft/client/entity/EntityClientPlayerMP"] = createCM("bjk", {
            {"rotationPitch", {"z", "F"}},
            {"rotationYaw", {"y", "F"}},
            {"prevRotationYaw", {"A", "F"}},
            {"prevRotationPitch", {"B", "F"}},
            {"posX", {"s", "D"}},
            {"posY", {"t", "D"}},
            {"posZ", {"u", "D"}},
            {"motionX", {"v", "D"}},
            {"motionY", {"w", "D"}},
            {"motionZ", {"x", "D"}},
            {"maxHurtResistantTime", {"aH", "I"}},
            {"hurtResistantTime", {"ad", "I"}},
            {"lastTickPosX", {"S", "D"}},
            {"lastTickPosY", {"T", "D"}},
            {"lastTickPosZ", {"U", "D"}},
            {"prevPosX", {"p", "D"}},
            {"prevPosY", {"q", "D"}},
            {"prevPosZ", {"r", "D"}},
            {"isSneaking", {"an", "Z"}},
            {"rotationYawHead", {"aO", "F"}},
            {"onGround", {"D", "Z"}},
            {"inWater", {"ac", "Z"}},
            {"moveForward", {"be", "F"}},
            {"moveStrafing", {"bd", "F"}},
            {"inventory", {"bm", "Lyx;"}}
        }, {
            {"getHealth", {"aS", "()F"}},
            {"getHeldItem", {"be", "()Ladd;"}},
            {"isInvisible", {"ap", "()Z"}}
        });

        // WorldClient class
        lookup["net/minecraft/client/multiplayer/WorldClient"] = createCM("bjf", {
            {"playerEntities", {"h", "Ljava/util/List;"}}
        }, {
            {"getBlock", {"a", "(III)Laji;"}}
        });

        // Entity class
        lookup["net/minecraft/entity/Entity"] = createCM("sa", {
            {"posX", {"s", "D"}},
            {"posY", {"t", "D"}},
            {"posZ", {"u", "D"}},
            {"motionX", {"v", "D"}},
            {"motionY", {"w", "D"}},
            {"motionZ", {"x", "D"}},
            {"rotationPitch", {"z", "F"}},
            {"rotationYaw", {"y", "F"}},
            {"prevRotationYaw", {"A", "F"}},
            {"prevRotationPitch", {"B", "F"}},
            {"boundingBox", {"C", "Lazt;"}},
            {"onGround", {"D", "Z"}},
            {"maxHurtResistantTime", {"aH", "I"}},
            {"hurtResistantTime", {"ad", "I"}},
            {"prevPosX", {"p", "D"}},
            {"prevPosY", {"q", "D"}},
            {"prevPosZ", {"r", "D"}}
        }, {
            {"setAlwaysRenderNameTag", {"g", "(Z)V"}}
        });

        // RenderManager class
        lookup["net/minecraft/client/renderer/entity/RenderManager"] = createCM("bnn", {
            {"renderPosX", {"b", "D"}},
            {"renderPosY", {"c", "D"}},
            {"renderPosZ", {"d", "D"}},
            {"viewerPosX", {"m", "D"}},
            {"viewerPosY", {"n", "D"}},
            {"viewerPosZ", {"o", "D"}}
        }, {
            {"getEntityRenderObject", {"a", "(Lsa;)Lbno;"}}
        });

        // ActiveRenderInfo class
        lookup["net/minecraft/client/renderer/ActiveRenderInfo"] = createCM("baj", {
            {"PROJECTION", {"k", "[F"}},
            {"MODELVIEW", {"j", "[F"}}
        }, {});

        // Timer class
        lookup["net/minecraft/util/Timer"] = createCM("bbr", {
            {"renderPartialTicks", {"c", "F"}},
            {"timerSpeed", {"d", "F"}}
        }, {});

        // AxisAlignedBB class
        lookup["net/minecraft/util/AxisAlignedBB"] = createCM("azt", {
            {"minX", {"a", "D"}},
            {"minY", {"b", "D"}},
            {"minZ", {"c", "D"}},
            {"maxX", {"d", "D"}},
            {"maxY", {"e", "D"}},
            {"maxZ", {"f", "D"}}
        }, {});

        // FontRenderer class
        lookup["net/minecraft/client/gui/FontRenderer"] = createCM("bbu", {}, {
            {"getStringWidth", {"a", "(Ljava/lang/String;)I"}},
            {"drawString", {"a", "(Ljava/lang/String;III)I"}}
        });

        // Gui class
        lookup["net/minecraft/client/gui/Gui"] = createCM("bbw", {}, {
            {"drawRect", {"a", "(IIIII)V"}}
        });

        // ItemStack class
        lookup["net/minecraft/item/ItemStack"] = createCM("add", {
            {"item", {"e", "Ladb;"}},
            {"metadata", {"f", "I"}}
        }, {
            {"getItem", {"b", "()Ladb;"}}
        });

        // MovingObjectPosition class
        lookup["net/minecraft/util/MovingObjectPosition"] = createCM("azu", {
            {"typeOfHit", {"a", "Lazv;"}}
        }, {});

        // InventoryPlayer class
        lookup["net/minecraft/entity/player/InventoryPlayer"] = createCM("yx", {
            {"currentItem", {"c", "I"}}
        }, {
            {"getStackInSlot", {"a", "(I)Ladd;"}}
        });

        // GameSettings class
        lookup["net/minecraft/client/settings/GameSettings"] = createCM("bbj", {}, {});

        // KeyBinding class
        lookup["net/minecraft/client/settings/KeyBinding"] = createCM("bal", {
            {"pressed", {"h", "Z"}}
        }, {});

        break;
    }
    
    case CASUAL_1_8: {
        g_Mappings = {
            {"net/minecraft/client/Minecraft", "ave"},
            {"net/minecraft/client/entity/EntityClientPlayerMP", "bew"},
            {"net/minecraft/client/multiplayer/WorldClient", "bdb"},
            {"net/minecraft/client/gui/GuiScreen", "axu"},
            {"net/minecraft/client/renderer/entity/RenderManager", "biu"},
            { "net/minecraft/entity/Entity", "pk" },
            {"net/minecraft/client/renderer/entity/Render", "biv"},
            {"net/minecraft/client/model/ModelBase", "bbo"},
            {"net/minecraft/client/renderer/ActiveRenderInfo", "auz"},
            { "net/minecraft/util/IChatComponent", "eu" },
            {"net/minecraft/client/model/ModelRenderer", "bct"},
            {"net/minecraft/util/Timer", "avl"},
            {"net/minecraft/client/gui/Gui", "avp"},
            {"net/minecraft/client/gui/FontRenderer", "avn"},
            {"net/minecraft/util/AxisAlignedBB", "aug"},
            {"net/minecraft/client/gui/GuiChat", "awv"},
            {"net/minecraft/client/renderer/entity/RendererLivingEntity", "bjl"},
            {"net/minecraft/item/ItemStack", "zx"},
            {"net/minecraft/item/ItemSword", "aay"},
            {"net/minecraft/item/ItemAxe", "yl"},
            {"net/minecraft/item/Item", "zw"},
            { "net/minecraft/item/ItemBlock", "yo" },
            {"net/minecraft/util/MovingObjectPosition$MovingObjectType", "auh$a"},
            {"net/minecraft/util/MovingObjectPosition", "auh"},
            {"net/minecraft/client/gui/inventory/GuiInventory", "azc"},
            {"net/minecraft/entity/player/InventoryPlayer", "wm"},
            {"net/minecraft/item/ItemEnderPearl", "zk"},
            { "net/minecraft/block/Block", "afh" },
            { "net/minecraft/block/state/IBlockState", "alz" },
            {"net/minecraft/util/BlockPos", "cj"},
            {"net/minecraft/client/settings/GameSettings", "avh"},
            { "net/minecraft/client/settings/KeyBinding", "net/minecraft/client/settings/KeyBinding" },
            {"net/minecraft/block/BlockAir", "net/minecraft/block/BlockAir"}
        };

        std::lock_guard<std::mutex> lock(lookup_mutex);
        
        // Minecraft class - Obfuscated 1.8
        lookup["net/minecraft/client/Minecraft"] = createCM("ave", {
            {"thePlayer", {"h", "Lbew;"}},
            {"theWorld", {"f", "Lbdb;"}},
            {"gameSettings", {"t", "Lavh;"}},
            {"currentScreen", {"m", "Laxu;"}},
            {"objectMouseOver", {"s", "Lauh;"}},
            {"timer", {"Y", "Lavl;"}},
            {"displayWidth", {"d", "I"}},
            {"displayHeight", {"e", "I"}},
            {"fontRendererObj", {"k", "Lavn;"}}
        }, {
            {"theMinecraft", {"S", "()Lave;"}},
            {"getRenderManager", {"af", "()Lbiu;"}}
        });

        // EntityPlayerSP class (1.8 version)
        lookup["net/minecraft/client/entity/EntityClientPlayerMP"] = createCM("bew", {
            {"rotationPitch", {"z", "F"}},
            {"rotationYaw", {"y", "F"}},
            {"prevRotationYaw", {"A", "F"}},
            {"prevRotationPitch", {"B", "F"}},
            {"posX", {"s", "D"}},
            {"posY", {"t", "D"}},
            {"posZ", {"u", "D"}},
            {"motionX", {"v", "D"}},
            {"motionY", {"w", "D"}},
            {"motionZ", {"x", "D"}},
            {"prevPosX", {"p", "D"}},
            {"prevPosY", {"q", "D"}},
            {"prevPosZ", {"r", "D"}},
            {"maxHurtResistantTime", {"aD", "I"}},
            {"hurtResistantTime", {"Z", "I"}},
            {"lastTickPosX", {"P", "D"}},
            {"lastTickPosY", {"Q", "D"}},
            {"lastTickPosZ", {"R", "D"}},
            {"rotationYawHead", {"aK", "F"}},
            {"onGround", {"C", "Z"}},
            {"inWater", {"Y", "Z"}},
            {"moveForward", {"ba", "F"}},
            {"moveStrafing", {"aZ", "F"}},
            {"inventory", {"bi", "Lwm;"}}
        }, {
            {"getHealth", {"bn", "()F"}},
            {"getHeldItem", {"bA", "()Lzx;"}},
            {"isInvisible", {"ax", "()Z"}},
            {"isSneaking", {"av", "()Z"}}
        });

        // WorldClient class
        lookup["net/minecraft/client/multiplayer/WorldClient"] = createCM("bdb", {
            {"playerEntities", {"j", "Ljava/util/List;"}}
        }, {
            {"getBlockState", {"p", "(Lcj;)Lalz;"}},
            {"getBlock", {"c", "(Lcj;)Lafh;"}}
        });

        // Entity class
        lookup["net/minecraft/entity/Entity"] = createCM("pk", {
            {"posX", {"s", "D"}},
            {"posY", {"t", "D"}},
            {"posZ", {"u", "D"}},
            {"motionX", {"v", "D"}},
            {"motionY", {"w", "D"}},
            {"motionZ", {"x", "D"}},
            {"rotationPitch", {"z", "F"}},
            {"rotationYaw", {"y", "F"}},
            {"prevRotationYaw", {"A", "F"}},
            {"prevRotationPitch", {"B", "F"}},
            {"boundingBox", {"f", "Laug;"}},
            {"onGround", {"C", "Z"}},
            {"maxHurtResistantTime", {"aD", "I"}},
            {"hurtResistantTime", {"Z", "I"}},
            {"prevPosX", {"p", "D"}},
            {"prevPosY", {"q", "D"}},
            {"prevPosZ", {"r", "D"}}
        }, {
            {"setAlwaysRenderNameTag", {"g", "(Z)V"}}
        });

        // Additional classes for 1.8
        lookup["net/minecraft/util/Timer"] = createCM("avl", {
            {"renderPartialTicks", {"c", "F"}},
            {"timerSpeed", {"d", "F"}}
        }, {});

        lookup["net/minecraft/util/AxisAlignedBB"] = createCM("aug", {
            {"minX", {"a", "D"}},
            {"minY", {"b", "D"}},
            {"minZ", {"c", "D"}},
            {"maxX", {"d", "D"}},
            {"maxY", {"e", "D"}},
            {"maxZ", {"f", "D"}}
        }, {});

        break;
    }
    
    case FEATHER_1_8: {
        // Feather Client uses deobfuscated names similar to Forge
        g_Mappings = {
            {"net/minecraft/client/Minecraft", "net/minecraft/client/Minecraft"},
            {"net/minecraft/client/entity/EntityClientPlayerMP", "net/minecraft/client/entity/EntityPlayerSP"},
            {"net/minecraft/client/multiplayer/WorldClient", "net/minecraft/client/multiplayer/WorldClient"},
            {"net/minecraft/client/gui/GuiScreen", "net/minecraft/client/gui/GuiScreen"},
            {"net/minecraft/client/renderer/entity/RenderManager", "net/minecraft/client/renderer/entity/RenderManager"},
            { "net/minecraft/entity/Entity", "net/minecraft/entity/Entity" },
            {"net/minecraft/client/renderer/entity/Render", "net/minecraft/client/renderer/entity/Render"},
            {"net/minecraft/client/model/ModelBase", "net/minecraft/client/model/ModelBase"},
            {"net/minecraft/client/renderer/ActiveRenderInfo", "net/minecraft/client/renderer/ActiveRenderInfo"},
            { "net/minecraft/util/IChatComponent", "net/minecraft/util/IChatComponent" },
            {"net/minecraft/client/model/ModelRenderer", "net/minecraft/client/model/ModelRenderer"},
            {"net/minecraft/util/Timer", "net/minecraft/util/Timer"},
            {"net/minecraft/client/gui/Gui", "net/minecraft/client/gui/Gui"},
            {"net/minecraft/client/gui/FontRenderer", "net/minecraft/client/gui/FontRenderer"},
            {"net/minecraft/util/AxisAlignedBB", "net/minecraft/util/AxisAlignedBB"},
            {"net/minecraft/client/gui/GuiChat", "net/minecraft/client/gui/GuiChat"},
            {"net/minecraft/client/renderer/entity/RendererLivingEntity", "net/minecraft/client/renderer/entity/RendererLivingEntity"},
            { "net/minecraft/item/ItemStack", "net/minecraft/item/ItemStack" },
            { "net/minecraft/item/ItemSword", "net/minecraft/item/ItemSword" },
            { "net/minecraft/item/ItemAxe", "net/minecraft/item/ItemAxe" },
            { "net/minecraft/item/Item", "net/minecraft/item/Item" },
            { "net/minecraft/item/ItemBlock", "net/minecraft/item/ItemBlock" },
            {"net/minecraft/util/MovingObjectPosition$MovingObjectType", "net/minecraft/util/MovingObjectPosition$MovingObjectType"},
            {"net/minecraft/util/MovingObjectPosition", "net/minecraft/util/MovingObjectPosition"},
            {"net/minecraft/client/gui/inventory/GuiInventory", "net/minecraft/client/gui/inventory/GuiInventory"},
            {"net/minecraft/entity/player/InventoryPlayer", "net/minecraft/entity/player/InventoryPlayer"},
            {"net/minecraft/item/ItemEnderPearl", "net/minecraft/item/ItemEnderPearl"},
            { "net/minecraft/block/Block", "net/minecraft/block/Block" },
            { "net/minecraft/block/state/IBlockState", "net/minecraft/block/state/IBlockState" },
            {"net/minecraft/util/BlockPos", "net/minecraft/util/BlockPos"},
            {"net/minecraft/client/settings/GameSettings", "net/minecraft/client/settings/GameSettings"},
            { "net/minecraft/client/settings/KeyBinding", "net/minecraft/client/settings/KeyBinding" },
            {"net/minecraft/block/BlockAir", "net/minecraft/block/BlockAir"}
        };

        std::lock_guard<std::mutex> lock(lookup_mutex);
        
        // Same as Forge 1.8 essentially
        lookup["net/minecraft/client/Minecraft"] = createCM("net/minecraft/client/Minecraft", {
            {"thePlayer", {"field_71439_g", "Lnet/minecraft/client/entity/EntityPlayerSP;"}},
            {"theWorld", {"field_71441_e", "Lnet/minecraft/client/multiplayer/WorldClient;"}},
            {"gameSettings", {"field_71474_y", "Lnet/minecraft/client/settings/GameSettings;"}},
            {"currentScreen", {"field_71462_r", "Lnet/minecraft/client/gui/GuiScreen;"}},
            {"objectMouseOver", {"field_71476_x", "Lnet/minecraft/util/MovingObjectPosition;"}},
            {"timer", {"field_71428_T", "Lnet/minecraft/util/Timer;"}}
        }, {
            {"theMinecraft", {"field_71432_P", "()Lnet/minecraft/client/Minecraft;"}},
            {"getRenderManager", {"func_175598_ae", "()Lnet/minecraft/client/renderer/entity/RenderManager;"}}
        });

        break;
    }
    
    case LUNAR_1_7_10: {
        // Lunar Client uses clean deobfuscated names
        g_Mappings = {
            {"net/minecraft/client/Minecraft", "net/minecraft/client/Minecraft"},
            {"net/minecraft/client/entity/EntityClientPlayerMP", "net/minecraft/client/entity/EntityClientPlayerMP"},
            {"net/minecraft/client/multiplayer/WorldClient", "net/minecraft/client/multiplayer/WorldClient"},
            {"net/minecraft/client/gui/GuiScreen", "net/minecraft/client/gui/GuiScreen"},
            {"net/minecraft/client/renderer/entity/RenderManager", "net/minecraft/client/renderer/entity/RenderManager"},
            { "net/minecraft/entity/Entity", "net/minecraft/entity/Entity" },
            {"net/minecraft/client/renderer/entity/Render", "net/minecraft/client/renderer/entity/Render"},
            {"net/minecraft/client/model/ModelBase", "net/minecraft/client/model/ModelBase"},
            {"net/minecraft/client/renderer/ActiveRenderInfo", "net/minecraft/client/renderer/ActiveRenderInfo"},
            { "net/minecraft/util/IChatComponent", "net/minecraft/util/IChatComponent" },
            {"net/minecraft/client/model/ModelRenderer", "net/minecraft/client/model/ModelRenderer"},
            {"net/minecraft/util/Timer", "net/minecraft/util/Timer"},
            {"net/minecraft/client/gui/Gui", "net/minecraft/client/gui/Gui"},
            {"net/minecraft/client/gui/FontRenderer", "net/minecraft/client/gui/FontRenderer"},
            {"net/minecraft/util/AxisAlignedBB", "net/minecraft/util/AxisAlignedBB"},
            {"net/minecraft/client/gui/GuiChat", "net/minecraft/client/gui/GuiChat"},
            {"net/minecraft/client/renderer/entity/RendererLivingEntity", "net/minecraft/client/renderer/entity/RendererLivingEntity"},
            { "net/minecraft/item/ItemStack", "net/minecraft/item/ItemStack" },
            { "net/minecraft/item/ItemSword", "net/minecraft/item/ItemSword" },
            { "net/minecraft/item/ItemAxe", "net/minecraft/item/ItemAxe" },
            { "net/minecraft/item/Item", "net/minecraft/item/Item" },
            { "net/minecraft/item/ItemBlock", "net/minecraft/item/ItemBlock" },
            {"net/minecraft/util/MovingObjectPosition$MovingObjectType", "net/minecraft/util/MovingObjectPosition_MovingObjectType"},
            {"net/minecraft/util/MovingObjectPosition", "net/minecraft/util/MovingObjectPosition"},
            {"net/minecraft/client/gui/inventory/GuiInventory", "net/minecraft/client/gui/inventory/GuiInventory"},
            {"net/minecraft/entity/player/InventoryPlayer", "net/minecraft/entity/player/InventoryPlayer"},
            {"net/minecraft/item/ItemEnderPearl", "net/minecraft/item/ItemEnderPearl"},
            { "net/minecraft/block/Block", "net/minecraft/block/Block" },
            {"net/minecraft/client/settings/GameSettings", "net/minecraft/client/settings/GameSettings"},
            { "net/minecraft/client/settings/KeyBinding", "net/minecraft/client/settings/KeyBinding" },
            {"net/minecraft/block/BlockAir", "net/minecraft/block/BlockAir"}
        };

        std::lock_guard<std::mutex> lock(lookup_mutex);
        
        // Lunar uses clean names without field_ prefixes
        lookup["net/minecraft/client/Minecraft"] = createCM("net/minecraft/client/Minecraft", {
            {"thePlayer", {"thePlayer", "Lnet/minecraft/client/entity/EntityClientPlayerMP;"}},
            {"theWorld", {"theWorld", "Lnet/minecraft/client/multiplayer/WorldClient;"}},
            {"gameSettings", {"gameSettings", "Lnet/minecraft/client/settings/GameSettings;"}},
            {"currentScreen", {"currentScreen", "Lnet/minecraft/client/gui/GuiScreen;"}},
            {"objectMouseOver", {"objectMouseOver", "Lnet/minecraft/util/MovingObjectPosition;"}},
            {"timer", {"timer", "Lnet/minecraft/util/Timer;"}},
            {"displayWidth", {"displayWidth", "I"}},
            {"displayHeight", {"displayHeight", "I"}},
            {"fontRendererObj", {"fontRendererObj", "Lnet/minecraft/client/gui/FontRenderer;"}}
        }, {
            {"theMinecraft", {"theMinecraft", "()Lnet/minecraft/client/Minecraft;"}}
        });

        lookup["net/minecraft/client/entity/EntityClientPlayerMP"] = createCM("net/minecraft/client/entity/EntityClientPlayerMP", {
            {"rotationPitch", {"rotationPitch", "F"}},
            {"rotationYaw", {"rotationYaw", "F"}},
            {"prevRotationYaw", {"prevRotationYaw", "F"}},
            {"prevRotationPitch", {"prevRotationPitch", "F"}},
            {"posX", {"posX", "D"}},
            {"posY", {"posY", "D"}},
            {"posZ", {"posZ", "D"}},
            {"motionX", {"motionX", "D"}},
            {"motionY", {"motionY", "D"}},
            {"motionZ", {"motionZ", "D"}},
            {"maxHurtResistantTime", {"maxHurtResistantTime", "I"}},
            {"hurtResistantTime", {"hurtResistantTime", "I"}},
            {"lastTickPosX", {"lastTickPosX", "D"}},
            {"lastTickPosY", {"lastTickPosY", "D"}},
            {"lastTickPosZ", {"lastTickPosZ", "D"}},
            {"prevPosX", {"prevPosX", "D"}},
            {"prevPosY", {"prevPosY", "D"}},
            {"prevPosZ", {"prevPosZ", "D"}},
            {"rotationYawHead", {"rotationYawHead", "F"}},
            {"onGround", {"onGround", "Z"}},
            {"inWater", {"inWater", "Z"}},
            {"moveForward", {"moveForward", "F"}},
            {"moveStrafing", {"moveStrafing", "F"}},
            {"inventory", {"inventory", "Lnet/minecraft/entity/player/InventoryPlayer;"}}
        }, {
            {"getHealth", {"getHealth", "()F"}},
            {"getHeldItem", {"getHeldItem", "()Lnet/minecraft/item/ItemStack;"}},
            {"isInvisible", {"isInvisible", "()Z"}},
            {"isSneaking", {"isSneaking", "()Z"}}
        });

        break;
    }
    
    case LUNAR_1_8: {
        // Lunar 1.8 uses clean deobfuscated names similar to 1.7.10
        g_Mappings = {
            {"net/minecraft/client/Minecraft", "net/minecraft/client/Minecraft"},
            {"net/minecraft/client/entity/EntityClientPlayerMP", "net/minecraft/client/entity/EntityPlayerSP"},
            {"net/minecraft/client/multiplayer/WorldClient", "net/minecraft/client/multiplayer/WorldClient"},
            {"net/minecraft/client/gui/GuiScreen", "net/minecraft/client/gui/GuiScreen"},
            {"net/minecraft/client/renderer/entity/RenderManager", "net/minecraft/client/renderer/entity/RenderManager"},
            { "net/minecraft/entity/Entity", "net/minecraft/entity/Entity" },
            {"net/minecraft/client/renderer/entity/Render", "net/minecraft/client/renderer/entity/Render"},
            {"net/minecraft/client/model/ModelBase", "net/minecraft/client/model/ModelBase"},
            {"net/minecraft/client/renderer/ActiveRenderInfo", "net/minecraft/client/renderer/ActiveRenderInfo"},
            { "net/minecraft/util/IChatComponent", "net/minecraft/util/IChatComponent" },
            {"net/minecraft/client/model/ModelRenderer", "net/minecraft/client/model/ModelRenderer"},
            {"net/minecraft/util/Timer", "net/minecraft/util/Timer"},
            {"net/minecraft/client/gui/Gui", "net/minecraft/client/gui/Gui"},
            {"net/minecraft/client/gui/FontRenderer", "net/minecraft/client/gui/FontRenderer"},
            {"net/minecraft/util/AxisAlignedBB", "net/minecraft/util/AxisAlignedBB"},
            {"net/minecraft/client/gui/GuiChat", "net/minecraft/client/gui/GuiChat"},
            {"net/minecraft/client/renderer/entity/RendererLivingEntity", "net/minecraft/client/renderer/entity/RendererLivingEntity"},
            { "net/minecraft/item/ItemStack", "net/minecraft/item/ItemStack" },
            { "net/minecraft/item/ItemSword", "net/minecraft/item/ItemSword" },
            { "net/minecraft/item/ItemAxe", "net/minecraft/item/ItemAxe" },
            { "net/minecraft/item/Item", "net/minecraft/item/Item" },
            { "net/minecraft/item/ItemBlock", "net/minecraft/item/ItemBlock" },
            {"net/minecraft/util/MovingObjectPosition$MovingObjectType", "net/minecraft/util/MovingObjectPosition$MovingObjectType"},
            {"net/minecraft/util/MovingObjectPosition", "net/minecraft/util/MovingObjectPosition"},
            {"net/minecraft/client/gui/inventory/GuiInventory", "net/minecraft/client/gui/inventory/GuiInventory"},
            {"net/minecraft/entity/player/InventoryPlayer", "net/minecraft/entity/player/InventoryPlayer"},
            {"net/minecraft/item/ItemEnderPearl", "net/minecraft/item/ItemEnderPearl"},
            { "net/minecraft/block/Block", "net/minecraft/block/Block" },
            { "net/minecraft/block/state/IBlockState", "net/minecraft/block/state/IBlockState" },
            { "net/minecraft/util/BlockPos", "net/minecraft/util/BlockPos"},
            {"net/minecraft/client/settings/GameSettings", "net/minecraft/client/settings/GameSettings"},
            { "net/minecraft/client/settings/KeyBinding", "net/minecraft/client/settings/KeyBinding" },
            {"net/minecraft/block/BlockAir", "net/minecraft/block/BlockAir"}
        };

        std::lock_guard<std::mutex> lock(lookup_mutex);
        
        // Lunar 1.8 with clean names
        lookup["net/minecraft/client/Minecraft"] = createCM("net/minecraft/client/Minecraft", {
            {"thePlayer", {"thePlayer", "Lnet/minecraft/client/entity/EntityPlayerSP;"}},
            {"theWorld", {"theWorld", "Lnet/minecraft/client/multiplayer/WorldClient;"}},
            {"gameSettings", {"gameSettings", "Lnet/minecraft/client/settings/GameSettings;"}},
            {"currentScreen", {"currentScreen", "Lnet/minecraft/client/gui/GuiScreen;"}},
            {"objectMouseOver", {"objectMouseOver", "Lnet/minecraft/util/MovingObjectPosition;"}},
            {"timer", {"timer", "Lnet/minecraft/util/Timer;"}},
            {"displayWidth", {"displayWidth", "I"}},
            {"displayHeight", {"displayHeight", "I"}},
            {"fontRendererObj", {"fontRendererObj", "Lnet/minecraft/client/gui/FontRenderer;"}}
        }, {
            {"theMinecraft", {"theMinecraft", "()Lnet/minecraft/client/Minecraft;"}},
            {"getRenderManager", {"getRenderManager", "()Lnet/minecraft/client/renderer/entity/RenderManager;"}}
        });

        lookup["net/minecraft/client/entity/EntityClientPlayerMP"] = createCM("net/minecraft/client/entity/EntityPlayerSP", {
            {"rotationPitch", {"rotationPitch", "F"}},
            {"rotationYaw", {"rotationYaw", "F"}},
            {"prevRotationYaw", {"prevRotationYaw", "F"}},
            {"prevRotationPitch", {"prevRotationPitch", "F"}},
            {"posX", {"posX", "D"}},
            {"posY", {"posY", "D"}},
            {"posZ", {"posZ", "D"}},
            {"motionX", {"motionX", "D"}},
            {"motionY", {"motionY", "D"}},
            {"motionZ", {"motionZ", "D"}},
            {"maxHurtResistantTime", {"maxHurtResistantTime", "I"}},
            {"hurtResistantTime", {"hurtResistantTime", "I"}},
            {"lastTickPosX", {"lastTickPosX", "D"}},
            {"lastTickPosY", {"lastTickPosY", "D"}},
            {"lastTickPosZ", {"lastTickPosZ", "D"}},
            {"prevPosX", {"prevPosX", "D"}},
            {"prevPosY", {"prevPosY", "D"}},
            {"prevPosZ", {"prevPosZ", "D"}},
            {"rotationYawHead", {"rotationYawHead", "F"}},
            {"onGround", {"onGround", "Z"}},
            {"inWater", {"inWater", "Z"}},
            {"moveForward", {"moveForward", "F"}},
            {"moveStrafing", {"moveStrafing", "F"}},
            {"inventory", {"inventory", "Lnet/minecraft/entity/player/InventoryPlayer;"}}
        }, {
            {"getHealth", {"getHealth", "()F"}},
            {"getHeldItem", {"getHeldItem", "()Lnet/minecraft/item/ItemStack;"}},
            {"isInvisible", {"isInvisible", "()Z"}},
            {"isSneaking", {"isSneaking", "()Z"}}
        });

        // Add more classes for completeness
        lookup["net/minecraft/util/Timer"] = createCM("net/minecraft/util/Timer", {
            {"renderPartialTicks", {"renderPartialTicks", "F"}},
            {"timerSpeed", {"timerSpeed", "F"}}
        }, {});

        lookup["net/minecraft/util/AxisAlignedBB"] = createCM("net/minecraft/util/AxisAlignedBB", {
            {"minX", {"minX", "D"}},
            {"minY", {"minY", "D"}},
            {"minZ", {"minZ", "D"}},
            {"maxX", {"maxX", "D"}},
            {"maxY", {"maxY", "D"}},
            {"maxZ", {"maxZ", "D"}}
        }, {});

        lookup["net/minecraft/item/ItemStack"] = createCM("net/minecraft/item/ItemStack", {
            {"item", {"item", "Lnet/minecraft/item/Item;"}},
            {"metadata", {"itemDamage", "I"}}
        }, {
            {"getItem", {"getItem", "()Lnet/minecraft/item/Item;"}}
        });

        break;
    }
    
    case FORGE_1_7_10: {
        g_Mappings = {
            {"net/minecraft/client/Minecraft", "net/minecraft/client/Minecraft"},
            {"net/minecraft/client/entity/EntityClientPlayerMP", "net/minecraft/client/entity/EntityClientPlayerMP"},
            {"net/minecraft/client/multiplayer/WorldClient", "net/minecraft/client/multiplayer/WorldClient"},
            {"net/minecraft/client/gui/GuiScreen", "net/minecraft/client/gui/GuiScreen"},
            {"net/minecraft/client/renderer/entity/RenderManager", "net/minecraft/client/renderer/entity/RenderManager"},
            { "net/minecraft/entity/Entity", "net/minecraft/entity/Entity" },
            {"net/minecraft/client/renderer/entity/Render", "net/minecraft/client/renderer/entity/Render"},
            {"net/minecraft/client/model/ModelBase", "net/minecraft/client/model/ModelBase"},
            {"net/minecraft/client/renderer/ActiveRenderInfo", "net/minecraft/client/renderer/ActiveRenderInfo"},
            { "net/minecraft/util/IChatComponent", "net/minecraft/util/IChatComponent" },
            {"net/minecraft/client/model/ModelRenderer", "net/minecraft/client/model/ModelRenderer"},
            {"net/minecraft/util/Timer", "net/minecraft/util/Timer"},
            {"net/minecraft/client/gui/Gui", "net/minecraft/client/gui/Gui"},
            {"net/minecraft/client/gui/FontRenderer", "net/minecraft/client/gui/FontRenderer"},
            {"net/minecraft/util/AxisAlignedBB", "net/minecraft/util/AxisAlignedBB"},
            {"net/minecraft/client/gui/GuiChat", "net/minecraft/client/gui/GuiChat"},
            {"net/minecraft/client/renderer/entity/RendererLivingEntity", "net/minecraft/client/renderer/entity/RendererLivingEntity"},
            { "net/minecraft/item/ItemStack", "net/minecraft/item/ItemStack" },
            { "net/minecraft/item/ItemSword", "net/minecraft/item/ItemSword" },
            { "net/minecraft/item/ItemAxe", "net/minecraft/item/ItemAxe" },
            { "net/minecraft/item/Item", "net/minecraft/item/Item" },
            { "net/minecraft/item/ItemBlock", "net/minecraft/item/ItemBlock" },
            {"net/minecraft/util/MovingObjectPosition$MovingObjectType", "net/minecraft/util/MovingObjectPosition_MovingObjectType"},
            {"net/minecraft/util/MovingObjectPosition", "net/minecraft/util/MovingObjectPosition"},
            {"net/minecraft/client/gui/inventory/GuiInventory", "net/minecraft/client/gui/inventory/GuiInventory"},
            {"net/minecraft/entity/player/InventoryPlayer", "net/minecraft/entity/player/InventoryPlayer"},
            {"net/minecraft/item/ItemEnderPearl", "net/minecraft/item/ItemEnderPearl"},
            { "net/minecraft/block/Block", "net/minecraft/block/Block" },
            {"net/minecraft/client/settings/GameSettings", "net/minecraft/client/settings/GameSettings"},
            { "net/minecraft/client/settings/KeyBinding", "net/minecraft/client/settings/KeyBinding" },
            {"net/minecraft/block/BlockAir", "net/minecraft/block/BlockAir"}
        };

        std::lock_guard<std::mutex> lock(lookup_mutex);
        
        // Minecraft class with MCP names
        lookup["net/minecraft/client/Minecraft"] = createCM("net/minecraft/client/Minecraft", {
            {"thePlayer", {"field_71439_g", "Lnet/minecraft/client/entity/EntityClientPlayerMP;"}},
            {"theWorld", {"field_71441_e", "Lnet/minecraft/client/multiplayer/WorldClient;"}},
            {"gameSettings", {"field_71474_y", "Lnet/minecraft/client/settings/GameSettings;"}},
            {"currentScreen", {"field_71462_r", "Lnet/minecraft/client/gui/GuiScreen;"}},
            {"objectMouseOver", {"field_71476_x", "Lnet/minecraft/util/MovingObjectPosition;"}},
            {"timer", {"field_71428_T", "Lnet/minecraft/util/Timer;"}},
            {"displayWidth", {"field_71443_c", "I"}},
            {"displayHeight", {"field_71440_d", "I"}},
            {"fontRendererObj", {"field_71466_p", "Lnet/minecraft/client/gui/FontRenderer;"}}
        }, {
            {"theMinecraft", {"field_71432_P", "()Lnet/minecraft/client/Minecraft;"}}
        });

        // EntityClientPlayerMP class with MCP names
        lookup["net/minecraft/client/entity/EntityClientPlayerMP"] = createCM("net/minecraft/client/entity/EntityClientPlayerMP", {
            {"rotationPitch", {"field_70125_A", "F"}},
            {"rotationYaw", {"field_70177_z", "F"}},
            {"prevRotationYaw", {"field_70126_B", "F"}},
            {"prevRotationPitch", {"field_70127_C", "F"}},
            {"posX", {"field_70165_t", "D"}},
            {"posY", {"field_70163_u", "D"}},
            {"posZ", {"field_70161_v", "D"}},
            {"motionX", {"field_70159_w", "D"}},
            {"motionY", {"field_70181_x", "D"}},
            {"motionZ", {"field_70179_y", "D"}},
            {"maxHurtResistantTime", {"field_70771_an", "I"}},
            {"hurtResistantTime", {"field_70172_ad", "I"}},
            {"lastTickPosX", {"field_70142_S", "D"}},
            {"lastTickPosY", {"field_70137_T", "D"}},
            {"lastTickPosZ", {"field_70136_U", "D"}},
            {"prevPosX", {"field_70169_q", "D"}},
            {"prevPosY", {"field_70167_r", "D"}},
            {"prevPosZ", {"field_70166_s", "D"}},
            {"rotationYawHead", {"field_70759_as", "F"}},
            {"onGround", {"field_70122_E", "Z"}},
            {"inWater", {"field_70171_ac", "Z"}},
            {"moveForward", {"field_70701_bs", "F"}},
            {"moveStrafing", {"field_70702_br", "F"}},
            {"inventory", {"field_71071_by", "Lnet/minecraft/entity/player/InventoryPlayer;"}}
        }, {
            {"getHealth", {"func_110143_aJ", "()F"}},
            {"getHeldItem", {"func_70694_bm", "()Lnet/minecraft/item/ItemStack;"}},
            {"isInvisible", {"func_82150_aj", "()Z"}},
            {"isSneaking", {"func_70093_af", "()Z"}}
        });

        break;
    }
    
    case FORGE_1_8: {
        g_Mappings = {
            {"net/minecraft/client/Minecraft", "net/minecraft/client/Minecraft"},
            {"net/minecraft/client/entity/EntityClientPlayerMP", "net/minecraft/client/entity/EntityPlayerSP"},
            {"net/minecraft/client/multiplayer/WorldClient", "net/minecraft/client/multiplayer/WorldClient"},
            {"net/minecraft/client/gui/GuiScreen", "net/minecraft/client/gui/GuiScreen"},
            {"net/minecraft/client/renderer/entity/RenderManager", "net/minecraft/client/renderer/entity/RenderManager"},
            { "net/minecraft/entity/Entity", "net/minecraft/entity/Entity" },
            {"net/minecraft/client/renderer/entity/Render", "net/minecraft/client/renderer/entity/Render"},
            {"net/minecraft/client/model/ModelBase", "net/minecraft/client/model/ModelBase"},
            {"net/minecraft/client/renderer/ActiveRenderInfo", "net/minecraft/client/renderer/ActiveRenderInfo"},
            { "net/minecraft/util/IChatComponent", "net/minecraft/util/IChatComponent" },
            {"net/minecraft/client/model/ModelRenderer", "net/minecraft/client/model/ModelRenderer"},
            {"net/minecraft/util/Timer", "net/minecraft/util/Timer"},
            {"net/minecraft/client/gui/Gui", "net/minecraft/client/gui/Gui"},
            {"net/minecraft/client/gui/FontRenderer", "net/minecraft/client/gui/FontRenderer"},
            {"net/minecraft/util/AxisAlignedBB", "net/minecraft/util/AxisAlignedBB"},
            {"net/minecraft/client/gui/GuiChat", "net/minecraft/client/gui/GuiChat"},
            {"net/minecraft/client/renderer/entity/RendererLivingEntity", "net/minecraft/client/renderer/entity/RendererLivingEntity"},
            { "net/minecraft/item/ItemStack", "net/minecraft/item/ItemStack" },
            { "net/minecraft/item/ItemSword", "net/minecraft/item/ItemSword" },
            { "net/minecraft/item/ItemAxe", "net/minecraft/item/ItemAxe" },
            { "net/minecraft/item/Item", "net/minecraft/item/Item" },
            { "net/minecraft/item/ItemBlock", "net/minecraft/item/ItemBlock" },
            {"net/minecraft/util/MovingObjectPosition$MovingObjectType", "net/minecraft/util/MovingObjectPosition$MovingObjectType"},
            {"net/minecraft/util/MovingObjectPosition", "net/minecraft/util/MovingObjectPosition"},
            {"net/minecraft/client/gui/inventory/GuiInventory", "net/minecraft/client/gui/inventory/GuiInventory"},
            {"net/minecraft/entity/player/InventoryPlayer", "net/minecraft/entity/player/InventoryPlayer"},
            {"net/minecraft/item/ItemEnderPearl", "net/minecraft/item/ItemEnderPearl"},
            { "net/minecraft/block/Block", "net/minecraft/block/Block" },
            { "net/minecraft/block/state/IBlockState", "net/minecraft/block/state/IBlockState" },
            {"net/minecraft/util/BlockPos", "net/minecraft/util/BlockPos"},
            {"net/minecraft/client/settings/GameSettings", "net/minecraft/client/settings/GameSettings"},
            { "net/minecraft/client/settings/KeyBinding", "net/minecraft/client/settings/KeyBinding" },
            {"net/minecraft/block/BlockAir", "net/minecraft/block/BlockAir"}
        };

        std::lock_guard<std::mutex> lock(lookup_mutex);
        
        // Minecraft class with MCP names (1.8)
        lookup["net/minecraft/client/Minecraft"] = createCM("net/minecraft/client/Minecraft", {
            {"thePlayer", {"field_71439_g", "Lnet/minecraft/client/entity/EntityPlayerSP;"}},
            {"theWorld", {"field_71441_e", "Lnet/minecraft/client/multiplayer/WorldClient;"}},
            {"gameSettings", {"field_71474_y", "Lnet/minecraft/client/settings/GameSettings;"}},
            {"currentScreen", {"field_71462_r", "Lnet/minecraft/client/gui/GuiScreen;"}},
            {"objectMouseOver", {"field_71476_x", "Lnet/minecraft/util/MovingObjectPosition;"}},
            {"timer", {"field_71428_T", "Lnet/minecraft/util/Timer;"}},
            {"displayWidth", {"field_71443_c", "I"}},
            {"displayHeight", {"field_71440_d", "I"}},
            {"fontRendererObj", {"field_71466_p", "Lnet/minecraft/client/gui/FontRenderer;"}}
        }, {
            {"theMinecraft", {"field_71432_P", "()Lnet/minecraft/client/Minecraft;"}},
            {"getRenderManager", {"func_175598_ae", "()Lnet/minecraft/client/renderer/entity/RenderManager;"}}
        });

        // EntityPlayerSP class with MCP names (1.8 version)
        lookup["net/minecraft/client/entity/EntityClientPlayerMP"] = createCM("net/minecraft/client/entity/EntityPlayerSP", {
            {"rotationPitch", {"field_70125_A", "F"}},
            {"rotationYaw", {"field_70177_z", "F"}},
            {"prevRotationYaw", {"field_70126_B", "F"}},
            {"prevRotationPitch", {"field_70127_C", "F"}},
            {"posX", {"field_70165_t", "D"}},
            {"posY", {"field_70163_u", "D"}},
            {"posZ", {"field_70161_v", "D"}},
            {"motionX", {"field_70159_w", "D"}},
            {"motionY", {"field_70181_x", "D"}},
            {"motionZ", {"field_70179_y", "D"}},
            {"prevPosX", {"field_70169_q", "D"}},
            {"prevPosY", {"field_70167_r", "D"}},
            {"prevPosZ", {"field_70166_s", "D"}},
            {"maxHurtResistantTime", {"field_70771_an", "I"}},
            {"hurtResistantTime", {"field_70172_ad", "I"}},
            {"lastTickPosX", {"field_70142_S", "D"}},
            {"lastTickPosY", {"field_70137_T", "D"}},
            {"lastTickPosZ", {"field_70136_U", "D"}},
            {"rotationYawHead", {"field_70759_as", "F"}},
            {"onGround", {"field_70122_E", "Z"}},
            {"inWater", {"field_70171_ac", "Z"}},
            {"moveForward", {"field_70701_bs", "F"}},
            {"moveStrafing", {"field_70702_br", "F"}},
            {"inventory", {"field_71071_by", "Lnet/minecraft/entity/player/InventoryPlayer;"}}
        }, {
            {"getHealth", {"func_110143_aJ", "()F"}},
            {"getHeldItem", {"func_70694_bm", "()Lnet/minecraft/item/ItemStack;"}},
            {"isInvisible", {"func_82150_aj", "()Z"}},
            {"isSneaking", {"func_70093_af", "()Z"}}
        });

        break;
    }
    
    } // End of switch statement
}

/*
    * type:
    * 1: classic class/field
    * 2: class type (L...;)
    * 3: method class type (()L...;)
*/
std::string Mapping::Get(const char* mapping, int type)
{
    if (!mapping)
        return std::string("");

    auto it = g_Mappings.find(mapping);
    if (it == g_Mappings.end())
        return std::string("");

    const std::string_view mapped = it->second;

    switch (type)
    {
    case 1:
        return std::string(mapped);
    case 2:
        return "L" + std::string(mapped) + ";";
    case 3:
        return "()L" + std::string(mapped) + ";";
    default:
        return std::string(mapped);
    }
}

// New helper functions for CM-based access
std::string Mapping::getFieldName(const char* className, const char* fieldName) {
    CM* cm = getClass(className);
    if (!cm) return "";
    
    auto it = cm->fields.find(fieldName);
    if (it == cm->fields.end()) return "";
    
    return it->second.name;
}

std::string Mapping::getMethodName(const char* className, const char* methodName) {
    CM* cm = getClass(className);
    if (!cm) return "";
    
    auto it = cm->methods.find(methodName);
    if (it == cm->methods.end()) return "";
    
    return it->second.name;
}

std::string Mapping::getFieldDesc(const char* className, const char* fieldName) {
    CM* cm = getClass(className);
    if (!cm) return "";
    
    auto it = cm->fields.find(fieldName);
    if (it == cm->fields.end()) return "";
    
    return it->second.desc;
}

std::string Mapping::getMethodDesc(const char* className, const char* methodName) {
    CM* cm = getClass(className);
    if (!cm) return "";
    
    auto it = cm->methods.find(methodName);
    if (it == cm->methods.end()) return "";
    
    return it->second.desc;
}

