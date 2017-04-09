/*
 * Generic hashmap manipulation functions
 *
 * Originally by Elliot C Back - http://elliottback.com/wp/hashmap-implementation-in-c/
 *
 * Modified by Pete Warden to fix a serious performance problem, support strings as keys
 * and removed thread synchronization - http://petewarden.typepad.com
 */
#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MAP_MISSING     (-3)    /* No such element */
#define MAP_FULL        (-2)    /* Hashmap is full */
#define MAP_OMEM        (-1)    /* Out of Memory */
#define MAP_OK          (0)     /* OK */

/*
 * any_t is a pointer.  This allows you to put arbitrary structures in
 * the hashmap.
 */
typedef void *any_t;

/*
 * PFany is a pointer to a function that can take two any_t arguments
 * and return an integer. Returns status code..
 */
typedef int (*PFany)(any_t userarg, char *key, any_t data);

/*
 * map_t is a pointer to an internally maintained data structure.
 * Clients of this package do not need to know how hashmaps are
 * represented.  They see and manipulate only map_t's.
 */
typedef any_t map_t;

/*
 * Return an empty hashmap. Returns NULL if empty.
*/
extern map_t hashmap_new();

/*
 * Iteratively call f with argument (item, data) for
 * each element data in the hashmap. The function must
 * return a map status code. If it returns anything other
 * than MAP_OK the traversal is terminated. f must
 * not reenter any hashmap functions, or deadlock may arise.
 */
extern int hashmap_iterate(map_t in, PFany f, any_t item);

/*
 * Add an element to the hashmap. Return MAP_OK or MAP_OMEM.
 */
extern int hashmap_put(map_t in, char *key, any_t value);

/*
 * Get an element from the hashmap. Return MAP_OK or MAP_MISSING.
 */
extern int hashmap_get(map_t in, char *key, any_t *arg);

/*
 * Remove an element from the hashmap. Return MAP_OK or MAP_MISSING.
 */
extern int hashmap_remove(map_t in, char** key);

/*
 * Get any element. Return MAP_OK or MAP_MISSING.
 * remove - should the element be removed from the hashmap
 */
extern int hashmap_get_one(map_t in, any_t *arg, int remove);

/*
 * Free the hashmap
 */
extern void hashmap_free(map_t in);

/*
 * Get the current size of a hashmap
 */
extern int hashmap_length(map_t in);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <new>
#include <string>
#include <utility>
#include <vector>

template<typename T>
struct StringKeyConverter;

template<typename TKey, typename TValue>
class HashMap
{
public:
    static constexpr int MAX_KEY_SIZE = 32;
    HashMap(void) {
        _map = hashmap_new();
    }

    ~HashMap(void) {
        hashmap_free(_map);
    }

    bool put(const TKey& key, TValue&& value) noexcept {
        TValue* data = new (std::nothrow) TValue(std::move(value));
        if (nullptr == data) {
            return false;
        }

        char* stringKey = new (std::nothrow) char[MAX_KEY_SIZE];
        if (nullptr == stringKey) {
            delete(data);
            return false;
        }

        StringKeyConverter<TKey>::generate_key(key, stringKey, MAX_KEY_SIZE);
        hashmap_put(_map, stringKey, data);
        return true;
    }

    bool put(const TKey& key, const TValue& value) noexcept {
        TValue* data = new (std::nothrow) TValue(value);
        if (nullptr == data) {
            return false;
        }

        char* stringKey = new (std::nothrow) char[MAX_KEY_SIZE];
        if (nullptr == stringKey) {
            delete(data);
            return false;
        }

        StringKeyConverter<TKey>::generate_key(key, stringKey, MAX_KEY_SIZE);
        hashmap_put(_map, stringKey, data);
        return true;
    }

    TValue* get(const TKey& key) const noexcept {
        TValue* valuePtr = nullptr;
        char stringKey[MAX_KEY_SIZE];
        StringKeyConverter<TKey>::generate_key(key, stringKey, sizeof(stringKey));

        if (MAP_OK != hashmap_get(_map, stringKey, (any_t *) &valuePtr)) {
            return nullptr;
        }

        return valuePtr;
    }

    bool remove(const TKey& key) noexcept {
        TValue* value = nullptr;
        char stringKey[MAX_KEY_SIZE];

        StringKeyConverter<TKey>::generate_key(key, stringKey, sizeof(stringKey));
        if (MAP_OK != hashmap_get(_map, stringKey, (any_t *) &value)) {
            return false;
        }

        char* oldKey = stringKey;

        if (MAP_OK != hashmap_remove(_map, &oldKey)) {
            return false;
        }

        delete(value);
        delete[](oldKey);
        return true;
    }

    void clear(void) {
        std::vector<std::string> keys;
        keys.reserve(this->size());

        this->iterate([](any_t arg, char * key, any_t data) -> int {
            std::vector<std::string>* keys = (std::vector<std::string>*)arg;
            keys->push_back(std::string(key));
            return MAP_OK;
        }, &keys);

        for (const auto& key : keys) {
            this->remove(key.c_str());
        }
    }

//    template<typename TFunc>
    void iterate(PFany f, any_t item) const {
        hashmap_iterate(_map, f, item);
    }

    int size(void) const {
        return hashmap_length(_map);
    }

private:
    map_t _map;
};

template<>
struct StringKeyConverter<const char*> {
    static void generate_key(const char* key, char* output_key, size_t size) {
        strncpy(output_key, key, size - 1);
    }
};

template<>
struct StringKeyConverter<std::string> {
    static void generate_key(const std::string& key, char* output_key, size_t size) {
        strncpy(output_key, key.c_str(), size - 1);
    }
};

template<>
struct StringKeyConverter<uint32_t> {
    static void generate_key(const uint32_t key, char* output_key, size_t size) {
        sprintf(output_key, "%08x", (unsigned int) key);
    }
};

template<>
struct StringKeyConverter<int> {
    static void generate_key(const int key, char* output_key, size_t size) {
        sprintf(output_key, "%08x", (unsigned int) key);
    }
};
#endif
#endif // __HASHMAP_H__