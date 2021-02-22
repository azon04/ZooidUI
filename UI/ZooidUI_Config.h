/************************************************************************/
/* Zooid UI Configuration. Change UI Behavior here                      */
/************************************************************************/

// Put Additional Includes here if Needed
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <cassert>

// Array Type that use in this code. Change to other type if needed
#define UIArray std::vector

// Stack Type that use in this code
#define UIStack std::vector

// HasMap type that use in this code. Change to other type if needed
#define UIHashMap std::unordered_map

// HashMap Find
#define HashMapHas(hashmap, key) hashmap.find(key) != hashmap.end()

// HashMap Find and assign
#define HashMapHasAndAssign(hashmap, key, defaultValue, out) \
	{ auto iter = hashmap.find(key); out = iter == hashmap.end() ? defaultValue : iter->second;  }

// Allocation function. Change to other if needed
#define UINEW(Class) new Class

// Deallocation function. Change to other if needed
#define UIFREE(object) delete object

// Assertion Helper: Change to match the engine/where it's used
#define UIASSERT(condition, text, ...) assert(((void)text __VA_ARGS__,condition))
#define UICHECK(condition) assert(condition)

// Rendering: Font rendering using font instancing
#define ZUI_USE_FONT_INSTANCING

// Rendering: Font instancing happen only per text not entire text.
#if defined(ZUI_USE_FONT_INSTANCING)
	#define ZUI_USE_SINGLE_TEXT_ONLY
#endif

// By Default Font using hash table to map character into glyph index, enable this for performance if hash map is slow.
// Note: This might take more memory.
#define ZUI_FONT_USING_ARRAY_LOOKUP

// Rendering: Render rectangles and images using one rectangle and instance per object
#define ZUI_USE_RECT_INSTANCING

// Rendering: Rendering group by texture
//#define ZUI_GROUP_PER_TEXTURE

// Assets: Main Asset Folder - Where the UI engine looking for assets to use as default
#define RESOURCE_FOLDER "../../Resource/"

// Assets: Default Font File (Relative to Resource Folder)
#define DEFAULT_FONT_PATH "OpenSans-Regular.ttf"

// Keys: Special Keys Define
#define ZOOID_KEY_BACKSPACE	8
#define ZOOID_KEY_ENTER 13
#define ZOOID_KEY_DELETE 127
#define ZOOID_KEY_ARROW_LEFT 11
#define ZOOID_KEY_ARROW_RIGHT 12
#define ZOOID_KEY_ARROW_UP 13
#define ZOOID_KEY_ARROW_DOWN 14
#define ZOOID_KEY_HOME 2
#define ZOOID_KEY_END 3

// Text Input: Length of temp text input (use as temp buffer)
#define TEXT_TEMP_LENGTH 246

// Data Types: Change Data Types if needed
namespace ZE
{
	typedef float Float32;

	typedef std::int32_t Int32;
	
	typedef std::uint32_t UInt32;
	
	typedef char UIChar;
	
	typedef UIChar* UIText;
	
	typedef std::uint8_t UInt8;
}