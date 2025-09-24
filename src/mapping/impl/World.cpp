#include "World.h"
#include "EntityPlayerSP.h"
#include "../Klass.h"
#include "../Field.h"
#include "../Method.h"
#include "../Mapping.h"

std::vector<Player*> World::GetPlayerEntities(JNIEnv* env)
{
    std::vector<Player*> ret;
    // Get the class of this world object
    jclass worldClazz = env->GetObjectClass((jobject)this);
    if (!worldClazz)
        return ret;
    // Wrap world class
    Klass* kWorld = new Klass(worldClazz);
    // Get the field "playerEntities" (java.util.List)
    Field* playerEntitiesField = kWorld->GetField(
        env,
        Mapping::Get("playerEntities").c_str(),
        "Ljava/util/List;"
    );
    if (!playerEntitiesField)
        return ret;
    // Grab the list object from the world
    jobject playerEntitiesObject = playerEntitiesField->GetObjectField(env, (jobject)this);
    if (!playerEntitiesObject)
        return ret;
    // Find List class and method toArray
    Klass* listClazz = Klass::Find(env, "java/util/List");
    if (!listClazz)
        return ret;
    Method* toArrayMethod = listClazz->GetMethod(env, "toArray", "()[Ljava/lang/Object;");
    if (!toArrayMethod)
        return ret;
    // Convert list to array
    jobjectArray playerListArray = (jobjectArray)toArrayMethod->CallObjectMethod(env, playerEntitiesObject);
    if (!playerListArray)
        return ret;
    jsize playerListSize = env->GetArrayLength(playerListArray);
    for (jsize i = 0; i < playerListSize; i++)
    {
        jobject playerObj = env->GetObjectArrayElement(playerListArray, i);
        if (!playerObj)
            continue;
        // FIX: Pass both env and playerObj to Player constructor
        Player* pl = new Player(env, playerObj);
        if (!pl)
            continue;
        if (pl->IsNPC())
        {
            delete pl;
            continue;
        }
        ret.push_back(pl);
        env->DeleteLocalRef(playerObj);
    }
    env->DeleteLocalRef(playerEntitiesObject);
    env->DeleteLocalRef(playerListArray);
    env->DeleteLocalRef(worldClazz);
    
    // Clean up allocated objects
    delete kWorld;
    
    return ret;
}
