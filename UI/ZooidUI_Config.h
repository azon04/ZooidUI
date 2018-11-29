/************************************************************************/
/* Zooid UI Configuration. Change UI Behavior here                      */
/************************************************************************/

// Put Additional Includes here if Needed
#include <cstdint>
#include <vector>
#include <unordered_map>

// Array Type that use in this code. Change to other type if needed
#define UIArray std::vector

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

// Rendering: Font rendering using font instancing
#define ZUI_USE_FONT_INSTANCING

// Rendering: Font instancing happen only per text not entire text.
#if defined(ZUI_USE_FONT_INSTANCING)
	#define ZUI_USE_SINGLE_TEXT_ONLY
#endif

// Rendering: Render rectangles and images using one rectangle and instance per object
#define ZUI_USE_RECT_INSTANCING

// Rendering: Rendering group by texture
//#define ZUI_GROUP_PER_TEXTURE

// Assets: Main Asset Folder - Where the UI engine looking for assets to use as default
#define RESOURCE_FOLDER "../../Resource/"

// Assets: Default Font File (Relative to Resource Folder)
#define DEFAULT_FONT_PATH "OpenSans-Regular.ttf"

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